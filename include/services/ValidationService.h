#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace healthcare::services {

class ValidationService {
public:
    ValidationService() = default;
    ~ValidationService() = default;
    
    bool validateEmail(const std::string& email);
    bool validatePhoneNumber(const std::string& phone);
    bool validatePassword(const std::string& password);
    std::vector<std::string> validateJson(const nlohmann::json& data, const std::string& schema_name);
};

} // namespace healthcare::services