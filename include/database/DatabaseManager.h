#pragma once

#include <string>
#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <pqxx/pqxx>
#include <sw/redis++/redis++.h>
#include <nlohmann/json.hpp>
#include <atomic>

namespace healthcare::database {

struct DatabaseConfig {
    std::string host = "localhost";
    int port = 5432;
    std::string database = "healthcare_db";
    std::string username = "postgres";
    std::string password = "";
    int max_connections = 10;
    int min_connections = 2;
    int connection_timeout_seconds = 30;
    int query_timeout_seconds = 30;
    bool enable_ssl = false;
    std::string ssl_mode = "prefer";
    bool auto_reconnect = true;
    int retry_attempts = 3;
    int retry_delay_ms = 1000;
};

struct RedisConfig {
    std::string host = "localhost";
    int port = 6379;
    std::string password = "";
    int database = 0;
    int connection_timeout_ms = 5000;
    int socket_timeout_ms = 5000;
    int max_connections = 10;
    bool enable_cluster = false;
    std::vector<std::string> cluster_nodes;
    int retry_attempts = 3;
    int retry_delay_ms = 500;
};

struct DatabaseStats {
    int total_connections;
    int active_connections;
    int idle_connections;
    int failed_connections;
    long long total_queries;
    long long successful_queries;
    long long failed_queries;
    double average_query_time_ms;
    std::chrono::system_clock::time_point last_connection_time;
    std::chrono::system_clock::time_point last_query_time;
};

class ConnectionPool {
public:
    ConnectionPool(const DatabaseConfig& config);
    ~ConnectionPool();
    
    std::unique_ptr<pqxx::connection> getConnection();
    void returnConnection(std::unique_ptr<pqxx::connection> conn);
    void closeAllConnections();
    
    int getActiveConnectionCount() const;
    int getIdleConnectionCount() const;
    int getTotalConnectionCount() const;
    bool isHealthy() const;

private:
    DatabaseConfig config_;
    std::queue<std::unique_ptr<pqxx::connection>> available_connections_;
    std::vector<std::unique_ptr<pqxx::connection>> all_connections_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic<int> active_connections_{0};
    std::atomic<bool> is_shutdown_{false};
    
    std::unique_ptr<pqxx::connection> createConnection();
    bool isConnectionValid(pqxx::connection* conn);
    void initializePool();
    void cleanupInvalidConnections();
};

class DatabaseManager {
public:
    static DatabaseManager& getInstance();
    
    // Configuration and initialization
    void configure(const DatabaseConfig& db_config, const RedisConfig& redis_config);
    bool connect();
    void disconnect();
    bool isConnected() const;
    
    // Database operations
    bool migrateDatabase();
    bool createTables();
    bool dropTables();
    bool seedDatabase();
    bool backupDatabase(const std::string& backup_file);
    bool restoreDatabase(const std::string& backup_file);
    
    // Connection management
    std::unique_ptr<pqxx::connection> getConnection();
    void returnConnection(std::unique_ptr<pqxx::connection> conn);
    
    // Redis operations
    sw::redis::Redis& getRedisClient();
    bool isRedisConnected() const;
    void flushRedisCache();
    
    // Transaction management
    class Transaction {
    public:
        Transaction(DatabaseManager& manager);
        ~Transaction();
        
        pqxx::work& getWork() { return *work_; }
        void commit();
        void rollback();
        bool isCommitted() const { return committed_; }
        bool isRolledBack() const { return rolled_back_; }
        
    private:
        std::unique_ptr<pqxx::connection> connection_;
        std::unique_ptr<pqxx::work> work_;
        DatabaseManager& manager_;
        bool committed_ = false;
        bool rolled_back_ = false;
    };
    
    std::unique_ptr<Transaction> beginTransaction();
    
    // Query execution helpers
    pqxx::result executeQuery(const std::string& query);
    pqxx::result executeQuery(const std::string& query, const std::vector<std::string>& params);
    bool executeNonQuery(const std::string& query);
    bool executeNonQuery(const std::string& query, const std::vector<std::string>& params);
    
    // Prepared statements
    void prepareStatement(const std::string& name, const std::string& query);
    pqxx::result executePrepared(const std::string& name, const std::vector<std::string>& params = {});
    
    // Health monitoring
    bool performHealthCheck();
    DatabaseStats getStats() const;
    nlohmann::json getHealthStatus() const;
    
    // Cache operations (Redis)
    bool setCache(const std::string& key, const std::string& value, int ttl_seconds = 3600);
    std::string getCache(const std::string& key);
    bool deleteCache(const std::string& key);
    bool existsCache(const std::string& key);
    void clearCache(const std::string& pattern = "*");
    
    // JSON cache operations
    bool setCacheJson(const std::string& key, const nlohmann::json& data, int ttl_seconds = 3600);
    nlohmann::json getCacheJson(const std::string& key);
    
    // Bulk operations
    bool bulkInsert(const std::string& table, const std::vector<std::vector<std::string>>& data);
    bool bulkUpdate(const std::string& table, const std::vector<std::pair<std::string, std::vector<std::string>>>& updates);
    
    // Database maintenance
    bool vacuum();
    bool reindex();
    bool analyze();
    bool optimizeDatabase();
    
    // Event logging
    void logQuery(const std::string& query, double duration_ms, bool success = true);
    void logConnection(bool success = true);
    void logError(const std::string& operation, const std::string& error);

private:
    DatabaseManager() = default;
    ~DatabaseManager() = default;
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    
    DatabaseConfig db_config_;
    RedisConfig redis_config_;
    std::unique_ptr<ConnectionPool> connection_pool_;
    std::unique_ptr<sw::redis::Redis> redis_client_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    DatabaseStats stats_;
    
    // Helper methods
    std::string buildConnectionString() const;
    std::string buildRedisConnectionString() const;
    bool testConnection();
    bool testRedisConnection();
    void updateStats(bool query_success, double duration_ms = 0.0);
    std::string escapeString(const std::string& input);
    
    // Migration scripts
    std::vector<std::string> getMigrationScripts() const;
    std::string getCreateTablesScript() const;
    std::string getDropTablesScript() const;
    std::vector<std::string> getSeedDataScripts() const;
    
    // Error handling
    void handleDatabaseError(const std::exception& e, const std::string& operation);
    void handleRedisError(const std::exception& e, const std::string& operation);
};

// Database helper functions
std::string formatTimestamp(const std::chrono::system_clock::time_point& time_point);
std::chrono::system_clock::time_point parseTimestamp(const std::string& timestamp);
std::string generatePlaceholders(int count);
std::vector<std::string> splitQuery(const std::string& multi_query);

// Database exceptions
class DatabaseException : public std::exception {
public:
    explicit DatabaseException(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }
    
private:
    std::string message_;
};

class ConnectionException : public DatabaseException {
public:
    explicit ConnectionException(const std::string& message) 
        : DatabaseException("Connection error: " + message) {}
};

class QueryException : public DatabaseException {
public:
    explicit QueryException(const std::string& message) 
        : DatabaseException("Query error: " + message) {}
};

class TransactionException : public DatabaseException {
public:
    explicit TransactionException(const std::string& message) 
        : DatabaseException("Transaction error: " + message) {}
};

} // namespace healthcare::database