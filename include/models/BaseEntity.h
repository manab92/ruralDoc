#pragma once

#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

namespace healthcare::models {

class BaseEntity {
public:
    BaseEntity();
    virtual ~BaseEntity() = default;

    // Getters
    const std::string& getId() const { return id_; }
    const std::chrono::system_clock::time_point& getCreatedAt() const { return created_at_; }
    const std::chrono::system_clock::time_point& getUpdatedAt() const { return updated_at_; }
    bool isDeleted() const { return is_deleted_; }

    // Setters
    void setId(const std::string& id) { id_ = id; }
    void setCreatedAt(const std::chrono::system_clock::time_point& created_at) { created_at_ = created_at; }
    void setUpdatedAt(const std::chrono::system_clock::time_point& updated_at) { updated_at_ = updated_at; }
    void setDeleted(bool deleted) { is_deleted_ = deleted; }

    // Utility methods
    void updateTimestamp() { updated_at_ = std::chrono::system_clock::now(); }
    void markAsDeleted() { is_deleted_ = true; updateTimestamp(); }

    // Pure virtual methods for serialization
    virtual nlohmann::json toJson() const = 0;
    virtual void fromJson(const nlohmann::json& json) = 0;

protected:
    std::string generateUUID();

private:
    std::string id_;
    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point updated_at_;
    bool is_deleted_;
};

} // namespace healthcare::models