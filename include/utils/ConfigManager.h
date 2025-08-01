#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>

namespace healthcare::utils {

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager() = default;

    // Configuration loading
    bool loadConfig(const std::string& config_file);
    bool loadFromJson(const nlohmann::json& config_json);
    bool loadFromEnvironment();
    bool reloadConfig();
    
    // String getters
    std::string getString(const std::string& key, const std::string& default_value = "") const;
    std::string getStringRequired(const std::string& key) const;
    
    // Numeric getters
    int getInt(const std::string& key, int default_value = 0) const;
    double getDouble(const std::string& key, double default_value = 0.0) const;
    long getLong(const std::string& key, long default_value = 0) const;
    
    // Boolean getter
    bool getBool(const std::string& key, bool default_value = false) const;
    
    // Array getters
    std::vector<std::string> getStringArray(const std::string& key) const;
    std::vector<int> getIntArray(const std::string& key) const;
    std::vector<double> getDoubleArray(const std::string& key) const;
    
    // Object getters
    nlohmann::json getObject(const std::string& key) const;
    std::map<std::string, std::string> getStringMap(const std::string& key) const;
    
    // Nested configuration access (using dot notation: "database.host")
    std::string getNestedString(const std::string& nested_key, const std::string& default_value = "") const;
    int getNestedInt(const std::string& nested_key, int default_value = 0) const;
    bool getNestedBool(const std::string& nested_key, bool default_value = false) const;
    
    // Configuration setters
    void setString(const std::string& key, const std::string& value);
    void setInt(const std::string& key, int value);
    void setDouble(const std::string& key, double value);
    void setBool(const std::string& key, bool value);
    void setStringArray(const std::string& key, const std::vector<std::string>& values);
    void setObject(const std::string& key, const nlohmann::json& object);
    
    // Configuration validation
    bool hasKey(const std::string& key) const;
    bool hasNestedKey(const std::string& nested_key) const;
    bool validateRequiredKeys(const std::vector<std::string>& required_keys) const;
    std::vector<std::string> getMissingKeys(const std::vector<std::string>& required_keys) const;
    
    // Environment variable integration
    void enableEnvironmentOverride(bool enable = true) { env_override_enabled_ = enable; }
    std::string getFromEnvironment(const std::string& env_var, const std::string& default_value = "") const;
    bool setFromEnvironment(const std::string& config_key, const std::string& env_var);
    
    // Configuration sections
    ConfigManager getSection(const std::string& section_name) const;
    std::vector<std::string> getSectionKeys(const std::string& section_name = "") const;
    
    // File operations
    bool saveConfig(const std::string& output_file) const;
    bool backupConfig(const std::string& backup_file) const;
    
    // Utility methods
    void clear();
    bool isEmpty() const;
    size_t size() const;
    std::string getConfigSource() const { return config_file_path_; }
    nlohmann::json getRawConfig() const { return config_; }
    
    // Pretty printing
    std::string toString(int indent = 2) const;
    void printConfig() const;
    void printSection(const std::string& section_name) const;
    
    // Configuration merging
    void merge(const ConfigManager& other, bool overwrite = true);
    void mergeFromFile(const std::string& config_file, bool overwrite = true);
    
    // Default configurations for different environments
    static ConfigManager createDevelopmentConfig();
    static ConfigManager createProductionConfig();
    static ConfigManager createTestConfig();
    
    // Validation schemas
    bool validateSchema(const nlohmann::json& schema) const;
    static nlohmann::json getDefaultSchema();
    
    // Configuration templates
    void applyDatabaseDefaults();
    void applyServerDefaults();
    void applySecurityDefaults();
    void applyLoggingDefaults();
    void applyRedisDefaults();
    
    // Watch for config file changes
    void enableFileWatching(bool enable = true) { file_watching_enabled_ = enable; }
    bool isConfigChanged() const;
    void setChangeCallback(std::function<void(const ConfigManager&)> callback);

private:
    nlohmann::json config_;
    std::string config_file_path_;
    bool env_override_enabled_;
    bool file_watching_enabled_;
    mutable std::time_t last_file_check_;
    mutable std::time_t file_last_modified_;
    std::function<void(const ConfigManager&)> change_callback_;
    
    // Helper methods
    nlohmann::json getValue(const std::string& key) const;
    nlohmann::json getNestedValue(const std::string& nested_key) const;
    void setValue(const std::string& key, const nlohmann::json& value);
    std::vector<std::string> splitKey(const std::string& nested_key) const;
    bool fileExists(const std::string& file_path) const;
    std::time_t getFileModificationTime(const std::string& file_path) const;
    std::string environmentKeyToConfigKey(const std::string& env_key) const;
    std::string configKeyToEnvironmentKey(const std::string& config_key) const;
    
    // Default configuration templates
    void setServerDefaults();
    void setDatabaseDefaults();
    void setRedisDefaults();
    void setSecurityDefaults();
    void setLoggingDefaults();
    void setPaymentDefaults();
    void setNotificationDefaults();
};

// Singleton pattern for global configuration access
class GlobalConfig {
public:
    static ConfigManager& getInstance();
    static bool initialize(const std::string& config_file);
    static void destroy();
    
private:
    static std::unique_ptr<ConfigManager> instance_;
    GlobalConfig() = default;
};

// Convenience macros for configuration access
#define GET_CONFIG_STRING(key, default_val) healthcare::utils::GlobalConfig::getInstance().getString(key, default_val)
#define GET_CONFIG_INT(key, default_val) healthcare::utils::GlobalConfig::getInstance().getInt(key, default_val)
#define GET_CONFIG_BOOL(key, default_val) healthcare::utils::GlobalConfig::getInstance().getBool(key, default_val)
#define GET_CONFIG_DOUBLE(key, default_val) healthcare::utils::GlobalConfig::getInstance().getDouble(key, default_val)

#define CONFIG_REQUIRED_STRING(key) healthcare::utils::GlobalConfig::getInstance().getStringRequired(key)
#define CONFIG_HAS_KEY(key) healthcare::utils::GlobalConfig::getInstance().hasKey(key)

// Environment-specific configuration loading
#define LOAD_DEV_CONFIG() healthcare::utils::ConfigManager::createDevelopmentConfig()
#define LOAD_PROD_CONFIG() healthcare::utils::ConfigManager::createProductionConfig()
#define LOAD_TEST_CONFIG() healthcare::utils::ConfigManager::createTestConfig()

} // namespace healthcare::utils