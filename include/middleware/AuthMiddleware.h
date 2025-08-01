#pragma once

#include <crow.h>
#include <jwt-cpp/jwt.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <unordered_set>
#include "../models/User.h"

namespace healthcare {
namespace middleware {

struct AuthContext {
    std::string user_id;
    std::string email;
    models::UserRole role;
    bool is_authenticated = false;
    bool is_verified = false;
    bool is_active = false;
    std::string jwt_token;
    std::chrono::system_clock::time_point token_expiry;
};

class AuthMiddleware {
private:
    std::string jwt_secret_;
    std::string jwt_issuer_;
    int token_expiry_hours_;
    std::unordered_set<std::string> public_endpoints_;
    std::unordered_set<std::string> admin_endpoints_;

public:
    struct context {
        AuthContext auth_context;
    };

    AuthMiddleware();
    AuthMiddleware(const std::string& jwt_secret, const std::string& jwt_issuer, int token_expiry_hours = 24);

    // Middleware execution
    void before_handle(crow::request& req, crow::response& res, context& ctx);
    void after_handle(crow::request& req, crow::response& res, context& ctx);

    // Token operations
    std::string generateToken(const models::User& user);
    bool validateToken(const std::string& token, AuthContext& auth_context);
    std::string refreshToken(const std::string& current_token);
    bool revokeToken(const std::string& token);

    // Configuration
    void addPublicEndpoint(const std::string& endpoint);
    void addAdminEndpoint(const std::string& endpoint);
    void setJwtSecret(const std::string& secret);
    void setJwtIssuer(const std::string& issuer);
    void setTokenExpiryHours(int hours);

    // Utility methods
    static AuthContext getAuthContext(const crow::request& req);
    static bool isAuthenticated(const crow::request& req);
    static bool hasRole(const crow::request& req, models::UserRole required_role);
    static bool isAdmin(const crow::request& req);
    static std::string getUserId(const crow::request& req);
    static std::string getUserEmail(const crow::request& req);
    static models::UserRole getUserRole(const crow::request& req);

private:
    // Helper methods
    std::string extractTokenFromHeader(const crow::request& req);
    bool isPublicEndpoint(const std::string& endpoint);
    bool isAdminEndpoint(const std::string& endpoint);
    bool requiresAuthentication(const std::string& endpoint);
    bool requiresAdminAccess(const std::string& endpoint);
    std::string normalizeEndpoint(const std::string& endpoint);
    
    // JWT helper methods
    jwt::decoded_jwt<jwt::traits::nlohmann_json> decodeToken(const std::string& token);
    bool isTokenExpired(const jwt::decoded_jwt<jwt::traits::nlohmann_json>& decoded);
    AuthContext extractAuthContextFromToken(const jwt::decoded_jwt<jwt::traits::nlohmann_json>& decoded);
    
    // Response helpers
    crow::response createUnauthorizedResponse(const std::string& message = "Unauthorized");
    crow::response createForbiddenResponse(const std::string& message = "Forbidden");
    crow::response createTokenExpiredResponse();

    // Validation methods
    bool validateTokenSignature(const jwt::decoded_jwt<jwt::traits::nlohmann_json>& decoded);
    bool validateTokenClaims(const jwt::decoded_jwt<jwt::traits::nlohmann_json>& decoded);
    bool isTokenRevoked(const std::string& token);

    // Role-based access control
    bool hasPermission(models::UserRole user_role, models::UserRole required_role);
    std::vector<std::string> getPermissions(models::UserRole role);
};

// Helper functions for token management
namespace TokenUtils {
    std::string roleToString(models::UserRole role);
    models::UserRole stringToRole(const std::string& role_str);
    std::string timePointToString(const std::chrono::system_clock::time_point& tp);
    std::chrono::system_clock::time_point stringToTimePoint(const std::string& time_str);
}

} // namespace middleware
} // namespace healthcare