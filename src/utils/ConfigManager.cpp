#include "../../include/utils/ConfigManager.h"
#include "../../include/utils/Logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

namespace healthcare::utils {

ConfigManager::ConfigManager() : env_override_enabled_(true), file_watching_enabled_(false) {
}

ConfigManager::~ConfigManager() {
    if (file_watch_thread_.joinable()) {
        file_watching_enabled_ = false;
        file_watch_thread_.join();
    }
}

bool ConfigManager::loadFromFile(const std::string& file_path) {
    try {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open config file: {}", file_path);
            return false;
        }
        
        nlohmann::json json_config;
        file >> json_config;
        file.close();
        
        config_ = json_config;
        config_file_path_ = file_path;
        
        // Apply environment variable overrides
        if (env_override_enabled_) {
            applyEnvironmentOverrides();
        }
        
        // Start file watching if enabled
        if (file_watching_enabled_) {
            startFileWatching();
        }
        
        LOG_INFO("Configuration loaded from: {}", file_path);
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load config file: {}", e.what());
        return false;
    }
}

bool ConfigManager::loadFromJson(const nlohmann::json& json_config) {
    try {
        config_ = json_config;
        
        // Apply environment variable overrides
        if (env_override_enabled_) {
            applyEnvironmentOverrides();
        }
        
        LOG_INFO("Configuration loaded from JSON");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load config from JSON: {}", e.what());
        return false;
    }
}

bool ConfigManager::loadFromEnvironment() {
    try {
        config_ = nlohmann::json::object();
        
        // Load all environment variables with a specific prefix
        const std::string prefix = "HEALTHCARE_";
        
        extern char** environ;
        for (char** env = environ; *env != nullptr; ++env) {
            std::string env_var(*env);
            size_t eq_pos = env_var.find('=');
            
            if (eq_pos != std::string::npos) {
                std::string key = env_var.substr(0, eq_pos);
                std::string value = env_var.substr(eq_pos + 1);
                
                if (key.substr(0, prefix.length()) == prefix) {
                    // Convert environment variable name to config path
                    std::string config_path = envVarToConfigPath(key.substr(prefix.length()));
                    setNestedValue(config_path, value);
                }
            }
        }
        
        LOG_INFO("Configuration loaded from environment variables");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load config from environment: {}", e.what());
        return false;
    }
}

std::string ConfigManager::getString(const std::string& key, const std::string& default_value) const {
    try {
        auto value = getNestedValue(key);
        if (value.is_string()) {
            return value.get<std::string>();
        }
    } catch (const std::exception&) {
        // Key not found or wrong type
    }
    
    return default_value;
}

int ConfigManager::getInt(const std::string& key, int default_value) const {
    try {
        auto value = getNestedValue(key);
        if (value.is_number_integer()) {
            return value.get<int>();
        } else if (value.is_string()) {
            return std::stoi(value.get<std::string>());
        }
    } catch (const std::exception&) {
        // Key not found or conversion failed
    }
    
    return default_value;
}

double ConfigManager::getDouble(const std::string& key, double default_value) const {
    try {
        auto value = getNestedValue(key);
        if (value.is_number()) {
            return value.get<double>();
        } else if (value.is_string()) {
            return std::stod(value.get<std::string>());
        }
    } catch (const std::exception&) {
        // Key not found or conversion failed
    }
    
    return default_value;
}

bool ConfigManager::getBool(const std::string& key, bool default_value) const {
    try {
        auto value = getNestedValue(key);
        if (value.is_boolean()) {
            return value.get<bool>();
        } else if (value.is_string()) {
            std::string str_value = value.get<std::string>();
            std::transform(str_value.begin(), str_value.end(), str_value.begin(), ::tolower);
            return str_value == "true" || str_value == "1" || str_value == "yes" || str_value == "on";
        } else if (value.is_number_integer()) {
            return value.get<int>() != 0;
        }
    } catch (const std::exception&) {
        // Key not found or conversion failed
    }
    
    return default_value;
}

std::vector<std::string> ConfigManager::getStringArray(const std::string& key) const {
    std::vector<std::string> result;
    
    try {
        auto value = getNestedValue(key);
        if (value.is_array()) {
            for (const auto& item : value) {
                if (item.is_string()) {
                    result.push_back(item.get<std::string>());
                }
            }
        }
    } catch (const std::exception&) {
        // Key not found or wrong type
    }
    
    return result;
}

nlohmann::json ConfigManager::getObject(const std::string& key) const {
    try {
        auto value = getNestedValue(key);
        if (value.is_object()) {
            return value;
        }
    } catch (const std::exception&) {
        // Key not found or wrong type
    }
    
    return nlohmann::json::object();
}

void ConfigManager::set(const std::string& key, const nlohmann::json& value) {
    setNestedValue(key, value);
}

bool ConfigManager::has(const std::string& key) const {
    try {
        getNestedValue(key);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void ConfigManager::remove(const std::string& key) {
    removeNestedValue(key);
}

bool ConfigManager::validate(const nlohmann::json& schema) const {
    // This is a simplified validation. In production, use a proper JSON schema validator
    try {
        validateAgainstSchema(config_, schema);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Configuration validation failed: {}", e.what());
        return false;
    }
}

std::vector<std::string> ConfigManager::getValidationErrors() const {
    // This would be populated during validation
    return validation_errors_;
}

void ConfigManager::setEnvironmentVariable(const std::string& name, const std::string& value) {
    environment_overrides_[name] = value;
    
    // Reapply overrides if enabled
    if (env_override_enabled_) {
        applyEnvironmentOverrides();
    }
}

std::string ConfigManager::getEnvironmentVariable(const std::string& name, const std::string& default_value) const {
    // Check our overrides first
    auto it = environment_overrides_.find(name);
    if (it != environment_overrides_.end()) {
        return it->second;
    }
    
    // Check actual environment
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : default_value;
}

nlohmann::json ConfigManager::getSection(const std::string& section) const {
    return getObject(section);
}

void ConfigManager::setSection(const std::string& section, const nlohmann::json& data) {
    set(section, data);
}

bool ConfigManager::saveToFile(const std::string& file_path) const {
    try {
        std::ofstream file(file_path);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open file for writing: {}", file_path);
            return false;
        }
        
        file << config_.dump(4); // Pretty print with 4 spaces
        file.close();
        
        LOG_INFO("Configuration saved to: {}", file_path);
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to save config file: {}", e.what());
        return false;
    }
}

bool ConfigManager::reload() {
    if (config_file_path_.empty()) {
        LOG_WARN("No config file path set, cannot reload");
        return false;
    }
    
    return loadFromFile(config_file_path_);
}

void ConfigManager::merge(const nlohmann::json& other_config) {
    mergeJson(config_, other_config);
}

void ConfigManager::clear() {
    config_ = nlohmann::json::object();
    validation_errors_.clear();
}

nlohmann::json ConfigManager::getDefaultDevelopmentConfig() {
    nlohmann::json config;
    
    // Server configuration
    config["server"]["host"] = "localhost";
    config["server"]["port"] = 8080;
    config["server"]["threads"] = 4;
    
    // Database configuration
    config["database"]["host"] = "localhost";
    config["database"]["port"] = 5432;
    config["database"]["name"] = "healthcare_dev";
    config["database"]["username"] = "postgres";
    config["database"]["password"] = "postgres";
    config["database"]["pool"]["min_connections"] = 2;
    config["database"]["pool"]["max_connections"] = 10;
    
    // Redis configuration
    config["redis"]["host"] = "localhost";
    config["redis"]["port"] = 6379;
    config["redis"]["database"] = 0;
    
    // Logging configuration
    config["logging"]["level"] = "debug";
    config["logging"]["console"] = true;
    config["logging"]["file"]["enabled"] = true;
    config["logging"]["file"]["path"] = "logs/healthcare_dev.log";
    
    // Security configuration
    config["security"]["jwt_secret"] = "dev_secret_key_change_in_production";
    config["security"]["jwt_expiration_hours"] = 24;
    config["security"]["password_hash_rounds"] = 10;
    config["security"]["enable_cors"] = true;
    config["security"]["cors"]["allowed_origins"] = {"http://localhost:3000"};
    
    return config;
}

nlohmann::json ConfigManager::getDefaultProductionConfig() {
    nlohmann::json config;
    
    // Server configuration
    config["server"]["host"] = "0.0.0.0";
    config["server"]["port"] = 8080;
    config["server"]["threads"] = std::thread::hardware_concurrency();
    
    // Database configuration
    config["database"]["host"] = "database";
    config["database"]["port"] = 5432;
    config["database"]["name"] = "healthcare_prod";
    config["database"]["username"] = "postgres";
    config["database"]["password"] = ""; // Should be set via environment
    config["database"]["pool"]["min_connections"] = 10;
    config["database"]["pool"]["max_connections"] = 50;
    config["database"]["enable_ssl"] = true;
    
    // Redis configuration
    config["redis"]["host"] = "redis";
    config["redis"]["port"] = 6379;
    config["redis"]["database"] = 0;
    config["redis"]["password"] = ""; // Should be set via environment
    
    // Logging configuration
    config["logging"]["level"] = "info";
    config["logging"]["console"] = false;
    config["logging"]["file"]["enabled"] = true;
    config["logging"]["file"]["path"] = "/var/log/healthcare/app.log";
    config["logging"]["file"]["max_size"] = 104857600; // 100MB
    config["logging"]["file"]["max_files"] = 10;
    
    // Security configuration
    config["security"]["jwt_secret"] = ""; // Must be set via environment
    config["security"]["jwt_expiration_hours"] = 12;
    config["security"]["password_hash_rounds"] = 12;
    config["security"]["enable_cors"] = true;
    config["security"]["cors"]["allowed_origins"] = {"https://healthcare.com"};
    config["security"]["enable_rate_limiting"] = true;
    config["security"]["rate_limit"]["requests_per_minute"] = 60;
    
    return config;
}

nlohmann::json ConfigManager::getDefaultTestConfig() {
    nlohmann::json config = getDefaultDevelopmentConfig();
    
    // Override for testing
    config["database"]["name"] = "healthcare_test";
    config["redis"]["database"] = 1;
    config["logging"]["level"] = "error";
    config["logging"]["file"]["enabled"] = false;
    
    return config;
}

nlohmann::json ConfigManager::getNestedValue(const std::string& key) const {
    std::vector<std::string> parts = splitKey(key);
    
    const nlohmann::json* current = &config_;
    for (const auto& part : parts) {
        if (!current->is_object() || !current->contains(part)) {
            throw std::runtime_error("Key not found: " + key);
        }
        current = &(*current)[part];
    }
    
    return *current;
}

void ConfigManager::setNestedValue(const std::string& key, const nlohmann::json& value) {
    std::vector<std::string> parts = splitKey(key);
    
    if (parts.empty()) return;
    
    nlohmann::json* current = &config_;
    
    // Navigate to the parent object, creating objects as needed
    for (size_t i = 0; i < parts.size() - 1; ++i) {
        if (!current->is_object()) {
            *current = nlohmann::json::object();
        }
        
        if (!current->contains(parts[i])) {
            (*current)[parts[i]] = nlohmann::json::object();
        }
        
        current = &(*current)[parts[i]];
    }
    
    // Set the final value
    (*current)[parts.back()] = value;
}

void ConfigManager::removeNestedValue(const std::string& key) {
    std::vector<std::string> parts = splitKey(key);
    
    if (parts.empty()) return;
    
    nlohmann::json* current = &config_;
    
    // Navigate to the parent object
    for (size_t i = 0; i < parts.size() - 1; ++i) {
        if (!current->is_object() || !current->contains(parts[i])) {
            return; // Key doesn't exist
        }
        current = &(*current)[parts[i]];
    }
    
    // Remove the final key
    if (current->is_object()) {
        current->erase(parts.back());
    }
}

std::vector<std::string> ConfigManager::splitKey(const std::string& key) const {
    std::vector<std::string> parts;
    std::stringstream ss(key);
    std::string part;
    
    while (std::getline(ss, part, '.')) {
        if (!part.empty()) {
            parts.push_back(part);
        }
    }
    
    return parts;
}

void ConfigManager::applyEnvironmentOverrides() {
    // Apply environment variable overrides
    const std::string prefix = "HEALTHCARE_";
    
    extern char** environ;
    for (char** env = environ; *env != nullptr; ++env) {
        std::string env_var(*env);
        size_t eq_pos = env_var.find('=');
        
        if (eq_pos != std::string::npos) {
            std::string key = env_var.substr(0, eq_pos);
            std::string value = env_var.substr(eq_pos + 1);
            
            if (key.substr(0, prefix.length()) == prefix) {
                std::string config_path = envVarToConfigPath(key.substr(prefix.length()));
                
                // Try to parse as JSON first
                try {
                    nlohmann::json json_value = nlohmann::json::parse(value);
                    setNestedValue(config_path, json_value);
                } catch (const std::exception&) {
                    // If not valid JSON, treat as string
                    setNestedValue(config_path, value);
                }
            }
        }
    }
}

std::string ConfigManager::envVarToConfigPath(const std::string& env_var) const {
    std::string config_path = env_var;
    
    // Convert to lowercase
    std::transform(config_path.begin(), config_path.end(), config_path.begin(), ::tolower);
    
    // Replace underscores with dots
    std::replace(config_path.begin(), config_path.end(), '_', '.');
    
    return config_path;
}

void ConfigManager::validateAgainstSchema(const nlohmann::json& data, const nlohmann::json& schema) const {
    // This is a simplified schema validation
    // In production, use a proper JSON schema validator library
    
    if (!schema.is_object()) {
        throw std::runtime_error("Schema must be an object");
    }
    
    if (schema.contains("required") && schema["required"].is_array()) {
        for (const auto& required_field : schema["required"]) {
            if (required_field.is_string()) {
                std::string field = required_field.get<std::string>();
                if (!data.contains(field)) {
                    throw std::runtime_error("Required field missing: " + field);
                }
            }
        }
    }
    
    if (schema.contains("properties") && schema["properties"].is_object()) {
        for (const auto& [key, prop_schema] : schema["properties"].items()) {
            if (data.contains(key)) {
                validatePropertyType(data[key], prop_schema);
            }
        }
    }
}

void ConfigManager::validatePropertyType(const nlohmann::json& value, const nlohmann::json& schema) const {
    if (!schema.contains("type")) return;
    
    std::string expected_type = schema["type"].get<std::string>();
    
    bool valid = false;
    if (expected_type == "string" && value.is_string()) valid = true;
    else if (expected_type == "number" && value.is_number()) valid = true;
    else if (expected_type == "integer" && value.is_number_integer()) valid = true;
    else if (expected_type == "boolean" && value.is_boolean()) valid = true;
    else if (expected_type == "object" && value.is_object()) valid = true;
    else if (expected_type == "array" && value.is_array()) valid = true;
    
    if (!valid) {
        throw std::runtime_error("Type mismatch: expected " + expected_type);
    }
}

void ConfigManager::mergeJson(nlohmann::json& target, const nlohmann::json& source) {
    for (auto& [key, value] : source.items()) {
        if (value.is_object() && target.contains(key) && target[key].is_object()) {
            // Recursively merge objects
            mergeJson(target[key], value);
        } else {
            // Overwrite value
            target[key] = value;
        }
    }
}

void ConfigManager::startFileWatching() {
    if (config_file_path_.empty() || !file_watching_enabled_) return;
    
    // Stop existing thread if any
    if (file_watch_thread_.joinable()) {
        file_watching_enabled_ = false;
        file_watch_thread_.join();
    }
    
    file_watching_enabled_ = true;
    file_watch_thread_ = std::thread([this]() {
        auto last_write_time = std::filesystem::last_write_time(config_file_path_);
        
        while (file_watching_enabled_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            try {
                auto current_write_time = std::filesystem::last_write_time(config_file_path_);
                if (current_write_time != last_write_time) {
                    LOG_INFO("Configuration file changed, reloading...");
                    reload();
                    last_write_time = current_write_time;
                }
            } catch (const std::exception& e) {
                LOG_ERROR("Error checking config file: {}", e.what());
            }
        }
    });
}

// GlobalConfig implementation
GlobalConfig& GlobalConfig::getInstance() {
    static GlobalConfig instance;
    return instance;
}

} // namespace healthcare::utils