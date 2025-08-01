#pragma once

#include "DatabaseManager.h"
#include "../models/BaseEntity.h"
#include "../utils/Logger.h"
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <optional>
#include <pqxx/pqxx>

namespace healthcare::database {

template<typename T>
struct QueryResult {
    bool success = false;
    std::vector<T> data;
    std::string error_message;
    int total_count = 0;
    
    QueryResult() = default;
    QueryResult(bool success) : success(success) {}
    QueryResult(const std::vector<T>& data) : success(true), data(data), total_count(data.size()) {}
    QueryResult(const std::string& error) : success(false), error_message(error) {}
    
    bool hasData() const { return success && !data.empty(); }
    T getFirst() const { return hasData() ? data[0] : T{}; }
    std::optional<T> getFirstOptional() const { 
        return hasData() ? std::make_optional(data[0]) : std::nullopt; 
    }
};

struct PaginationParams {
    int page = 1;
    int page_size = 20;
    std::string order_by = "created_at";
    std::string order_direction = "DESC";
    
    int getOffset() const { return (page - 1) * page_size; }
    std::string getOrderClause() const { 
        return "ORDER BY " + order_by + " " + order_direction;
    }
    std::string getLimitClause() const {
        return "LIMIT " + std::to_string(page_size) + " OFFSET " + std::to_string(getOffset());
    }
};

struct FilterParams {
    std::map<std::string, std::string> string_filters;
    std::map<std::string, int> int_filters;
    std::map<std::string, bool> bool_filters;
    std::map<std::string, std::chrono::system_clock::time_point> date_filters;
    std::vector<std::string> search_fields;
    std::string search_term;
    
    bool hasFilters() const {
        return !string_filters.empty() || !int_filters.empty() || 
               !bool_filters.empty() || !date_filters.empty() || !search_term.empty();
    }
    
    std::string buildWhereClause() const;
    std::vector<std::string> getParameterValues() const;
};

template<typename T>
class BaseRepository {
public:
    explicit BaseRepository(const std::string& table_name) 
        : table_name_(table_name), db_manager_(DatabaseManager::getInstance()) {}
    
    virtual ~BaseRepository() = default;

    // Basic CRUD operations
    virtual QueryResult<T> create(const T& entity);
    virtual QueryResult<T> findById(const std::string& id);
    virtual QueryResult<T> update(const T& entity);
    virtual bool deleteById(const std::string& id);
    virtual bool softDeleteById(const std::string& id);
    
    // Bulk operations
    virtual QueryResult<T> createBatch(const std::vector<T>& entities);
    virtual QueryResult<T> updateBatch(const std::vector<T>& entities);
    virtual bool deleteBatch(const std::vector<std::string>& ids);
    
    // Query operations
    virtual QueryResult<T> findAll(const PaginationParams& pagination = {});
    virtual QueryResult<T> findByFilter(const FilterParams& filters, 
                                       const PaginationParams& pagination = {});
    virtual QueryResult<T> findByQuery(const std::string& custom_query, 
                                      const std::vector<std::string>& params = {});
    
    // Count operations
    virtual int countAll();
    virtual int countByFilter(const FilterParams& filters);
    virtual int countByQuery(const std::string& custom_query, 
                           const std::vector<std::string>& params = {});
    
    // Existence checks
    virtual bool exists(const std::string& id);
    virtual bool existsByFilter(const FilterParams& filters);
    
    // Search operations
    virtual QueryResult<T> search(const std::string& search_term, 
                                 const std::vector<std::string>& search_fields,
                                 const PaginationParams& pagination = {});
    
    // Transaction support
    virtual QueryResult<T> createInTransaction(const T& entity, DatabaseManager::Transaction& transaction);
    virtual QueryResult<T> updateInTransaction(const T& entity, DatabaseManager::Transaction& transaction);
    virtual bool deleteInTransaction(const std::string& id, DatabaseManager::Transaction& transaction);
    
    // Cache operations
    virtual void cacheEntity(const T& entity, int ttl_seconds = 3600);
    virtual std::optional<T> getCachedEntity(const std::string& id);
    virtual void removeCachedEntity(const std::string& id);
    virtual void clearEntityCache();
    
    // Utility methods
    std::string getTableName() const { return table_name_; }
    virtual std::string getSelectQuery() const;
    virtual std::string getInsertQuery() const;
    virtual std::string getUpdateQuery() const;
    virtual std::string getDeleteQuery() const;
    
    // Statistics and monitoring
    struct RepositoryStats {
        long long total_queries = 0;
        long long successful_queries = 0;
        long long failed_queries = 0;
        long long cache_hits = 0;
        long long cache_misses = 0;
        double average_query_time_ms = 0.0;
        std::chrono::system_clock::time_point last_query_time;
    };
    
    RepositoryStats getStats() const { return stats_; }
    void resetStats() { stats_ = RepositoryStats{}; }

protected:
    std::string table_name_;
    DatabaseManager& db_manager_;
    mutable RepositoryStats stats_;
    
    // Pure virtual methods that must be implemented by derived classes
    virtual T mapRowToEntity(const pqxx::row& row) const = 0;
    virtual std::vector<std::string> getInsertValues(const T& entity) const = 0;
    virtual std::vector<std::string> getUpdateValues(const T& entity) const = 0;
    virtual std::string getIdColumn() const { return "id"; }
    virtual std::vector<std::string> getColumnNames() const = 0;
    virtual std::vector<std::string> getSearchableColumns() const = 0;
    
    // Helper methods
    std::string buildSelectQuery(const std::string& where_clause = "", 
                                const std::string& order_clause = "",
                                const std::string& limit_clause = "") const;
    
    std::string buildInsertQuery(const std::vector<std::string>& columns) const;
    std::string buildUpdateQuery(const std::vector<std::string>& columns, 
                                const std::string& where_clause) const;
    std::string buildDeleteQuery(const std::string& where_clause) const;
    std::string buildCountQuery(const std::string& where_clause = "") const;
    
    std::string escapeIdentifier(const std::string& identifier) const;
    std::string buildPlaceholders(int count, int start_index = 1) const;
    
    // Error handling and logging
    void logQuery(const std::string& query, double duration_ms, bool success = true) const;
    void logError(const std::string& operation, const std::string& error) const;
    void updateStats(bool success, double duration_ms) const;
    
    // Cache key generation
    std::string generateCacheKey(const std::string& id) const;
    std::string generateListCacheKey(const std::string& suffix = "") const;
    
    // Validation helpers
    bool validateEntity(const T& entity) const;
    bool validateId(const std::string& id) const;
    
    // Query execution with error handling and timing
    template<typename Func>
    auto executeWithTiming(Func&& func) const -> decltype(func()) {
        auto start = std::chrono::high_resolution_clock::now();
        bool success = false;
        
        try {
            auto result = func();
            success = true;
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            double duration_ms = duration.count() / 1000.0;
            
            updateStats(success, duration_ms);
            return result;
            
        } catch (const std::exception& e) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            double duration_ms = duration.count() / 1000.0;
            
            updateStats(false, duration_ms);
            logError("Query execution", e.what());
            throw;
        }
    }
    
    // Batch processing helpers
    std::vector<std::vector<T>> chunkEntities(const std::vector<T>& entities, int chunk_size = 100) const;
    QueryResult<T> processBatchInChunks(const std::vector<T>& entities, 
                                       std::function<QueryResult<T>(const std::vector<T>&)> processor) const;
};

// Repository factory for creating typed repositories
class RepositoryFactory {
public:
    template<typename T, typename RepoType>
    static std::unique_ptr<RepoType> create(const std::string& table_name) {
        return std::make_unique<RepoType>(table_name);
    }
    
    template<typename RepoType>
    static std::unique_ptr<RepoType> createTyped() {
        return std::make_unique<RepoType>();
    }
};

// Convenience macros for repository operations
#define REPO_TRY_EXECUTE(operation, error_msg) \
    try { \
        return operation; \
    } catch (const std::exception& e) { \
        logError(error_msg, e.what()); \
        return QueryResult<T>(error_msg + ": " + e.what()); \
    }

#define REPO_VALIDATE_ID(id) \
    if (!validateId(id)) { \
        return QueryResult<T>("Invalid ID provided"); \
    }

#define REPO_VALIDATE_ENTITY(entity) \
    if (!validateEntity(entity)) { \
        return QueryResult<T>("Invalid entity data"); \
    }

} // namespace healthcare::database