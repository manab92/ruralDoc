#include "../../include/models/Doctor.h"
#include <algorithm>
#include <ctime>
#include <sstream>

namespace healthcare::models {

Doctor::Doctor() 
    : BaseEntity(),
      years_of_experience_(0),
      status_(DoctorStatus::PENDING_VERIFICATION),
      consultation_fee_(0.0),
      consultation_duration_minutes_(30),
      rating_(0.0),
      total_reviews_(0),
      is_available_today_(false) {
}

bool Doctor::hasSpecialization(const std::string& specialization_name) const {
    return std::any_of(specializations_.begin(), specializations_.end(),
        [&specialization_name](const Specialization& spec) {
            return spec.name == specialization_name;
        });
}

bool Doctor::supportsConsultationType(ConsultationType type) const {
    return std::find(consultation_types_.begin(), consultation_types_.end(), type) != consultation_types_.end();
}

void Doctor::addSpecialization(const Specialization& specialization) {
    if (!hasSpecialization(specialization.name)) {
        specializations_.push_back(specialization);
        updateTimestamp();
    }
}

void Doctor::removeSpecialization(const std::string& specialization_id) {
    auto it = std::remove_if(specializations_.begin(), specializations_.end(),
        [&specialization_id](const Specialization& spec) {
            return spec.id == specialization_id;
        });
    
    if (it != specializations_.end()) {
        specializations_.erase(it, specializations_.end());
        updateTimestamp();
    }
}

void Doctor::addClinic(const std::string& clinic_id) {
    if (std::find(clinic_ids_.begin(), clinic_ids_.end(), clinic_id) == clinic_ids_.end()) {
        clinic_ids_.push_back(clinic_id);
        updateTimestamp();
    }
}

void Doctor::removeClinic(const std::string& clinic_id) {
    auto it = std::remove(clinic_ids_.begin(), clinic_ids_.end(), clinic_id);
    if (it != clinic_ids_.end()) {
        clinic_ids_.erase(it, clinic_ids_.end());
        updateTimestamp();
    }
}

void Doctor::addDocument(const DoctorDocument& document) {
    documents_.push_back(document);
    updateTimestamp();
}

void Doctor::updateRating(double new_rating, int review_count) {
    // Calculate new average rating
    double total_rating = rating_ * total_reviews_ + new_rating * review_count;
    total_reviews_ += review_count;
    rating_ = total_reviews_ > 0 ? total_rating / total_reviews_ : 0.0;
    updateTimestamp();
}

std::vector<TimeSlot> Doctor::getAvailableSlots(
    const std::chrono::system_clock::time_point& start_date,
    const std::chrono::system_clock::time_point& end_date,
    ConsultationType type) const {
    
    std::vector<TimeSlot> available_slots;
    
    // Parse availability pattern JSON and generate slots
    if (!availability_pattern_.empty()) {
        try {
            nlohmann::json pattern = nlohmann::json::parse(availability_pattern_);
            
            // Iterate through each day between start and end date
            auto current = start_date;
            while (current <= end_date) {
                // Get day of week
                std::time_t time_t = std::chrono::system_clock::to_time_t(current);
                std::tm* tm = std::localtime(&time_t);
                int day_of_week = tm->tm_wday;
                
                // Check if doctor works on this day
                std::string day_key = std::to_string(day_of_week);
                if (pattern.contains(day_key)) {
                    auto day_schedule = pattern[day_key];
                    
                    // Generate slots for this day
                    for (const auto& slot : day_schedule) {
                        TimeSlot ts;
                        ts.start_time = current; // Would need proper time parsing
                        ts.end_time = current + std::chrono::minutes(consultation_duration_minutes_);
                        ts.is_available = true;
                        ts.consultation_type = type;
                        
                        if (supportsConsultationType(type)) {
                            available_slots.push_back(ts);
                        }
                    }
                }
                
                // Move to next day
                current += std::chrono::hours(24);
            }
        } catch (const std::exception& e) {
            // Log error parsing availability pattern
        }
    }
    
    return available_slots;
}

bool Doctor::isAvailableAt(const std::chrono::system_clock::time_point& time, ConsultationType type) const {
    if (!supportsConsultationType(type)) {
        return false;
    }
    
    // Check if the time falls within available slots
    auto slots = getAvailableSlots(time, time, type);
    return !slots.empty();
}

nlohmann::json Doctor::toJson() const {
    nlohmann::json json;
    
    // Base entity fields
    json["id"] = getId();
    json["created_at"] = std::chrono::system_clock::to_time_t(getCreatedAt());
    json["updated_at"] = std::chrono::system_clock::to_time_t(getUpdatedAt());
    json["is_deleted"] = isDeleted();
    
    // Doctor fields
    json["user_id"] = user_id_;
    json["medical_license_number"] = medical_license_number_;
    json["qualification"] = qualification_;
    json["years_of_experience"] = years_of_experience_;
    json["status"] = doctorStatusToString(status_);
    json["consultation_fee"] = consultation_fee_;
    json["consultation_duration_minutes"] = consultation_duration_minutes_;
    
    // Consultation types
    nlohmann::json consultation_types_json = nlohmann::json::array();
    for (const auto& type : consultation_types_) {
        consultation_types_json.push_back(consultationTypeToString(type));
    }
    json["consultation_types"] = consultation_types_json;
    
    json["rating"] = rating_;
    json["total_reviews"] = total_reviews_;
    json["availability_pattern"] = availability_pattern_;
    json["is_available_today"] = is_available_today_;
    json["bio"] = bio_;
    json["languages"] = languages_;
    
    // Specializations
    nlohmann::json specializations_json = nlohmann::json::array();
    for (const auto& spec : specializations_) {
        nlohmann::json spec_json;
        spec_json["id"] = spec.id;
        spec_json["name"] = spec.name;
        spec_json["description"] = spec.description;
        spec_json["category"] = spec.category;
        specializations_json.push_back(spec_json);
    }
    json["specializations"] = specializations_json;
    
    json["clinic_ids"] = clinic_ids_;
    
    // Documents
    nlohmann::json documents_json = nlohmann::json::array();
    for (const auto& doc : documents_) {
        nlohmann::json doc_json;
        doc_json["id"] = doc.id;
        doc_json["type"] = doc.type;
        doc_json["url"] = doc.url;
        doc_json["is_verified"] = doc.is_verified;
        doc_json["uploaded_at"] = std::chrono::system_clock::to_time_t(doc.uploaded_at);
        doc_json["verified_at"] = std::chrono::system_clock::to_time_t(doc.verified_at);
        documents_json.push_back(doc_json);
    }
    json["documents"] = documents_json;
    
    return json;
}

void Doctor::fromJson(const nlohmann::json& json) {
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
    
    // Doctor fields
    if (json.contains("user_id")) user_id_ = json["user_id"].get<std::string>();
    if (json.contains("medical_license_number")) medical_license_number_ = json["medical_license_number"].get<std::string>();
    if (json.contains("qualification")) qualification_ = json["qualification"].get<std::string>();
    if (json.contains("years_of_experience")) years_of_experience_ = json["years_of_experience"].get<int>();
    if (json.contains("status")) status_ = stringToDoctorStatus(json["status"].get<std::string>());
    if (json.contains("consultation_fee")) consultation_fee_ = json["consultation_fee"].get<double>();
    if (json.contains("consultation_duration_minutes")) consultation_duration_minutes_ = json["consultation_duration_minutes"].get<int>();
    
    // Consultation types
    if (json.contains("consultation_types")) {
        consultation_types_.clear();
        for (const auto& type_str : json["consultation_types"]) {
            consultation_types_.push_back(stringToConsultationType(type_str.get<std::string>()));
        }
    }
    
    if (json.contains("rating")) rating_ = json["rating"].get<double>();
    if (json.contains("total_reviews")) total_reviews_ = json["total_reviews"].get<int>();
    if (json.contains("availability_pattern")) availability_pattern_ = json["availability_pattern"].get<std::string>();
    if (json.contains("is_available_today")) is_available_today_ = json["is_available_today"].get<bool>();
    if (json.contains("bio")) bio_ = json["bio"].get<std::string>();
    if (json.contains("languages")) languages_ = json["languages"].get<std::string>();
    
    // Specializations
    if (json.contains("specializations")) {
        specializations_.clear();
        for (const auto& spec_json : json["specializations"]) {
            Specialization spec;
            spec.id = spec_json["id"].get<std::string>();
            spec.name = spec_json["name"].get<std::string>();
            spec.description = spec_json["description"].get<std::string>();
            spec.category = spec_json["category"].get<std::string>();
            specializations_.push_back(spec);
        }
    }
    
    if (json.contains("clinic_ids")) {
        clinic_ids_ = json["clinic_ids"].get<std::vector<std::string>>();
    }
    
    // Documents
    if (json.contains("documents")) {
        documents_.clear();
        for (const auto& doc_json : json["documents"]) {
            DoctorDocument doc;
            doc.id = doc_json["id"].get<std::string>();
            doc.type = doc_json["type"].get<std::string>();
            doc.url = doc_json["url"].get<std::string>();
            doc.is_verified = doc_json["is_verified"].get<bool>();
            doc.uploaded_at = std::chrono::system_clock::from_time_t(doc_json["uploaded_at"].get<std::time_t>());
            doc.verified_at = std::chrono::system_clock::from_time_t(doc_json["verified_at"].get<std::time_t>());
            documents_.push_back(doc);
        }
    }
}

// Utility functions
std::string doctorStatusToString(DoctorStatus status) {
    switch (status) {
        case DoctorStatus::PENDING_VERIFICATION: return "PENDING_VERIFICATION";
        case DoctorStatus::VERIFIED: return "VERIFIED";
        case DoctorStatus::SUSPENDED: return "SUSPENDED";
        case DoctorStatus::INACTIVE: return "INACTIVE";
        default: return "PENDING_VERIFICATION";
    }
}

DoctorStatus stringToDoctorStatus(const std::string& status_str) {
    if (status_str == "VERIFIED") return DoctorStatus::VERIFIED;
    if (status_str == "SUSPENDED") return DoctorStatus::SUSPENDED;
    if (status_str == "INACTIVE") return DoctorStatus::INACTIVE;
    return DoctorStatus::PENDING_VERIFICATION;
}

std::string consultationTypeToString(ConsultationType type) {
    switch (type) {
        case ConsultationType::ONLINE: return "ONLINE";
        case ConsultationType::OFFLINE: return "OFFLINE";
        case ConsultationType::BOTH: return "BOTH";
        default: return "BOTH";
    }
}

ConsultationType stringToConsultationType(const std::string& type_str) {
    if (type_str == "ONLINE") return ConsultationType::ONLINE;
    if (type_str == "OFFLINE") return ConsultationType::OFFLINE;
    return ConsultationType::BOTH;
}

} // namespace healthcare::models