#pragma once

#include "BaseRepository.h"
#include "../models/User.h"

namespace healthcare {
namespace database {

class UserRepository : public BaseRepository<models::User> {
public:
    UserRepository() : BaseRepository("users", "user") {}

    // Override base methods
    std::unique_ptr<models::User> fromRow(const pqxx::row& row) override;
    std::string getInsertQuery(const models::User& user) override;
    std::string getUpdateQuery(const models::User& user) override;
    std::vector<std::string> getInsertParams(const models::User& user) override;
    std::vector<std::string> getUpdateParams(const models::User& user) override;

    // User-specific methods
    std::optional<std::unique_ptr<models::User>> findByEmail(const std::string& email);
    std::optional<std::unique_ptr<models::User>> findByPhoneNumber(const std::string& phone_number);
    std::vector<std::unique_ptr<models::User>> findByRole(models::UserRole role);
    std::vector<std::unique_ptr<models::User>> findByCity(const std::string& city);
    bool emailExists(const std::string& email);
    bool phoneNumberExists(const std::string& phone_number);
    
    // Authentication related
    std::optional<std::unique_ptr<models::User>> authenticateUser(const std::string& email, 
                                                                 const std::string& password_hash);
    bool updatePassword(const std::string& user_id, const std::string& new_password_hash);
    bool verifyUser(const std::string& user_id);
    bool deactivateUser(const std::string& user_id);
    bool activateUser(const std::string& user_id);

    // FCM token management
    bool updateFcmToken(const std::string& user_id, const std::string& fcm_token);
    std::vector<std::string> getFcmTokensByRole(models::UserRole role);

    // Statistics
    int getTotalUsersByRole(models::UserRole role);
    int getNewUsersCount(const std::chrono::system_clock::time_point& since);
    std::vector<std::pair<std::string, int>> getUsersByCity();

private:
    std::string roleToString(models::UserRole role);
    models::UserRole stringToRole(const std::string& role_str);
    std::string genderToString(models::Gender gender);
    models::Gender stringToGender(const std::string& gender_str);
};

} // namespace database
} // namespace healthcare