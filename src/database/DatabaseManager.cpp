#include "../../include/database/DatabaseManager.h"
#include "../../include/utils/Logger.h"
#include <sstream>
#include <fstream>
#include <filesystem>

namespace healthcare::database {

// ConnectionPool implementation
ConnectionPool::ConnectionPool(const DatabaseConfig& config) : config_(config) {
    initializePool();
}

ConnectionPool::~ConnectionPool() {
    closeAllConnections();
}

std::unique_ptr<pqxx::connection> ConnectionPool::getConnection() {
    std::unique_lock<std::mutex> lock(mutex_);
    
    // Wait for available connection
    condition_.wait(lock, [this] {
        return !available_connections_.empty() || is_shutdown_;
    });
    
    if (is_shutdown_) {
        throw ConnectionException("Connection pool is shut down");
    }
    
    // Get connection from pool
    auto conn = std::move(available_connections_.front());
    available_connections_.pop();
    
    // Validate connection
    if (!isConnectionValid(conn.get())) {
        // Try to create a new connection
        conn = createConnection();
    }
    
    active_connections_++;
    return conn;
}

void ConnectionPool::returnConnection(std::unique_ptr<pqxx::connection> conn) {
    if (!conn) return;
    
    std::unique_lock<std::mutex> lock(mutex_);
    
    if (is_shutdown_) {
        return; // Just let the connection be destroyed
    }
    
    // Check if connection is still valid
    if (isConnectionValid(conn.get())) {
        available_connections_.push(std::move(conn));
    } else {
        // Replace with new connection
        try {
            available_connections_.push(createConnection());
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create replacement connection: {}", e.what());
        }
    }
    
    active_connections_--;
    condition_.notify_one();
}

void ConnectionPool::closeAllConnections() {
    std::unique_lock<std::mutex> lock(mutex_);
    is_shutdown_ = true;
    
    // Clear available connections
    while (!available_connections_.empty()) {
        available_connections_.pop();
    }
    
    all_connections_.clear();
    condition_.notify_all();
}

int ConnectionPool::getActiveConnectionCount() const {
    return active_connections_;
}

int ConnectionPool::getIdleConnectionCount() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return static_cast<int>(available_connections_.size());
}

int ConnectionPool::getTotalConnectionCount() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return static_cast<int>(all_connections_.size());
}

bool ConnectionPool::isHealthy() const {
    return !is_shutdown_ && getTotalConnectionCount() >= config_.min_connections;
}

std::unique_ptr<pqxx::connection> ConnectionPool::createConnection() {
    std::ostringstream conn_str;
    conn_str << "host=" << config_.host
             << " port=" << config_.port
             << " dbname=" << config_.database
             << " user=" << config_.username;
    
    if (!config_.password.empty()) {
        conn_str << " password=" << config_.password;
    }
    
    if (config_.enable_ssl) {
        conn_str << " sslmode=" << config_.ssl_mode;
    }
    
    conn_str << " connect_timeout=" << config_.connection_timeout_seconds;
    
    auto conn = std::make_unique<pqxx::connection>(conn_str.str());
    return conn;
}

bool ConnectionPool::isConnectionValid(pqxx::connection* conn) {
    if (!conn) return false;
    
    try {
        pqxx::work txn(*conn);
        txn.exec("SELECT 1");
        txn.commit();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void ConnectionPool::initializePool() {
    std::unique_lock<std::mutex> lock(mutex_);
    
    // Create initial connections
    for (int i = 0; i < config_.min_connections; ++i) {
        try {
            auto conn = createConnection();
            available_connections_.push(std::move(conn));
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create initial connection: {}", e.what());
            throw ConnectionException("Failed to initialize connection pool");
        }
    }
    
    LOG_INFO("Connection pool initialized with {} connections", config_.min_connections);
}

void ConnectionPool::cleanupInvalidConnections() {
    std::unique_lock<std::mutex> lock(mutex_);
    
    std::queue<std::unique_ptr<pqxx::connection>> valid_connections;
    
    while (!available_connections_.empty()) {
        auto conn = std::move(available_connections_.front());
        available_connections_.pop();
        
        if (isConnectionValid(conn.get())) {
            valid_connections.push(std::move(conn));
        }
    }
    
    available_connections_ = std::move(valid_connections);
}

// DatabaseManager implementation
DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

void DatabaseManager::configure(const DatabaseConfig& db_config, const RedisConfig& redis_config) {
    db_config_ = db_config;
    redis_config_ = redis_config;
}

bool DatabaseManager::connect() {
    try {
        // Initialize connection pool
        connection_pool_ = std::make_unique<ConnectionPool>(db_config_);
        
        // Test database connection
        if (!testConnection()) {
            LOG_ERROR("Failed to establish database connection");
            return false;
        }
        
        // Initialize Redis client
        sw::redis::ConnectionOptions redis_opts;
        redis_opts.host = redis_config_.host;
        redis_opts.port = redis_config_.port;
        redis_opts.password = redis_config_.password;
        redis_opts.db = redis_config_.database;
        redis_opts.connect_timeout = std::chrono::milliseconds(redis_config_.connection_timeout_ms);
        redis_opts.socket_timeout = std::chrono::milliseconds(redis_config_.socket_timeout_ms);
        
        sw::redis::ConnectionPoolOptions pool_opts;
        pool_opts.size = redis_config_.max_connections;
        
        redis_client_ = std::make_unique<sw::redis::Redis>(redis_opts, pool_opts);
        
        // Test Redis connection
        if (!testRedisConnection()) {
            LOG_WARN("Redis connection failed, caching will be disabled");
        }
        
        logConnection(true);
        LOG_INFO("Database manager connected successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to connect to database: {}", e.what());
        logConnection(false);
        return false;
    }
}

void DatabaseManager::disconnect() {
    if (connection_pool_) {
        connection_pool_->closeAllConnections();
        connection_pool_.reset();
    }
    
    redis_client_.reset();
    LOG_INFO("Database manager disconnected");
}

bool DatabaseManager::isConnected() const {
    return connection_pool_ && connection_pool_->isHealthy();
}

bool DatabaseManager::migrateDatabase() {
    try {
        auto scripts = getMigrationScripts();
        auto conn = getConnection();
        
        for (const auto& script : scripts) {
            try {
                pqxx::work txn(*conn);
                txn.exec(script);
                txn.commit();
                LOG_INFO("Migration script executed successfully");
            } catch (const std::exception& e) {
                LOG_ERROR("Migration script failed: {}", e.what());
                returnConnection(std::move(conn));
                return false;
            }
        }
        
        returnConnection(std::move(conn));
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Database migration failed: {}", e.what());
        return false;
    }
}

bool DatabaseManager::createTables() {
    try {
        auto script = getCreateTablesScript();
        auto conn = getConnection();
        
        pqxx::work txn(*conn);
        txn.exec(script);
        txn.commit();
        
        returnConnection(std::move(conn));
        LOG_INFO("Database tables created successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create tables: {}", e.what());
        return false;
    }
}

bool DatabaseManager::dropTables() {
    try {
        auto script = getDropTablesScript();
        auto conn = getConnection();
        
        pqxx::work txn(*conn);
        txn.exec(script);
        txn.commit();
        
        returnConnection(std::move(conn));
        LOG_INFO("Database tables dropped successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to drop tables: {}", e.what());
        return false;
    }
}

bool DatabaseManager::seedDatabase() {
    try {
        auto scripts = getSeedDataScripts();
        auto conn = getConnection();
        
        for (const auto& script : scripts) {
            pqxx::work txn(*conn);
            txn.exec(script);
            txn.commit();
        }
        
        returnConnection(std::move(conn));
        LOG_INFO("Database seeded successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to seed database: {}", e.what());
        return false;
    }
}

bool DatabaseManager::backupDatabase(const std::string& backup_file) {
    try {
        std::ostringstream cmd;
        cmd << "pg_dump"
            << " -h " << db_config_.host
            << " -p " << db_config_.port
            << " -U " << db_config_.username
            << " -d " << db_config_.database
            << " -f " << backup_file;
        
        int result = std::system(cmd.str().c_str());
        
        if (result == 0) {
            LOG_INFO("Database backed up to: {}", backup_file);
            return true;
        } else {
            LOG_ERROR("Database backup failed with code: {}", result);
            return false;
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to backup database: {}", e.what());
        return false;
    }
}

bool DatabaseManager::restoreDatabase(const std::string& backup_file) {
    try {
        std::ostringstream cmd;
        cmd << "psql"
            << " -h " << db_config_.host
            << " -p " << db_config_.port
            << " -U " << db_config_.username
            << " -d " << db_config_.database
            << " -f " << backup_file;
        
        int result = std::system(cmd.str().c_str());
        
        if (result == 0) {
            LOG_INFO("Database restored from: {}", backup_file);
            return true;
        } else {
            LOG_ERROR("Database restore failed with code: {}", result);
            return false;
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to restore database: {}", e.what());
        return false;
    }
}

std::unique_ptr<pqxx::connection> DatabaseManager::getConnection() {
    if (!connection_pool_) {
        throw ConnectionException("Connection pool not initialized");
    }
    return connection_pool_->getConnection();
}

void DatabaseManager::returnConnection(std::unique_ptr<pqxx::connection> conn) {
    if (connection_pool_) {
        connection_pool_->returnConnection(std::move(conn));
    }
}

sw::redis::Redis& DatabaseManager::getRedisClient() {
    if (!redis_client_) {
        throw ConnectionException("Redis client not initialized");
    }
    return *redis_client_;
}

bool DatabaseManager::isRedisConnected() const {
    if (!redis_client_) return false;
    
    try {
        redis_client_->ping();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void DatabaseManager::flushRedisCache() {
    if (redis_client_) {
        try {
            redis_client_->flushdb();
            LOG_INFO("Redis cache flushed");
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to flush Redis cache: {}", e.what());
        }
    }
}

// Transaction implementation
DatabaseManager::Transaction::Transaction(DatabaseManager& manager)
    : manager_(manager) {
    connection_ = manager.getConnection();
    work_ = std::make_unique<pqxx::work>(*connection_);
}

DatabaseManager::Transaction::~Transaction() {
    if (!committed_ && !rolled_back_) {
        rollback();
    }
    manager_.returnConnection(std::move(connection_));
}

void DatabaseManager::Transaction::commit() {
    if (!committed_ && !rolled_back_) {
        work_->commit();
        committed_ = true;
    }
}

void DatabaseManager::Transaction::rollback() {
    if (!committed_ && !rolled_back_) {
        work_->abort();
        rolled_back_ = true;
    }
}

std::unique_ptr<DatabaseManager::Transaction> DatabaseManager::beginTransaction() {
    return std::make_unique<Transaction>(*this);
}

pqxx::result DatabaseManager::executeQuery(const std::string& query) {
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        auto conn = getConnection();
        pqxx::work txn(*conn);
        auto result = txn.exec(query);
        txn.commit();
        returnConnection(std::move(conn));
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double duration_ms = duration.count() / 1000.0;
        
        logQuery(query, duration_ms, true);
        updateStats(true, duration_ms);
        
        return result;
        
    } catch (const std::exception& e) {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double duration_ms = duration.count() / 1000.0;
        
        logQuery(query, duration_ms, false);
        updateStats(false, duration_ms);
        handleDatabaseError(e, "executeQuery");
        throw;
    }
}

pqxx::result DatabaseManager::executeQuery(const std::string& query, const std::vector<std::string>& params) {
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        auto conn = getConnection();
        pqxx::work txn(*conn);
        
        // Build parameterized query
        std::string parameterized_query = query;
        for (size_t i = 0; i < params.size(); ++i) {
            std::string placeholder = "$" + std::to_string(i + 1);
            size_t pos = parameterized_query.find(placeholder);
            if (pos != std::string::npos) {
                parameterized_query.replace(pos, placeholder.length(), txn.quote(params[i]));
            }
        }
        
        auto result = txn.exec(parameterized_query);
        txn.commit();
        returnConnection(std::move(conn));
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double duration_ms = duration.count() / 1000.0;
        
        logQuery(parameterized_query, duration_ms, true);
        updateStats(true, duration_ms);
        
        return result;
        
    } catch (const std::exception& e) {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double duration_ms = duration.count() / 1000.0;
        
        logQuery(query, duration_ms, false);
        updateStats(false, duration_ms);
        handleDatabaseError(e, "executeQuery");
        throw;
    }
}

bool DatabaseManager::executeNonQuery(const std::string& query) {
    try {
        executeQuery(query);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool DatabaseManager::executeNonQuery(const std::string& query, const std::vector<std::string>& params) {
    try {
        executeQuery(query, params);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void DatabaseManager::prepareStatement(const std::string& name, const std::string& query) {
    auto conn = getConnection();
    try {
        conn->prepare(name, query);
        returnConnection(std::move(conn));
    } catch (const std::exception& e) {
        returnConnection(std::move(conn));
        throw;
    }
}

pqxx::result DatabaseManager::executePrepared(const std::string& name, const std::vector<std::string>& params) {
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        auto conn = getConnection();
        pqxx::work txn(*conn);
        
        pqxx::result result;
        if (params.empty()) {
            result = txn.exec_prepared(name);
        } else {
            // Convert string params to const char* for pqxx
            std::vector<const char*> param_ptrs;
            for (const auto& param : params) {
                param_ptrs.push_back(param.c_str());
            }
            result = txn.exec_prepared(name, param_ptrs);
        }
        
        txn.commit();
        returnConnection(std::move(conn));
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double duration_ms = duration.count() / 1000.0;
        
        logQuery("EXECUTE " + name, duration_ms, true);
        updateStats(true, duration_ms);
        
        return result;
        
    } catch (const std::exception& e) {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double duration_ms = duration.count() / 1000.0;
        
        logQuery("EXECUTE " + name, duration_ms, false);
        updateStats(false, duration_ms);
        handleDatabaseError(e, "executePrepared");
        throw;
    }
}

bool DatabaseManager::performHealthCheck() {
    bool db_healthy = testConnection();
    bool redis_healthy = testRedisConnection();
    
    return db_healthy; // Redis is optional
}

DatabaseStats DatabaseManager::getStats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

nlohmann::json DatabaseManager::getHealthStatus() const {
    nlohmann::json status;
    
    status["connected"] = isConnected();
    status["redis_connected"] = isRedisConnected();
    
    if (connection_pool_) {
        status["connections"]["active"] = connection_pool_->getActiveConnectionCount();
        status["connections"]["idle"] = connection_pool_->getIdleConnectionCount();
        status["connections"]["total"] = connection_pool_->getTotalConnectionCount();
    }
    
    auto stats = getStats();
    status["stats"]["total_queries"] = stats.total_queries;
    status["stats"]["successful_queries"] = stats.successful_queries;
    status["stats"]["failed_queries"] = stats.failed_queries;
    status["stats"]["average_query_time_ms"] = stats.average_query_time_ms;
    
    return status;
}

bool DatabaseManager::setCache(const std::string& key, const std::string& value, int ttl_seconds) {
    if (!redis_client_) return false;
    
    try {
        if (ttl_seconds > 0) {
            redis_client_->setex(key, ttl_seconds, value);
        } else {
            redis_client_->set(key, value);
        }
        return true;
    } catch (const std::exception& e) {
        handleRedisError(e, "setCache");
        return false;
    }
}

std::string DatabaseManager::getCache(const std::string& key) {
    if (!redis_client_) return "";
    
    try {
        auto val = redis_client_->get(key);
        return val.value_or("");
    } catch (const std::exception& e) {
        handleRedisError(e, "getCache");
        return "";
    }
}

bool DatabaseManager::deleteCache(const std::string& key) {
    if (!redis_client_) return false;
    
    try {
        redis_client_->del(key);
        return true;
    } catch (const std::exception& e) {
        handleRedisError(e, "deleteCache");
        return false;
    }
}

bool DatabaseManager::existsCache(const std::string& key) {
    if (!redis_client_) return false;
    
    try {
        return redis_client_->exists(key) > 0;
    } catch (const std::exception& e) {
        handleRedisError(e, "existsCache");
        return false;
    }
}

void DatabaseManager::clearCache(const std::string& pattern) {
    if (!redis_client_) return;
    
    try {
        std::vector<std::string> keys;
        redis_client_->keys(pattern, std::back_inserter(keys));
        
        if (!keys.empty()) {
            redis_client_->del(keys.begin(), keys.end());
        }
    } catch (const std::exception& e) {
        handleRedisError(e, "clearCache");
    }
}

bool DatabaseManager::setCacheJson(const std::string& key, const nlohmann::json& data, int ttl_seconds) {
    return setCache(key, data.dump(), ttl_seconds);
}

nlohmann::json DatabaseManager::getCacheJson(const std::string& key) {
    auto value = getCache(key);
    if (value.empty()) return nlohmann::json();
    
    try {
        return nlohmann::json::parse(value);
    } catch (const std::exception&) {
        return nlohmann::json();
    }
}

bool DatabaseManager::bulkInsert(const std::string& table, const std::vector<std::vector<std::string>>& data) {
    if (data.empty()) return true;
    
    try {
        auto conn = getConnection();
        pqxx::work txn(*conn);
        
        // Build bulk insert query
        std::ostringstream query;
        query << "INSERT INTO " << table << " VALUES ";
        
        for (size_t i = 0; i < data.size(); ++i) {
            if (i > 0) query << ", ";
            query << "(";
            
            for (size_t j = 0; j < data[i].size(); ++j) {
                if (j > 0) query << ", ";
                query << txn.quote(data[i][j]);
            }
            
            query << ")";
        }
        
        txn.exec(query.str());
        txn.commit();
        returnConnection(std::move(conn));
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Bulk insert failed: {}", e.what());
        return false;
    }
}

bool DatabaseManager::bulkUpdate(const std::string& table, const std::vector<std::pair<std::string, std::vector<std::string>>>& updates) {
    if (updates.empty()) return true;
    
    try {
        auto transaction = beginTransaction();
        
        for (const auto& [condition, values] : updates) {
            std::ostringstream query;
            query << "UPDATE " << table << " SET ";
            
            // Assuming values are in format: column=value
            for (size_t i = 0; i < values.size(); ++i) {
                if (i > 0) query << ", ";
                query << values[i];
            }
            
            query << " WHERE " << condition;
            
            transaction->getWork().exec(query.str());
        }
        
        transaction->commit();
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Bulk update failed: {}", e.what());
        return false;
    }
}

bool DatabaseManager::vacuum() {
    try {
        auto conn = getConnection();
        pqxx::nontransaction ntxn(*conn);
        ntxn.exec("VACUUM ANALYZE");
        returnConnection(std::move(conn));
        LOG_INFO("Database vacuum completed");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Vacuum failed: {}", e.what());
        return false;
    }
}

bool DatabaseManager::reindex() {
    try {
        auto conn = getConnection();
        pqxx::nontransaction ntxn(*conn);
        ntxn.exec("REINDEX DATABASE " + db_config_.database);
        returnConnection(std::move(conn));
        LOG_INFO("Database reindex completed");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Reindex failed: {}", e.what());
        return false;
    }
}

bool DatabaseManager::analyze() {
    try {
        auto conn = getConnection();
        pqxx::nontransaction ntxn(*conn);
        ntxn.exec("ANALYZE");
        returnConnection(std::move(conn));
        LOG_INFO("Database analyze completed");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Analyze failed: {}", e.what());
        return false;
    }
}

bool DatabaseManager::optimizeDatabase() {
    bool success = true;
    success &= vacuum();
    success &= analyze();
    success &= reindex();
    return success;
}

void DatabaseManager::logQuery(const std::string& query, double duration_ms, bool success) {
    LOG_DEBUG("Query: {} ({}ms) - {}", query.substr(0, 100), duration_ms, success ? "SUCCESS" : "FAILED");
}

void DatabaseManager::logConnection(bool success) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    if (success) {
        stats_.total_connections++;
        stats_.last_connection_time = std::chrono::system_clock::now();
    } else {
        stats_.failed_connections++;
    }
}

void DatabaseManager::logError(const std::string& operation, const std::string& error) {
    LOG_ERROR("Database operation '{}' failed: {}", operation, error);
}

std::string DatabaseManager::buildConnectionString() const {
    std::ostringstream conn_str;
    conn_str << "host=" << db_config_.host
             << " port=" << db_config_.port
             << " dbname=" << db_config_.database
             << " user=" << db_config_.username;
    
    if (!db_config_.password.empty()) {
        conn_str << " password=" << db_config_.password;
    }
    
    if (db_config_.enable_ssl) {
        conn_str << " sslmode=" << db_config_.ssl_mode;
    }
    
    return conn_str.str();
}

std::string DatabaseManager::buildRedisConnectionString() const {
    std::ostringstream conn_str;
    if (!redis_config_.password.empty()) {
        conn_str << redis_config_.password << "@";
    }
    conn_str << redis_config_.host << ":" << redis_config_.port << "/" << redis_config_.database;
    return conn_str.str();
}

bool DatabaseManager::testConnection() {
    try {
        auto conn = getConnection();
        pqxx::work txn(*conn);
        txn.exec("SELECT 1");
        txn.commit();
        returnConnection(std::move(conn));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Database connection test failed: {}", e.what());
        return false;
    }
}

bool DatabaseManager::testRedisConnection() {
    if (!redis_client_) return false;
    
    try {
        redis_client_->ping();
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Redis connection test failed: {}", e.what());
        return false;
    }
}

void DatabaseManager::updateStats(bool query_success, double duration_ms) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    stats_.total_queries++;
    if (query_success) {
        stats_.successful_queries++;
    } else {
        stats_.failed_queries++;
    }
    
    // Update average query time
    double total_time = stats_.average_query_time_ms * (stats_.total_queries - 1) + duration_ms;
    stats_.average_query_time_ms = total_time / stats_.total_queries;
    
    stats_.last_query_time = std::chrono::system_clock::now();
}

std::string DatabaseManager::escapeString(const std::string& input) {
    // This is a simplified version. In production, use proper escaping
    std::string escaped = input;
    size_t pos = 0;
    while ((pos = escaped.find("'", pos)) != std::string::npos) {
        escaped.replace(pos, 1, "''");
        pos += 2;
    }
    return escaped;
}

std::vector<std::string> DatabaseManager::getMigrationScripts() const {
    std::vector<std::string> scripts;
    
    // Create migrations table
    scripts.push_back(R"(
        CREATE TABLE IF NOT EXISTS schema_migrations (
            version VARCHAR(255) PRIMARY KEY,
            applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    )");
    
    // Add more migration scripts here
    
    return scripts;
}

std::string DatabaseManager::getCreateTablesScript() const {
    return R"(
        -- Users table
        CREATE TABLE IF NOT EXISTS users (
            id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
            email VARCHAR(255) UNIQUE NOT NULL,
            password_hash VARCHAR(255) NOT NULL,
            salt VARCHAR(255) NOT NULL,
            first_name VARCHAR(100) NOT NULL,
            last_name VARCHAR(100) NOT NULL,
            phone_number VARCHAR(20),
            role VARCHAR(50) NOT NULL,
            gender VARCHAR(20),
            date_of_birth DATE,
            address TEXT,
            city VARCHAR(100),
            state VARCHAR(100),
            pincode VARCHAR(20),
            profile_image_url TEXT,
            is_verified BOOLEAN DEFAULT FALSE,
            verification_token VARCHAR(255),
            fcm_token TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            is_deleted BOOLEAN DEFAULT FALSE
        );

        -- Doctors table
        CREATE TABLE IF NOT EXISTS doctors (
            id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
            user_id UUID REFERENCES users(id) ON DELETE CASCADE,
            medical_license_number VARCHAR(100) UNIQUE NOT NULL,
            qualification TEXT,
            years_of_experience INTEGER DEFAULT 0,
            status VARCHAR(50) DEFAULT 'PENDING_VERIFICATION',
            consultation_fee DECIMAL(10, 2),
            consultation_duration_minutes INTEGER DEFAULT 30,
            consultation_types TEXT[],
            rating DECIMAL(3, 2) DEFAULT 0.0,
            total_reviews INTEGER DEFAULT 0,
            availability_pattern JSONB,
            is_available_today BOOLEAN DEFAULT FALSE,
            bio TEXT,
            languages TEXT,
            specializations JSONB,
            clinic_ids UUID[],
            documents JSONB,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            is_deleted BOOLEAN DEFAULT FALSE
        );

        -- Clinics table
        CREATE TABLE IF NOT EXISTS clinics (
            id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
            name VARCHAR(255) NOT NULL,
            description TEXT,
            registration_number VARCHAR(100) UNIQUE,
            status VARCHAR(50) DEFAULT 'PENDING_VERIFICATION',
            contact_info JSONB,
            address JSONB,
            working_hours JSONB,
            facilities JSONB,
            services TEXT[],
            logo_url TEXT,
            image_urls TEXT[],
            rating DECIMAL(3, 2) DEFAULT 0.0,
            total_reviews INTEGER DEFAULT 0,
            owner_id UUID REFERENCES users(id),
            doctor_ids UUID[],
            has_emergency_services BOOLEAN DEFAULT FALSE,
            emergency_contact VARCHAR(20),
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            is_deleted BOOLEAN DEFAULT FALSE
        );

        -- Appointments table
        CREATE TABLE IF NOT EXISTS appointments (
            id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
            user_id UUID REFERENCES users(id),
            doctor_id UUID REFERENCES doctors(id),
            clinic_id UUID REFERENCES clinics(id),
            appointment_date DATE NOT NULL,
            start_time TIMESTAMP NOT NULL,
            end_time TIMESTAMP NOT NULL,
            type VARCHAR(50) NOT NULL,
            status VARCHAR(50) DEFAULT 'PENDING',
            symptoms TEXT,
            notes TEXT,
            is_emergency BOOLEAN DEFAULT FALSE,
            patient_age VARCHAR(10),
            patient_gender VARCHAR(20),
            consultation_fee DECIMAL(10, 2),
            payment_info JSONB,
            confirmation_code VARCHAR(50) UNIQUE,
            booked_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            confirmed_at TIMESTAMP,
            consultation_info JSONB,
            cancellation_info JSONB,
            prescription_id UUID,
            follow_up_date TIMESTAMP,
            follow_up_notes TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            is_deleted BOOLEAN DEFAULT FALSE
        );

        -- Prescriptions table
        CREATE TABLE IF NOT EXISTS prescriptions (
            id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
            appointment_id UUID REFERENCES appointments(id),
            doctor_id UUID REFERENCES doctors(id),
            patient_id UUID REFERENCES users(id),
            clinic_id UUID REFERENCES clinics(id),
            status VARCHAR(50) DEFAULT 'ACTIVE',
            diagnosis JSONB,
            vital_signs JSONB,
            medicines JSONB,
            doctor_notes TEXT,
            general_instructions TEXT,
            diet_recommendations TEXT,
            lifestyle_advice TEXT,
            follow_up_instruction JSONB,
            lab_tests TEXT[],
            imaging_tests TEXT[],
            issued_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            valid_until TIMESTAMP,
            prescription_number VARCHAR(100) UNIQUE,
            digital_signature TEXT,
            qr_code TEXT,
            is_digitally_verified BOOLEAN DEFAULT FALSE,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            is_deleted BOOLEAN DEFAULT FALSE
        );

        -- Create indexes
        CREATE INDEX idx_users_email ON users(email);
        CREATE INDEX idx_users_phone ON users(phone_number);
        CREATE INDEX idx_doctors_user_id ON doctors(user_id);
        CREATE INDEX idx_doctors_status ON doctors(status);
        CREATE INDEX idx_appointments_user_id ON appointments(user_id);
        CREATE INDEX idx_appointments_doctor_id ON appointments(doctor_id);
        CREATE INDEX idx_appointments_date ON appointments(appointment_date);
        CREATE INDEX idx_appointments_status ON appointments(status);
        CREATE INDEX idx_prescriptions_appointment_id ON prescriptions(appointment_id);
        CREATE INDEX idx_prescriptions_patient_id ON prescriptions(patient_id);
    )";
}

std::string DatabaseManager::getDropTablesScript() const {
    return R"(
        DROP TABLE IF EXISTS prescriptions CASCADE;
        DROP TABLE IF EXISTS appointments CASCADE;
        DROP TABLE IF EXISTS clinics CASCADE;
        DROP TABLE IF EXISTS doctors CASCADE;
        DROP TABLE IF EXISTS users CASCADE;
        DROP TABLE IF EXISTS schema_migrations CASCADE;
    )";
}

std::vector<std::string> DatabaseManager::getSeedDataScripts() const {
    std::vector<std::string> scripts;
    
    // Add seed data scripts here
    scripts.push_back(R"(
        -- Insert sample specializations
        -- This would be part of seed data
    )");
    
    return scripts;
}

void DatabaseManager::handleDatabaseError(const std::exception& e, const std::string& operation) {
    logError(operation, e.what());
    
    // Check if it's a connection error and attempt reconnect
    std::string error_msg = e.what();
    if (error_msg.find("connection") != std::string::npos ||
        error_msg.find("Connection") != std::string::npos) {
        LOG_WARN("Connection error detected, attempting to reconnect...");
        // Could implement reconnection logic here
    }
}

void DatabaseManager::handleRedisError(const std::exception& e, const std::string& operation) {
    LOG_WARN("Redis operation '{}' failed: {}", operation, e.what());
    // Redis errors are non-critical, just log them
}

// Helper functions
std::string formatTimestamp(const std::chrono::system_clock::time_point& time_point) {
    auto time_t = std::chrono::system_clock::to_time_t(time_point);
    std::tm* tm = std::localtime(&time_t);
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::chrono::system_clock::time_point parseTimestamp(const std::string& timestamp) {
    std::tm tm = {};
    std::istringstream iss(timestamp);
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::string generatePlaceholders(int count) {
    std::ostringstream oss;
    for (int i = 1; i <= count; ++i) {
        if (i > 1) oss << ", ";
        oss << "$" << i;
    }
    return oss.str();
}

std::vector<std::string> splitQuery(const std::string& multi_query) {
    std::vector<std::string> queries;
    std::string current_query;
    bool in_string = false;
    char string_delimiter = '\0';
    
    for (size_t i = 0; i < multi_query.length(); ++i) {
        char c = multi_query[i];
        
        if (!in_string && (c == '\'' || c == '"')) {
            in_string = true;
            string_delimiter = c;
        } else if (in_string && c == string_delimiter) {
            // Check if it's escaped
            if (i > 0 && multi_query[i-1] != '\\') {
                in_string = false;
            }
        }
        
        current_query += c;
        
        if (!in_string && c == ';') {
            queries.push_back(current_query);
            current_query.clear();
        }
    }
    
    if (!current_query.empty()) {
        queries.push_back(current_query);
    }
    
    return queries;
}

} // namespace healthcare::database