#pragma once

#include <string>
#include <vector>
#include <regex>
#include <chrono>
#include <nlohmann/json.hpp>

namespace healthcare::utils {

struct ValidationResult {
    bool is_valid;
    std::vector<std::string> errors;
    
    ValidationResult(bool valid = true) : is_valid(valid) {}
    
    void addError(const std::string& error) {
        is_valid = false;
        errors.push_back(error);
    }
    
    bool hasErrors() const { return !errors.empty(); }
    std::string getFirstError() const { return errors.empty() ? "" : errors[0]; }
    std::string getAllErrors(const std::string& separator = "; ") const;
};

class ValidationUtils {
public:
    // Basic validation
    static bool isEmailValid(const std::string& email);
    static bool isPhoneNumberValid(const std::string& phone);
    static bool isPasswordValid(const std::string& password);
    static bool isUUIDValid(const std::string& uuid);
    static bool isNumericString(const std::string& str);
    static bool isAlphaString(const std::string& str);
    static bool isAlphaNumericString(const std::string& str);
    
    // Date and time validation
    static bool isDateValid(const std::string& date, const std::string& format = "YYYY-MM-DD");
    static bool isTimeValid(const std::string& time, const std::string& format = "HH:MM");
    static bool isDateTimeValid(const std::string& datetime, const std::string& format = "YYYY-MM-DD HH:MM:SS");
    static bool isFutureDate(const std::chrono::system_clock::time_point& date);
    static bool isPastDate(const std::chrono::system_clock::time_point& date);
    static bool isDateInRange(const std::chrono::system_clock::time_point& date,
                             const std::chrono::system_clock::time_point& start,
                             const std::chrono::system_clock::time_point& end);
    
    // Medical validation
    static bool isValidAge(int age);
    static bool isValidWeight(double weight_kg);
    static bool isValidHeight(double height_cm);
    static bool isValidBloodPressure(int systolic, int diastolic);
    static bool isValidHeartRate(int bpm);
    static bool isValidTemperature(double celsius);
    static bool isValidMedicineFrequency(const std::string& frequency);
    static bool isValidDosage(const std::string& dosage);
    
    // Financial validation
    static bool isValidAmount(double amount);
    static bool isValidCurrency(const std::string& currency);
    static bool isValidPaymentMethod(const std::string& method);
    
    // Geographic validation
    static bool isValidPincode(const std::string& pincode);
    static bool isValidLatitude(double latitude);
    static bool isValidLongitude(double longitude);
    static bool isValidCoordinates(double latitude, double longitude);
    
    // Text validation
    static bool isValidName(const std::string& name);
    static bool isValidDescription(const std::string& description, int max_length = 1000);
    static bool isValidUrl(const std::string& url);
    static bool isValidFilePath(const std::string& path);
    
    // Comprehensive validation methods
    static ValidationResult validateUserRegistration(const nlohmann::json& user_data);
    static ValidationResult validateDoctorProfile(const nlohmann::json& doctor_data);
    static ValidationResult validateAppointmentBooking(const nlohmann::json& booking_data);
    static ValidationResult validatePrescription(const nlohmann::json& prescription_data);
    static ValidationResult validateClinicInfo(const nlohmann::json& clinic_data);
    static ValidationResult validatePaymentInfo(const nlohmann::json& payment_data);
    
    // JSON validation
    static ValidationResult validateJsonStructure(const nlohmann::json& data,
                                                 const nlohmann::json& schema);
    static bool hasRequiredFields(const nlohmann::json& data,
                                 const std::vector<std::string>& required_fields);
    static ValidationResult validateFieldTypes(const nlohmann::json& data,
                                             const std::map<std::string, std::string>& field_types);
    
    // Security validation
    static bool isSafeString(const std::string& input);  // XSS protection
    static bool isSqlSafe(const std::string& input);     // SQL injection protection
    static bool isValidSessionToken(const std::string& token);
    static bool isValidApiKey(const std::string& api_key);
    
    // File validation
    static bool isValidImageFile(const std::string& filename);
    static bool isValidDocumentFile(const std::string& filename);
    static bool isValidFileSize(size_t file_size, size_t max_size_mb = 10);
    
    // Advanced validation
    static ValidationResult validateAppointmentTimeSlot(
        const std::chrono::system_clock::time_point& start_time,
        const std::chrono::system_clock::time_point& end_time,
        const std::vector<std::chrono::system_clock::time_point>& existing_appointments = {}
    );
    
    static ValidationResult validateMedicineInteractions(
        const std::vector<std::string>& medicine_names
    );
    
    static ValidationResult validateDoctorAvailability(
        const std::string& doctor_id,
        const std::chrono::system_clock::time_point& requested_time
    );
    
    // Utility methods
    static std::string sanitizeInput(const std::string& input);
    static std::string normalizePhoneNumber(const std::string& phone);
    static std::string normalizeEmail(const std::string& email);
    static std::string extractNumericOnly(const std::string& input);
    static std::string generateValidationId();
    
    // Configuration
    static void setMinPasswordLength(int length) { min_password_length_ = length; }
    static void setMaxPasswordLength(int length) { max_password_length_ = length; }
    static void setRequirePasswordSpecialChars(bool require) { require_password_special_chars_ = require; }
    static void setValidPhoneCountries(const std::vector<std::string>& countries) { valid_phone_countries_ = countries; }

private:
    // Configuration variables
    static int min_password_length_;
    static int max_password_length_;
    static bool require_password_special_chars_;
    static std::vector<std::string> valid_phone_countries_;
    
    // Helper methods
    static bool matchesRegex(const std::string& input, const std::string& pattern);
    static bool isInRange(double value, double min, double max);
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::string trim(const std::string& str);
    static std::string toLowerCase(const std::string& str);
    
    // Validation patterns
    static const std::regex email_pattern_;
    static const std::regex phone_pattern_;
    static const std::regex uuid_pattern_;
    static const std::regex url_pattern_;
    static const std::regex name_pattern_;
    static const std::regex pincode_pattern_;
    static const std::regex date_pattern_;
    static const std::regex time_pattern_;
    static const std::regex datetime_pattern_;
};

// Convenience macros for validation
#define VALIDATE_EMAIL(email) healthcare::utils::ValidationUtils::isEmailValid(email)
#define VALIDATE_PHONE(phone) healthcare::utils::ValidationUtils::isPhoneNumberValid(phone)
#define VALIDATE_PASSWORD(password) healthcare::utils::ValidationUtils::isPasswordValid(password)
#define VALIDATE_UUID(uuid) healthcare::utils::ValidationUtils::isUUIDValid(uuid)

#define VALIDATE_REQUIRED_FIELDS(data, fields) \
    healthcare::utils::ValidationUtils::hasRequiredFields(data, fields)

#define SANITIZE_INPUT(input) healthcare::utils::ValidationUtils::sanitizeInput(input)

} // namespace healthcare::utils