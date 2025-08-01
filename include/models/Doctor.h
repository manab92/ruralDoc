#pragma once

#include "BaseEntity.h"
#include "User.h"
#include <string>
#include <vector>
#include <chrono>

namespace healthcare::models {

enum class DoctorStatus {
    PENDING_VERIFICATION,
    VERIFIED,
    SUSPENDED,
    INACTIVE
};

enum class ConsultationType {
    ONLINE,
    OFFLINE,
    BOTH
};

struct Specialization {
    std::string id;
    std::string name;
    std::string description;
    std::string category;
};

struct DoctorDocument {
    std::string id;
    std::string type;  // medical_license, degree_certificate, etc.
    std::string url;
    bool is_verified;
    std::chrono::system_clock::time_point uploaded_at;
    std::chrono::system_clock::time_point verified_at;
};

struct TimeSlot {
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    bool is_available;
    ConsultationType consultation_type;
};

class Doctor : public BaseEntity {
public:
    Doctor();
    ~Doctor() override = default;

    // Basic info
    const std::string& getUserId() const { return user_id_; }
    const std::string& getMedicalLicenseNumber() const { return medical_license_number_; }
    const std::string& getQualification() const { return qualification_; }
    int getYearsOfExperience() const { return years_of_experience_; }
    DoctorStatus getStatus() const { return status_; }
    
    // Consultation details
    double getConsultationFee() const { return consultation_fee_; }
    int getConsultationDuration() const { return consultation_duration_minutes_; }
    const std::vector<ConsultationType>& getConsultationTypes() const { return consultation_types_; }
    
    // Rating and reviews
    double getRating() const { return rating_; }
    int getTotalReviews() const { return total_reviews_; }
    
    // Availability
    const std::string& getAvailabilityPattern() const { return availability_pattern_; }
    bool isAvailableToday() const { return is_available_today_; }
    
    // Professional details
    const std::string& getBio() const { return bio_; }
    const std::string& getLanguages() const { return languages_; }
    const std::vector<Specialization>& getSpecializations() const { return specializations_; }
    const std::vector<std::string>& getClinicIds() const { return clinic_ids_; }
    const std::vector<DoctorDocument>& getDocuments() const { return documents_; }

    // Setters
    void setUserId(const std::string& user_id) { user_id_ = user_id; }
    void setMedicalLicenseNumber(const std::string& license) { medical_license_number_ = license; }
    void setQualification(const std::string& qualification) { qualification_ = qualification; }
    void setYearsOfExperience(int years) { years_of_experience_ = years; }
    void setStatus(DoctorStatus status) { status_ = status; }
    void setConsultationFee(double fee) { consultation_fee_ = fee; }
    void setConsultationDuration(int minutes) { consultation_duration_minutes_ = minutes; }
    void setConsultationTypes(const std::vector<ConsultationType>& types) { consultation_types_ = types; }
    void setRating(double rating) { rating_ = rating; }
    void setTotalReviews(int count) { total_reviews_ = count; }
    void setAvailabilityPattern(const std::string& pattern) { availability_pattern_ = pattern; }
    void setAvailableToday(bool available) { is_available_today_ = available; }
    void setBio(const std::string& bio) { bio_ = bio; }
    void setLanguages(const std::string& languages) { languages_ = languages; }
    void setSpecializations(const std::vector<Specialization>& specializations) { specializations_ = specializations; }
    void setClinicIds(const std::vector<std::string>& clinic_ids) { clinic_ids_ = clinic_ids; }
    void setDocuments(const std::vector<DoctorDocument>& documents) { documents_ = documents; }

    // Utility methods
    bool isVerified() const { return status_ == DoctorStatus::VERIFIED; }
    bool isActive() const { return status_ == DoctorStatus::VERIFIED || status_ == DoctorStatus::PENDING_VERIFICATION; }
    bool hasSpecialization(const std::string& specialization_name) const;
    bool supportsConsultationType(ConsultationType type) const;
    void addSpecialization(const Specialization& specialization);
    void removeSpecialization(const std::string& specialization_id);
    void addClinic(const std::string& clinic_id);
    void removeClinic(const std::string& clinic_id);
    void addDocument(const DoctorDocument& document);
    void updateRating(double new_rating, int review_count);

    // Availability management
    std::vector<TimeSlot> getAvailableSlots(
        const std::chrono::system_clock::time_point& start_date,
        const std::chrono::system_clock::time_point& end_date,
        ConsultationType type = ConsultationType::BOTH
    ) const;
    
    bool isAvailableAt(const std::chrono::system_clock::time_point& time, ConsultationType type) const;

    // Serialization
    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& json) override;

private:
    std::string user_id_;  // Reference to User table
    std::string medical_license_number_;
    std::string qualification_;
    int years_of_experience_;
    DoctorStatus status_;
    
    // Consultation details
    double consultation_fee_;
    int consultation_duration_minutes_;
    std::vector<ConsultationType> consultation_types_;
    
    // Rating and reviews
    double rating_;
    int total_reviews_;
    
    // Availability
    std::string availability_pattern_;  // JSON string for complex patterns
    bool is_available_today_;
    
    // Professional details
    std::string bio_;
    std::string languages_;  // Comma-separated language codes
    std::vector<Specialization> specializations_;
    std::vector<std::string> clinic_ids_;
    std::vector<DoctorDocument> documents_;
};

// Utility functions
std::string doctorStatusToString(DoctorStatus status);
DoctorStatus stringToDoctorStatus(const std::string& status_str);
std::string consultationTypeToString(ConsultationType type);
ConsultationType stringToConsultationType(const std::string& type_str);

} // namespace healthcare::models