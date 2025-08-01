#pragma once

#include "BaseEntity.h"
#include <string>
#include <vector>
#include <chrono>

namespace healthcare::models {

enum class ClinicStatus {
    ACTIVE,
    INACTIVE,
    PENDING_VERIFICATION,
    SUSPENDED
};

struct ContactInfo {
    std::string phone_primary;
    std::string phone_secondary;
    std::string email;
    std::string website;
};

struct Address {
    std::string street_address;
    std::string landmark;
    std::string city;
    std::string state;
    std::string pincode;
    std::string country;
    double latitude;
    double longitude;
};

struct WorkingHours {
    std::string day_of_week;  // MONDAY, TUESDAY, etc.
    std::string start_time;   // HH:MM format
    std::string end_time;     // HH:MM format
    bool is_closed;
    std::string break_start;  // Optional lunch break
    std::string break_end;
};

struct Facility {
    std::string name;
    std::string description;
    bool is_available;
};

class Clinic : public BaseEntity {
public:
    Clinic();
    ~Clinic() override = default;

    // Basic information
    const std::string& getName() const { return name_; }
    const std::string& getDescription() const { return description_; }
    const std::string& getRegistrationNumber() const { return registration_number_; }
    ClinicStatus getStatus() const { return status_; }
    
    // Contact and location
    const ContactInfo& getContactInfo() const { return contact_info_; }
    const Address& getAddress() const { return address_; }
    
    // Operational details
    const std::vector<WorkingHours>& getWorkingHours() const { return working_hours_; }
    const std::vector<Facility>& getFacilities() const { return facilities_; }
    const std::vector<std::string>& getServices() const { return services_; }
    
    // Media and branding
    const std::string& getLogoUrl() const { return logo_url_; }
    const std::vector<std::string>& getImageUrls() const { return image_urls_; }
    
    // Rating and reviews
    double getRating() const { return rating_; }
    int getTotalReviews() const { return total_reviews_; }
    
    // Administrative
    const std::string& getOwnerId() const { return owner_id_; }
    const std::vector<std::string>& getDoctorIds() const { return doctor_ids_; }
    
    // Emergency details
    bool hasEmergencyServices() const { return has_emergency_services_; }
    const std::string& getEmergencyContact() const { return emergency_contact_; }

    // Setters
    void setName(const std::string& name) { name_ = name; }
    void setDescription(const std::string& description) { description_ = description; }
    void setRegistrationNumber(const std::string& reg_number) { registration_number_ = reg_number; }
    void setStatus(ClinicStatus status) { status_ = status; }
    void setContactInfo(const ContactInfo& contact_info) { contact_info_ = contact_info; }
    void setAddress(const Address& address) { address_ = address; }
    void setWorkingHours(const std::vector<WorkingHours>& working_hours) { working_hours_ = working_hours; }
    void setFacilities(const std::vector<Facility>& facilities) { facilities_ = facilities; }
    void setServices(const std::vector<std::string>& services) { services_ = services; }
    void setLogoUrl(const std::string& logo_url) { logo_url_ = logo_url; }
    void setImageUrls(const std::vector<std::string>& image_urls) { image_urls_ = image_urls; }
    void setRating(double rating) { rating_ = rating; }
    void setTotalReviews(int count) { total_reviews_ = count; }
    void setOwnerId(const std::string& owner_id) { owner_id_ = owner_id; }
    void setDoctorIds(const std::vector<std::string>& doctor_ids) { doctor_ids_ = doctor_ids; }
    void setEmergencyServices(bool has_emergency) { has_emergency_services_ = has_emergency; }
    void setEmergencyContact(const std::string& contact) { emergency_contact_ = contact; }

    // Utility methods
    bool isOperational() const { return status_ == ClinicStatus::ACTIVE; }
    bool isOpenNow() const;
    bool isOpenAt(const std::chrono::system_clock::time_point& time) const;
    std::string getFullAddress() const;
    double getDistanceFrom(double latitude, double longitude) const;
    
    // Management operations
    void addDoctor(const std::string& doctor_id);
    void removeDoctor(const std::string& doctor_id);
    void addService(const std::string& service);
    void removeService(const std::string& service);
    void addFacility(const Facility& facility);
    void removeFacility(const std::string& facility_name);
    void updateWorkingHours(const std::string& day, const std::string& start, const std::string& end);
    void updateRating(double new_rating, int review_count);
    
    // Search and filter helpers
    bool hasService(const std::string& service) const;
    bool hasFacility(const std::string& facility_name) const;
    bool hasDoctor(const std::string& doctor_id) const;
    std::vector<std::string> getAvailableDays() const;

    // Serialization
    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& json) override;

private:
    // Basic information
    std::string name_;
    std::string description_;
    std::string registration_number_;
    ClinicStatus status_;
    
    // Contact and location
    ContactInfo contact_info_;
    Address address_;
    
    // Operational details
    std::vector<WorkingHours> working_hours_;
    std::vector<Facility> facilities_;
    std::vector<std::string> services_;
    
    // Media and branding
    std::string logo_url_;
    std::vector<std::string> image_urls_;
    
    // Rating and reviews
    double rating_;
    int total_reviews_;
    
    // Administrative
    std::string owner_id_;  // Reference to User who owns/manages clinic
    std::vector<std::string> doctor_ids_;  // Doctors associated with clinic
    
    // Emergency details
    bool has_emergency_services_;
    std::string emergency_contact_;
};

// Utility functions
std::string clinicStatusToString(ClinicStatus status);
ClinicStatus stringToClinicStatus(const std::string& status_str);
std::string getCurrentDayOfWeek();
bool isTimeInRange(const std::string& current_time, const std::string& start_time, const std::string& end_time);

} // namespace healthcare::models