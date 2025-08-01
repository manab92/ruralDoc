#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <functional>
#include "DatabaseManager.h"
#include "../models/BaseEntity.h"

namespace healthcare {
namespace database {

template<typename T>
class BaseRepository {
protected:
    DatabaseManager& db_manager_;
    std::string table_name_;
    std::string cache_prefix_;

public:
    explicit BaseRepository(const std::string& table_name, const std::string& cache_prefix = "")
        : db_manager_(DatabaseManager::getInstance()), 
          table_name_(table_name),
          cache_prefix_(cache_prefix.empty() ? table_name + ":" : cache_prefix + ":") {}

    virtual ~BaseRepository() = default;

    // Pure virtual methods to be implemented by derived classes
    virtual std::unique_ptr<T> fromRow(const pqxx::row& row) = 0;
    virtual std::string getInsertQuery(const T& entity) = 0;
    virtual std::string getUpdateQuery(const T& entity) = 0;
    virtual std::vector<std::string> getInsertParams(const T& entity) = 0;
    virtual std::vector<std::string> getUpdateParams(const T& entity) = 0;

    // CRUD operations
    virtual std::optional<std::unique_ptr<T>> findById(const std::string& id) {
        // Try cache first
        if (!cache_prefix_.empty()) {
            std::string cache_key = cache_prefix_ + id;
            auto cached_json = db_manager_.getCacheJson(cache_key);
            if (!cached_json.is_null()) {
                auto entity = std::make_unique<T>();
                entity->fromJson(cached_json);
                return entity;
            }
        }

        // Query database
        std::string query = "SELECT * FROM " + table_name_ + " WHERE id = $1";
        auto result = db_manager_.executeQuery(query, {id});
        
        if (result.empty()) {
            return std::nullopt;
        }

        auto entity = fromRow(result[0]);
        
        // Cache the result
        if (!cache_prefix_.empty() && entity) {
            std::string cache_key = cache_prefix_ + id;
            db_manager_.setCacheJson(cache_key, entity->toJson(), 3600);
        }

        return entity;
    }

    virtual std::vector<std::unique_ptr<T>> findAll(int limit = 100, int offset = 0) {
        std::string query = "SELECT * FROM " + table_name_ + 
                           " ORDER BY created_at DESC LIMIT $1 OFFSET $2";
        auto result = db_manager_.executeQuery(query, {std::to_string(limit), std::to_string(offset)});
        
        std::vector<std::unique_ptr<T>> entities;
        for (const auto& row : result) {
            entities.push_back(fromRow(row));
        }
        
        return entities;
    }

    virtual std::vector<std::unique_ptr<T>> findByCondition(const std::string& condition, 
                                                           const std::vector<std::string>& params = {}) {
        std::string query = "SELECT * FROM " + table_name_ + " WHERE " + condition;
        auto result = db_manager_.executeQuery(query, params);
        
        std::vector<std::unique_ptr<T>> entities;
        for (const auto& row : result) {
            entities.push_back(fromRow(row));
        }
        
        return entities;
    }

    virtual bool create(T& entity) {
        try {
            auto transaction = db_manager_.beginTransaction();
            
            // Generate ID if not set
            if (entity.getId().empty()) {
                entity.setId(generateUUID());
            }
            
            // Set timestamps
            auto now = std::chrono::system_clock::now();
            entity.setCreatedAt(now);
            entity.setUpdatedAt(now);

            std::string query = getInsertQuery(entity);
            std::vector<std::string> params = getInsertParams(entity);
            
            auto result = transaction->exec_params(query, 
                params[0], params[1], params[2], params[3], params[4], 
                params[5], params[6], params[7], params[8], params[9]);
            
            db_manager_.commitTransaction(transaction);
            
            // Cache the entity
            if (!cache_prefix_.empty()) {
                std::string cache_key = cache_prefix_ + entity.getId();
                db_manager_.setCacheJson(cache_key, entity.toJson(), 3600);
            }
            
            return true;
        } catch (const std::exception& e) {
            throw DatabaseException("Failed to create entity: " + std::string(e.what()));
        }
    }

    virtual bool update(T& entity) {
        try {
            auto transaction = db_manager_.beginTransaction();
            
            // Update timestamp
            entity.updateTimestamp();

            std::string query = getUpdateQuery(entity);
            std::vector<std::string> params = getUpdateParams(entity);
            
            auto result = transaction->exec_params(query, 
                params[0], params[1], params[2], params[3], params[4], 
                params[5], params[6], params[7], params[8], params[9]);
            
            if (result.affected_rows() == 0) {
                db_manager_.rollbackTransaction(transaction);
                return false;
            }
            
            db_manager_.commitTransaction(transaction);
            
            // Update cache
            if (!cache_prefix_.empty()) {
                std::string cache_key = cache_prefix_ + entity.getId();
                db_manager_.setCacheJson(cache_key, entity.toJson(), 3600);
            }
            
            return true;
        } catch (const std::exception& e) {
            throw DatabaseException("Failed to update entity: " + std::string(e.what()));
        }
    }

    virtual bool deleteById(const std::string& id) {
        try {
            std::string query = "DELETE FROM " + table_name_ + " WHERE id = $1";
            auto result = db_manager_.executeQuery(query, {id});
            
            if (result.affected_rows() == 0) {
                return false;
            }
            
            // Remove from cache
            if (!cache_prefix_.empty()) {
                std::string cache_key = cache_prefix_ + id;
                db_manager_.deleteCache(cache_key);
            }
            
            return true;
        } catch (const std::exception& e) {
            throw DatabaseException("Failed to delete entity: " + std::string(e.what()));
        }
    }

    virtual bool exists(const std::string& id) {
        // Check cache first
        if (!cache_prefix_.empty()) {
            std::string cache_key = cache_prefix_ + id;
            if (db_manager_.existsInCache(cache_key)) {
                return true;
            }
        }
        
        std::string query = "SELECT 1 FROM " + table_name_ + " WHERE id = $1 LIMIT 1";
        auto result = db_manager_.executeQuery(query, {id});
        return !result.empty();
    }

    virtual int count() {
        std::string query = "SELECT COUNT(*) FROM " + table_name_;
        auto result = db_manager_.executeQuery(query);
        return result[0][0].as<int>();
    }

    virtual int countByCondition(const std::string& condition, 
                               const std::vector<std::string>& params = {}) {
        std::string query = "SELECT COUNT(*) FROM " + table_name_ + " WHERE " + condition;
        auto result = db_manager_.executeQuery(query, params);
        return result[0][0].as<int>();
    }

    // Batch operations
    virtual bool createBatch(std::vector<T>& entities) {
        try {
            auto transaction = db_manager_.beginTransaction();
            
            for (auto& entity : entities) {
                if (entity.getId().empty()) {
                    entity.setId(generateUUID());
                }
                
                auto now = std::chrono::system_clock::now();
                entity.setCreatedAt(now);
                entity.setUpdatedAt(now);

                std::string query = getInsertQuery(entity);
                std::vector<std::string> params = getInsertParams(entity);
                
                transaction->exec_params(query, 
                    params[0], params[1], params[2], params[3], params[4], 
                    params[5], params[6], params[7], params[8], params[9]);
            }
            
            db_manager_.commitTransaction(transaction);
            
            // Cache entities
            if (!cache_prefix_.empty()) {
                for (const auto& entity : entities) {
                    std::string cache_key = cache_prefix_ + entity.getId();
                    db_manager_.setCacheJson(cache_key, entity.toJson(), 3600);
                }
            }
            
            return true;
        } catch (const std::exception& e) {
            throw DatabaseException("Failed to create batch: " + std::string(e.what()));
        }
    }

    virtual void clearCache() {
        if (!cache_prefix_.empty()) {
            // This is a simplified implementation - in production, you'd want to use Redis SCAN
            // to find and delete all keys with the prefix
            db_manager_.clearCache();
        }
    }

protected:
    std::string generateUUID() {
        // Simple UUID generation - in production, use a proper UUID library
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static std::uniform_int_distribution<> dis2(8, 11);
        
        std::stringstream ss;
        ss << std::hex;
        for (int i = 0; i < 8; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (int i = 0; i < 4; i++) {
            ss << dis(gen);
        }
        ss << "-4";
        for (int i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        ss << dis2(gen);
        for (int i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (int i = 0; i < 12; i++) {
            ss << dis(gen);
        }
        return ss.str();
    }

    std::string timePointToString(const std::chrono::system_clock::time_point& tp) {
        auto time_t = std::chrono::system_clock::to_time_t(tp);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    std::chrono::system_clock::time_point stringToTimePoint(const std::string& str) {
        std::tm tm = {};
        std::stringstream ss(str);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        return std::chrono::system_clock::from_time_t(std::mktime(&tm));
    }
};

} // namespace database
} // namespace healthcare