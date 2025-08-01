#pragma once

#include <crow.h>
#include <nlohmann/json.hpp>
#include "../services/UserService.h"
#include "../middleware/AuthMiddleware.h"
#include "../utils/ResponseHelper.h"

namespace healthcare {
namespace controllers {

class UserController {
private:
    std::unique_ptr<services::UserService> user_service_;
    std::unique_ptr<middleware::AuthMiddleware> auth_middleware_;

public:
    UserController();
    ~UserController() = default;

    // Route registration
    void registerRoutes(crow::Crow<crow::CookieParser, middleware::AuthMiddleware>& app);

    // Authentication endpoints
    crow::response register_user(const crow::request& req);
    crow::response login_user(const crow::request& req);
    crow::response refresh_token(const crow::request& req);
    crow::response logout_user(const crow::request& req);

    // Profile management
    crow::response get_profile(const crow::request& req, const std::string& user_id);
    crow::response update_profile(const crow::request& req, const std::string& user_id);
    crow::response change_password(const crow::request& req, const std::string& user_id);
    crow::response delete_account(const crow::request& req, const std::string& user_id);

    // Email verification
    crow::response send_verification_email(const crow::request& req);
    crow::response verify_email(const crow::request& req, const std::string& token);

    // Password reset
    crow::response forgot_password(const crow::request& req);
    crow::response reset_password(const crow::request& req, const std::string& token);

    // Profile picture
    crow::response upload_profile_picture(const crow::request& req, const std::string& user_id);
    crow::response delete_profile_picture(const crow::request& req, const std::string& user_id);
    crow::response get_profile_picture(const crow::request& req, const std::string& user_id);

    // User search and discovery
    crow::response search_users(const crow::request& req);
    crow::response get_users_by_city(const crow::request& req, const std::string& city);

    // Admin endpoints
    crow::response get_all_users(const crow::request& req);
    crow::response get_user_by_id(const crow::request& req, const std::string& user_id);
    crow::response activate_user(const crow::request& req, const std::string& user_id);
    crow::response deactivate_user(const crow::request& req, const std::string& user_id);
    crow::response change_user_role(const crow::request& req, const std::string& user_id);

    // Statistics endpoints (Admin only)
    crow::response get_user_statistics(const crow::request& req);
    crow::response get_registration_trends(const crow::request& req);
    crow::response get_users_by_city_stats(const crow::request& req);

    // FCM token management
    crow::response update_fcm_token(const crow::request& req, const std::string& user_id);

private:
    // Helper methods
    nlohmann::json userToJson(const models::User& user, bool include_sensitive = false);
    services::RegistrationRequest parseRegistrationRequest(const nlohmann::json& json);
    services::LoginRequest parseLoginRequest(const nlohmann::json& json);
    services::ProfileUpdateRequest parseProfileUpdateRequest(const nlohmann::json& json);
    services::PasswordChangeRequest parsePasswordChangeRequest(const nlohmann::json& json);

    // Validation helpers
    bool validateRegistrationInput(const nlohmann::json& json, std::string& error_message);
    bool validateLoginInput(const nlohmann::json& json, std::string& error_message);
    bool validateProfileUpdateInput(const nlohmann::json& json, std::string& error_message);
    bool validatePasswordChangeInput(const nlohmann::json& json, std::string& error_message);

    // Authentication helpers
    bool isUserAuthorized(const crow::request& req, const std::string& user_id);
    bool isAdminUser(const crow::request& req);
    std::string getUserIdFromToken(const crow::request& req);
    models::UserRole getUserRoleFromToken(const crow::request& req);

    // Response helpers
    crow::response createSuccessResponse(const nlohmann::json& data);
    crow::response createErrorResponse(const std::string& error, int status_code = 400);
    crow::response createUserServiceErrorResponse(services::UserServiceError error, const std::string& message);
    
    // File upload helpers
    bool validateImageFile(const std::string& content_type, size_t file_size);
    std::string extractImageData(const crow::request& req);
};

} // namespace controllers
} // namespace healthcare