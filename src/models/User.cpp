#include "../../include/models/User.h"
#include "../../include/utils/CryptoUtils.h"
#include "../../include/utils/ValidationUtils.h"
#include <regex>
#include <algorithm>

namespace healthcare::models {

User::User() 
    : BaseEntity(),
      is_verified_(false),
      role_(UserRole::USER),
      gender_(Gender::PREFER_NOT_TO_SAY) {
}

bool User::isValidEmail() const {
    const std::regex email_pattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return std::regex_match(email_, email_pattern);
}

bool User::isValidPhoneNumber() const {
    const std::regex phone_pattern(R"(^\+?[1-9]\d{1,14}$)");
    return std::regex_match(phone_number_, phone_pattern);
}

nlohmann::json User::toJson() const {
    nlohmann::json json;
    
    // Base entity fields
    json["id"] = getId();
    json["created_at"] = std::chrono::system_clock::to_time_t(getCreatedAt());
    json["updated_at"] = std::chrono::system_clock::to_time_t(getUpdatedAt());
    json["is_deleted"] = isDeleted();
    
    // User fields
    json["email"] = email_;
    json["first_name"] = first_name_;
    json["last_name"] = last_name_;
    json["phone_number"] = phone_number_;
    json["role"] = userRoleToString(role_);
    json["gender"] = genderToString(gender_);
    json["date_of_birth"] = date_of_birth_;
    json["address"] = address_;
    json["city"] = city_;
    json["state"] = state_;
    json["pincode"] = pincode_;
    json["profile_image_url"] = profile_image_url_;
    json["is_verified"] = is_verified_;
    json["fcm_token"] = fcm_token_;
    
    return json;
}

void User::fromJson(const nlohmann::json& json) {
    // Base entity fields
    if (json.contains("id")) setId(json["id"].get<std::string>());
    if (json.contains("created_at")) {
        auto timestamp = json["created_at"].get<std::time_t>();
        setCreatedAt(std::chrono::system_clock::from_time_t(timestamp));
    }
    if (json.contains("updated_at")) {
        auto timestamp = json["updated_at"].get<std::time_t>();
        setUpdatedAt(std::chrono::system_clock::from_time_t(timestamp));
    }
    if (json.contains("is_deleted")) setDeleted(json["is_deleted"].get<bool>());
    
    // User fields
    if (json.contains("email")) email_ = json["email"].get<std::string>();
    if (json.contains("password_hash")) password_hash_ = json["password_hash"].get<std::string>();
    if (json.contains("salt")) salt_ = json["salt"].get<std::string>();
    if (json.contains("first_name")) first_name_ = json["first_name"].get<std::string>();
    if (json.contains("last_name")) last_name_ = json["last_name"].get<std::string>();
    if (json.contains("phone_number")) phone_number_ = json["phone_number"].get<std::string>();
    if (json.contains("role")) role_ = stringToUserRole(json["role"].get<std::string>());
    if (json.contains("gender")) gender_ = stringToGender(json["gender"].get<std::string>());
    if (json.contains("date_of_birth")) date_of_birth_ = json["date_of_birth"].get<std::string>();
    if (json.contains("address")) address_ = json["address"].get<std::string>();
    if (json.contains("city")) city_ = json["city"].get<std::string>();
    if (json.contains("state")) state_ = json["state"].get<std::string>();
    if (json.contains("pincode")) pincode_ = json["pincode"].get<std::string>();
    if (json.contains("profile_image_url")) profile_image_url_ = json["profile_image_url"].get<std::string>();
    if (json.contains("is_verified")) is_verified_ = json["is_verified"].get<bool>();
    if (json.contains("verification_token")) verification_token_ = json["verification_token"].get<std::string>();
    if (json.contains("fcm_token")) fcm_token_ = json["fcm_token"].get<std::string>();
}

bool User::verifyPassword(const std::string& password) const {
    return utils::CryptoUtils::verifyPassword(password, password_hash_, salt_);
}

void User::setPassword(const std::string& password) {
    auto hash_result = utils::CryptoUtils::hashPassword(password);
    if (hash_result.success) {
        password_hash_ = hash_result.hash;
        salt_ = hash_result.salt;
    }
}

void User::generateVerificationToken() {
    verification_token_ = utils::CryptoUtils::generateRandomString(32);
}

// Utility functions
std::string userRoleToString(UserRole role) {
    switch (role) {
        case UserRole::USER: return "USER";
        case UserRole::DOCTOR: return "DOCTOR";
        case UserRole::ADMIN: return "ADMIN";
        default: return "USER";
    }
}

UserRole stringToUserRole(const std::string& role_str) {
    std::string upper_role = role_str;
    std::transform(upper_role.begin(), upper_role.end(), upper_role.begin(), ::toupper);
    
    if (upper_role == "DOCTOR") return UserRole::DOCTOR;
    if (upper_role == "ADMIN") return UserRole::ADMIN;
    return UserRole::USER;
}

std::string genderToString(Gender gender) {
    switch (gender) {
        case Gender::MALE: return "MALE";
        case Gender::FEMALE: return "FEMALE";
        case Gender::OTHER: return "OTHER";
        case Gender::PREFER_NOT_TO_SAY: return "PREFER_NOT_TO_SAY";
        default: return "PREFER_NOT_TO_SAY";
    }
}

Gender stringToGender(const std::string& gender_str) {
    std::string upper_gender = gender_str;
    std::transform(upper_gender.begin(), upper_gender.end(), upper_gender.begin(), ::toupper);
    
    if (upper_gender == "MALE") return Gender::MALE;
    if (upper_gender == "FEMALE") return Gender::FEMALE;
    if (upper_gender == "OTHER") return Gender::OTHER;
    return Gender::PREFER_NOT_TO_SAY;
}

} // namespace healthcare::models