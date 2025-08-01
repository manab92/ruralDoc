#include "../../include/services/ValidationService.h"
#include "../../include/utils/ValidationUtils.h"

namespace healthcare::services {

bool ValidationService::validateEmail(const std::string& email) {
    return utils::ValidationUtils::isValidEmail(email);
}

bool ValidationService::validatePhoneNumber(const std::string& phone) {
    return utils::ValidationUtils::isValidPhoneNumber(phone);
}

bool ValidationService::validatePassword(const std::string& password) {
    return utils::ValidationUtils::isValidPassword(password);
}

std::vector<std::string> ValidationService::validateJson(const nlohmann::json& data, const std::string& schema_name) {
    // Stub implementation - in production, this would validate against JSON schemas
    return {};
}

} // namespace healthcare::services