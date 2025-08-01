#pragma once

#include <string>
#include <memory>
#include <vector>
#include <pqxx/pqxx>
#include <redis++/redis++.h>
#include <nlohmann/json.hpp>

namespace healthcare {
namespace database {

struct DatabaseConfig {
    std::string host = "localhost";
    int port = 5432;
    std::string database = "healthcare_db";
    std::string username = "postgres";
    std::string password = "";
    int max_connections = 10;
    int connection_timeout = 30;
};

struct RedisConfig {
    std::string host = "localhost";
    int port = 6379;
    std::string password = "";
    int database = 0;
    int connection_timeout = 5;
};

class DatabaseManager {
private:
    static std::unique_ptr<DatabaseManager> instance_;
    static std::mutex instance_mutex_;
    
    DatabaseConfig db_config_;
    RedisConfig redis_config_;
    std::unique_ptr<pqxx::connection> pg_connection_;
    std::unique_ptr<sw::redis::Redis> redis_connection_;
    std::mutex connection_mutex_;

    // Private constructor for singleton
    DatabaseManager() = default;

public:
    // Singleton instance
    static DatabaseManager& getInstance();
    
    // Delete copy constructor and assignment operator
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // Configuration
    void configure(const DatabaseConfig& db_config, const RedisConfig& redis_config);
    
    // Connection management
    bool connect();
    void disconnect();
    bool isConnected() const;
    bool testConnection() const;

    // PostgreSQL operations
    pqxx::result executeQuery(const std::string& query);
    pqxx::result executeQuery(const std::string& query, const std::vector<std::string>& params);
    std::string executePreparedStatement(const std::string& statement_name, 
                                       const std::vector<std::string>& params);
    
    // Transaction management
    std::unique_ptr<pqxx::work> beginTransaction();
    void commitTransaction(std::unique_ptr<pqxx::work>& transaction);
    void rollbackTransaction(std::unique_ptr<pqxx::work>& transaction);

    // Redis operations
    bool setCache(const std::string& key, const std::string& value, int ttl_seconds = 3600);
    std::string getCache(const std::string& key);
    bool deleteCache(const std::string& key);
    bool existsInCache(const std::string& key);
    void clearCache();

    // JSON operations
    bool setCacheJson(const std::string& key, const nlohmann::json& value, int ttl_seconds = 3600);
    nlohmann::json getCacheJson(const std::string& key);

    // Batch operations
    std::vector<std::string> getMultipleCache(const std::vector<std::string>& keys);
    bool setMultipleCache(const std::vector<std::pair<std::string, std::string>>& key_values, 
                         int ttl_seconds = 3600);

    // Database schema management
    bool createTables();
    bool dropTables();
    bool migrateDatabase();
    std::string getDatabaseVersion();

    // Health check
    nlohmann::json getHealthStatus();

private:
    // Helper methods
    bool connectPostgreSQL();
    bool connectRedis();
    void prepareStatements();
    std::string escapeString(const std::string& input);
    void logError(const std::string& error);
};

// Database exception class
class DatabaseException : public std::exception {
private:
    std::string message_;

public:
    explicit DatabaseException(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }
};

} // namespace database
} // namespace healthcare