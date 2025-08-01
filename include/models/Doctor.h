#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include "User.h"

namespace healthcare {
namespace models {

enum class DoctorStatus {
    PENDING_VERIFICATION = 1,
    VERIFIED = 2,
    SUSPENDED = 3,
    INACTIVE = 4
};

enum class ConsultationType {
    ONLINE = 1,
    OFFLINE = 2,
    BOTH = 3
};

struct Specialization {
    std::string id;
    std::string name;
    std::string description;
};

struct Education {
    std::string degree;
    std::string institution;
    int year_of_completion;
    std::string certificate_url;
};

struct Experience {
    std::string hospital_name;
    std::string position;
    int years;
    std::string description;
};

struct TimeSlot {
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    bool is_available;
};

class Doctor : public User {
private:
    std::string license_number_;
    std::vector<Specialization> specializations_;
    std::vector<Education> education_;
    std::vector<Experience> experience_;
    int years_of_experience_;
    double consultation_fee_;
    double rating_;
    int total_reviews_;
    std::string bio_;
    DoctorStatus status_;
    ConsultationType consultation_type_;
    std::string clinic_id_;  // Associated clinic
    std::vector<TimeSlot> availability_;
    std::string calendar_id_;  // For calendar integration
    bool is_available_now_;
    std::chrono::minutes consultation_duration_;  // Default consultation time
    std::vector<std::string> languages_;
    std::string qualification_certificate_url_;
    std::chrono::system_clock::time_point verification_date_;

public:
    // Constructors
    Doctor();
    Doctor(const std::string& email, const std::string& password_hash,
           const std::string& first_name, const std::string& last_name,
           const std::string& phone_number, const std::string& license_number);

    // Getters
    const std::string& getLicenseNumber() const { return license_number_; }
    const std::vector<Specialization>& getSpecializations() const { return specializations_; }
    const std::vector<Education>& getEducation() const { return education_; }
    const std::vector<Experience>& getExperience() const { return experience_; }
    int getYearsOfExperience() const { return years_of_experience_; }
    double getConsultationFee() const { return consultation_fee_; }
    double getRating() const { return rating_; }
    int getTotalReviews() const { return total_reviews_; }
    const std::string& getBio() const { return bio_; }
    DoctorStatus getStatus() const { return status_; }
    ConsultationType getConsultationType() const { return consultation_type_; }
    const std::string& getClinicId() const { return clinic_id_; }
    const std::vector<TimeSlot>& getAvailability() const { return availability_; }
    const std::string& getCalendarId() const { return calendar_id_; }
    bool isAvailableNow() const { return is_available_now_; }
    std::chrono::minutes getConsultationDuration() const { return consultation_duration_; }
    const std::vector<std::string>& getLanguages() const { return languages_; }
    const std::string& getQualificationCertificateUrl() const { return qualification_certificate_url_; }
    const std::chrono::system_clock::time_point& getVerificationDate() const { return verification_date_; }

    // Setters
    void setLicenseNumber(const std::string& license_number) { license_number_ = license_number; }
    void setSpecializations(const std::vector<Specialization>& specializations) { specializations_ = specializations; }
    void setEducation(const std::vector<Education>& education) { education_ = education; }
    void setExperience(const std::vector<Experience>& experience) { experience_ = experience; }
    void setYearsOfExperience(int years_of_experience) { years_of_experience_ = years_of_experience; }
    void setConsultationFee(double consultation_fee) { consultation_fee_ = consultation_fee; }
    void setRating(double rating) { rating_ = rating; }
    void setTotalReviews(int total_reviews) { total_reviews_ = total_reviews; }
    void setBio(const std::string& bio) { bio_ = bio; }
    void setStatus(DoctorStatus status) { status_ = status; }
    void setConsultationType(ConsultationType consultation_type) { consultation_type_ = consultation_type; }
    void setClinicId(const std::string& clinic_id) { clinic_id_ = clinic_id; }
    void setAvailability(const std::vector<TimeSlot>& availability) { availability_ = availability; }
    void setCalendarId(const std::string& calendar_id) { calendar_id_ = calendar_id; }
    void setAvailableNow(bool is_available_now) { is_available_now_ = is_available_now; }
    void setConsultationDuration(std::chrono::minutes consultation_duration) { consultation_duration_ = consultation_duration; }
    void setLanguages(const std::vector<std::string>& languages) { languages_ = languages; }
    void setQualificationCertificateUrl(const std::string& qualification_certificate_url) { qualification_certificate_url_ = qualification_certificate_url; }
    void setVerificationDate(const std::chrono::system_clock::time_point& verification_date) { verification_date_ = verification_date; }

    // Business methods
    void addSpecialization(const Specialization& specialization);
    void removeSpecialization(const std::string& specialization_id);
    void addEducation(const Education& education);
    void addExperience(const Experience& experience);
    void updateRating(double new_rating);
    
    // Availability management
    void setAvailableSlot(const std::chrono::system_clock::time_point& start, 
                         const std::chrono::system_clock::time_point& end);
    void blockTimeSlot(const std::chrono::system_clock::time_point& start,
                      const std::chrono::system_clock::time_point& end);
    bool isAvailableAt(const std::chrono::system_clock::time_point& time) const;
    std::vector<TimeSlot> getAvailableSlots(const std::chrono::system_clock::time_point& date) const;

    // Validation
    bool isQualified() const;
    bool canConsultOnline() const;
    bool canConsultOffline() const;

    // Serialization
    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& json) override;
};

} // namespace models
} // namespace healthcare