#include "../../include/database/UserRepository.h"
#include "../../include/utils/Logger.h"
#include <sstream>

namespace healthcare::database {

UserRepository::UserRepository() : BaseRepository<models::User>("users") {
}

QueryResult<models::User> UserRepository::findByEmail(const std::string& email) {
    return executeWithTiming([&]() {
        try {
            std::string query = buildSelectQuery("email = $1 AND is_deleted = false");
            auto result = db_manager_.executeQuery(query, {email});
            
            if (result.empty()) {
                return QueryResult<models::User>("User not found with email: " + email);
            }
            
            models::User user = mapRowToEntity(result[0]);
            cacheEntity(user);
            
            return QueryResult<models::User>({user});
            
        } catch (const std::exception& e) {
            logError("findByEmail", e.what());
            return QueryResult<models::User>(std::string("Find by email failed: ") + e.what());
        }
    });
}

QueryResult<models::User> UserRepository::findByPhoneNumber(const std::string& phone_number) {
    return executeWithTiming([&]() {
        try {
            std::string query = buildSelectQuery("phone_number = $1 AND is_deleted = false");
            auto result = db_manager_.executeQuery(query, {phone_number});
            
            if (result.empty()) {
                return QueryResult<models::User>("User not found with phone: " + phone_number);
            }
            
            models::User user = mapRowToEntity(result[0]);
            cacheEntity(user);
            
            return QueryResult<models::User>({user});
            
        } catch (const std::exception& e) {
            logError("findByPhoneNumber", e.what());
            return QueryResult<models::User>(std::string("Find by phone failed: ") + e.what());
        }
    });
}

QueryResult<models::User> UserRepository::findByRole(models::UserRole role, const PaginationParams& pagination) {
    return executeWithTiming([&]() {
        try {
            std::string role_str = models::userRoleToString(role);
            std::string query = buildSelectQuery("role = $1 AND is_deleted = false",
                                               pagination.getOrderClause(),
                                               pagination.getLimitClause());
            
            auto result = db_manager_.executeQuery(query, {role_str});
            
            std::vector<models::User> users;
            for (const auto& row : result) {
                users.push_back(mapRowToEntity(row));
            }
            
            QueryResult<models::User> query_result(users);
            
            // Get total count
            auto count_result = db_manager_.executeQuery(
                buildCountQuery("role = $1 AND is_deleted = false"), {role_str}
            );
            if (!count_result.empty()) {
                query_result.total_count = count_result[0][0].as<int>();
            }
            
            return query_result;
            
        } catch (const std::exception& e) {
            logError("findByRole", e.what());
            return QueryResult<models::User>(std::string("Find by role failed: ") + e.what());
        }
    });
}

QueryResult<models::User> UserRepository::findByCity(const std::string& city, const PaginationParams& pagination) {
    return executeWithTiming([&]() {
        try {
            std::string query = buildSelectQuery("city = $1 AND is_deleted = false",
                                               pagination.getOrderClause(),
                                               pagination.getLimitClause());
            
            auto result = db_manager_.executeQuery(query, {city});
            
            std::vector<models::User> users;
            for (const auto& row : result) {
                users.push_back(mapRowToEntity(row));
            }
            
            QueryResult<models::User> query_result(users);
            
            // Get total count
            auto count_result = db_manager_.executeQuery(
                buildCountQuery("city = $1 AND is_deleted = false"), {city}
            );
            if (!count_result.empty()) {
                query_result.total_count = count_result[0][0].as<int>();
            }
            
            return query_result;
            
        } catch (const std::exception& e) {
            logError("findByCity", e.what());
            return QueryResult<models::User>(std::string("Find by city failed: ") + e.what());
        }
    });
}

QueryResult<models::User> UserRepository::findVerifiedUsers(const PaginationParams& pagination) {
    return executeWithTiming([&]() {
        try {
            std::string query = buildSelectQuery("is_verified = true AND is_deleted = false",
                                               pagination.getOrderClause(),
                                               pagination.getLimitClause());
            
            auto result = db_manager_.executeQuery(query);
            
            std::vector<models::User> users;
            for (const auto& row : result) {
                users.push_back(mapRowToEntity(row));
            }
            
            QueryResult<models::User> query_result(users);
            
            // Get total count
            auto count_result = db_manager_.executeQuery(
                buildCountQuery("is_verified = true AND is_deleted = false")
            );
            if (!count_result.empty()) {
                query_result.total_count = count_result[0][0].as<int>();
            }
            
            return query_result;
            
        } catch (const std::exception& e) {
            logError("findVerifiedUsers", e.what());
            return QueryResult<models::User>(std::string("Find verified users failed: ") + e.what());
        }
    });
}

QueryResult<models::User> UserRepository::findUnverifiedUsers(const PaginationParams& pagination) {
    return executeWithTiming([&]() {
        try {
            std::string query = buildSelectQuery("is_verified = false AND is_deleted = false",
                                               pagination.getOrderClause(),
                                               pagination.getLimitClause());
            
            auto result = db_manager_.executeQuery(query);
            
            std::vector<models::User> users;
            for (const auto& row : result) {
                users.push_back(mapRowToEntity(row));
            }
            
            QueryResult<models::User> query_result(users);
            
            // Get total count
            auto count_result = db_manager_.executeQuery(
                buildCountQuery("is_verified = false AND is_deleted = false")
            );
            if (!count_result.empty()) {
                query_result.total_count = count_result[0][0].as<int>();
            }
            
            return query_result;
            
        } catch (const std::exception& e) {
            logError("findUnverifiedUsers", e.what());
            return QueryResult<models::User>(std::string("Find unverified users failed: ") + e.what());
        }
    });
}

QueryResult<models::User> UserRepository::findByVerificationToken(const std::string& token) {
    return executeWithTiming([&]() {
        try {
            std::string query = buildSelectQuery("verification_token = $1 AND is_deleted = false");
            auto result = db_manager_.executeQuery(query, {token});
            
            if (result.empty()) {
                return QueryResult<models::User>("User not found with verification token");
            }
            
            models::User user = mapRowToEntity(result[0]);
            cacheEntity(user);
            
            return QueryResult<models::User>({user});
            
        } catch (const std::exception& e) {
            logError("findByVerificationToken", e.what());
            return QueryResult<models::User>(std::string("Find by verification token failed: ") + e.what());
        }
    });
}

bool UserRepository::emailExists(const std::string& email) {
    return executeWithTiming([&]() {
        try {
            std::string query = "SELECT EXISTS(SELECT 1 FROM users WHERE email = $1 AND is_deleted = false)";
            auto result = db_manager_.executeQuery(query, {email});
            
            return !result.empty() && result[0][0].as<bool>();
            
        } catch (const std::exception& e) {
            logError("emailExists", e.what());
            return false;
        }
    });
}

bool UserRepository::phoneNumberExists(const std::string& phone_number) {
    return executeWithTiming([&]() {
        try {
            std::string query = "SELECT EXISTS(SELECT 1 FROM users WHERE phone_number = $1 AND is_deleted = false)";
            auto result = db_manager_.executeQuery(query, {phone_number});
            
            return !result.empty() && result[0][0].as<bool>();
            
        } catch (const std::exception& e) {
            logError("phoneNumberExists", e.what());
            return false;
        }
    });
}

bool UserRepository::updateLastLogin(const std::string& user_id) {
    return executeWithTiming([&]() {
        try {
            std::string query = "UPDATE users SET updated_at = CURRENT_TIMESTAMP WHERE id = $1";
            db_manager_.executeQuery(query, {user_id});
            
            // Clear cache for this user
            removeCachedEntity(user_id);
            
            return true;
            
        } catch (const std::exception& e) {
            logError("updateLastLogin", e.what());
            return false;
        }
    });
}

bool UserRepository::updateVerificationStatus(const std::string& user_id, bool is_verified) {
    return executeWithTiming([&]() {
        try {
            std::string query = "UPDATE users SET is_verified = $1, verification_token = NULL, "
                              "updated_at = CURRENT_TIMESTAMP WHERE id = $2";
            
            db_manager_.executeQuery(query, {is_verified ? "true" : "false", user_id});
            
            // Clear cache for this user
            removeCachedEntity(user_id);
            
            return true;
            
        } catch (const std::exception& e) {
            logError("updateVerificationStatus", e.what());
            return false;
        }
    });
}

bool UserRepository::updateFcmToken(const std::string& user_id, const std::string& fcm_token) {
    return executeWithTiming([&]() {
        try {
            std::string query = "UPDATE users SET fcm_token = $1, updated_at = CURRENT_TIMESTAMP WHERE id = $2";
            db_manager_.executeQuery(query, {fcm_token, user_id});
            
            // Clear cache for this user
            removeCachedEntity(user_id);
            
            return true;
            
        } catch (const std::exception& e) {
            logError("updateFcmToken", e.what());
            return false;
        }
    });
}

bool UserRepository::updatePassword(const std::string& user_id, const std::string& password_hash, const std::string& salt) {
    return executeWithTiming([&]() {
        try {
            std::string query = "UPDATE users SET password_hash = $1, salt = $2, "
                              "updated_at = CURRENT_TIMESTAMP WHERE id = $3";
            
            db_manager_.executeQuery(query, {password_hash, salt, user_id});
            
            // Clear cache for this user
            removeCachedEntity(user_id);
            
            return true;
            
        } catch (const std::exception& e) {
            logError("updatePassword", e.what());
            return false;
        }
    });
}

std::vector<std::string> UserRepository::getFcmTokensByRole(models::UserRole role) {
    return executeWithTiming([&]() {
        std::vector<std::string> tokens;
        
        try {
            std::string role_str = models::userRoleToString(role);
            std::string query = "SELECT fcm_token FROM users WHERE role = $1 AND is_deleted = false "
                              "AND is_verified = true AND fcm_token IS NOT NULL AND fcm_token != ''";
            
            auto result = db_manager_.executeQuery(query, {role_str});
            
            for (const auto& row : result) {
                tokens.push_back(row["fcm_token"].as<std::string>());
            }
            
            return tokens;
            
        } catch (const std::exception& e) {
            logError("getFcmTokensByRole", e.what());
            return tokens;
        }
    });
}

int UserRepository::countByRole(models::UserRole role) {
    return executeWithTiming([&]() {
        try {
            std::string role_str = models::userRoleToString(role);
            std::string query = "SELECT COUNT(*) FROM users WHERE role = $1 AND is_deleted = false";
            
            auto result = db_manager_.executeQuery(query, {role_str});
            return result.empty() ? 0 : result[0][0].as<int>();
            
        } catch (const std::exception& e) {
            logError("countByRole", e.what());
            return 0;
        }
    });
}

int UserRepository::countVerifiedUsers() {
    return executeWithTiming([&]() {
        try {
            std::string query = "SELECT COUNT(*) FROM users WHERE is_verified = true AND is_deleted = false";
            auto result = db_manager_.executeQuery(query);
            
            return result.empty() ? 0 : result[0][0].as<int>();
            
        } catch (const std::exception& e) {
            logError("countVerifiedUsers", e.what());
            return 0;
        }
    });
}

std::map<std::string, int> UserRepository::getUserStatsByCity() {
    return executeWithTiming([&]() {
        std::map<std::string, int> stats;
        
        try {
            std::string query = "SELECT city, COUNT(*) as count FROM users "
                              "WHERE is_deleted = false AND city IS NOT NULL AND city != '' "
                              "GROUP BY city ORDER BY count DESC";
            
            auto result = db_manager_.executeQuery(query);
            
            for (const auto& row : result) {
                stats[row["city"].as<std::string>()] = row["count"].as<int>();
            }
            
            return stats;
            
        } catch (const std::exception& e) {
            logError("getUserStatsByCity", e.what());
            return stats;
        }
    });
}

std::map<std::string, int> UserRepository::getRegistrationTrends(int days) {
    return executeWithTiming([&]() {
        std::map<std::string, int> trends;
        
        try {
            std::string query = "SELECT DATE(created_at) as date, COUNT(*) as count FROM users "
                              "WHERE created_at >= CURRENT_DATE - INTERVAL '" + std::to_string(days) + " days' "
                              "AND is_deleted = false "
                              "GROUP BY DATE(created_at) ORDER BY date";
            
            auto result = db_manager_.executeQuery(query);
            
            for (const auto& row : result) {
                trends[row["date"].as<std::string>()] = row["count"].as<int>();
            }
            
            return trends;
            
        } catch (const std::exception& e) {
            logError("getRegistrationTrends", e.what());
            return trends;
        }
    });
}

models::User UserRepository::mapRowToEntity(const pqxx::row& row) const {
    models::User user;
    
    // Base entity fields
    user.setId(row["id"].as<std::string>());
    user.setCreatedAt(parseTimestamp(row["created_at"].as<std::string>()));
    user.setUpdatedAt(parseTimestamp(row["updated_at"].as<std::string>()));
    user.setDeleted(row["is_deleted"].as<bool>());
    
    // User fields
    user.setEmail(row["email"].as<std::string>());
    user.setPasswordHash(row["password_hash"].as<std::string>());
    user.setSalt(row["salt"].as<std::string>());
    user.setFirstName(row["first_name"].as<std::string>());
    user.setLastName(row["last_name"].as<std::string>());
    
    if (!row["phone_number"].is_null()) {
        user.setPhoneNumber(row["phone_number"].as<std::string>());
    }
    
    user.setRole(models::stringToUserRole(row["role"].as<std::string>()));
    
    if (!row["gender"].is_null()) {
        user.setGender(models::stringToGender(row["gender"].as<std::string>()));
    }
    
    if (!row["date_of_birth"].is_null()) {
        user.setDateOfBirth(row["date_of_birth"].as<std::string>());
    }
    
    if (!row["address"].is_null()) {
        user.setAddress(row["address"].as<std::string>());
    }
    
    if (!row["city"].is_null()) {
        user.setCity(row["city"].as<std::string>());
    }
    
    if (!row["state"].is_null()) {
        user.setState(row["state"].as<std::string>());
    }
    
    if (!row["pincode"].is_null()) {
        user.setPincode(row["pincode"].as<std::string>());
    }
    
    if (!row["profile_image_url"].is_null()) {
        user.setProfileImageUrl(row["profile_image_url"].as<std::string>());
    }
    
    user.setVerified(row["is_verified"].as<bool>());
    
    if (!row["verification_token"].is_null()) {
        user.setVerificationToken(row["verification_token"].as<std::string>());
    }
    
    if (!row["fcm_token"].is_null()) {
        user.setFcmToken(row["fcm_token"].as<std::string>());
    }
    
    return user;
}

std::vector<std::string> UserRepository::getInsertValues(const models::User& entity) const {
    std::vector<std::string> values;
    
    values.push_back(entity.getId());
    values.push_back(entity.getEmail());
    values.push_back(entity.getPasswordHash());
    values.push_back(entity.getSalt());
    values.push_back(entity.getFirstName());
    values.push_back(entity.getLastName());
    values.push_back(entity.getPhoneNumber());
    values.push_back(models::userRoleToString(entity.getRole()));
    values.push_back(models::genderToString(entity.getGender()));
    values.push_back(entity.getDateOfBirth());
    values.push_back(entity.getAddress());
    values.push_back(entity.getCity());
    values.push_back(entity.getState());
    values.push_back(entity.getPincode());
    values.push_back(entity.getProfileImageUrl());
    values.push_back(entity.isVerified() ? "true" : "false");
    values.push_back(entity.getVerificationToken());
    values.push_back(entity.getFcmToken());
    values.push_back(formatTimestamp(entity.getCreatedAt()));
    values.push_back(formatTimestamp(entity.getUpdatedAt()));
    values.push_back(entity.isDeleted() ? "true" : "false");
    
    return values;
}

std::vector<std::string> UserRepository::getUpdateValues(const models::User& entity) const {
    std::vector<std::string> values;
    
    values.push_back(entity.getEmail());
    values.push_back(entity.getPasswordHash());
    values.push_back(entity.getSalt());
    values.push_back(entity.getFirstName());
    values.push_back(entity.getLastName());
    values.push_back(entity.getPhoneNumber());
    values.push_back(models::userRoleToString(entity.getRole()));
    values.push_back(models::genderToString(entity.getGender()));
    values.push_back(entity.getDateOfBirth());
    values.push_back(entity.getAddress());
    values.push_back(entity.getCity());
    values.push_back(entity.getState());
    values.push_back(entity.getPincode());
    values.push_back(entity.getProfileImageUrl());
    values.push_back(entity.isVerified() ? "true" : "false");
    values.push_back(entity.getVerificationToken());
    values.push_back(entity.getFcmToken());
    values.push_back(formatTimestamp(entity.getUpdatedAt()));
    values.push_back(entity.isDeleted() ? "true" : "false");
    
    return values;
}

std::vector<std::string> UserRepository::getColumnNames() const {
    return {
        "id", "email", "password_hash", "salt", "first_name", "last_name",
        "phone_number", "role", "gender", "date_of_birth", "address",
        "city", "state", "pincode", "profile_image_url", "is_verified",
        "verification_token", "fcm_token", "created_at", "updated_at", "is_deleted"
    };
}

std::vector<std::string> UserRepository::getSearchableColumns() const {
    return {"email", "first_name", "last_name", "phone_number", "city"};
}

std::chrono::system_clock::time_point UserRepository::parseTimestamp(const std::string& timestamp) const {
    std::tm tm = {};
    std::istringstream iss(timestamp);
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

} // namespace healthcare::database