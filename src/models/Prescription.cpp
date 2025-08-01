#include "../../include/models/Prescription.h"
#include "../../include/utils/CryptoUtils.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <random>

namespace healthcare::models {

Prescription::Prescription() 
    : BaseEntity(),
      status_(PrescriptionStatus::ACTIVE),
      is_digitally_verified_(false) {
    issued_date_ = std::chrono::system_clock::now();
    valid_until_ = issued_date_ + std::chrono::hours(24 * 30); // Valid for 30 days by default
    generatePrescriptionNumber();
}

void Prescription::addMedicine(const Medicine& medicine) {
    medicines_.push_back(medicine);
    updateTimestamp();
}

void Prescription::removeMedicine(const std::string& medicine_id) {
    auto it = std::remove_if(medicines_.begin(), medicines_.end(),
        [&medicine_id](const Medicine& med) { return med.id == medicine_id; });
    
    if (it != medicines_.end()) {
        medicines_.erase(it, medicines_.end());
        updateTimestamp();
    }
}

void Prescription::updateMedicine(const std::string& medicine_id, const Medicine& updated_medicine) {
    auto it = std::find_if(medicines_.begin(), medicines_.end(),
        [&medicine_id](const Medicine& med) { return med.id == medicine_id; });
    
    if (it != medicines_.end()) {
        *it = updated_medicine;
        updateTimestamp();
    }
}

Medicine* Prescription::findMedicine(const std::string& medicine_id) {
    auto it = std::find_if(medicines_.begin(), medicines_.end(),
        [&medicine_id](const Medicine& med) { return med.id == medicine_id; });
    
    return (it != medicines_.end()) ? &(*it) : nullptr;
}

void Prescription::addLabTest(const std::string& test) {
    if (std::find(lab_tests_.begin(), lab_tests_.end(), test) == lab_tests_.end()) {
        lab_tests_.push_back(test);
        updateTimestamp();
    }
}

void Prescription::removeLabTest(const std::string& test) {
    auto it = std::remove(lab_tests_.begin(), lab_tests_.end(), test);
    if (it != lab_tests_.end()) {
        lab_tests_.erase(it, lab_tests_.end());
        updateTimestamp();
    }
}

void Prescription::addImagingTest(const std::string& test) {
    if (std::find(imaging_tests_.begin(), imaging_tests_.end(), test) == imaging_tests_.end()) {
        imaging_tests_.push_back(test);
        updateTimestamp();
    }
}

void Prescription::removeImagingTest(const std::string& test) {
    auto it = std::remove(imaging_tests_.begin(), imaging_tests_.end(), test);
    if (it != imaging_tests_.end()) {
        imaging_tests_.erase(it, imaging_tests_.end());
        updateTimestamp();
    }
}

bool Prescription::isExpired() const {
    return std::chrono::system_clock::now() > valid_until_;
}

bool Prescription::isValid() const {
    return status_ == PrescriptionStatus::ACTIVE && !isExpired() && !medicines_.empty();
}

bool Prescription::requiresFollowUp() const {
    return follow_up_instruction_.follow_up_date.time_since_epoch().count() > 0;
}

int Prescription::getActiveMedicines() const {
    return static_cast<int>(std::count_if(medicines_.begin(), medicines_.end(),
        [](const Medicine& med) { return true; })); // All medicines are considered active
}

std::chrono::days Prescription::getValidityDays() const {
    auto duration = valid_until_ - issued_date_;
    return std::chrono::duration_cast<std::chrono::days>(duration);
}

std::chrono::days Prescription::getDaysUntilExpiry() const {
    auto now = std::chrono::system_clock::now();
    if (now >= valid_until_) {
        return std::chrono::days(0);
    }
    auto duration = valid_until_ - now;
    return std::chrono::duration_cast<std::chrono::days>(duration);
}

bool Prescription::isValidPrescription() const {
    // Check if prescription has required fields
    if (doctor_id_.empty() || patient_id_.empty() || medicines_.empty()) {
        return false;
    }
    
    // Check if diagnosis is provided
    if (diagnosis_.primary_diagnosis.empty()) {
        return false;
    }
    
    // Check if all medicines have proper dosage and frequency
    for (const auto& medicine : medicines_) {
        if (medicine.name.empty() || medicine.dosage.empty() || 
            medicine.frequency == MedicineFrequency::CUSTOM && medicine.custom_frequency.empty()) {
            return false;
        }
    }
    
    return true;
}

bool Prescription::hasDangerousInteractions() const {
    // This would typically check a drug interaction database
    // For now, return false (no interactions detected)
    return false;
}

std::vector<std::string> Prescription::validateMedicines() const {
    std::vector<std::string> errors;
    
    for (const auto& medicine : medicines_) {
        if (medicine.name.empty()) {
            errors.push_back("Medicine name is required");
        }
        if (medicine.dosage.empty()) {
            errors.push_back("Dosage is required for " + medicine.name);
        }
        if (medicine.duration_days <= 0) {
            errors.push_back("Duration must be positive for " + medicine.name);
        }
        if (medicine.quantity <= 0) {
            errors.push_back("Quantity must be positive for " + medicine.name);
        }
    }
    
    return errors;
}

void Prescription::generatePrescriptionNumber() {
    std::ostringstream oss;
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time_t);
    
    oss << "RX" 
        << std::setfill('0') << std::setw(4) << (tm->tm_year + 1900)
        << std::setw(2) << (tm->tm_mon + 1)
        << std::setw(2) << tm->tm_mday
        << "-" << generateUniqueNumber();
    
    prescription_number_ = oss.str();
}

void Prescription::generateQrCode() {
    // Generate QR code data containing prescription information
    nlohmann::json qr_data;
    qr_data["prescription_id"] = getId();
    qr_data["prescription_number"] = prescription_number_;
    qr_data["patient_id"] = patient_id_;
    qr_data["doctor_id"] = doctor_id_;
    qr_data["issued_date"] = std::chrono::system_clock::to_time_t(issued_date_);
    qr_data["valid_until"] = std::chrono::system_clock::to_time_t(valid_until_);
    
    // In a real implementation, this would generate an actual QR code image
    qr_code_ = utils::CryptoUtils::base64Encode(qr_data.dump());
}

void Prescription::generateDigitalSignature() {
    // Generate digital signature for the prescription
    nlohmann::json sign_data;
    sign_data["prescription_id"] = getId();
    sign_data["prescription_number"] = prescription_number_;
    sign_data["doctor_id"] = doctor_id_;
    sign_data["patient_id"] = patient_id_;
    sign_data["medicines"] = nlohmann::json::array();
    
    for (const auto& medicine : medicines_) {
        nlohmann::json med;
        med["name"] = medicine.name;
        med["dosage"] = medicine.dosage;
        med["frequency"] = medicineFrequencyToString(medicine.frequency);
        med["duration_days"] = medicine.duration_days;
        sign_data["medicines"].push_back(med);
    }
    
    // In a real implementation, this would use actual digital signing
    digital_signature_ = utils::CryptoUtils::sha256(sign_data.dump());
    is_digitally_verified_ = true;
}

void Prescription::markAsCompleted() {
    status_ = PrescriptionStatus::COMPLETED;
    updateTimestamp();
}

void Prescription::markAsExpired() {
    status_ = PrescriptionStatus::EXPIRED;
    updateTimestamp();
}

void Prescription::extendValidity(int additional_days) {
    valid_until_ += std::chrono::hours(24 * additional_days);
    updateTimestamp();
}

std::vector<Medicine> Prescription::getMedicinesByType(MedicineType type) const {
    std::vector<Medicine> filtered;
    std::copy_if(medicines_.begin(), medicines_.end(), std::back_inserter(filtered),
        [type](const Medicine& med) { return med.type == type; });
    return filtered;
}

std::vector<Medicine> Prescription::getMedicinesByFrequency(MedicineFrequency frequency) const {
    std::vector<Medicine> filtered;
    std::copy_if(medicines_.begin(), medicines_.end(), std::back_inserter(filtered),
        [frequency](const Medicine& med) { return med.frequency == frequency; });
    return filtered;
}

bool Prescription::containsMedicine(const std::string& medicine_name) const {
    return std::any_of(medicines_.begin(), medicines_.end(),
        [&medicine_name](const Medicine& med) { 
            return med.name == medicine_name || med.generic_name == medicine_name;
        });
}

nlohmann::json Prescription::toJson() const {
    nlohmann::json json;
    
    // Base entity fields
    json["id"] = getId();
    json["created_at"] = std::chrono::system_clock::to_time_t(getCreatedAt());
    json["updated_at"] = std::chrono::system_clock::to_time_t(getUpdatedAt());
    json["is_deleted"] = isDeleted();
    
    // Core prescription fields
    json["appointment_id"] = appointment_id_;
    json["doctor_id"] = doctor_id_;
    json["patient_id"] = patient_id_;
    json["clinic_id"] = clinic_id_;
    json["status"] = prescriptionStatusToString(status_);
    
    // Diagnosis
    nlohmann::json diagnosis_json;
    diagnosis_json["primary_diagnosis"] = diagnosis_.primary_diagnosis;
    diagnosis_json["secondary_diagnoses"] = diagnosis_.secondary_diagnoses;
    diagnosis_json["icd_code"] = diagnosis_.icd_code;
    diagnosis_json["severity"] = diagnosis_.severity;
    diagnosis_json["description"] = diagnosis_.description;
    json["diagnosis"] = diagnosis_json;
    
    // Vital signs
    nlohmann::json vitals_json;
    vitals_json["blood_pressure_systolic"] = vital_signs_.blood_pressure_systolic;
    vitals_json["blood_pressure_diastolic"] = vital_signs_.blood_pressure_diastolic;
    vitals_json["heart_rate"] = vital_signs_.heart_rate;
    vitals_json["temperature"] = vital_signs_.temperature;
    vitals_json["weight"] = vital_signs_.weight;
    vitals_json["height"] = vital_signs_.height;
    vitals_json["oxygen_saturation"] = vital_signs_.oxygen_saturation;
    vitals_json["notes"] = vital_signs_.notes;
    json["vital_signs"] = vitals_json;
    
    // Medicines
    nlohmann::json medicines_json = nlohmann::json::array();
    for (const auto& medicine : medicines_) {
        nlohmann::json med_json;
        med_json["id"] = medicine.id;
        med_json["name"] = medicine.name;
        med_json["generic_name"] = medicine.generic_name;
        med_json["brand_name"] = medicine.brand_name;
        med_json["type"] = medicineTypeToString(medicine.type);
        med_json["dosage"] = medicine.dosage;
        med_json["frequency"] = medicineFrequencyToString(medicine.frequency);
        med_json["custom_frequency"] = medicine.custom_frequency;
        med_json["duration_days"] = medicine.duration_days;
        med_json["instructions"] = medicine.instructions;
        med_json["timing"] = medicine.timing;
        med_json["is_before_food"] = medicine.is_before_food;
        med_json["is_after_food"] = medicine.is_after_food;
        med_json["notes"] = medicine.notes;
        med_json["quantity"] = medicine.quantity;
        med_json["is_substitute_allowed"] = medicine.is_substitute_allowed;
        medicines_json.push_back(med_json);
    }
    json["medicines"] = medicines_json;
    
    // Instructions and recommendations
    json["doctor_notes"] = doctor_notes_;
    json["general_instructions"] = general_instructions_;
    json["diet_recommendations"] = diet_recommendations_;
    json["lifestyle_advice"] = lifestyle_advice_;
    
    // Follow-up instruction
    nlohmann::json followup_json;
    followup_json["follow_up_date"] = std::chrono::system_clock::to_time_t(follow_up_instruction_.follow_up_date);
    followup_json["reason"] = follow_up_instruction_.reason;
    followup_json["instructions"] = follow_up_instruction_.instructions;
    followup_json["is_urgent"] = follow_up_instruction_.is_urgent;
    followup_json["specialist_referral"] = follow_up_instruction_.specialist_referral;
    json["follow_up_instruction"] = followup_json;
    
    // Tests
    json["lab_tests"] = lab_tests_;
    json["imaging_tests"] = imaging_tests_;
    
    // Validity and document info
    json["issued_date"] = std::chrono::system_clock::to_time_t(issued_date_);
    json["valid_until"] = std::chrono::system_clock::to_time_t(valid_until_);
    json["prescription_number"] = prescription_number_;
    json["digital_signature"] = digital_signature_;
    json["qr_code"] = qr_code_;
    json["is_digitally_verified"] = is_digitally_verified_;
    
    return json;
}

void Prescription::fromJson(const nlohmann::json& json) {
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
    
    // Core prescription fields
    if (json.contains("appointment_id")) appointment_id_ = json["appointment_id"].get<std::string>();
    if (json.contains("doctor_id")) doctor_id_ = json["doctor_id"].get<std::string>();
    if (json.contains("patient_id")) patient_id_ = json["patient_id"].get<std::string>();
    if (json.contains("clinic_id")) clinic_id_ = json["clinic_id"].get<std::string>();
    if (json.contains("status")) status_ = stringToPrescriptionStatus(json["status"].get<std::string>());
    
    // Diagnosis
    if (json.contains("diagnosis")) {
        auto diag_json = json["diagnosis"];
        diagnosis_.primary_diagnosis = diag_json.value("primary_diagnosis", "");
        diagnosis_.secondary_diagnoses = diag_json.value("secondary_diagnoses", std::vector<std::string>{});
        diagnosis_.icd_code = diag_json.value("icd_code", "");
        diagnosis_.severity = diag_json.value("severity", "");
        diagnosis_.description = diag_json.value("description", "");
    }
    
    // Vital signs
    if (json.contains("vital_signs")) {
        auto vitals_json = json["vital_signs"];
        vital_signs_.blood_pressure_systolic = vitals_json.value("blood_pressure_systolic", 0.0);
        vital_signs_.blood_pressure_diastolic = vitals_json.value("blood_pressure_diastolic", 0.0);
        vital_signs_.heart_rate = vitals_json.value("heart_rate", 0.0);
        vital_signs_.temperature = vitals_json.value("temperature", 0.0);
        vital_signs_.weight = vitals_json.value("weight", 0.0);
        vital_signs_.height = vitals_json.value("height", 0.0);
        vital_signs_.oxygen_saturation = vitals_json.value("oxygen_saturation", 0.0);
        vital_signs_.notes = vitals_json.value("notes", "");
    }
    
    // Medicines
    if (json.contains("medicines")) {
        medicines_.clear();
        for (const auto& med_json : json["medicines"]) {
            Medicine medicine;
            medicine.id = med_json.value("id", "");
            medicine.name = med_json.value("name", "");
            medicine.generic_name = med_json.value("generic_name", "");
            medicine.brand_name = med_json.value("brand_name", "");
            medicine.type = stringToMedicineType(med_json.value("type", "TABLET"));
            medicine.dosage = med_json.value("dosage", "");
            medicine.frequency = stringToMedicineFrequency(med_json.value("frequency", "ONCE_DAILY"));
            medicine.custom_frequency = med_json.value("custom_frequency", "");
            medicine.duration_days = med_json.value("duration_days", 0);
            medicine.instructions = med_json.value("instructions", "");
            medicine.timing = med_json.value("timing", "");
            medicine.is_before_food = med_json.value("is_before_food", false);
            medicine.is_after_food = med_json.value("is_after_food", false);
            medicine.notes = med_json.value("notes", "");
            medicine.quantity = med_json.value("quantity", 0.0);
            medicine.is_substitute_allowed = med_json.value("is_substitute_allowed", true);
            medicines_.push_back(medicine);
        }
    }
    
    // Instructions and recommendations
    if (json.contains("doctor_notes")) doctor_notes_ = json["doctor_notes"].get<std::string>();
    if (json.contains("general_instructions")) general_instructions_ = json["general_instructions"].get<std::string>();
    if (json.contains("diet_recommendations")) diet_recommendations_ = json["diet_recommendations"].get<std::string>();
    if (json.contains("lifestyle_advice")) lifestyle_advice_ = json["lifestyle_advice"].get<std::string>();
    
    // Follow-up instruction
    if (json.contains("follow_up_instruction")) {
        auto followup_json = json["follow_up_instruction"];
        if (followup_json.contains("follow_up_date")) {
            follow_up_instruction_.follow_up_date = std::chrono::system_clock::from_time_t(
                followup_json["follow_up_date"].get<std::time_t>()
            );
        }
        follow_up_instruction_.reason = followup_json.value("reason", "");
        follow_up_instruction_.instructions = followup_json.value("instructions", "");
        follow_up_instruction_.is_urgent = followup_json.value("is_urgent", false);
        follow_up_instruction_.specialist_referral = followup_json.value("specialist_referral", "");
    }
    
    // Tests
    if (json.contains("lab_tests")) lab_tests_ = json["lab_tests"].get<std::vector<std::string>>();
    if (json.contains("imaging_tests")) imaging_tests_ = json["imaging_tests"].get<std::vector<std::string>>();
    
    // Validity and document info
    if (json.contains("issued_date")) {
        issued_date_ = std::chrono::system_clock::from_time_t(json["issued_date"].get<std::time_t>());
    }
    if (json.contains("valid_until")) {
        valid_until_ = std::chrono::system_clock::from_time_t(json["valid_until"].get<std::time_t>());
    }
    if (json.contains("prescription_number")) prescription_number_ = json["prescription_number"].get<std::string>();
    if (json.contains("digital_signature")) digital_signature_ = json["digital_signature"].get<std::string>();
    if (json.contains("qr_code")) qr_code_ = json["qr_code"].get<std::string>();
    if (json.contains("is_digitally_verified")) is_digitally_verified_ = json["is_digitally_verified"].get<bool>();
}

nlohmann::json Prescription::toPrintableJson() const {
    nlohmann::json json = toJson();
    
    // Add formatted dates for printing
    auto format_time = [](const std::chrono::system_clock::time_point& time) {
        auto time_t = std::chrono::system_clock::to_time_t(time);
        std::tm* tm = std::localtime(&time_t);
        std::ostringstream oss;
        oss << std::put_time(tm, "%d-%m-%Y %H:%M");
        return oss.str();
    };
    
    json["issued_date_formatted"] = format_time(issued_date_);
    json["valid_until_formatted"] = format_time(valid_until_);
    
    return json;
}

std::string Prescription::toPlainText() const {
    std::ostringstream oss;
    
    oss << "PRESCRIPTION\n";
    oss << "============\n\n";
    oss << "Prescription Number: " << prescription_number_ << "\n";
    oss << "Date: " << std::chrono::system_clock::to_time_t(issued_date_) << "\n\n";
    
    oss << "DIAGNOSIS:\n";
    oss << diagnosis_.primary_diagnosis << "\n\n";
    
    oss << "MEDICINES:\n";
    for (const auto& medicine : medicines_) {
        oss << "- " << medicine.name << " " << medicine.dosage << "\n";
        oss << "  " << medicineFrequencyToString(medicine.frequency) << " for " << medicine.duration_days << " days\n";
        if (!medicine.instructions.empty()) {
            oss << "  Instructions: " << medicine.instructions << "\n";
        }
        oss << "\n";
    }
    
    if (!lab_tests_.empty()) {
        oss << "LAB TESTS:\n";
        for (const auto& test : lab_tests_) {
            oss << "- " << test << "\n";
        }
        oss << "\n";
    }
    
    if (!general_instructions_.empty()) {
        oss << "INSTRUCTIONS:\n";
        oss << general_instructions_ << "\n\n";
    }
    
    return oss.str();
}

std::string Prescription::generateUniqueNumber() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(10000, 99999);
    return std::to_string(dis(gen));
}

bool Prescription::checkMedicineInteractions(const Medicine& medicine) const {
    // This would check against a drug interaction database
    // For now, return false (no interactions)
    return false;
}

// Utility functions
std::string prescriptionStatusToString(PrescriptionStatus status) {
    switch (status) {
        case PrescriptionStatus::ACTIVE: return "ACTIVE";
        case PrescriptionStatus::COMPLETED: return "COMPLETED";
        case PrescriptionStatus::CANCELLED: return "CANCELLED";
        case PrescriptionStatus::EXPIRED: return "EXPIRED";
        default: return "ACTIVE";
    }
}

PrescriptionStatus stringToPrescriptionStatus(const std::string& status_str) {
    if (status_str == "COMPLETED") return PrescriptionStatus::COMPLETED;
    if (status_str == "CANCELLED") return PrescriptionStatus::CANCELLED;
    if (status_str == "EXPIRED") return PrescriptionStatus::EXPIRED;
    return PrescriptionStatus::ACTIVE;
}

std::string medicineFrequencyToString(MedicineFrequency frequency) {
    switch (frequency) {
        case MedicineFrequency::ONCE_DAILY: return "ONCE_DAILY";
        case MedicineFrequency::TWICE_DAILY: return "TWICE_DAILY";
        case MedicineFrequency::THREE_TIMES_DAILY: return "THREE_TIMES_DAILY";
        case MedicineFrequency::FOUR_TIMES_DAILY: return "FOUR_TIMES_DAILY";
        case MedicineFrequency::AS_NEEDED: return "AS_NEEDED";
        case MedicineFrequency::WEEKLY: return "WEEKLY";
        case MedicineFrequency::CUSTOM: return "CUSTOM";
        default: return "ONCE_DAILY";
    }
}

MedicineFrequency stringToMedicineFrequency(const std::string& frequency_str) {
    if (frequency_str == "TWICE_DAILY") return MedicineFrequency::TWICE_DAILY;
    if (frequency_str == "THREE_TIMES_DAILY") return MedicineFrequency::THREE_TIMES_DAILY;
    if (frequency_str == "FOUR_TIMES_DAILY") return MedicineFrequency::FOUR_TIMES_DAILY;
    if (frequency_str == "AS_NEEDED") return MedicineFrequency::AS_NEEDED;
    if (frequency_str == "WEEKLY") return MedicineFrequency::WEEKLY;
    if (frequency_str == "CUSTOM") return MedicineFrequency::CUSTOM;
    return MedicineFrequency::ONCE_DAILY;
}

std::string medicineTypeToString(MedicineType type) {
    switch (type) {
        case MedicineType::TABLET: return "TABLET";
        case MedicineType::CAPSULE: return "CAPSULE";
        case MedicineType::SYRUP: return "SYRUP";
        case MedicineType::INJECTION: return "INJECTION";
        case MedicineType::DROPS: return "DROPS";
        case MedicineType::CREAM: return "CREAM";
        case MedicineType::OINTMENT: return "OINTMENT";
        case MedicineType::INHALER: return "INHALER";
        case MedicineType::OTHER: return "OTHER";
        default: return "TABLET";
    }
}

MedicineType stringToMedicineType(const std::string& type_str) {
    if (type_str == "CAPSULE") return MedicineType::CAPSULE;
    if (type_str == "SYRUP") return MedicineType::SYRUP;
    if (type_str == "INJECTION") return MedicineType::INJECTION;
    if (type_str == "DROPS") return MedicineType::DROPS;
    if (type_str == "CREAM") return MedicineType::CREAM;
    if (type_str == "OINTMENT") return MedicineType::OINTMENT;
    if (type_str == "INHALER") return MedicineType::INHALER;
    if (type_str == "OTHER") return MedicineType::OTHER;
    return MedicineType::TABLET;
}

} // namespace healthcare::models