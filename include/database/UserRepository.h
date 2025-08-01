#pragma once

#include "BaseRepository.h"
#include "../models/User.h"
#include <string>
#include <vector>
#include <optional>
#include <chrono>

namespace healthcare::database {

class UserRepository : public BaseRepository<models::User> {
public:
    UserRepository();
    ~UserRepository() override = default;

    // User-specific queries
    QueryResult<models::User> findByEmail(const std::string& email);
    QueryResult<models::User> findByPhoneNumber(const std::string& phone);
    QueryResult<models::User> findByRole(models::UserRole role, const PaginationParams& pagination = {});
    QueryResult<models::User> findByVerificationStatus(bool verified, const PaginationParams& pagination = {});
    QueryResult<models::User> findByCity(const std::string& city, const PaginationParams& pagination = {});
    QueryResult<models::User> findByState(const std::string& state, const PaginationParams& pagination = {});
    
    // Authentication queries
    QueryResult<models::User> findByEmailAndPassword(const std::string& email, const std::string& password_hash);
    QueryResult<models::User> findByVerificationToken(const std::string& token);
    
    // Admin queries
    QueryResult<models::User> findDoctorsPendingVerification(const PaginationParams& pagination = {});
    QueryResult<models::User> findActiveUsers(const PaginationParams& pagination = {});
    QueryResult<models::User> findUsersByDateRange(const std::chrono::system_clock::time_point& start_date,
                                                   const std::chrono::system_clock::time_point& end_date,
                                                   const PaginationParams& pagination = {});
    
    // User management operations
    bool updatePassword(const std::string& user_id, const std::string& new_password_hash, const std::string& salt);
    bool updateEmail(const std::string& user_id, const std::string& new_email);
    bool updateVerificationStatus(const std::string& user_id, bool verified);
    bool updateVerificationToken(const std::string& user_id, const std::string& token);
    bool updateFcmToken(const std::string& user_id, const std::string& fcm_token);
    bool updateProfile(const std::string& user_id, const models::User& updated_user);
    
    // Existence checks
    bool emailExists(const std::string& email);
    bool phoneExists(const std::string& phone);
    bool verificationTokenExists(const std::string& token);
    
    // Statistics
    int countByRole(models::UserRole role);
    int countVerifiedUsers();
    int countUnverifiedUsers();
    int countUsersByCity(const std::string& city);
    int countUsersByState(const std::string& state);
    int countUsersRegisteredToday();
    int countUsersRegisteredThisWeek();
    int countUsersRegisteredThisMonth();
    
    // Search operations
    QueryResult<models::User> searchUsers(const std::string& search_term, 
                                         const PaginationParams& pagination = {});
    QueryResult<models::User> searchUsersByName(const std::string& name, 
                                               const PaginationParams& pagination = {});
    QueryResult<models::User> searchDoctors(const std::string& search_term,
                                           const PaginationParams& pagination = {});
    
    // Analytics
    std::map<std::string, int> getUserCountByCity();
    std::map<std::string, int> getUserCountByState();
    std::map<models::UserRole, int> getUserCountByRole();
    std::map<std::string, int> getRegistrationCountByMonth(int year);
    
    // Bulk operations
    QueryResult<models::User> createUsers(const std::vector<models::User>& users);
    bool updateUsersStatus(const std::vector<std::string>& user_ids, bool verified);
    bool deleteUsers(const std::vector<std::string>& user_ids);
    
    // Password management
    bool updatePasswordWithVerification(const std::string& user_id, 
                                       const std::string& old_password_hash,
                                       const std::string& new_password_hash,
                                       const std::string& new_salt);
    
    // Session management
    bool updateLastLoginTime(const std::string& user_id);
    QueryResult<models::User> findUsersWithRecentActivity(int days = 30, 
                                                          const PaginationParams& pagination = {});
    
    // Data export
    QueryResult<models::User> exportUsers(const FilterParams& filters = {});
    std::string generateUserReport(const FilterParams& filters = {});

protected:
    // BaseRepository implementation
    models::User mapRowToEntity(const pqxx::row& row) const override;
    std::vector<std::string> getInsertValues(const models::User& user) const override;
    std::vector<std::string> getUpdateValues(const models::User& user) const override;
    std::vector<std::string> getColumnNames() const override;
    std::vector<std::string> getSearchableColumns() const override;

private:
    // Helper methods
    std::string buildUserSearchQuery(const std::string& search_term) const;
    std::string buildDoctorSearchQuery(const std::string& search_term) const;
    std::string buildDateRangeQuery(const std::chrono::system_clock::time_point& start_date,
                                   const std::chrono::system_clock::time_point& end_date) const;
    
    // Validation helpers
    bool validateEmail(const std::string& email) const;
    bool validatePhoneNumber(const std::string& phone) const;
    bool validateUserData(const models::User& user) const;
    
    // Cache operations specific to User
    void cacheUserByEmail(const models::User& user) const;
    void cacheUserByPhone(const models::User& user) const;
    std::optional<models::User> getCachedUserByEmail(const std::string& email) const;
    std::optional<models::User> getCachedUserByPhone(const std::string& phone) const;
    void removeCachedUserByEmail(const std::string& email) const;
    void removeCachedUserByPhone(const std::string& phone) const;
    
    // Prepared statements
    void prepareDynamicQueries();
    static const std::string FIND_BY_EMAIL_QUERY;
    static const std::string FIND_BY_PHONE_QUERY;
    static const std::string FIND_BY_ROLE_QUERY;
    static const std::string UPDATE_PASSWORD_QUERY;
    static const std::string UPDATE_EMAIL_QUERY;
    static const std::string UPDATE_VERIFICATION_QUERY;
    static const std::string COUNT_BY_ROLE_QUERY;
    static const std::string SEARCH_USERS_QUERY;
    static const std::string SEARCH_DOCTORS_QUERY;
};

// User repository helper functions
std::string userRoleToString(models::UserRole role);
models::UserRole stringToUserRole(const std::string& role_str);
std::string genderToString(models::Gender gender);
models::Gender stringToGender(const std::string& gender_str);

// User-specific exceptions
class UserNotFoundException : public DatabaseException {
public:
    explicit UserNotFoundException(const std::string& identifier) 
        : DatabaseException("User not found: " + identifier) {}
};

class EmailAlreadyExistsException : public DatabaseException {
public:
    explicit EmailAlreadyExistsException(const std::string& email) 
        : DatabaseException("Email already exists: " + email) {}
};

class PhoneAlreadyExistsException : public DatabaseException {
public:
    explicit PhoneAlreadyExistsException(const std::string& phone) 
        : DatabaseException("Phone number already exists: " + phone) {}
};

class InvalidUserDataException : public DatabaseException {
public:
    explicit InvalidUserDataException(const std::string& reason) 
        : DatabaseException("Invalid user data: " + reason) {}
};

} // namespace healthcare::database