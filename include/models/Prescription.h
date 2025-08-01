#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include "BaseEntity.h"

namespace healthcare {
namespace models {

enum class PrescriptionStatus {
    DRAFT = 1,
    ISSUED = 2,
    DISPENSED = 3,
    CANCELLED = 4
};

enum class MedicineType {
    TABLET = 1,
    CAPSULE = 2,
    SYRUP = 3,
    INJECTION = 4,
    CREAM = 5,
    DROPS = 6,
    OTHER = 7
};

struct Medicine {
    std::string name;
    std::string generic_name;
    std::string dosage;
    std::string frequency;  // e.g., "2 times daily"
    int duration_days;
    std::string instructions;  // e.g., "Take after meals"
    MedicineType type;
    int quantity;
    bool is_substitutable;
};

struct VitalSigns {
    double temperature;  // Celsius
    int blood_pressure_systolic;
    int blood_pressure_diastolic;
    int heart_rate;  // bpm
    double weight;  // kg
    double height;  // cm
    std::string additional_notes;
};

class Prescription : public BaseEntity {
private:
    std::string appointment_id_;
    std::string doctor_id_;
    std::string user_id_;
    std::string clinic_id_;
    std::vector<Medicine> medicines_;
    std::string diagnosis_;
    std::string symptoms_;
    std::string treatment_advice_;
    std::string follow_up_instructions_;
    std::chrono::system_clock::time_point follow_up_date_;
    VitalSigns vital_signs_;
    PrescriptionStatus status_;
    std::string prescription_file_url_;  // PDF file URL
    std::string prescription_number_;  // Unique prescription number
    std::string digital_signature_;
    bool is_digital_;
    std::string pharmacy_id_;  // If dispensed from specific pharmacy
    std::chrono::system_clock::time_point dispensed_at_;
    std::string dispensed_by_;  // Pharmacist ID
    std::string lab_tests_recommended_;
    std::string allergies_noted_;
    std::string emergency_contact_info_;

public:
    // Constructors
    Prescription();
    Prescription(const std::string& appointment_id, const std::string& doctor_id,
                 const std::string& user_id, const std::string& diagnosis);

    // Getters
    const std::string& getAppointmentId() const { return appointment_id_; }
    const std::string& getDoctorId() const { return doctor_id_; }
    const std::string& getUserId() const { return user_id_; }
    const std::string& getClinicId() const { return clinic_id_; }
    const std::vector<Medicine>& getMedicines() const { return medicines_; }
    const std::string& getDiagnosis() const { return diagnosis_; }
    const std::string& getSymptoms() const { return symptoms_; }
    const std::string& getTreatmentAdvice() const { return treatment_advice_; }
    const std::string& getFollowUpInstructions() const { return follow_up_instructions_; }
    const std::chrono::system_clock::time_point& getFollowUpDate() const { return follow_up_date_; }
    const VitalSigns& getVitalSigns() const { return vital_signs_; }
    PrescriptionStatus getStatus() const { return status_; }
    const std::string& getPrescriptionFileUrl() const { return prescription_file_url_; }
    const std::string& getPrescriptionNumber() const { return prescription_number_; }
    const std::string& getDigitalSignature() const { return digital_signature_; }
    bool isDigital() const { return is_digital_; }
    const std::string& getPharmacyId() const { return pharmacy_id_; }
    const std::chrono::system_clock::time_point& getDispensedAt() const { return dispensed_at_; }
    const std::string& getDispensedBy() const { return dispensed_by_; }
    const std::string& getLabTestsRecommended() const { return lab_tests_recommended_; }
    const std::string& getAllergiesNoted() const { return allergies_noted_; }
    const std::string& getEmergencyContactInfo() const { return emergency_contact_info_; }

    // Setters
    void setAppointmentId(const std::string& appointment_id) { appointment_id_ = appointment_id; }
    void setDoctorId(const std::string& doctor_id) { doctor_id_ = doctor_id; }
    void setUserId(const std::string& user_id) { user_id_ = user_id; }
    void setClinicId(const std::string& clinic_id) { clinic_id_ = clinic_id; }
    void setMedicines(const std::vector<Medicine>& medicines) { medicines_ = medicines; }
    void setDiagnosis(const std::string& diagnosis) { diagnosis_ = diagnosis; }
    void setSymptoms(const std::string& symptoms) { symptoms_ = symptoms; }
    void setTreatmentAdvice(const std::string& treatment_advice) { treatment_advice_ = treatment_advice; }
    void setFollowUpInstructions(const std::string& follow_up_instructions) { follow_up_instructions_ = follow_up_instructions; }
    void setFollowUpDate(const std::chrono::system_clock::time_point& follow_up_date) { follow_up_date_ = follow_up_date; }
    void setVitalSigns(const VitalSigns& vital_signs) { vital_signs_ = vital_signs; }
    void setStatus(PrescriptionStatus status) { status_ = status; }
    void setPrescriptionFileUrl(const std::string& prescription_file_url) { prescription_file_url_ = prescription_file_url; }
    void setPrescriptionNumber(const std::string& prescription_number) { prescription_number_ = prescription_number; }
    void setDigitalSignature(const std::string& digital_signature) { digital_signature_ = digital_signature; }
    void setDigital(bool is_digital) { is_digital_ = is_digital; }
    void setPharmacyId(const std::string& pharmacy_id) { pharmacy_id_ = pharmacy_id; }
    void setDispensedAt(const std::chrono::system_clock::time_point& dispensed_at) { dispensed_at_ = dispensed_at; }
    void setDispensedBy(const std::string& dispensed_by) { dispensed_by_ = dispensed_by; }
    void setLabTestsRecommended(const std::string& lab_tests_recommended) { lab_tests_recommended_ = lab_tests_recommended; }
    void setAllergiesNoted(const std::string& allergies_noted) { allergies_noted_ = allergies_noted; }
    void setEmergencyContactInfo(const std::string& emergency_contact_info) { emergency_contact_info_ = emergency_contact_info; }

    // Medicine management
    void addMedicine(const Medicine& medicine);
    void removeMedicine(const std::string& medicine_name);
    void updateMedicine(const std::string& medicine_name, const Medicine& updated_medicine);

    // Status management
    void issuePrescription();
    void markAsDispensed(const std::string& pharmacy_id, const std::string& dispensed_by);
    void cancelPrescription();

    // Utility methods
    std::string generatePrescriptionNumber();
    void generateDigitalSignature(const std::string& doctor_private_key);
    bool verifyDigitalSignature(const std::string& doctor_public_key) const;
    bool needsFollowUp() const;
    bool isExpired() const;  // Check if prescription is too old to be valid
    int getTotalMedicineCount() const;
    std::vector<std::string> getMedicineNames() const;

    // PDF generation
    std::string generatePrescriptionPDF() const;
    bool uploadPrescriptionFile(const std::string& file_path);

    // Validation
    bool isValidPrescription() const;
    bool canBeDispensed() const;
    bool hasDangerousInteractions() const;

    // Serialization
    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& json) override;
};

} // namespace models
} // namespace healthcare