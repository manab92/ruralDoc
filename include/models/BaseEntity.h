#pragma once

#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

namespace healthcare {
namespace models {

class BaseEntity {
protected:
    std::string id_;
    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point updated_at_;

public:
    // Constructors
    BaseEntity();
    BaseEntity(const std::string& id);

    // Virtual destructor for proper inheritance
    virtual ~BaseEntity() = default;

    // Common getters
    const std::string& getId() const { return id_; }
    const std::chrono::system_clock::time_point& getCreatedAt() const { return created_at_; }
    const std::chrono::system_clock::time_point& getUpdatedAt() const { return updated_at_; }

    // Common setters
    void setId(const std::string& id) { id_ = id; }
    void setCreatedAt(const std::chrono::system_clock::time_point& created_at) { created_at_ = created_at; }
    void setUpdatedAt(const std::chrono::system_clock::time_point& updated_at) { updated_at_ = updated_at; }

    // Utility methods
    void updateTimestamp();
    std::string getCreatedAtString() const;
    std::string getUpdatedAtString() const;

    // Pure virtual methods for serialization
    virtual nlohmann::json toJson() const = 0;
    virtual void fromJson(const nlohmann::json& json) = 0;

    // Equality operator
    bool operator==(const BaseEntity& other) const;
    bool operator!=(const BaseEntity& other) const;
};

} // namespace models
} // namespace healthcare