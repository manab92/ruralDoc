#include "../../include/middleware/CorsMiddleware.h"
#include <algorithm>
#include <sstream>

namespace healthcare::middleware {

CorsMiddleware::CorsMiddleware() {
    // Set default CORS configuration
    config_.allowed_origins = {"*"};
    config_.allowed_methods = {"GET", "POST", "PUT", "DELETE", "OPTIONS", "PATCH"};
    config_.allowed_headers = {
        "Content-Type",
        "Authorization",
        "X-Requested-With",
        "Accept",
        "Origin",
        "X-CSRF-Token"
    };
    config_.exposed_headers = {
        "X-Request-ID",
        "X-RateLimit-Limit",
        "X-RateLimit-Remaining",
        "X-RateLimit-Reset"
    };
    config_.allow_credentials = true;
    config_.max_age = 86400; // 24 hours
}

void CorsMiddleware::before_handle(crow::request& req, crow::response& res, context& ctx) {
    // Store request origin
    auto origin_header = req.headers.find("Origin");
    if (origin_header != req.headers.end()) {
        ctx.request_origin = origin_header->second;
    }
    
    // Handle preflight requests
    if (req.method == crow::HTTPMethod::Options) {
        handlePreflightRequest(req, res, ctx);
        res.end();
        return;
    }
    
    // Add CORS headers to all responses
    addCorsHeaders(res, ctx.request_origin);
}

void CorsMiddleware::after_handle(crow::request& req, crow::response& res, context& ctx) {
    // Ensure CORS headers are present even if they were removed
    if (!ctx.request_origin.empty()) {
        addCorsHeaders(res, ctx.request_origin);
    }
}

void CorsMiddleware::configure(const CorsConfig& config) {
    config_ = config;
}

void CorsMiddleware::addAllowedOrigin(const std::string& origin) {
    config_.allowed_origins.push_back(origin);
}

void CorsMiddleware::addAllowedMethod(const std::string& method) {
    config_.allowed_methods.push_back(method);
}

void CorsMiddleware::addAllowedHeader(const std::string& header) {
    config_.allowed_headers.push_back(header);
}

void CorsMiddleware::addExposedHeader(const std::string& header) {
    config_.exposed_headers.push_back(header);
}

void CorsMiddleware::handlePreflightRequest(crow::request& req, crow::response& res, context& ctx) {
    // Get the requested method
    auto method_header = req.headers.find("Access-Control-Request-Method");
    if (method_header != req.headers.end()) {
        ctx.requested_method = method_header->second;
    }
    
    // Get the requested headers
    auto headers_header = req.headers.find("Access-Control-Request-Headers");
    if (headers_header != req.headers.end()) {
        ctx.requested_headers = parseHeaderList(headers_header->second);
    }
    
    // Validate the request
    if (!isOriginAllowed(ctx.request_origin)) {
        res.code = 403;
        res.body = "Origin not allowed";
        return;
    }
    
    if (!isMethodAllowed(ctx.requested_method)) {
        res.code = 405;
        res.body = "Method not allowed";
        return;
    }
    
    for (const auto& header : ctx.requested_headers) {
        if (!isHeaderAllowed(header)) {
            res.code = 403;
            res.body = "Header not allowed: " + header;
            return;
        }
    }
    
    // Add CORS headers
    addCorsHeaders(res, ctx.request_origin);
    
    // Add preflight-specific headers
    res.add_header("Access-Control-Allow-Methods", joinStrings(config_.allowed_methods));
    res.add_header("Access-Control-Allow-Headers", joinStrings(config_.allowed_headers));
    res.add_header("Access-Control-Max-Age", std::to_string(config_.max_age));
    
    res.code = 204; // No Content
}

void CorsMiddleware::addCorsHeaders(crow::response& res, const std::string& origin) {
    // Set allowed origin
    if (isOriginAllowed(origin)) {
        if (config_.allowed_origins.size() == 1 && config_.allowed_origins[0] == "*") {
            res.add_header("Access-Control-Allow-Origin", "*");
        } else {
            res.add_header("Access-Control-Allow-Origin", origin);
            res.add_header("Vary", "Origin");
        }
    }
    
    // Set credentials
    if (config_.allow_credentials) {
        res.add_header("Access-Control-Allow-Credentials", "true");
    }
    
    // Set exposed headers
    if (!config_.exposed_headers.empty()) {
        res.add_header("Access-Control-Expose-Headers", joinStrings(config_.exposed_headers));
    }
}

bool CorsMiddleware::isOriginAllowed(const std::string& origin) const {
    if (origin.empty()) {
        return true; // Allow requests without Origin header (same-origin)
    }
    
    // Check if all origins are allowed
    if (std::find(config_.allowed_origins.begin(), config_.allowed_origins.end(), "*") 
        != config_.allowed_origins.end()) {
        return true;
    }
    
    // Check specific origins
    return std::find(config_.allowed_origins.begin(), config_.allowed_origins.end(), origin) 
           != config_.allowed_origins.end();
}

bool CorsMiddleware::isMethodAllowed(const std::string& method) const {
    if (method.empty()) {
        return true;
    }
    
    std::string upper_method = method;
    std::transform(upper_method.begin(), upper_method.end(), upper_method.begin(), ::toupper);
    
    return std::find(config_.allowed_methods.begin(), config_.allowed_methods.end(), upper_method) 
           != config_.allowed_methods.end();
}

bool CorsMiddleware::isHeaderAllowed(const std::string& header) const {
    if (header.empty()) {
        return true;
    }
    
    // Simple headers are always allowed
    static const std::set<std::string> simple_headers = {
        "accept",
        "accept-language",
        "content-language",
        "content-type"
    };
    
    std::string lower_header = header;
    std::transform(lower_header.begin(), lower_header.end(), lower_header.begin(), ::tolower);
    
    if (simple_headers.find(lower_header) != simple_headers.end()) {
        return true;
    }
    
    // Check configured allowed headers (case-insensitive)
    for (const auto& allowed : config_.allowed_headers) {
        std::string lower_allowed = allowed;
        std::transform(lower_allowed.begin(), lower_allowed.end(), lower_allowed.begin(), ::tolower);
        if (lower_allowed == lower_header) {
            return true;
        }
    }
    
    return false;
}

std::vector<std::string> CorsMiddleware::parseHeaderList(const std::string& header_list) const {
    std::vector<std::string> headers;
    std::stringstream ss(header_list);
    std::string header;
    
    while (std::getline(ss, header, ',')) {
        // Trim whitespace
        header.erase(0, header.find_first_not_of(" \t"));
        header.erase(header.find_last_not_of(" \t") + 1);
        
        if (!header.empty()) {
            headers.push_back(header);
        }
    }
    
    return headers;
}

std::string CorsMiddleware::joinStrings(const std::vector<std::string>& strings) const {
    std::ostringstream oss;
    for (size_t i = 0; i < strings.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << strings[i];
    }
    return oss.str();
}

} // namespace healthcare::middleware