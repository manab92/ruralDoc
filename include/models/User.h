#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include "BaseEntity.h"

namespace healthcare {
namespace models {

enum class UserRole {
    USER = 1,
    DOCTOR = 2,
    ADMIN = 3
};

enum class Gender {
    MALE = 1,
    FEMALE = 2,
    OTHER = 3
};

class User : public BaseEntity {
private:
    std::string email_;
    std::string password_hash_;
    std::string first_name_;
    std::string last_name_;
    std::string phone_number_;
    std::chrono::system_clock::time_point date_of_birth_;
    Gender gender_;
    UserRole role_;
    std::string profile_picture_url_;
    std::string address_;
    std::string city_;
    std::string state_;
    std::string pincode_;
    bool is_verified_;
    bool is_active_;
    std::string fcm_token_;  // For push notifications

public:
    // Constructors
    User();
    User(const std::string& email, const std::string& password_hash, 
         const std::string& first_name, const std::string& last_name,
         const std::string& phone_number, UserRole role);

    // Getters
    const std::string& getEmail() const { return email_; }
    const std::string& getPasswordHash() const { return password_hash_; }
    const std::string& getFirstName() const { return first_name_; }
    const std::string& getLastName() const { return last_name_; }
    const std::string& getPhoneNumber() const { return phone_number_; }
    const std::chrono::system_clock::time_point& getDateOfBirth() const { return date_of_birth_; }
    Gender getGender() const { return gender_; }
    UserRole getRole() const { return role_; }
    const std::string& getProfilePictureUrl() const { return profile_picture_url_; }
    const std::string& getAddress() const { return address_; }
    const std::string& getCity() const { return city_; }
    const std::string& getState() const { return state_; }
    const std::string& getPincode() const { return pincode_; }
    bool isVerified() const { return is_verified_; }
    bool isActive() const { return is_active_; }
    const std::string& getFcmToken() const { return fcm_token_; }

    // Setters
    void setEmail(const std::string& email) { email_ = email; }
    void setPasswordHash(const std::string& password_hash) { password_hash_ = password_hash; }
    void setFirstName(const std::string& first_name) { first_name_ = first_name; }
    void setLastName(const std::string& last_name) { last_name_ = last_name; }
    void setPhoneNumber(const std::string& phone_number) { phone_number_ = phone_number; }
    void setDateOfBirth(const std::chrono::system_clock::time_point& date_of_birth) { date_of_birth_ = date_of_birth; }
    void setGender(Gender gender) { gender_ = gender; }
    void setRole(UserRole role) { role_ = role; }
    void setProfilePictureUrl(const std::string& profile_picture_url) { profile_picture_url_ = profile_picture_url; }
    void setAddress(const std::string& address) { address_ = address; }
    void setCity(const std::string& city) { city_ = city; }
    void setState(const std::string& state) { state_ = state; }
    void setPincode(const std::string& pincode) { pincode_ = pincode; }
    void setVerified(bool is_verified) { is_verified_ = is_verified; }
    void setActive(bool is_active) { is_active_ = is_active; }
    void setFcmToken(const std::string& fcm_token) { fcm_token_ = fcm_token; }

    // Utility methods
    std::string getFullName() const;
    int getAge() const;
    bool validateEmail() const;
    bool validatePhoneNumber() const;

    // Serialization
    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& json) override;
};

} // namespace models
} // namespace healthcare