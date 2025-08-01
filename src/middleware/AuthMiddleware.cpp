#include "../../include/middleware/AuthMiddleware.h"
#include "../../include/utils/CryptoUtils.h"
#include "../../include/utils/Logger.h"
#include "../../include/database/DatabaseManager.h"
#include <algorithm>
#include <sstream>

namespace healthcare::middleware {

AuthMiddleware::AuthMiddleware() {
    initializeDefaultConfig();
}

void AuthMiddleware::before_handle(crow::request& req, crow::response& res, context& ctx) {
    // Skip auth for public endpoints
    if (isPublicEndpoint(req.url)) {
        ctx.is_authenticated = false;
        return;
    }
    
    // Extract token
    std::string token = extractToken(req);
    if (token.empty()) {
        handleUnauthorized(res, "No authentication token provided");
        return;
    }
    
    // Verify token
    auto jwt_result = utils::CryptoUtils::verifyJwtToken(token, jwt_secret_);
    if (!jwt_result.valid) {
        handleUnauthorized(res, jwt_result.error);
        return;
    }
    
    // Extract user info from token
    ctx.user_id = jwt_result.claims["user_id"];
    ctx.user_role = jwt_result.claims["role"];
    ctx.is_authenticated = true;
    
    // Check session validity
    if (session_validation_enabled_ && !isSessionValid(ctx.user_id, token)) {
        handleUnauthorized(res, "Invalid or expired session");
        return;
    }
    
    // Check role-based access
    if (!hasRequiredRole(req.url, ctx.user_role)) {
        handleForbidden(res, "Insufficient permissions");
        return;
    }
    
    // Apply rate limiting
    if (rate_limiting_enabled_ && !checkRateLimit(ctx.user_id, req.remote_ip_address)) {
        handleTooManyRequests(res);
        return;
    }
    
    // Update last activity
    updateLastActivity(ctx.user_id);
    
    // Add user context to request
    req.middleware_context = &ctx;
}

void AuthMiddleware::after_handle(crow::request& req, crow::response& res, context& ctx) {
    // Update stats
    updateStats(ctx.is_authenticated, res.code);
    
    // Log authentication events
    if (ctx.is_authenticated && res.code == 401) {
        LOG_WARN("Authentication failed for user: {}", ctx.user_id);
    }
}

void AuthMiddleware::addPublicEndpoint(const std::string& endpoint) {
    public_endpoints_.insert(endpoint);
}

void AuthMiddleware::addRoleRequirement(const std::string& endpoint, const std::string& required_role) {
    role_requirements_[endpoint] = required_role;
}

bool AuthMiddleware::validateSession(const std::string& user_id, const std::string& session_token) {
    if (!session_validation_enabled_) {
        return true;
    }
    
    try {
        auto& db = database::DatabaseManager::getInstance();
        std::string cached_token = db.getCache("session:" + user_id);
        
        if (cached_token.empty() || cached_token != session_token) {
            return false;
        }
        
        // Extend session
        db.setCache("session:" + user_id, session_token, session_timeout_seconds_);
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Session validation error: {}", e.what());
        return false;
    }
}

void AuthMiddleware::createSession(const std::string& user_id, const std::string& session_token) {
    if (!session_validation_enabled_) {
        return;
    }
    
    try {
        auto& db = database::DatabaseManager::getInstance();
        db.setCache("session:" + user_id, session_token, session_timeout_seconds_);
        
        // Store session metadata
        nlohmann::json session_data;
        session_data["created_at"] = std::chrono::system_clock::now().time_since_epoch().count();
        session_data["last_activity"] = session_data["created_at"];
        
        db.setCacheJson("session_meta:" + user_id, session_data, session_timeout_seconds_);
        
    } catch (const std::exception& e) {
        LOG_ERROR("Session creation error: {}", e.what());
    }
}

void AuthMiddleware::invalidateSession(const std::string& user_id) {
    if (!session_validation_enabled_) {
        return;
    }
    
    try {
        auto& db = database::DatabaseManager::getInstance();
        db.deleteCache("session:" + user_id);
        db.deleteCache("session_meta:" + user_id);
        
        // Clear rate limit data
        clearRateLimit(user_id);
        
    } catch (const std::exception& e) {
        LOG_ERROR("Session invalidation error: {}", e.what());
    }
}

void AuthMiddleware::initializeDefaultConfig() {
    // Public endpoints
    public_endpoints_ = {
        "/",
        "/health",
        "/api/v1/auth/register",
        "/api/v1/auth/login",
        "/api/v1/auth/forgot-password",
        "/api/v1/auth/reset-password",
        "/api/v1/auth/verify-email",
        "/api/v1/public/*"
    };
    
    // Role requirements
    role_requirements_ = {
        {"/api/v1/admin/*", "ADMIN"},
        {"/api/v1/doctor/*", "DOCTOR"},
        {"/api/v1/appointments/create", "USER"},
        {"/api/v1/appointments/*/cancel", "USER"},
        {"/api/v1/prescriptions/*/download", "USER"}
    };
    
    // Rate limits by endpoint pattern
    rate_limits_ = {
        {"/api/v1/auth/login", {5, 300}},        // 5 requests per 5 minutes
        {"/api/v1/auth/register", {3, 3600}},    // 3 requests per hour
        {"/api/v1/auth/forgot-password", {3, 900}}, // 3 requests per 15 minutes
        {"/api/v1/*", {100, 60}}                 // 100 requests per minute (default)
    };
}

std::string AuthMiddleware::extractToken(const crow::request& req) {
    // Check Authorization header
    auto auth_header = req.headers.find("Authorization");
    if (auth_header != req.headers.end()) {
        const std::string& auth_value = auth_header->second;
        const std::string bearer_prefix = "Bearer ";
        
        if (auth_value.substr(0, bearer_prefix.length()) == bearer_prefix) {
            return auth_value.substr(bearer_prefix.length());
        }
    }
    
    // Check cookie
    auto cookie_header = req.headers.find("Cookie");
    if (cookie_header != req.headers.end()) {
        std::string token = extractCookieValue(cookie_header->second, "auth_token");
        if (!token.empty()) {
            return token;
        }
    }
    
    // Check query parameter (less secure, for special cases)
    if (req.url_params.has("auth_token")) {
        return req.url_params.get("auth_token");
    }
    
    return "";
}

std::string AuthMiddleware::extractCookieValue(const std::string& cookie_header, const std::string& name) {
    std::stringstream ss(cookie_header);
    std::string cookie;
    
    while (std::getline(ss, cookie, ';')) {
        // Trim whitespace
        cookie.erase(0, cookie.find_first_not_of(" \t"));
        cookie.erase(cookie.find_last_not_of(" \t") + 1);
        
        size_t eq_pos = cookie.find('=');
        if (eq_pos != std::string::npos) {
            std::string cookie_name = cookie.substr(0, eq_pos);
            if (cookie_name == name) {
                return cookie.substr(eq_pos + 1);
            }
        }
    }
    
    return "";
}

bool AuthMiddleware::isPublicEndpoint(const std::string& url) const {
    // Remove query parameters
    size_t query_pos = url.find('?');
    std::string path = (query_pos != std::string::npos) ? url.substr(0, query_pos) : url;
    
    // Check exact matches
    if (public_endpoints_.find(path) != public_endpoints_.end()) {
        return true;
    }
    
    // Check wildcard patterns
    for (const auto& pattern : public_endpoints_) {
        if (pattern.back() == '*') {
            std::string prefix = pattern.substr(0, pattern.length() - 1);
            if (path.substr(0, prefix.length()) == prefix) {
                return true;
            }
        }
    }
    
    return false;
}

bool AuthMiddleware::hasRequiredRole(const std::string& url, const std::string& user_role) const {
    // Remove query parameters
    size_t query_pos = url.find('?');
    std::string path = (query_pos != std::string::npos) ? url.substr(0, query_pos) : url;
    
    // Check exact matches
    auto it = role_requirements_.find(path);
    if (it != role_requirements_.end()) {
        return checkRole(user_role, it->second);
    }
    
    // Check wildcard patterns
    for (const auto& [pattern, required_role] : role_requirements_) {
        if (pattern.back() == '*') {
            std::string prefix = pattern.substr(0, pattern.length() - 1);
            if (path.substr(0, prefix.length()) == prefix) {
                return checkRole(user_role, required_role);
            }
        }
    }
    
    // No specific requirement, allow access
    return true;
}

bool AuthMiddleware::checkRole(const std::string& user_role, const std::string& required_role) const {
    // Role hierarchy: ADMIN > DOCTOR > USER
    if (required_role == "USER") {
        return true; // All authenticated users have at least USER role
    }
    
    if (required_role == "DOCTOR") {
        return user_role == "DOCTOR" || user_role == "ADMIN";
    }
    
    if (required_role == "ADMIN") {
        return user_role == "ADMIN";
    }
    
    return false;
}

bool AuthMiddleware::checkRateLimit(const std::string& user_id, const std::string& ip_address) {
    if (!rate_limiting_enabled_) {
        return true;
    }
    
    std::string key = user_id.empty() ? "ip:" + ip_address : "user:" + user_id;
    
    try {
        auto& db = database::DatabaseManager::getInstance();
        
        // Get current request count
        std::string count_key = "rate_limit:" + key;
        std::string count_str = db.getCache(count_key);
        int count = count_str.empty() ? 0 : std::stoi(count_str);
        
        // Find applicable rate limit
        RateLimit limit = getApplicableRateLimit(key);
        
        if (count >= limit.max_requests) {
            return false;
        }
        
        // Increment count
        count++;
        db.setCache(count_key, std::to_string(count), limit.window_seconds);
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Rate limit check error: {}", e.what());
        return true; // Allow on error
    }
}

AuthMiddleware::RateLimit AuthMiddleware::getApplicableRateLimit(const std::string& endpoint) const {
    // Check specific endpoint limits
    auto it = rate_limits_.find(endpoint);
    if (it != rate_limits_.end()) {
        return it->second;
    }
    
    // Check wildcard patterns
    for (const auto& [pattern, limit] : rate_limits_) {
        if (pattern.back() == '*') {
            std::string prefix = pattern.substr(0, pattern.length() - 1);
            if (endpoint.substr(0, prefix.length()) == prefix) {
                return limit;
            }
        }
    }
    
    // Default rate limit
    return {100, 60}; // 100 requests per minute
}

void AuthMiddleware::clearRateLimit(const std::string& user_id) {
    if (!rate_limiting_enabled_) {
        return;
    }
    
    try {
        auto& db = database::DatabaseManager::getInstance();
        db.deleteCache("rate_limit:user:" + user_id);
    } catch (const std::exception& e) {
        LOG_ERROR("Clear rate limit error: {}", e.what());
    }
}

bool AuthMiddleware::isSessionValid(const std::string& user_id, const std::string& token) {
    return validateSession(user_id, token);
}

void AuthMiddleware::updateLastActivity(const std::string& user_id) {
    if (!session_validation_enabled_) {
        return;
    }
    
    try {
        auto& db = database::DatabaseManager::getInstance();
        auto session_data = db.getCacheJson("session_meta:" + user_id);
        
        if (!session_data.empty()) {
            session_data["last_activity"] = std::chrono::system_clock::now().time_since_epoch().count();
            db.setCacheJson("session_meta:" + user_id, session_data, session_timeout_seconds_);
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Update last activity error: {}", e.what());
    }
}

void AuthMiddleware::handleUnauthorized(crow::response& res, const std::string& message) {
    res.code = 401;
    res.set_header("Content-Type", "application/json");
    
    nlohmann::json error;
    error["success"] = false;
    error["error"] = "Unauthorized";
    error["message"] = message;
    error["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();
    
    res.body = error.dump();
    res.end();
}

void AuthMiddleware::handleForbidden(crow::response& res, const std::string& message) {
    res.code = 403;
    res.set_header("Content-Type", "application/json");
    
    nlohmann::json error;
    error["success"] = false;
    error["error"] = "Forbidden";
    error["message"] = message;
    error["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();
    
    res.body = error.dump();
    res.end();
}

void AuthMiddleware::handleTooManyRequests(crow::response& res) {
    res.code = 429;
    res.set_header("Content-Type", "application/json");
    res.set_header("Retry-After", "60"); // Suggest retry after 60 seconds
    
    nlohmann::json error;
    error["success"] = false;
    error["error"] = "Too Many Requests";
    error["message"] = "Rate limit exceeded. Please try again later.";
    error["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();
    
    res.body = error.dump();
    res.end();
}

void AuthMiddleware::updateStats(bool authenticated, int status_code) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    stats_.total_requests++;
    
    if (authenticated) {
        stats_.authenticated_requests++;
    } else {
        stats_.unauthenticated_requests++;
    }
    
    if (status_code == 401) {
        stats_.failed_authentications++;
    } else if (status_code == 403) {
        stats_.forbidden_requests++;
    } else if (status_code == 429) {
        stats_.rate_limited_requests++;
    }
    
    stats_.last_request_time = std::chrono::system_clock::now();
}

} // namespace healthcare::middleware