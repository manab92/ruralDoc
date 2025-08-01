#include "../../include/services/UserService.h"
#include "../../include/utils/CryptoUtils.h"
#include "../../include/utils/ValidationUtils.h"
#include "../../include/utils/Logger.h"
#include "../../include/database/DatabaseManager.h"
#include <algorithm>

namespace healthcare::services {

UserService::UserService() 
    : user_repository_(std::make_unique<database::UserRepository>()),
      validation_service_(std::make_unique<ValidationService>()),
      notification_service_(std::make_unique<NotificationService>()) {
}

UserServiceResult UserService::registerUser(const RegistrationRequest& request) {
    UserServiceResult result;
    
    // Validate registration request
    result.error = validateRegistrationRequest(request);
    if (result.error != UserServiceError::SUCCESS) {
        result.message = "Validation failed";
        return result;
    }
    
    // Check if email already exists
    if (user_repository_->emailExists(request.email)) {
        result.error = UserServiceError::EMAIL_ALREADY_EXISTS;
        result.message = "Email already registered";
        return result;
    }
    
    // Check if phone already exists
    if (!request.phone_number.empty() && user_repository_->phoneNumberExists(request.phone_number)) {
        result.error = UserServiceError::PHONE_ALREADY_EXISTS;
        result.message = "Phone number already registered";
        return result;
    }
    
    try {
        // Create new user
        auto user = std::make_unique<models::User>();
        user->setEmail(request.email);
        user->setFirstName(request.first_name);
        user->setLastName(request.last_name);
        user->setPhoneNumber(request.phone_number);
        user->setRole(request.role);
        user->setGender(request.gender);
        user->setDateOfBirth(request.date_of_birth);
        user->setAddress(request.address);
        user->setCity(request.city);
        user->setState(request.state);
        user->setPincode(request.pincode);
        
        // Hash password
        auto hash_result = utils::CryptoUtils::hashPassword(request.password);
        if (!hash_result.success) {
            result.error = UserServiceError::DATABASE_ERROR;
            result.message = "Failed to hash password";
            return result;
        }
        
        user->setPassword(hash_result.hash, hash_result.salt);
        
        // Generate verification token
        user->generateVerificationToken();
        
        // Save user to database
        auto create_result = user_repository_->create(*user);
        if (!create_result.success) {
            result.error = UserServiceError::DATABASE_ERROR;
            result.message = create_result.error;
            return result;
        }
        
        // Send verification email
        sendVerificationEmail(create_result.data[0].getId());
        
        // Send welcome notification
        sendWelcomeNotification(create_result.data[0]);
        
        // Generate JWT token
        result.jwt_token = generateJwtToken(create_result.data[0]);
        result.user = std::make_unique<models::User>(create_result.data[0]);
        result.error = UserServiceError::SUCCESS;
        result.message = "User registered successfully";
        
        // Log activity
        logUserActivity(result.user->getId(), "User registered");
        
    } catch (const std::exception& e) {
        LOG_ERROR("User registration failed: {}", e.what());
        result.error = UserServiceError::DATABASE_ERROR;
        result.message = "Registration failed";
    }
    
    return result;
}

UserServiceResult UserService::loginUser(const LoginRequest& request) {
    UserServiceResult result;
    
    // Validate email
    if (!utils::ValidationUtils::isValidEmail(request.email)) {
        result.error = UserServiceError::INVALID_EMAIL_FORMAT;
        result.message = "Invalid email format";
        return result;
    }
    
    try {
        // Find user by email
        auto user_result = user_repository_->findByEmail(request.email);
        if (!user_result.success || user_result.data.empty()) {
            result.error = UserServiceError::INVALID_CREDENTIALS;
            result.message = "Invalid email or password";
            return result;
        }
        
        auto& user = user_result.data[0];
        
        // Verify password
        if (!user.verifyPassword(request.password)) {
            result.error = UserServiceError::INVALID_CREDENTIALS;
            result.message = "Invalid email or password";
            return result;
        }
        
        // Check if user is verified
        if (!user.getIsEmailVerified()) {
            result.error = UserServiceError::USER_NOT_VERIFIED;
            result.message = "Please verify your email before logging in";
            return result;
        }
        
        // Check if user is active
        if (!user.getIsActive()) {
            result.error = UserServiceError::USER_DEACTIVATED;
            result.message = "Your account has been deactivated";
            return result;
        }
        
        // Update FCM token if provided
        if (!request.fcm_token.empty()) {
            user.setFcmToken(request.fcm_token);
            user_repository_->updateFcmToken(user.getId(), request.fcm_token);
        }
        
        // Update last login
        user_repository_->updateLastLogin(user.getId());
        
        // Generate JWT token
        result.jwt_token = generateJwtToken(user);
        result.user = std::make_unique<models::User>(user);
        result.error = UserServiceError::SUCCESS;
        result.message = "Login successful";
        
        // Log activity
        logUserActivity(user.getId(), "User logged in");
        
    } catch (const std::exception& e) {
        LOG_ERROR("User login failed: {}", e.what());
        result.error = UserServiceError::DATABASE_ERROR;
        result.message = "Login failed";
    }
    
    return result;
}

UserServiceResult UserService::refreshToken(const std::string& refresh_token) {
    UserServiceResult result;
    
    try {
        // Verify refresh token
        auto jwt_result = utils::CryptoUtils::verifyJwtToken(refresh_token, "refresh_secret");
        if (!jwt_result.valid) {
            result.error = UserServiceError::UNAUTHORIZED;
            result.message = "Invalid refresh token";
            return result;
        }
        
        // Get user ID from token
        std::string user_id = jwt_result.claims["user_id"];
        
        // Get user from database
        auto user_result = user_repository_->findById(user_id);
        if (!user_result.success || user_result.data.empty()) {
            result.error = UserServiceError::USER_NOT_FOUND;
            result.message = "User not found";
            return result;
        }
        
        auto& user = user_result.data[0];
        
        // Check if user is active
        if (!user.getIsActive()) {
            result.error = UserServiceError::USER_DEACTIVATED;
            result.message = "User account is deactivated";
            return result;
        }
        
        // Generate new JWT token
        result.jwt_token = generateJwtToken(user);
        result.user = std::make_unique<models::User>(user);
        result.error = UserServiceError::SUCCESS;
        result.message = "Token refreshed successfully";
        
    } catch (const std::exception& e) {
        LOG_ERROR("Token refresh failed: {}", e.what());
        result.error = UserServiceError::DATABASE_ERROR;
        result.message = "Token refresh failed";
    }
    
    return result;
}

bool UserService::logoutUser(const std::string& user_id, const std::string& fcm_token) {
    try {
        // Clear FCM token if provided
        if (!fcm_token.empty()) {
            user_repository_->updateFcmToken(user_id, "");
        }
        
        // Log activity
        logUserActivity(user_id, "User logged out");
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("User logout failed: {}", e.what());
        return false;
    }
}

UserServiceResult UserService::getUserById(const std::string& user_id) {
    UserServiceResult result;
    
    try {
        auto user_result = user_repository_->findById(user_id);
        if (!user_result.success || user_result.data.empty()) {
            result.error = UserServiceError::USER_NOT_FOUND;
            result.message = "User not found";
            return result;
        }
        
        result.user = std::make_unique<models::User>(user_result.data[0]);
        result.error = UserServiceError::SUCCESS;
        result.message = "User found";
        
    } catch (const std::exception& e) {
        LOG_ERROR("Get user by ID failed: {}", e.what());
        result.error = UserServiceError::DATABASE_ERROR;
        result.message = "Failed to get user";
    }
    
    return result;
}

UserServiceResult UserService::getUserByEmail(const std::string& email) {
    UserServiceResult result;
    
    try {
        auto user_result = user_repository_->findByEmail(email);
        if (!user_result.success || user_result.data.empty()) {
            result.error = UserServiceError::USER_NOT_FOUND;
            result.message = "User not found";
            return result;
        }
        
        result.user = std::make_unique<models::User>(user_result.data[0]);
        result.error = UserServiceError::SUCCESS;
        result.message = "User found";
        
    } catch (const std::exception& e) {
        LOG_ERROR("Get user by email failed: {}", e.what());
        result.error = UserServiceError::DATABASE_ERROR;
        result.message = "Failed to get user";
    }
    
    return result;
}

UserServiceResult UserService::updateProfile(const std::string& user_id, const ProfileUpdateRequest& request) {
    UserServiceResult result;
    
    // Validate update request
    result.error = validateProfileUpdateRequest(request);
    if (result.error != UserServiceError::SUCCESS) {
        result.message = "Validation failed";
        return result;
    }
    
    try {
        // Get existing user
        auto user_result = user_repository_->findById(user_id);
        if (!user_result.success || user_result.data.empty()) {
            result.error = UserServiceError::USER_NOT_FOUND;
            result.message = "User not found";
            return result;
        }
        
        auto& user = user_result.data[0];
        
        // Update user fields
        if (!request.first_name.empty()) user.setFirstName(request.first_name);
        if (!request.last_name.empty()) user.setLastName(request.last_name);
        if (!request.phone_number.empty()) {
            // Check if phone number is already taken
            if (user_repository_->phoneNumberExists(request.phone_number) && 
                user.getPhoneNumber() != request.phone_number) {
                result.error = UserServiceError::PHONE_ALREADY_EXISTS;
                result.message = "Phone number already in use";
                return result;
            }
            user.setPhoneNumber(request.phone_number);
        }
        user.setGender(request.gender);
        user.setDateOfBirth(request.date_of_birth);
        if (!request.address.empty()) user.setAddress(request.address);
        if (!request.city.empty()) user.setCity(request.city);
        if (!request.state.empty()) user.setState(request.state);
        if (!request.pincode.empty()) user.setPincode(request.pincode);
        if (!request.profile_picture_url.empty()) user.setProfilePictureUrl(request.profile_picture_url);
        
        // Update in database
        auto update_result = user_repository_->update(user);
        if (!update_result.success) {
            result.error = UserServiceError::DATABASE_ERROR;
            result.message = update_result.error;
            return result;
        }
        
        result.user = std::make_unique<models::User>(update_result.data[0]);
        result.error = UserServiceError::SUCCESS;
        result.message = "Profile updated successfully";
        
        // Log activity
        logUserActivity(user_id, "Profile updated");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Profile update failed: {}", e.what());
        result.error = UserServiceError::DATABASE_ERROR;
        result.message = "Profile update failed";
    }
    
    return result;
}

UserServiceResult UserService::changePassword(const std::string& user_id, const PasswordChangeRequest& request) {
    UserServiceResult result;
    
    // Validate new password
    if (!validatePassword(request.new_password)) {
        result.error = UserServiceError::WEAK_PASSWORD;
        result.message = "Password does not meet security requirements";
        return result;
    }
    
    try {
        // Get user
        auto user_result = user_repository_->findById(user_id);
        if (!user_result.success || user_result.data.empty()) {
            result.error = UserServiceError::USER_NOT_FOUND;
            result.message = "User not found";
            return result;
        }
        
        auto& user = user_result.data[0];
        
        // Verify current password
        if (!user.verifyPassword(request.current_password)) {
            result.error = UserServiceError::INVALID_CREDENTIALS;
            result.message = "Current password is incorrect";
            return result;
        }
        
        // Update password
        user.setPassword(request.new_password);
        
        // Save to database
        user_repository_->updatePassword(user_id, user.getPasswordHash(), user.getPasswordSalt());
        
        result.user = std::make_unique<models::User>(user);
        result.error = UserServiceError::SUCCESS;
        result.message = "Password changed successfully";
        
        // Log activity
        logUserActivity(user_id, "Password changed");
        
        // Send notification
        notification_service_->sendPasswordChangeNotification(user);
        
    } catch (const std::exception& e) {
        LOG_ERROR("Password change failed: {}", e.what());
        result.error = UserServiceError::DATABASE_ERROR;
        result.message = "Password change failed";
    }
    
    return result;
}

bool UserService::deleteUser(const std::string& user_id) {
    try {
        // Soft delete the user
        bool success = user_repository_->softDeleteById(user_id);
        
        if (success) {
            logUserActivity(user_id, "User account deleted");
        }
        
        return success;
        
    } catch (const std::exception& e) {
        LOG_ERROR("User deletion failed: {}", e.what());
        return false;
    }
}

bool UserService::sendVerificationEmail(const std::string& user_id) {
    try {
        auto user_result = user_repository_->findById(user_id);
        if (!user_result.success || user_result.data.empty()) {
            return false;
        }
        
        auto& user = user_result.data[0];
        
        // Generate new verification token if needed
        if (user.getVerificationToken().empty()) {
            user.generateVerificationToken();
            user_repository_->update(user);
        }
        
        // Send verification email
        return notification_service_->sendVerificationEmail(user);
        
    } catch (const std::exception& e) {
        LOG_ERROR("Send verification email failed: {}", e.what());
        return false;
    }
}

bool UserService::verifyEmail(const std::string& user_id, const std::string& verification_token) {
    try {
        auto user_result = user_repository_->findByVerificationToken(verification_token);
        if (!user_result.success || user_result.data.empty()) {
            return false;
        }
        
        auto& user = user_result.data[0];
        
        // Check if user ID matches
        if (user.getId() != user_id) {
            return false;
        }
        
        // Check if already verified
        if (user.getIsEmailVerified()) {
            return true;
        }
        
        // Update verification status
        return user_repository_->updateVerificationStatus(user_id, true);
        
    } catch (const std::exception& e) {
        LOG_ERROR("Email verification failed: {}", e.what());
        return false;
    }
}

bool UserService::sendPasswordResetEmail(const std::string& email) {
    try {
        auto user_result = user_repository_->findByEmail(email);
        if (!user_result.success || user_result.data.empty()) {
            // Don't reveal if email exists
            return true;
        }
        
        auto& user = user_result.data[0];
        
        // Generate reset token
        std::string reset_token = generateResetToken();
        
        // Store reset token in cache with expiry
        auto& db = database::DatabaseManager::getInstance();
        db.setCache("password_reset:" + reset_token, user.getId(), 3600); // 1 hour expiry
        
        // Send reset email
        return notification_service_->sendPasswordResetEmail(user, reset_token);
        
    } catch (const std::exception& e) {
        LOG_ERROR("Send password reset email failed: {}", e.what());
        return false;
    }
}

bool UserService::resetPassword(const std::string& reset_token, const std::string& new_password) {
    try {
        // Validate new password
        if (!validatePassword(new_password)) {
            return false;
        }
        
        // Get user ID from reset token
        auto& db = database::DatabaseManager::getInstance();
        std::string user_id = db.getCache("password_reset:" + reset_token);
        
        if (user_id.empty()) {
            return false;
        }
        
        // Get user
        auto user_result = user_repository_->findById(user_id);
        if (!user_result.success || user_result.data.empty()) {
            return false;
        }
        
        auto& user = user_result.data[0];
        
        // Update password
        user.setPassword(new_password);
        bool success = user_repository_->updatePassword(user_id, user.getPasswordHash(), user.getPasswordSalt());
        
        if (success) {
            // Delete reset token
            db.deleteCache("password_reset:" + reset_token);
            
            // Log activity
            logUserActivity(user_id, "Password reset");
            
            // Send notification
            notification_service_->sendPasswordResetSuccessNotification(user);
        }
        
        return success;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Password reset failed: {}", e.what());
        return false;
    }
}

std::vector<std::unique_ptr<models::User>> UserService::getAllUsers(models::UserRole role, int page, int page_size) {
    std::vector<std::unique_ptr<models::User>> users;
    
    try {
        database::PaginationParams pagination;
        pagination.page = page;
        pagination.page_size = page_size;
        
        auto result = user_repository_->findByRole(role, pagination);
        if (result.success) {
            for (auto& user : result.data) {
                users.push_back(std::make_unique<models::User>(user));
            }
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Get all users failed: {}", e.what());
    }
    
    return users;
}

bool UserService::activateUser(const std::string& user_id) {
    try {
        auto user_result = user_repository_->findById(user_id);
        if (!user_result.success || user_result.data.empty()) {
            return false;
        }
        
        auto& user = user_result.data[0];
        user.setIsActive(true);
        
        auto update_result = user_repository_->update(user);
        
        if (update_result.success) {
            logUserActivity(user_id, "User activated");
        }
        
        return update_result.success;
        
    } catch (const std::exception& e) {
        LOG_ERROR("User activation failed: {}", e.what());
        return false;
    }
}

bool UserService::deactivateUser(const std::string& user_id) {
    try {
        auto user_result = user_repository_->findById(user_id);
        if (!user_result.success || user_result.data.empty()) {
            return false;
        }
        
        auto& user = user_result.data[0];
        user.setIsActive(false);
        
        auto update_result = user_repository_->update(user);
        
        if (update_result.success) {
            logUserActivity(user_id, "User deactivated");
        }
        
        return update_result.success;
        
    } catch (const std::exception& e) {
        LOG_ERROR("User deactivation failed: {}", e.what());
        return false;
    }
}

bool UserService::changeUserRole(const std::string& user_id, models::UserRole new_role) {
    try {
        auto user_result = user_repository_->findById(user_id);
        if (!user_result.success || user_result.data.empty()) {
            return false;
        }
        
        auto& user = user_result.data[0];
        user.setRole(new_role);
        
        auto update_result = user_repository_->update(user);
        
        if (update_result.success) {
            logUserActivity(user_id, "User role changed to " + models::userRoleToString(new_role));
        }
        
        return update_result.success;
        
    } catch (const std::exception& e) {
        LOG_ERROR("User role change failed: {}", e.what());
        return false;
    }
}

std::vector<std::unique_ptr<models::User>> UserService::searchUsers(const std::string& query, models::UserRole role) {
    std::vector<std::unique_ptr<models::User>> users;
    
    try {
        database::PaginationParams pagination;
        pagination.page_size = 50;
        
        auto result = user_repository_->search(query, {"first_name", "last_name", "email", "phone_number"}, pagination);
        
        if (result.success) {
            for (auto& user : result.data) {
                if (role == models::UserRole::USER || user.getRole() == role) {
                    users.push_back(std::make_unique<models::User>(user));
                }
            }
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("User search failed: {}", e.what());
    }
    
    return users;
}

std::vector<std::unique_ptr<models::User>> UserService::getUsersByCity(const std::string& city) {
    std::vector<std::unique_ptr<models::User>> users;
    
    try {
        auto result = user_repository_->findByCity(city);
        if (result.success) {
            for (auto& user : result.data) {
                users.push_back(std::make_unique<models::User>(user));
            }
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Get users by city failed: {}", e.what());
    }
    
    return users;
}

std::vector<std::unique_ptr<models::User>> UserService::getNewUsers(const std::chrono::system_clock::time_point& since) {
    std::vector<std::unique_ptr<models::User>> users;
    
    try {
        database::FilterParams filters;
        filters.addFilter("created_at", ">=", since);
        
        database::PaginationParams pagination;
        pagination.order_by = "created_at";
        pagination.order_direction = "DESC";
        pagination.page_size = 100;
        
        auto result = user_repository_->findByFilter(filters, pagination);
        
        if (result.success) {
            for (auto& user : result.data) {
                users.push_back(std::make_unique<models::User>(user));
            }
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Get new users failed: {}", e.what());
    }
    
    return users;
}

bool UserService::uploadProfilePicture(const std::string& user_id, const std::string& image_data) {
    try {
        // Upload image to storage
        std::string image_url = uploadImageToStorage(image_data, user_id);
        
        if (image_url.empty()) {
            return false;
        }
        
        // Update user profile picture URL
        auto user_result = user_repository_->findById(user_id);
        if (!user_result.success || user_result.data.empty()) {
            return false;
        }
        
        auto& user = user_result.data[0];
        user.setProfilePictureUrl(image_url);
        
        auto update_result = user_repository_->update(user);
        
        if (update_result.success) {
            logUserActivity(user_id, "Profile picture uploaded");
        }
        
        return update_result.success;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Profile picture upload failed: {}", e.what());
        return false;
    }
}

bool UserService::deleteProfilePicture(const std::string& user_id) {
    try {
        auto user_result = user_repository_->findById(user_id);
        if (!user_result.success || user_result.data.empty()) {
            return false;
        }
        
        auto& user = user_result.data[0];
        user.setProfilePictureUrl("");
        
        auto update_result = user_repository_->update(user);
        
        if (update_result.success) {
            logUserActivity(user_id, "Profile picture deleted");
        }
        
        return update_result.success;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Profile picture deletion failed: {}", e.what());
        return false;
    }
}

bool UserService::updateFcmToken(const std::string& user_id, const std::string& fcm_token) {
    try {
        return user_repository_->updateFcmToken(user_id, fcm_token);
    } catch (const std::exception& e) {
        LOG_ERROR("FCM token update failed: {}", e.what());
        return false;
    }
}

std::vector<std::string> UserService::getFcmTokensByRole(models::UserRole role) {
    try {
        return user_repository_->getFcmTokensByRole(role);
    } catch (const std::exception& e) {
        LOG_ERROR("Get FCM tokens by role failed: {}", e.what());
        return {};
    }
}

int UserService::getTotalUsers() {
    try {
        return user_repository_->countAll();
    } catch (const std::exception& e) {
        LOG_ERROR("Get total users failed: {}", e.what());
        return 0;
    }
}

int UserService::getTotalUsersByRole(models::UserRole role) {
    try {
        return user_repository_->countByRole(role);
    } catch (const std::exception& e) {
        LOG_ERROR("Get total users by role failed: {}", e.what());
        return 0;
    }
}

int UserService::getActiveUsersCount() {
    try {
        database::FilterParams filters;
        filters.addFilter("is_active", "=", true);
        return user_repository_->countByFilter(filters);
    } catch (const std::exception& e) {
        LOG_ERROR("Get active users count failed: {}", e.what());
        return 0;
    }
}

int UserService::getVerifiedUsersCount() {
    try {
        return user_repository_->countVerifiedUsers();
    } catch (const std::exception& e) {
        LOG_ERROR("Get verified users count failed: {}", e.what());
        return 0;
    }
}

std::map<std::string, int> UserService::getUserStatsByCity() {
    try {
        return user_repository_->getUserStatsByCity();
    } catch (const std::exception& e) {
        LOG_ERROR("Get user stats by city failed: {}", e.what());
        return {};
    }
}

std::map<std::string, int> UserService::getUserRegistrationTrends(int days) {
    try {
        return user_repository_->getRegistrationTrends(days);
    } catch (const std::exception& e) {
        LOG_ERROR("Get user registration trends failed: {}", e.what());
        return {};
    }
}

bool UserService::validateUserPermissions(const std::string& user_id, models::UserRole required_role) {
    try {
        auto user_result = user_repository_->findById(user_id);
        if (!user_result.success || user_result.data.empty()) {
            return false;
        }
        
        auto& user = user_result.data[0];
        
        // Admin has all permissions
        if (user.getRole() == models::UserRole::ADMIN) {
            return true;
        }
        
        // Check specific role
        return user.getRole() == required_role;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Validate user permissions failed: {}", e.what());
        return false;
    }
}

bool UserService::isUserActive(const std::string& user_id) {
    try {
        auto user_result = user_repository_->findById(user_id);
        if (!user_result.success || user_result.data.empty()) {
            return false;
        }
        
        return user_result.data[0].getIsActive();
        
    } catch (const std::exception& e) {
        LOG_ERROR("Check user active status failed: {}", e.what());
        return false;
    }
}

bool UserService::isUserVerified(const std::string& user_id) {
    try {
        auto user_result = user_repository_->findById(user_id);
        if (!user_result.success || user_result.data.empty()) {
            return false;
        }
        
        return user_result.data[0].getIsEmailVerified();
        
    } catch (const std::exception& e) {
        LOG_ERROR("Check user verified status failed: {}", e.what());
        return false;
    }
}

// Private helper methods

std::string UserService::hashPassword(const std::string& password) {
    auto result = utils::CryptoUtils::hashPassword(password);
    return result.success ? result.hash : "";
}

bool UserService::verifyPassword(const std::string& password, const std::string& hash) {
    return utils::CryptoUtils::verifyPassword(password, hash, "");
}

std::string UserService::generateJwtToken(const models::User& user) {
    nlohmann::json payload;
    payload["user_id"] = user.getId();
    payload["email"] = user.getEmail();
    payload["role"] = models::userRoleToString(user.getRole());
    
    auto result = utils::CryptoUtils::generateJwtToken(payload, "jwt_secret");
    return result.valid ? result.token : "";
}

std::string UserService::generateVerificationToken() {
    return utils::CryptoUtils::generateRandomString(32, true);
}

std::string UserService::generateResetToken() {
    return utils::CryptoUtils::generateRandomString(32, true);
}

bool UserService::validatePassword(const std::string& password) {
    return utils::ValidationUtils::isValidPassword(password);
}

UserServiceError UserService::validateRegistrationRequest(const RegistrationRequest& request) {
    if (!utils::ValidationUtils::isValidEmail(request.email)) {
        return UserServiceError::INVALID_EMAIL_FORMAT;
    }
    
    if (!utils::ValidationUtils::isValidPassword(request.password)) {
        return UserServiceError::WEAK_PASSWORD;
    }
    
    if (!request.phone_number.empty() && !utils::ValidationUtils::isValidPhoneNumber(request.phone_number)) {
        return UserServiceError::INVALID_PHONE_FORMAT;
    }
    
    if (!utils::ValidationUtils::isValidName(request.first_name) || 
        !utils::ValidationUtils::isValidName(request.last_name)) {
        return UserServiceError::VALIDATION_ERROR;
    }
    
    return UserServiceError::SUCCESS;
}

UserServiceError UserService::validateProfileUpdateRequest(const ProfileUpdateRequest& request) {
    if (!request.phone_number.empty() && !utils::ValidationUtils::isValidPhoneNumber(request.phone_number)) {
        return UserServiceError::INVALID_PHONE_FORMAT;
    }
    
    if (!request.first_name.empty() && !utils::ValidationUtils::isValidName(request.first_name)) {
        return UserServiceError::VALIDATION_ERROR;
    }
    
    if (!request.last_name.empty() && !utils::ValidationUtils::isValidName(request.last_name)) {
        return UserServiceError::VALIDATION_ERROR;
    }
    
    if (!request.profile_picture_url.empty() && !utils::ValidationUtils::isValidImageUrl(request.profile_picture_url)) {
        return UserServiceError::VALIDATION_ERROR;
    }
    
    return UserServiceError::SUCCESS;
}

std::string UserService::uploadImageToStorage(const std::string& image_data, const std::string& user_id) {
    // TODO: Implement actual image upload to storage service
    // For now, return a mock URL
    return "https://storage.example.com/profile-pictures/" + user_id + ".jpg";
}

void UserService::sendWelcomeNotification(const models::User& user) {
    try {
        notification_service_->sendWelcomeEmail(user);
    } catch (const std::exception& e) {
        LOG_ERROR("Send welcome notification failed: {}", e.what());
    }
}

void UserService::logUserActivity(const std::string& user_id, const std::string& activity) {
    try {
        nlohmann::json activity_data;
        activity_data["user_id"] = user_id;
        activity_data["activity"] = activity;
        activity_data["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();
        
        LOG_INFO("User activity: {}", activity_data.dump());
        
    } catch (const std::exception& e) {
        LOG_ERROR("Log user activity failed: {}", e.what());
    }
}

} // namespace healthcare::services