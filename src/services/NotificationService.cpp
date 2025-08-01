#include "../../include/services/NotificationService.h"
#include "../../include/utils/Logger.h"

namespace healthcare::services {

bool NotificationService::sendVerificationEmail(const models::User& user) {
    LOG_INFO("Sending verification email to: {}", user.getEmail());
    // Stub implementation - in production, this would integrate with email service
    return true;
}

bool NotificationService::sendWelcomeEmail(const models::User& user) {
    LOG_INFO("Sending welcome email to: {}", user.getEmail());
    // Stub implementation
    return true;
}

bool NotificationService::sendPasswordResetEmail(const models::User& user, const std::string& reset_token) {
    LOG_INFO("Sending password reset email to: {} with token: {}", user.getEmail(), reset_token);
    // Stub implementation
    return true;
}

bool NotificationService::sendPasswordResetSuccessNotification(const models::User& user) {
    LOG_INFO("Sending password reset success notification to: {}", user.getEmail());
    // Stub implementation
    return true;
}

bool NotificationService::sendPasswordChangeNotification(const models::User& user) {
    LOG_INFO("Sending password change notification to: {}", user.getEmail());
    // Stub implementation
    return true;
}

bool NotificationService::sendAppointmentConfirmation(const std::string& user_id, const std::string& appointment_id) {
    LOG_INFO("Sending appointment confirmation to user: {} for appointment: {}", user_id, appointment_id);
    // Stub implementation
    return true;
}

bool NotificationService::sendAppointmentReminder(const std::string& user_id, const std::string& appointment_id) {
    LOG_INFO("Sending appointment reminder to user: {} for appointment: {}", user_id, appointment_id);
    // Stub implementation
    return true;
}

bool NotificationService::sendPushNotification(const std::string& fcm_token, const std::string& title, const std::string& body) {
    LOG_INFO("Sending push notification to FCM token: {} - Title: {}", fcm_token, title);
    // Stub implementation - in production, this would integrate with FCM
    return true;
}

} // namespace healthcare::services