#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include "BaseEntity.h"

namespace healthcare {
namespace models {

enum class ClinicStatus {
    ACTIVE = 1,
    INACTIVE = 2,
    SUSPENDED = 3,
    PENDING_VERIFICATION = 4
};

struct OperatingHours {
    int day_of_week;  // 0=Sunday, 1=Monday, etc.
    std::chrono::system_clock::time_point opening_time;
    std::chrono::system_clock::time_point closing_time;
    bool is_closed;
};

struct ContactInfo {
    std::string phone_number;
    std::string email;
    std::string website;
    std::string emergency_contact;
};

struct Address {
    std::string street_address;
    std::string city;
    std::string state;
    std::string pincode;
    std::string country;
    double latitude;
    double longitude;
    std::string landmark;
};

struct Facility {
    std::string name;
    std::string description;
    bool is_available;
};

class Clinic : public BaseEntity {
private:
    std::string name_;
    std::string description_;
    Address address_;
    ContactInfo contact_info_;
    std::string license_number_;
    std::string registration_number_;
    ClinicStatus status_;
    std::vector<OperatingHours> operating_hours_;
    std::vector<std::string> doctor_ids_;  // Associated doctors
    std::vector<Facility> facilities_;
    std::vector<std::string> services_;
    std::vector<std::string> images_;  // URLs to clinic images
    double rating_;
    int total_reviews_;
    bool emergency_services_;
    bool pharmacy_available_;
    bool lab_services_;
    bool parking_available_;
    int total_beds_;
    int available_beds_;
    std::string admin_user_id_;  // Clinic admin
    std::chrono::system_clock::time_point verification_date_;
    std::string verification_document_url_;

public:
    // Constructors
    Clinic();
    Clinic(const std::string& name, const std::string& license_number,
           const Address& address, const ContactInfo& contact_info);

    // Getters
    const std::string& getName() const { return name_; }
    const std::string& getDescription() const { return description_; }
    const Address& getAddress() const { return address_; }
    const ContactInfo& getContactInfo() const { return contact_info_; }
    const std::string& getLicenseNumber() const { return license_number_; }
    const std::string& getRegistrationNumber() const { return registration_number_; }
    ClinicStatus getStatus() const { return status_; }
    const std::vector<OperatingHours>& getOperatingHours() const { return operating_hours_; }
    const std::vector<std::string>& getDoctorIds() const { return doctor_ids_; }
    const std::vector<Facility>& getFacilities() const { return facilities_; }
    const std::vector<std::string>& getServices() const { return services_; }
    const std::vector<std::string>& getImages() const { return images_; }
    double getRating() const { return rating_; }
    int getTotalReviews() const { return total_reviews_; }
    bool hasEmergencyServices() const { return emergency_services_; }
    bool hasPharmacy() const { return pharmacy_available_; }
    bool hasLabServices() const { return lab_services_; }
    bool hasParkingAvailable() const { return parking_available_; }
    int getTotalBeds() const { return total_beds_; }
    int getAvailableBeds() const { return available_beds_; }
    const std::string& getAdminUserId() const { return admin_user_id_; }
    const std::chrono::system_clock::time_point& getVerificationDate() const { return verification_date_; }
    const std::string& getVerificationDocumentUrl() const { return verification_document_url_; }

    // Setters
    void setName(const std::string& name) { name_ = name; }
    void setDescription(const std::string& description) { description_ = description; }
    void setAddress(const Address& address) { address_ = address; }
    void setContactInfo(const ContactInfo& contact_info) { contact_info_ = contact_info; }
    void setLicenseNumber(const std::string& license_number) { license_number_ = license_number; }
    void setRegistrationNumber(const std::string& registration_number) { registration_number_ = registration_number; }
    void setStatus(ClinicStatus status) { status_ = status; }
    void setOperatingHours(const std::vector<OperatingHours>& operating_hours) { operating_hours_ = operating_hours; }
    void setDoctorIds(const std::vector<std::string>& doctor_ids) { doctor_ids_ = doctor_ids; }
    void setFacilities(const std::vector<Facility>& facilities) { facilities_ = facilities; }
    void setServices(const std::vector<std::string>& services) { services_ = services; }
    void setImages(const std::vector<std::string>& images) { images_ = images; }
    void setRating(double rating) { rating_ = rating; }
    void setTotalReviews(int total_reviews) { total_reviews_ = total_reviews; }
    void setEmergencyServices(bool emergency_services) { emergency_services_ = emergency_services; }
    void setPharmacyAvailable(bool pharmacy_available) { pharmacy_available_ = pharmacy_available; }
    void setLabServices(bool lab_services) { lab_services_ = lab_services; }
    void setParkingAvailable(bool parking_available) { parking_available_ = parking_available; }
    void setTotalBeds(int total_beds) { total_beds_ = total_beds; }
    void setAvailableBeds(int available_beds) { available_beds_ = available_beds; }
    void setAdminUserId(const std::string& admin_user_id) { admin_user_id_ = admin_user_id; }
    void setVerificationDate(const std::chrono::system_clock::time_point& verification_date) { verification_date_ = verification_date; }
    void setVerificationDocumentUrl(const std::string& verification_document_url) { verification_document_url_ = verification_document_url; }

    // Business methods
    void addDoctor(const std::string& doctor_id);
    void removeDoctor(const std::string& doctor_id);
    void addFacility(const Facility& facility);
    void removeFacility(const std::string& facility_name);
    void addService(const std::string& service);
    void removeService(const std::string& service);
    void addImage(const std::string& image_url);
    void removeImage(const std::string& image_url);

    // Operating hours management
    void setOperatingHours(int day_of_week, 
                          const std::chrono::system_clock::time_point& opening_time,
                          const std::chrono::system_clock::time_point& closing_time);
    void markDayClosed(int day_of_week);
    bool isOpenAt(const std::chrono::system_clock::time_point& time) const;
    bool isOpenNow() const;
    OperatingHours getTodaysHours() const;

    // Bed management
    void updateAvailableBeds(int available_beds);
    bool hasBedAvailable() const;
    double getBedOccupancyRate() const;

    // Rating management
    void updateRating(double new_rating);

    // Utility methods
    double getDistanceFrom(double latitude, double longitude) const;
    bool isWithinRadius(double latitude, double longitude, double radius_km) const;
    std::vector<std::string> getAvailableDoctors() const;

    // Validation
    bool isValidLicense() const;
    bool isActive() const;
    bool canAcceptAppointments() const;

    // Serialization
    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& json) override;
};

} // namespace models
} // namespace healthcare