#pragma once

#include <string>
#include "../models/User.h"

namespace healthcare::services {

class NotificationService {
public:
    NotificationService() = default;
    ~NotificationService() = default;
    
    bool sendVerificationEmail(const models::User& user);
    bool sendWelcomeEmail(const models::User& user);
    bool sendPasswordResetEmail(const models::User& user, const std::string& reset_token);
    bool sendPasswordResetSuccessNotification(const models::User& user);
    bool sendPasswordChangeNotification(const models::User& user);
    bool sendAppointmentConfirmation(const std::string& user_id, const std::string& appointment_id);
    bool sendAppointmentReminder(const std::string& user_id, const std::string& appointment_id);
    bool sendPushNotification(const std::string& fcm_token, const std::string& title, const std::string& body);
};

} // namespace healthcare::services