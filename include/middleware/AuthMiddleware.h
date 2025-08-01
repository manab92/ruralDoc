#pragma once

#include <string>
#include <vector>
#include <set>
#include <functional>
#include <crow.h>
#include <nlohmann/json.hpp>
#include "../utils/CryptoUtils.h"
#include "../utils/Logger.h"
#include "../utils/ResponseHelper.h"

namespace healthcare::middleware {

struct AuthContext {
    std::string user_id;
    std::string email;
    std::string role;
    std::vector<std::string> permissions;
    std::string session_id;
    std::chrono::system_clock::time_point token_issued_at;
    std::chrono::system_clock::time_point token_expires_at;
    bool is_authenticated = false;
    bool is_admin = false;
    bool is_doctor = false;
    bool is_user = false;
};

class AuthMiddleware : public crow::ILocalMiddleware {
public:
    AuthMiddleware();
    ~AuthMiddleware() = default;

    struct context {
        AuthContext auth_context;
        std::string request_id;
        std::chrono::high_resolution_clock::time_point start_time;
    };

    // Middleware hooks
    void before_handle(crow::request& req, crow::response& res, context& ctx);
    void after_handle(crow::request& req, crow::response& res, context& ctx);

    // Configuration
    void setJwtSecret(const std::string& secret) { jwt_secret_ = secret; }
    void setJwtIssuer(const std::string& issuer) { jwt_issuer_ = issuer; }
    void setTokenExpiryHours(int hours) { token_expiry_hours_ = hours; }
    void setRefreshThresholdHours(int hours) { refresh_threshold_hours_ = hours; }
    
    // Endpoint configuration
    void addPublicEndpoint(const std::string& endpoint);
    void addAdminEndpoint(const std::string& endpoint);
    void addDoctorEndpoint(const std::string& endpoint);
    void addUserEndpoint(const std::string& endpoint);
    void addEndpointPermission(const std::string& endpoint, const std::string& permission);
    
    // Role-based access control
    void setRolePermissions(const std::string& role, const std::vector<std::string>& permissions);
    void addRolePermission(const std::string& role, const std::string& permission);
    void removeRolePermission(const std::string& role, const std::string& permission);
    
    // CORS configuration
    void setCorsEnabled(bool enabled) { cors_enabled_ = enabled; }
    void setAllowedOrigins(const std::vector<std::string>& origins) { allowed_origins_ = origins; }
    void setAllowedMethods(const std::vector<std::string>& methods) { allowed_methods_ = methods; }
    void setAllowedHeaders(const std::vector<std::string>& headers) { allowed_headers_ = headers; }
    void setMaxAge(int seconds) { cors_max_age_ = seconds; }
    
    // Rate limiting
    void setRateLimitEnabled(bool enabled) { rate_limit_enabled_ = enabled; }
    void setRateLimit(const std::string& endpoint, int requests_per_minute);
    void setGlobalRateLimit(int requests_per_minute) { global_rate_limit_ = requests_per_minute; }
    
    // Token management
    std::string generateToken(const utils::JwtPayload& payload) const;
    utils::JwtPayload validateToken(const std::string& token) const;
    bool isTokenExpired(const std::string& token) const;
    std::string refreshToken(const std::string& token) const;
    
    // Permission checking
    bool hasPermission(const AuthContext& context, const std::string& permission) const;
    bool hasRole(const AuthContext& context, const std::string& role) const;
    bool canAccessEndpoint(const AuthContext& context, const std::string& endpoint, const std::string& method) const;
    
    // Session management
    void addActiveSession(const std::string& session_id, const std::string& user_id);
    void removeActiveSession(const std::string& session_id);
    bool isSessionActive(const std::string& session_id) const;
    void invalidateUserSessions(const std::string& user_id);
    
    // Security features
    void enableSecurityHeaders(bool enabled) { security_headers_enabled_ = enabled; }
    void setMaxLoginAttempts(int attempts) { max_login_attempts_ = attempts; }
    void setLoginLockoutDuration(int minutes) { login_lockout_duration_ = minutes; }
    
    // Statistics and monitoring
    struct AuthStats {
        long long total_requests = 0;
        long long authenticated_requests = 0;
        long long failed_authentications = 0;
        long long rate_limited_requests = 0;
        long long admin_requests = 0;
        long long doctor_requests = 0;
        long long user_requests = 0;
        std::map<std::string, int> endpoint_access_count;
        std::chrono::system_clock::time_point last_request_time;
    };
    
    AuthStats getStats() const { return stats_; }
    void resetStats() { stats_ = AuthStats{}; }

private:
    // Configuration
    std::string jwt_secret_;
    std::string jwt_issuer_;
    int token_expiry_hours_;
    int refresh_threshold_hours_;
    
    // Endpoint permissions
    std::set<std::string> public_endpoints_;
    std::set<std::string> admin_endpoints_;
    std::set<std::string> doctor_endpoints_;
    std::set<std::string> user_endpoints_;
    std::map<std::string, std::vector<std::string>> endpoint_permissions_;
    
    // Role permissions
    std::map<std::string, std::set<std::string>> role_permissions_;
    
    // CORS settings
    bool cors_enabled_;
    std::vector<std::string> allowed_origins_;
    std::vector<std::string> allowed_methods_;
    std::vector<std::string> allowed_headers_;
    int cors_max_age_;
    
    // Rate limiting
    bool rate_limit_enabled_;
    std::map<std::string, int> endpoint_rate_limits_;
    int global_rate_limit_;
    mutable std::map<std::string, std::vector<std::chrono::system_clock::time_point>> rate_limit_tracker_;
    
    // Session management
    mutable std::map<std::string, std::string> active_sessions_;  // session_id -> user_id
    mutable std::map<std::string, std::vector<std::string>> user_sessions_;  // user_id -> session_ids
    
    // Security
    bool security_headers_enabled_;
    int max_login_attempts_;
    int login_lockout_duration_;
    mutable std::map<std::string, int> login_attempts_;  // user_id -> attempts
    mutable std::map<std::string, std::chrono::system_clock::time_point> lockout_times_;
    
    // Statistics
    mutable AuthStats stats_;
    mutable std::mutex stats_mutex_;
    
    // Helper methods
    bool isPublicEndpoint(const std::string& path) const;
    bool isAdminEndpoint(const std::string& path) const;
    bool isDoctorEndpoint(const std::string& path) const;
    bool isUserEndpoint(const std::string& path) const;
    
    std::string extractToken(const crow::request& req) const;
    AuthContext createAuthContext(const utils::JwtPayload& payload) const;
    
    bool checkRateLimit(const std::string& endpoint, const std::string& client_ip) const;
    void updateRateLimit(const std::string& endpoint, const std::string& client_ip) const;
    void cleanupOldRateLimitEntries() const;
    
    bool isOriginAllowed(const std::string& origin) const;
    void addCorsHeaders(crow::response& res, const std::string& origin = "") const;
    void addSecurityHeaders(crow::response& res) const;
    
    bool isUserLockedOut(const std::string& user_id) const;
    void recordLoginAttempt(const std::string& user_id, bool success) const;
    void lockoutUser(const std::string& user_id) const;
    
    std::string normalizeEndpoint(const std::string& path) const;
    std::string getClientIp(const crow::request& req) const;
    
    void logAuthEvent(const std::string& event, const std::string& user_id, 
                     const std::string& endpoint, const std::string& details = "") const;
    void updateStats(const AuthContext& context, const std::string& endpoint) const;
    
    // Default configurations
    void initializeDefaults();
    void setupDefaultRolePermissions();
    void setupDefaultEndpointPermissions();
};

// Authentication helper functions
bool extractBearerToken(const std::string& auth_header, std::string& token);
std::string generateSessionId();
bool isValidSessionId(const std::string& session_id);

// Permission constants
namespace Permissions {
    const std::string READ_USERS = "read:users";
    const std::string WRITE_USERS = "write:users";
    const std::string DELETE_USERS = "delete:users";
    const std::string READ_DOCTORS = "read:doctors";
    const std::string WRITE_DOCTORS = "write:doctors";
    const std::string DELETE_DOCTORS = "delete:doctors";
    const std::string READ_APPOINTMENTS = "read:appointments";
    const std::string WRITE_APPOINTMENTS = "write:appointments";
    const std::string DELETE_APPOINTMENTS = "delete:appointments";
    const std::string READ_PRESCRIPTIONS = "read:prescriptions";
    const std::string WRITE_PRESCRIPTIONS = "write:prescriptions";
    const std::string DELETE_PRESCRIPTIONS = "delete:prescriptions";
    const std::string READ_CLINICS = "read:clinics";
    const std::string WRITE_CLINICS = "write:clinics";
    const std::string DELETE_CLINICS = "delete:clinics";
    const std::string ADMIN_ACCESS = "admin:access";
    const std::string SYSTEM_CONFIG = "system:config";
    const std::string USER_MANAGEMENT = "user:management";
    const std::string PAYMENT_MANAGEMENT = "payment:management";
}

// Role constants
namespace Roles {
    const std::string USER = "USER";
    const std::string DOCTOR = "DOCTOR";
    const std::string ADMIN = "ADMIN";
    const std::string SUPER_ADMIN = "SUPER_ADMIN";
}

// Auth middleware exceptions
class AuthenticationException : public std::exception {
public:
    explicit AuthenticationException(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }
    
private:
    std::string message_;
};

class AuthorizationException : public std::exception {
public:
    explicit AuthorizationException(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }
    
private:
    std::string message_;
};

class RateLimitException : public std::exception {
public:
    explicit RateLimitException(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }
    
private:
    std::string message_;
};

} // namespace healthcare::middleware