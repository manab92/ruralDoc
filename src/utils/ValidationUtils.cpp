#include "../../include/utils/ValidationUtils.h"
#include <regex>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>

namespace healthcare::utils {

bool ValidationUtils::isValidEmail(const std::string& email) {
    if (email.empty() || email.length() > 320) { // RFC 5321
        return false;
    }
    
    const std::regex email_pattern(
        R"(^[a-zA-Z0-9.!#$%&'*+/=?^_`{|}~-]+@[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(?:\.[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$)"
    );
    
    return std::regex_match(email, email_pattern);
}

bool ValidationUtils::isValidPhoneNumber(const std::string& phone, const std::string& country_code) {
    if (phone.empty()) {
        return false;
    }
    
    // Remove all non-digit characters for validation
    std::string digits_only;
    std::copy_if(phone.begin(), phone.end(), std::back_inserter(digits_only),
                 [](char c) { return std::isdigit(c); });
    
    if (country_code == "IN" || country_code.empty()) { // Default to India
        // Indian phone numbers: 10 digits, optionally prefixed with 91
        if (digits_only.length() == 10) {
            return digits_only[0] >= '6' && digits_only[0] <= '9';
        } else if (digits_only.length() == 12 && digits_only.substr(0, 2) == "91") {
            return digits_only[2] >= '6' && digits_only[2] <= '9';
        }
    }
    
    // Generic international format validation
    return digits_only.length() >= 7 && digits_only.length() <= 15;
}

bool ValidationUtils::isValidPassword(const std::string& password) {
    if (password.length() < 8 || password.length() > 128) {
        return false;
    }
    
    bool has_upper = false;
    bool has_lower = false;
    bool has_digit = false;
    bool has_special = false;
    
    for (char c : password) {
        if (std::isupper(c)) has_upper = true;
        else if (std::islower(c)) has_lower = true;
        else if (std::isdigit(c)) has_digit = true;
        else if (std::ispunct(c)) has_special = true;
    }
    
    // Require at least 3 out of 4 character types
    int type_count = (has_upper ? 1 : 0) + (has_lower ? 1 : 0) + 
                     (has_digit ? 1 : 0) + (has_special ? 1 : 0);
    
    return type_count >= 3;
}

bool ValidationUtils::isValidName(const std::string& name) {
    if (name.empty() || name.length() > 100) {
        return false;
    }
    
    // Allow letters, spaces, hyphens, and apostrophes
    const std::regex name_pattern(R"(^[a-zA-Z\s\-']+$)");
    return std::regex_match(name, name_pattern);
}

bool ValidationUtils::isValidDate(const std::string& date, const std::string& format) {
    if (date.empty()) {
        return false;
    }
    
    std::istringstream iss(date);
    std::tm tm = {};
    
    if (format == "YYYY-MM-DD") {
        iss >> std::get_time(&tm, "%Y-%m-%d");
    } else if (format == "DD-MM-YYYY") {
        iss >> std::get_time(&tm, "%d-%m-%Y");
    } else if (format == "MM/DD/YYYY") {
        iss >> std::get_time(&tm, "%m/%d/%Y");
    } else {
        return false;
    }
    
    return !iss.fail() && iss.eof();
}

bool ValidationUtils::isValidTime(const std::string& time, bool is_24_hour) {
    if (time.empty()) {
        return false;
    }
    
    std::regex time_pattern;
    if (is_24_hour) {
        time_pattern = std::regex(R"(^([01]?[0-9]|2[0-3]):[0-5][0-9]$)");
    } else {
        time_pattern = std::regex(R"(^(0?[1-9]|1[0-2]):[0-5][0-9]\s?(AM|PM|am|pm)$)");
    }
    
    return std::regex_match(time, time_pattern);
}

bool ValidationUtils::isValidUUID(const std::string& uuid) {
    if (uuid.length() != 36) {
        return false;
    }
    
    const std::regex uuid_pattern(
        R"(^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$)"
    );
    
    return std::regex_match(uuid, uuid_pattern);
}

bool ValidationUtils::isValidPincode(const std::string& pincode, const std::string& country_code) {
    if (pincode.empty()) {
        return false;
    }
    
    if (country_code == "IN" || country_code.empty()) { // Default to India
        // Indian PIN codes: 6 digits, first digit cannot be 0
        const std::regex pincode_pattern(R"(^[1-9][0-9]{5}$)");
        return std::regex_match(pincode, pincode_pattern);
    }
    
    // Generic validation for other countries
    return pincode.length() >= 3 && pincode.length() <= 10;
}

bool ValidationUtils::isValidAge(int age, int min_age, int max_age) {
    return age >= min_age && age <= max_age;
}

bool ValidationUtils::isValidGender(const std::string& gender) {
    std::string lower_gender = gender;
    std::transform(lower_gender.begin(), lower_gender.end(), lower_gender.begin(), ::tolower);
    
    return lower_gender == "male" || lower_gender == "female" || 
           lower_gender == "other" || lower_gender == "prefer_not_to_say";
}

bool ValidationUtils::isValidMedicalLicense(const std::string& license_number) {
    if (license_number.empty() || license_number.length() < 5 || license_number.length() > 50) {
        return false;
    }
    
    // Allow alphanumeric characters and hyphens
    const std::regex license_pattern(R"(^[A-Za-z0-9\-]+$)");
    return std::regex_match(license_number, license_pattern);
}

bool ValidationUtils::isValidAmount(double amount, double min_amount, double max_amount) {
    return amount >= min_amount && amount <= max_amount;
}

bool ValidationUtils::isValidCurrency(const std::string& currency_code) {
    if (currency_code.length() != 3) {
        return false;
    }
    
    // ISO 4217 currency codes are 3 uppercase letters
    return std::all_of(currency_code.begin(), currency_code.end(), ::isupper);
}

bool ValidationUtils::isValidUrl(const std::string& url) {
    if (url.empty() || url.length() > 2048) {
        return false;
    }
    
    const std::regex url_pattern(
        R"(^(https?:\/\/)?([\da-z\.-]+)\.([a-z\.]{2,6})([\/\w \.-]*)*\/?$)"
    );
    
    return std::regex_match(url, url_pattern);
}

bool ValidationUtils::isValidImageUrl(const std::string& url) {
    if (!isValidUrl(url)) {
        return false;
    }
    
    // Check for common image extensions
    std::string lower_url = url;
    std::transform(lower_url.begin(), lower_url.end(), lower_url.begin(), ::tolower);
    
    return lower_url.find(".jpg") != std::string::npos ||
           lower_url.find(".jpeg") != std::string::npos ||
           lower_url.find(".png") != std::string::npos ||
           lower_url.find(".gif") != std::string::npos ||
           lower_url.find(".webp") != std::string::npos ||
           lower_url.find(".svg") != std::string::npos;
}

bool ValidationUtils::isValidBase64(const std::string& base64_string) {
    if (base64_string.empty()) {
        return false;
    }
    
    const std::regex base64_pattern(
        R"(^(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=)?$)"
    );
    
    return std::regex_match(base64_string, base64_pattern);
}

bool ValidationUtils::isFutureDate(const std::string& date) {
    std::tm tm = {};
    std::istringstream iss(date);
    iss >> std::get_time(&tm, "%Y-%m-%d");
    
    if (iss.fail()) {
        return false;
    }
    
    auto date_time = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    auto now = std::chrono::system_clock::now();
    
    return date_time > now;
}

bool ValidationUtils::isPastDate(const std::string& date) {
    std::tm tm = {};
    std::istringstream iss(date);
    iss >> std::get_time(&tm, "%Y-%m-%d");
    
    if (iss.fail()) {
        return false;
    }
    
    auto date_time = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    auto now = std::chrono::system_clock::now();
    
    return date_time < now;
}

bool ValidationUtils::isWithinDateRange(const std::string& date, const std::string& start_date, const std::string& end_date) {
    std::tm date_tm = {}, start_tm = {}, end_tm = {};
    
    std::istringstream date_iss(date), start_iss(start_date), end_iss(end_date);
    date_iss >> std::get_time(&date_tm, "%Y-%m-%d");
    start_iss >> std::get_time(&start_tm, "%Y-%m-%d");
    end_iss >> std::get_time(&end_tm, "%Y-%m-%d");
    
    if (date_iss.fail() || start_iss.fail() || end_iss.fail()) {
        return false;
    }
    
    auto date_time = std::mktime(&date_tm);
    auto start_time = std::mktime(&start_tm);
    auto end_time = std::mktime(&end_tm);
    
    return date_time >= start_time && date_time <= end_time;
}

std::string ValidationUtils::sanitizeInput(const std::string& input) {
    std::string sanitized;
    sanitized.reserve(input.size());
    
    for (char c : input) {
        // Remove control characters except newline and tab
        if ((c >= 32 && c <= 126) || c == '\n' || c == '\t') {
            sanitized += c;
        }
    }
    
    return sanitized;
}

std::string ValidationUtils::sanitizeHtml(const std::string& html) {
    std::string sanitized = html;
    
    // Basic HTML entity encoding
    size_t pos = 0;
    while ((pos = sanitized.find("&", pos)) != std::string::npos) {
        sanitized.replace(pos, 1, "&amp;");
        pos += 5;
    }
    
    pos = 0;
    while ((pos = sanitized.find("<", pos)) != std::string::npos) {
        sanitized.replace(pos, 1, "&lt;");
        pos += 4;
    }
    
    pos = 0;
    while ((pos = sanitized.find(">", pos)) != std::string::npos) {
        sanitized.replace(pos, 1, "&gt;");
        pos += 4;
    }
    
    pos = 0;
    while ((pos = sanitized.find("\"", pos)) != std::string::npos) {
        sanitized.replace(pos, 1, "&quot;");
        pos += 6;
    }
    
    pos = 0;
    while ((pos = sanitized.find("'", pos)) != std::string::npos) {
        sanitized.replace(pos, 1, "&#x27;");
        pos += 6;
    }
    
    return sanitized;
}

std::string ValidationUtils::trimWhitespace(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos) {
        return "";
    }
    
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}

std::string ValidationUtils::normalizePhoneNumber(const std::string& phone) {
    std::string normalized;
    
    // Remove all non-digit characters
    for (char c : phone) {
        if (std::isdigit(c)) {
            normalized += c;
        }
    }
    
    // Add country code if missing (assuming India)
    if (normalized.length() == 10) {
        normalized = "91" + normalized;
    }
    
    return normalized;
}

std::string ValidationUtils::normalizeEmail(const std::string& email) {
    std::string normalized = email;
    
    // Convert to lowercase
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    
    // Trim whitespace
    normalized = trimWhitespace(normalized);
    
    return normalized;
}

std::vector<std::string> ValidationUtils::validateRegistration(const nlohmann::json& data) {
    std::vector<std::string> errors;
    
    // Email validation
    if (!data.contains("email") || !data["email"].is_string()) {
        errors.push_back("Email is required");
    } else if (!isValidEmail(data["email"].get<std::string>())) {
        errors.push_back("Invalid email format");
    }
    
    // Password validation
    if (!data.contains("password") || !data["password"].is_string()) {
        errors.push_back("Password is required");
    } else if (!isValidPassword(data["password"].get<std::string>())) {
        errors.push_back("Password must be at least 8 characters and contain at least 3 of: uppercase, lowercase, digit, special character");
    }
    
    // Name validation
    if (!data.contains("first_name") || !data["first_name"].is_string()) {
        errors.push_back("First name is required");
    } else if (!isValidName(data["first_name"].get<std::string>())) {
        errors.push_back("Invalid first name format");
    }
    
    if (!data.contains("last_name") || !data["last_name"].is_string()) {
        errors.push_back("Last name is required");
    } else if (!isValidName(data["last_name"].get<std::string>())) {
        errors.push_back("Invalid last name format");
    }
    
    // Phone validation (optional)
    if (data.contains("phone_number") && data["phone_number"].is_string() && 
        !data["phone_number"].get<std::string>().empty()) {
        if (!isValidPhoneNumber(data["phone_number"].get<std::string>())) {
            errors.push_back("Invalid phone number format");
        }
    }
    
    // Role validation
    if (data.contains("role") && data["role"].is_string()) {
        std::string role = data["role"].get<std::string>();
        if (role != "USER" && role != "DOCTOR" && role != "ADMIN") {
            errors.push_back("Invalid role");
        }
    }
    
    return errors;
}

std::vector<std::string> ValidationUtils::validateAppointmentBooking(const nlohmann::json& data) {
    std::vector<std::string> errors;
    
    // Doctor ID validation
    if (!data.contains("doctor_id") || !data["doctor_id"].is_string()) {
        errors.push_back("Doctor ID is required");
    } else if (!isValidUUID(data["doctor_id"].get<std::string>())) {
        errors.push_back("Invalid doctor ID format");
    }
    
    // Clinic ID validation
    if (!data.contains("clinic_id") || !data["clinic_id"].is_string()) {
        errors.push_back("Clinic ID is required");
    } else if (!isValidUUID(data["clinic_id"].get<std::string>())) {
        errors.push_back("Invalid clinic ID format");
    }
    
    // Date validation
    if (!data.contains("appointment_date") || !data["appointment_date"].is_string()) {
        errors.push_back("Appointment date is required");
    } else {
        std::string date = data["appointment_date"].get<std::string>();
        if (!isValidDate(date)) {
            errors.push_back("Invalid date format (use YYYY-MM-DD)");
        } else if (!isFutureDate(date)) {
            errors.push_back("Appointment date must be in the future");
        }
    }
    
    // Time validation
    if (!data.contains("start_time") || !data["start_time"].is_string()) {
        errors.push_back("Start time is required");
    } else if (!isValidTime(data["start_time"].get<std::string>(), true)) {
        errors.push_back("Invalid start time format (use HH:MM)");
    }
    
    // Type validation
    if (!data.contains("type") || !data["type"].is_string()) {
        errors.push_back("Appointment type is required");
    } else {
        std::string type = data["type"].get<std::string>();
        if (type != "ONLINE" && type != "OFFLINE") {
            errors.push_back("Invalid appointment type (must be ONLINE or OFFLINE)");
        }
    }
    
    // Symptoms validation (optional but length check)
    if (data.contains("symptoms") && data["symptoms"].is_string()) {
        if (data["symptoms"].get<std::string>().length() > 1000) {
            errors.push_back("Symptoms description too long (max 1000 characters)");
        }
    }
    
    return errors;
}

std::vector<std::string> ValidationUtils::validatePrescription(const nlohmann::json& data) {
    std::vector<std::string> errors;
    
    // Appointment ID validation
    if (!data.contains("appointment_id") || !data["appointment_id"].is_string()) {
        errors.push_back("Appointment ID is required");
    } else if (!isValidUUID(data["appointment_id"].get<std::string>())) {
        errors.push_back("Invalid appointment ID format");
    }
    
    // Diagnosis validation
    if (!data.contains("diagnosis") || !data["diagnosis"].is_object()) {
        errors.push_back("Diagnosis is required");
    } else {
        auto diagnosis = data["diagnosis"];
        if (!diagnosis.contains("primary_diagnosis") || 
            !diagnosis["primary_diagnosis"].is_string() ||
            diagnosis["primary_diagnosis"].get<std::string>().empty()) {
            errors.push_back("Primary diagnosis is required");
        }
    }
    
    // Medicines validation
    if (!data.contains("medicines") || !data["medicines"].is_array()) {
        errors.push_back("Medicines list is required");
    } else {
        auto medicines = data["medicines"];
        if (medicines.empty()) {
            errors.push_back("At least one medicine is required");
        } else {
            for (size_t i = 0; i < medicines.size(); ++i) {
                auto medicine = medicines[i];
                std::string prefix = "Medicine " + std::to_string(i + 1) + ": ";
                
                if (!medicine.contains("name") || !medicine["name"].is_string() ||
                    medicine["name"].get<std::string>().empty()) {
                    errors.push_back(prefix + "Name is required");
                }
                
                if (!medicine.contains("dosage") || !medicine["dosage"].is_string() ||
                    medicine["dosage"].get<std::string>().empty()) {
                    errors.push_back(prefix + "Dosage is required");
                }
                
                if (!medicine.contains("frequency") || !medicine["frequency"].is_string()) {
                    errors.push_back(prefix + "Frequency is required");
                }
                
                if (!medicine.contains("duration_days") || !medicine["duration_days"].is_number_integer() ||
                    medicine["duration_days"].get<int>() <= 0) {
                    errors.push_back(prefix + "Duration must be positive");
                }
            }
        }
    }
    
    return errors;
}

std::vector<std::string> ValidationUtils::validatePayment(const nlohmann::json& data) {
    std::vector<std::string> errors;
    
    // Amount validation
    if (!data.contains("amount") || !data["amount"].is_number()) {
        errors.push_back("Amount is required");
    } else {
        double amount = data["amount"].get<double>();
        if (!isValidAmount(amount, 0.01, 1000000.0)) {
            errors.push_back("Invalid amount");
        }
    }
    
    // Currency validation
    if (!data.contains("currency") || !data["currency"].is_string()) {
        errors.push_back("Currency is required");
    } else if (!isValidCurrency(data["currency"].get<std::string>())) {
        errors.push_back("Invalid currency code");
    }
    
    // Payment method validation
    if (!data.contains("payment_method") || !data["payment_method"].is_string()) {
        errors.push_back("Payment method is required");
    } else {
        std::string method = data["payment_method"].get<std::string>();
        if (method != "CARD" && method != "UPI" && method != "NET_BANKING" && 
            method != "WALLET" && method != "CASH") {
            errors.push_back("Invalid payment method");
        }
    }
    
    // Transaction ID validation (for verification)
    if (data.contains("transaction_id") && data["transaction_id"].is_string()) {
        if (data["transaction_id"].get<std::string>().empty()) {
            errors.push_back("Transaction ID cannot be empty");
        }
    }
    
    return errors;
}

} // namespace healthcare::utils