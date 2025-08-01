#include "../../include/models/Clinic.h"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace healthcare::models {

Clinic::Clinic() 
    : BaseEntity(),
      status_(ClinicStatus::PENDING_VERIFICATION),
      rating_(0.0),
      total_reviews_(0),
      has_emergency_services_(false) {
}

bool Clinic::isOpenNow() const {
    auto now = std::chrono::system_clock::now();
    return isOpenAt(now);
}

bool Clinic::isOpenAt(const std::chrono::system_clock::time_point& time) const {
    std::time_t time_t = std::chrono::system_clock::to_time_t(time);
    std::tm* tm = std::localtime(&time_t);
    
    std::string day_of_week = getCurrentDayOfWeek();
    std::ostringstream current_time_stream;
    current_time_stream << std::setfill('0') << std::setw(2) << tm->tm_hour 
                       << ":" << std::setw(2) << tm->tm_min;
    std::string current_time = current_time_stream.str();
    
    for (const auto& hours : working_hours_) {
        if (hours.day_of_week == day_of_week && !hours.is_closed) {
            if (isTimeInRange(current_time, hours.start_time, hours.end_time)) {
                // Check if we're in break time
                if (!hours.break_start.empty() && !hours.break_end.empty()) {
                    if (isTimeInRange(current_time, hours.break_start, hours.break_end)) {
                        return false;
                    }
                }
                return true;
            }
        }
    }
    
    return false;
}

std::string Clinic::getFullAddress() const {
    std::ostringstream address_stream;
    address_stream << address_.street_address;
    
    if (!address_.landmark.empty()) {
        address_stream << ", " << address_.landmark;
    }
    
    address_stream << ", " << address_.city
                  << ", " << address_.state
                  << " - " << address_.pincode
                  << ", " << address_.country;
    
    return address_stream.str();
}

double Clinic::getDistanceFrom(double latitude, double longitude) const {
    // Haversine formula for calculating distance between two points on Earth
    const double R = 6371.0; // Earth's radius in kilometers
    
    double lat1_rad = address_.latitude * M_PI / 180.0;
    double lat2_rad = latitude * M_PI / 180.0;
    double delta_lat = (latitude - address_.latitude) * M_PI / 180.0;
    double delta_lon = (longitude - address_.longitude) * M_PI / 180.0;
    
    double a = sin(delta_lat / 2) * sin(delta_lat / 2) +
               cos(lat1_rad) * cos(lat2_rad) *
               sin(delta_lon / 2) * sin(delta_lon / 2);
    
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    
    return R * c; // Distance in kilometers
}

void Clinic::addDoctor(const std::string& doctor_id) {
    if (std::find(doctor_ids_.begin(), doctor_ids_.end(), doctor_id) == doctor_ids_.end()) {
        doctor_ids_.push_back(doctor_id);
        updateTimestamp();
    }
}

void Clinic::removeDoctor(const std::string& doctor_id) {
    auto it = std::remove(doctor_ids_.begin(), doctor_ids_.end(), doctor_id);
    if (it != doctor_ids_.end()) {
        doctor_ids_.erase(it, doctor_ids_.end());
        updateTimestamp();
    }
}

void Clinic::addService(const std::string& service) {
    if (std::find(services_.begin(), services_.end(), service) == services_.end()) {
        services_.push_back(service);
        updateTimestamp();
    }
}

void Clinic::removeService(const std::string& service) {
    auto it = std::remove(services_.begin(), services_.end(), service);
    if (it != services_.end()) {
        services_.erase(it, services_.end());
        updateTimestamp();
    }
}

void Clinic::addFacility(const Facility& facility) {
    auto it = std::find_if(facilities_.begin(), facilities_.end(),
        [&facility](const Facility& f) { return f.name == facility.name; });
    
    if (it == facilities_.end()) {
        facilities_.push_back(facility);
        updateTimestamp();
    }
}

void Clinic::removeFacility(const std::string& facility_name) {
    auto it = std::remove_if(facilities_.begin(), facilities_.end(),
        [&facility_name](const Facility& f) { return f.name == facility_name; });
    
    if (it != facilities_.end()) {
        facilities_.erase(it, facilities_.end());
        updateTimestamp();
    }
}

void Clinic::updateWorkingHours(const std::string& day, const std::string& start, const std::string& end) {
    auto it = std::find_if(working_hours_.begin(), working_hours_.end(),
        [&day](const WorkingHours& wh) { return wh.day_of_week == day; });
    
    if (it != working_hours_.end()) {
        it->start_time = start;
        it->end_time = end;
        it->is_closed = false;
    } else {
        WorkingHours new_hours;
        new_hours.day_of_week = day;
        new_hours.start_time = start;
        new_hours.end_time = end;
        new_hours.is_closed = false;
        working_hours_.push_back(new_hours);
    }
    updateTimestamp();
}

void Clinic::updateRating(double new_rating, int review_count) {
    double total_rating = rating_ * total_reviews_ + new_rating * review_count;
    total_reviews_ += review_count;
    rating_ = total_reviews_ > 0 ? total_rating / total_reviews_ : 0.0;
    updateTimestamp();
}

bool Clinic::hasService(const std::string& service) const {
    return std::find(services_.begin(), services_.end(), service) != services_.end();
}

bool Clinic::hasFacility(const std::string& facility_name) const {
    return std::any_of(facilities_.begin(), facilities_.end(),
        [&facility_name](const Facility& f) { return f.name == facility_name && f.is_available; });
}

bool Clinic::hasDoctor(const std::string& doctor_id) const {
    return std::find(doctor_ids_.begin(), doctor_ids_.end(), doctor_id) != doctor_ids_.end();
}

std::vector<std::string> Clinic::getAvailableDays() const {
    std::vector<std::string> available_days;
    for (const auto& hours : working_hours_) {
        if (!hours.is_closed) {
            available_days.push_back(hours.day_of_week);
        }
    }
    return available_days;
}

nlohmann::json Clinic::toJson() const {
    nlohmann::json json;
    
    // Base entity fields
    json["id"] = getId();
    json["created_at"] = std::chrono::system_clock::to_time_t(getCreatedAt());
    json["updated_at"] = std::chrono::system_clock::to_time_t(getUpdatedAt());
    json["is_deleted"] = isDeleted();
    
    // Clinic fields
    json["name"] = name_;
    json["description"] = description_;
    json["registration_number"] = registration_number_;
    json["status"] = clinicStatusToString(status_);
    
    // Contact info
    nlohmann::json contact_json;
    contact_json["phone_primary"] = contact_info_.phone_primary;
    contact_json["phone_secondary"] = contact_info_.phone_secondary;
    contact_json["email"] = contact_info_.email;
    contact_json["website"] = contact_info_.website;
    json["contact_info"] = contact_json;
    
    // Address
    nlohmann::json address_json;
    address_json["street_address"] = address_.street_address;
    address_json["landmark"] = address_.landmark;
    address_json["city"] = address_.city;
    address_json["state"] = address_.state;
    address_json["pincode"] = address_.pincode;
    address_json["country"] = address_.country;
    address_json["latitude"] = address_.latitude;
    address_json["longitude"] = address_.longitude;
    json["address"] = address_json;
    
    // Working hours
    nlohmann::json working_hours_json = nlohmann::json::array();
    for (const auto& hours : working_hours_) {
        nlohmann::json hours_json;
        hours_json["day_of_week"] = hours.day_of_week;
        hours_json["start_time"] = hours.start_time;
        hours_json["end_time"] = hours.end_time;
        hours_json["is_closed"] = hours.is_closed;
        hours_json["break_start"] = hours.break_start;
        hours_json["break_end"] = hours.break_end;
        working_hours_json.push_back(hours_json);
    }
    json["working_hours"] = working_hours_json;
    
    // Facilities
    nlohmann::json facilities_json = nlohmann::json::array();
    for (const auto& facility : facilities_) {
        nlohmann::json facility_json;
        facility_json["name"] = facility.name;
        facility_json["description"] = facility.description;
        facility_json["is_available"] = facility.is_available;
        facilities_json.push_back(facility_json);
    }
    json["facilities"] = facilities_json;
    
    json["services"] = services_;
    json["logo_url"] = logo_url_;
    json["image_urls"] = image_urls_;
    json["rating"] = rating_;
    json["total_reviews"] = total_reviews_;
    json["owner_id"] = owner_id_;
    json["doctor_ids"] = doctor_ids_;
    json["has_emergency_services"] = has_emergency_services_;
    json["emergency_contact"] = emergency_contact_;
    
    return json;
}

void Clinic::fromJson(const nlohmann::json& json) {
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
    
    // Clinic fields
    if (json.contains("name")) name_ = json["name"].get<std::string>();
    if (json.contains("description")) description_ = json["description"].get<std::string>();
    if (json.contains("registration_number")) registration_number_ = json["registration_number"].get<std::string>();
    if (json.contains("status")) status_ = stringToClinicStatus(json["status"].get<std::string>());
    
    // Contact info
    if (json.contains("contact_info")) {
        auto contact_json = json["contact_info"];
        contact_info_.phone_primary = contact_json.value("phone_primary", "");
        contact_info_.phone_secondary = contact_json.value("phone_secondary", "");
        contact_info_.email = contact_json.value("email", "");
        contact_info_.website = contact_json.value("website", "");
    }
    
    // Address
    if (json.contains("address")) {
        auto address_json = json["address"];
        address_.street_address = address_json.value("street_address", "");
        address_.landmark = address_json.value("landmark", "");
        address_.city = address_json.value("city", "");
        address_.state = address_json.value("state", "");
        address_.pincode = address_json.value("pincode", "");
        address_.country = address_json.value("country", "India");
        address_.latitude = address_json.value("latitude", 0.0);
        address_.longitude = address_json.value("longitude", 0.0);
    }
    
    // Working hours
    if (json.contains("working_hours")) {
        working_hours_.clear();
        for (const auto& hours_json : json["working_hours"]) {
            WorkingHours hours;
            hours.day_of_week = hours_json.value("day_of_week", "");
            hours.start_time = hours_json.value("start_time", "");
            hours.end_time = hours_json.value("end_time", "");
            hours.is_closed = hours_json.value("is_closed", false);
            hours.break_start = hours_json.value("break_start", "");
            hours.break_end = hours_json.value("break_end", "");
            working_hours_.push_back(hours);
        }
    }
    
    // Facilities
    if (json.contains("facilities")) {
        facilities_.clear();
        for (const auto& facility_json : json["facilities"]) {
            Facility facility;
            facility.name = facility_json.value("name", "");
            facility.description = facility_json.value("description", "");
            facility.is_available = facility_json.value("is_available", true);
            facilities_.push_back(facility);
        }
    }
    
    if (json.contains("services")) services_ = json["services"].get<std::vector<std::string>>();
    if (json.contains("logo_url")) logo_url_ = json["logo_url"].get<std::string>();
    if (json.contains("image_urls")) image_urls_ = json["image_urls"].get<std::vector<std::string>>();
    if (json.contains("rating")) rating_ = json["rating"].get<double>();
    if (json.contains("total_reviews")) total_reviews_ = json["total_reviews"].get<int>();
    if (json.contains("owner_id")) owner_id_ = json["owner_id"].get<std::string>();
    if (json.contains("doctor_ids")) doctor_ids_ = json["doctor_ids"].get<std::vector<std::string>>();
    if (json.contains("has_emergency_services")) has_emergency_services_ = json["has_emergency_services"].get<bool>();
    if (json.contains("emergency_contact")) emergency_contact_ = json["emergency_contact"].get<std::string>();
}

// Utility functions
std::string clinicStatusToString(ClinicStatus status) {
    switch (status) {
        case ClinicStatus::ACTIVE: return "ACTIVE";
        case ClinicStatus::INACTIVE: return "INACTIVE";
        case ClinicStatus::PENDING_VERIFICATION: return "PENDING_VERIFICATION";
        case ClinicStatus::SUSPENDED: return "SUSPENDED";
        default: return "PENDING_VERIFICATION";
    }
}

ClinicStatus stringToClinicStatus(const std::string& status_str) {
    if (status_str == "ACTIVE") return ClinicStatus::ACTIVE;
    if (status_str == "INACTIVE") return ClinicStatus::INACTIVE;
    if (status_str == "SUSPENDED") return ClinicStatus::SUSPENDED;
    return ClinicStatus::PENDING_VERIFICATION;
}

std::string getCurrentDayOfWeek() {
    auto now = std::chrono::system_clock::now();
    std::time_t time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time_t);
    
    const char* days[] = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};
    return days[tm->tm_wday];
}

bool isTimeInRange(const std::string& current_time, const std::string& start_time, const std::string& end_time) {
    return current_time >= start_time && current_time <= end_time;
}

} // namespace healthcare::models