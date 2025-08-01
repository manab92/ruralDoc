#pragma once

#include "BaseEntity.h"
#include <string>
#include <vector>

namespace healthcare::models {

enum class UserRole {
    USER,
    DOCTOR,
    ADMIN
};

enum class Gender {
    MALE,
    FEMALE,
    OTHER,
    PREFER_NOT_TO_SAY
};

class User : public BaseEntity {
public:
    User();
    ~User() override = default;

    // Authentication fields
    const std::string& getEmail() const { return email_; }
    const std::string& getPasswordHash() const { return password_hash_; }
    const std::string& getSalt() const { return salt_; }
    bool isVerified() const { return is_verified_; }
    const std::string& getVerificationToken() const { return verification_token_; }
    const std::string& getFcmToken() const { return fcm_token_; }

    // Profile fields
    const std::string& getFirstName() const { return first_name_; }
    const std::string& getLastName() const { return last_name_; }
    const std::string& getPhoneNumber() const { return phone_number_; }
    UserRole getRole() const { return role_; }
    Gender getGender() const { return gender_; }
    const std::string& getDateOfBirth() const { return date_of_birth_; }
    const std::string& getAddress() const { return address_; }
    const std::string& getCity() const { return city_; }
    const std::string& getState() const { return state_; }
    const std::string& getPincode() const { return pincode_; }
    const std::string& getProfileImageUrl() const { return profile_image_url_; }

    // Setters
    void setEmail(const std::string& email) { email_ = email; }
    void setPasswordHash(const std::string& password_hash) { password_hash_ = password_hash; }
    void setSalt(const std::string& salt) { salt_ = salt; }
    void setVerified(bool verified) { is_verified_ = verified; }
    void setVerificationToken(const std::string& token) { verification_token_ = token; }
    void setFcmToken(const std::string& token) { fcm_token_ = token; }
    void setFirstName(const std::string& first_name) { first_name_ = first_name; }
    void setLastName(const std::string& last_name) { last_name_ = last_name; }
    void setPhoneNumber(const std::string& phone_number) { phone_number_ = phone_number; }
    void setRole(UserRole role) { role_ = role; }
    void setGender(Gender gender) { gender_ = gender; }
    void setDateOfBirth(const std::string& date_of_birth) { date_of_birth_ = date_of_birth; }
    void setAddress(const std::string& address) { address_ = address; }
    void setCity(const std::string& city) { city_ = city; }
    void setState(const std::string& state) { state_ = state; }
    void setPincode(const std::string& pincode) { pincode_ = pincode; }
    void setProfileImageUrl(const std::string& url) { profile_image_url_ = url; }

    // Utility methods
    std::string getFullName() const { return first_name_ + " " + last_name_; }
    bool isDoctor() const { return role_ == UserRole::DOCTOR; }
    bool isAdmin() const { return role_ == UserRole::ADMIN; }
    bool isPatient() const { return role_ == UserRole::USER; }

    // Validation
    bool isValidEmail() const;
    bool isValidPhoneNumber() const;

    // Serialization
    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& json) override;

    // Authentication helpers
    bool verifyPassword(const std::string& password) const;
    void setPassword(const std::string& password);
    void generateVerificationToken();

private:
    // Authentication
    std::string email_;
    std::string password_hash_;
    std::string salt_;
    bool is_verified_;
    std::string verification_token_;
    std::string fcm_token_;

    // Profile
    std::string first_name_;
    std::string last_name_;
    std::string phone_number_;
    UserRole role_;
    Gender gender_;
    std::string date_of_birth_;
    std::string address_;
    std::string city_;
    std::string state_;
    std::string pincode_;
    std::string profile_image_url_;
};

// Utility functions
std::string userRoleToString(UserRole role);
UserRole stringToUserRole(const std::string& role_str);
std::string genderToString(Gender gender);
Gender stringToGender(const std::string& gender_str);

} // namespace healthcare::models