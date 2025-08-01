#pragma once

#include <string>
#include <memory>
#include <optional>
#include <vector>
#include "../models/User.h"
#include "../database/UserRepository.h"
#include "ValidationService.h"
#include "NotificationService.h"

namespace healthcare {
namespace services {

struct RegistrationRequest {
    std::string email;
    std::string password;
    std::string first_name;
    std::string last_name;
    std::string phone_number;
    models::UserRole role;
    models::Gender gender;
    std::chrono::system_clock::time_point date_of_birth;
    std::string address;
    std::string city;
    std::string state;
    std::string pincode;
};

struct LoginRequest {
    std::string email;
    std::string password;
    std::string fcm_token;
};

struct ProfileUpdateRequest {
    std::string first_name;
    std::string last_name;
    std::string phone_number;
    models::Gender gender;
    std::chrono::system_clock::time_point date_of_birth;
    std::string address;
    std::string city;
    std::string state;
    std::string pincode;
    std::string profile_picture_url;
};

struct PasswordChangeRequest {
    std::string current_password;
    std::string new_password;
};

enum class UserServiceError {
    SUCCESS = 0,
    EMAIL_ALREADY_EXISTS = 1,
    PHONE_ALREADY_EXISTS = 2,
    INVALID_EMAIL_FORMAT = 3,
    INVALID_PHONE_FORMAT = 4,
    WEAK_PASSWORD = 5,
    USER_NOT_FOUND = 6,
    INVALID_CREDENTIALS = 7,
    USER_NOT_VERIFIED = 8,
    USER_DEACTIVATED = 9,
    VALIDATION_ERROR = 10,
    DATABASE_ERROR = 11,
    UNAUTHORIZED = 12
};

struct UserServiceResult {
    UserServiceError error;
    std::string message;
    std::unique_ptr<models::User> user;
    std::string jwt_token;
};

class UserService {
private:
    std::unique_ptr<database::UserRepository> user_repository_;
    std::unique_ptr<ValidationService> validation_service_;
    std::unique_ptr<NotificationService> notification_service_;

public:
    UserService();
    ~UserService() = default;

    // Registration and Authentication
    UserServiceResult registerUser(const RegistrationRequest& request);
    UserServiceResult loginUser(const LoginRequest& request);
    UserServiceResult refreshToken(const std::string& refresh_token);
    bool logoutUser(const std::string& user_id, const std::string& fcm_token);

    // User Management
    UserServiceResult getUserById(const std::string& user_id);
    UserServiceResult getUserByEmail(const std::string& email);
    UserServiceResult updateProfile(const std::string& user_id, const ProfileUpdateRequest& request);
    UserServiceResult changePassword(const std::string& user_id, const PasswordChangeRequest& request);
    bool deleteUser(const std::string& user_id);

    // Verification and Activation
    bool sendVerificationEmail(const std::string& user_id);
    bool verifyEmail(const std::string& user_id, const std::string& verification_token);
    bool sendPasswordResetEmail(const std::string& email);
    bool resetPassword(const std::string& reset_token, const std::string& new_password);

    // Admin Operations
    std::vector<std::unique_ptr<models::User>> getAllUsers(models::UserRole role, int page = 1, int page_size = 20);
    bool activateUser(const std::string& user_id);
    bool deactivateUser(const std::string& user_id);
    bool changeUserRole(const std::string& user_id, models::UserRole new_role);

    // Search and Filter
    std::vector<std::unique_ptr<models::User>> searchUsers(const std::string& query, models::UserRole role);
    std::vector<std::unique_ptr<models::User>> getUsersByCity(const std::string& city);
    std::vector<std::unique_ptr<models::User>> getNewUsers(const std::chrono::system_clock::time_point& since);

    // Profile Picture Management
    bool uploadProfilePicture(const std::string& user_id, const std::string& image_data);
    bool deleteProfilePicture(const std::string& user_id);

    // FCM Token Management
    bool updateFcmToken(const std::string& user_id, const std::string& fcm_token);
    std::vector<std::string> getFcmTokensByRole(models::UserRole role);

    // Statistics
    int getTotalUsers();
    int getTotalUsersByRole(models::UserRole role);
    int getActiveUsersCount();
    int getVerifiedUsersCount();
    std::map<std::string, int> getUserStatsByCity();
    std::map<std::string, int> getUserRegistrationTrends(int days = 30);

    // Utility Methods
    bool validateUserPermissions(const std::string& user_id, models::UserRole required_role);
    bool isUserActive(const std::string& user_id);
    bool isUserVerified(const std::string& user_id);

private:
    // Helper methods
    std::string hashPassword(const std::string& password);
    bool verifyPassword(const std::string& password, const std::string& hash);
    std::string generateJwtToken(const models::User& user);
    std::string generateVerificationToken();
    std::string generateResetToken();
    bool validatePassword(const std::string& password);
    UserServiceError validateRegistrationRequest(const RegistrationRequest& request);
    UserServiceError validateProfileUpdateRequest(const ProfileUpdateRequest& request);
    std::string uploadImageToStorage(const std::string& image_data, const std::string& user_id);
    void sendWelcomeNotification(const models::User& user);
    void logUserActivity(const std::string& user_id, const std::string& activity);
};

} // namespace services
} // namespace healthcare