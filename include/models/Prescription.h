#pragma once

#include "BaseEntity.h"
#include <string>
#include <vector>
#include <chrono>

namespace healthcare::models {

enum class PrescriptionStatus {
    ACTIVE,
    COMPLETED,
    CANCELLED,
    EXPIRED
};

enum class MedicineFrequency {
    ONCE_DAILY,
    TWICE_DAILY,
    THREE_TIMES_DAILY,
    FOUR_TIMES_DAILY,
    AS_NEEDED,
    WEEKLY,
    CUSTOM
};

enum class MedicineType {
    TABLET,
    CAPSULE,
    SYRUP,
    INJECTION,
    DROPS,
    CREAM,
    OINTMENT,
    INHALER,
    OTHER
};

struct Medicine {
    std::string id;
    std::string name;
    std::string generic_name;
    std::string brand_name;
    MedicineType type;
    std::string dosage;           // e.g., "5mg", "10ml"
    MedicineFrequency frequency;
    std::string custom_frequency; // For CUSTOM frequency
    int duration_days;
    std::string instructions;     // e.g., "Take after meals"
    std::string timing;          // e.g., "Morning-Evening"
    bool is_before_food;
    bool is_after_food;
    std::string notes;
    double quantity;             // Number of tablets/bottles prescribed
    bool is_substitute_allowed;  // Can pharmacist substitute with generic
};

struct Diagnosis {
    std::string primary_diagnosis;
    std::vector<std::string> secondary_diagnoses;
    std::string icd_code;        // International Classification of Diseases code
    std::string severity;        // MILD, MODERATE, SEVERE
    std::string description;
};

struct VitalSigns {
    double blood_pressure_systolic;
    double blood_pressure_diastolic;
    double heart_rate;
    double temperature;
    double weight;
    double height;
    double oxygen_saturation;
    std::string notes;
};

struct FollowUpInstruction {
    std::chrono::system_clock::time_point follow_up_date;
    std::string reason;
    std::string instructions;
    bool is_urgent;
    std::string specialist_referral;  // If referral needed
};

class Prescription : public BaseEntity {
public:
    Prescription();
    ~Prescription() override = default;

    // Core information
    const std::string& getAppointmentId() const { return appointment_id_; }
    const std::string& getDoctorId() const { return doctor_id_; }
    const std::string& getPatientId() const { return patient_id_; }
    const std::string& getClinicId() const { return clinic_id_; }
    PrescriptionStatus getStatus() const { return status_; }

    // Medical information
    const Diagnosis& getDiagnosis() const { return diagnosis_; }
    const VitalSigns& getVitalSigns() const { return vital_signs_; }
    const std::vector<Medicine>& getMedicines() const { return medicines_; }
    
    // Instructions and notes
    const std::string& getDoctorNotes() const { return doctor_notes_; }
    const std::string& getGeneralInstructions() const { return general_instructions_; }
    const std::string& getDietRecommendations() const { return diet_recommendations_; }
    const std::string& getLifestyleAdvice() const { return lifestyle_advice_; }
    
    // Follow-up information
    const FollowUpInstruction& getFollowUpInstruction() const { return follow_up_instruction_; }
    const std::vector<std::string>& getLabTests() const { return lab_tests_; }
    const std::vector<std::string>& getImagingTests() const { return imaging_tests_; }
    
    // Validity and dates
    const std::chrono::system_clock::time_point& getIssuedDate() const { return issued_date_; }
    const std::chrono::system_clock::time_point& getValidUntil() const { return valid_until_; }
    
    // Document information
    const std::string& getPrescriptionNumber() const { return prescription_number_; }
    const std::string& getDigitalSignature() const { return digital_signature_; }
    const std::string& getQrCode() const { return qr_code_; }
    bool isDigitallyVerified() const { return is_digitally_verified_; }

    // Setters
    void setAppointmentId(const std::string& appointment_id) { appointment_id_ = appointment_id; }
    void setDoctorId(const std::string& doctor_id) { doctor_id_ = doctor_id; }
    void setPatientId(const std::string& patient_id) { patient_id_ = patient_id; }
    void setClinicId(const std::string& clinic_id) { clinic_id_ = clinic_id; }
    void setStatus(PrescriptionStatus status) { status_ = status; updateTimestamp(); }
    void setDiagnosis(const Diagnosis& diagnosis) { diagnosis_ = diagnosis; }
    void setVitalSigns(const VitalSigns& vital_signs) { vital_signs_ = vital_signs; }
    void setMedicines(const std::vector<Medicine>& medicines) { medicines_ = medicines; }
    void setDoctorNotes(const std::string& notes) { doctor_notes_ = notes; }
    void setGeneralInstructions(const std::string& instructions) { general_instructions_ = instructions; }
    void setDietRecommendations(const std::string& diet) { diet_recommendations_ = diet; }
    void setLifestyleAdvice(const std::string& advice) { lifestyle_advice_ = advice; }
    void setFollowUpInstruction(const FollowUpInstruction& instruction) { follow_up_instruction_ = instruction; }
    void setLabTests(const std::vector<std::string>& tests) { lab_tests_ = tests; }
    void setImagingTests(const std::vector<std::string>& tests) { imaging_tests_ = tests; }
    void setIssuedDate(const std::chrono::system_clock::time_point& date) { issued_date_ = date; }
    void setValidUntil(const std::chrono::system_clock::time_point& date) { valid_until_ = date; }
    void setPrescriptionNumber(const std::string& number) { prescription_number_ = number; }
    void setDigitalSignature(const std::string& signature) { digital_signature_ = signature; }
    void setQrCode(const std::string& qr_code) { qr_code_ = qr_code; }
    void setDigitallyVerified(bool verified) { is_digitally_verified_ = verified; }

    // Medicine management
    void addMedicine(const Medicine& medicine);
    void removeMedicine(const std::string& medicine_id);
    void updateMedicine(const std::string& medicine_id, const Medicine& updated_medicine);
    Medicine* findMedicine(const std::string& medicine_id);
    
    // Test management
    void addLabTest(const std::string& test);
    void removeLabTest(const std::string& test);
    void addImagingTest(const std::string& test);
    void removeImagingTest(const std::string& test);

    // Utility methods
    bool isActive() const { return status_ == PrescriptionStatus::ACTIVE; }
    bool isExpired() const;
    bool isValid() const;
    bool requiresFollowUp() const;
    bool hasLabTests() const { return !lab_tests_.empty(); }
    bool hasImagingTests() const { return !imaging_tests_.empty(); }
    bool hasMedicines() const { return !medicines_.empty(); }
    
    int getTotalMedicines() const { return medicines_.size(); }
    int getActiveMedicines() const;
    std::chrono::days getValidityDays() const;
    std::chrono::days getDaysUntilExpiry() const;
    
    // Validation
    bool isValidPrescription() const;
    bool hasDangerousInteractions() const;
    std::vector<std::string> validateMedicines() const;
    
    // Document operations
    void generatePrescriptionNumber();
    void generateQrCode();
    void generateDigitalSignature();
    void markAsCompleted();
    void markAsExpired();
    void extendValidity(int additional_days);

    // Search and filter
    std::vector<Medicine> getMedicinesByType(MedicineType type) const;
    std::vector<Medicine> getMedicinesByFrequency(MedicineFrequency frequency) const;
    bool containsMedicine(const std::string& medicine_name) const;

    // Serialization
    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& json) override;

    // Export formats
    nlohmann::json toPrintableJson() const;  // For printing/PDF generation
    std::string toPlainText() const;         // For simple text format

private:
    // Core information
    std::string appointment_id_;
    std::string doctor_id_;
    std::string patient_id_;
    std::string clinic_id_;
    PrescriptionStatus status_;

    // Medical information
    Diagnosis diagnosis_;
    VitalSigns vital_signs_;
    std::vector<Medicine> medicines_;

    // Instructions and notes
    std::string doctor_notes_;
    std::string general_instructions_;
    std::string diet_recommendations_;
    std::string lifestyle_advice_;

    // Follow-up and tests
    FollowUpInstruction follow_up_instruction_;
    std::vector<std::string> lab_tests_;
    std::vector<std::string> imaging_tests_;

    // Validity and document info
    std::chrono::system_clock::time_point issued_date_;
    std::chrono::system_clock::time_point valid_until_;
    std::string prescription_number_;
    std::string digital_signature_;
    std::string qr_code_;
    bool is_digitally_verified_;

    // Helper methods
    std::string generateUniqueNumber();
    bool checkMedicineInteractions(const Medicine& medicine) const;
};

// Utility functions
std::string prescriptionStatusToString(PrescriptionStatus status);
PrescriptionStatus stringToPrescriptionStatus(const std::string& status_str);
std::string medicineFrequencyToString(MedicineFrequency frequency);
MedicineFrequency stringToMedicineFrequency(const std::string& frequency_str);
std::string medicineTypeToString(MedicineType type);
MedicineType stringToMedicineType(const std::string& type_str);

} // namespace healthcare::models