#pragma once

// Template implementation file for BaseRepository
// Include this file at the end of BaseRepository.h

namespace healthcare::database {

template<typename T>
QueryResult<T> BaseRepository<T>::create(const T& entity) {
    REPO_VALIDATE_ENTITY(entity)
    
    return executeWithTiming([&]() {
        try {
            auto columns = getColumnNames();
            auto values = getInsertValues(entity);
            
            std::string query = buildInsertQuery(columns) + " RETURNING *";
            
            auto result = db_manager_.executeQuery(query, values);
            
            if (result.empty()) {
                return QueryResult<T>("Failed to create entity");
            }
            
            T created_entity;
            created_entity = mapRowToEntity(result[0]);
            
            // Cache the created entity
            cacheEntity(created_entity);
            
            return QueryResult<T>({created_entity});
            
        } catch (const std::exception& e) {
            logError("create", e.what());
            return QueryResult<T>(std::string("Create failed: ") + e.what());
        }
    });
}

template<typename T>
QueryResult<T> BaseRepository<T>::findById(const std::string& id) {
    REPO_VALIDATE_ID(id)
    
    // Check cache first
    auto cached = getCachedEntity(id);
    if (cached.has_value()) {
        stats_.cache_hits++;
        return QueryResult<T>({cached.value()});
    }
    
    stats_.cache_misses++;
    
    return executeWithTiming([&]() {
        try {
            std::string query = buildSelectQuery(getIdColumn() + " = $1");
            auto result = db_manager_.executeQuery(query, {id});
            
            if (result.empty()) {
                return QueryResult<T>("Entity not found");
            }
            
            T entity = mapRowToEntity(result[0]);
            
            // Cache the found entity
            cacheEntity(entity);
            
            return QueryResult<T>({entity});
            
        } catch (const std::exception& e) {
            logError("findById", e.what());
            return QueryResult<T>(std::string("Find failed: ") + e.what());
        }
    });
}

template<typename T>
QueryResult<T> BaseRepository<T>::update(const T& entity) {
    REPO_VALIDATE_ENTITY(entity)
    
    return executeWithTiming([&]() {
        try {
            auto columns = getColumnNames();
            auto values = getUpdateValues(entity);
            
            std::string where_clause = getIdColumn() + " = $" + std::to_string(values.size() + 1);
            std::string query = buildUpdateQuery(columns, where_clause) + " RETURNING *";
            
            // Add ID to the end of values
            values.push_back(entity.getId());
            
            auto result = db_manager_.executeQuery(query, values);
            
            if (result.empty()) {
                return QueryResult<T>("Failed to update entity");
            }
            
            T updated_entity = mapRowToEntity(result[0]);
            
            // Update cache
            cacheEntity(updated_entity);
            
            return QueryResult<T>({updated_entity});
            
        } catch (const std::exception& e) {
            logError("update", e.what());
            return QueryResult<T>(std::string("Update failed: ") + e.what());
        }
    });
}

template<typename T>
bool BaseRepository<T>::deleteById(const std::string& id) {
    REPO_VALIDATE_ID(id)
    
    return executeWithTiming([&]() {
        try {
            std::string query = buildDeleteQuery(getIdColumn() + " = $1");
            db_manager_.executeQuery(query, {id});
            
            // Remove from cache
            removeCachedEntity(id);
            
            return true;
            
        } catch (const std::exception& e) {
            logError("deleteById", e.what());
            return false;
        }
    });
}

template<typename T>
bool BaseRepository<T>::softDeleteById(const std::string& id) {
    REPO_VALIDATE_ID(id)
    
    return executeWithTiming([&]() {
        try {
            std::string query = "UPDATE " + table_name_ + 
                              " SET is_deleted = true, updated_at = CURRENT_TIMESTAMP" +
                              " WHERE " + getIdColumn() + " = $1";
            
            db_manager_.executeQuery(query, {id});
            
            // Remove from cache
            removeCachedEntity(id);
            
            return true;
            
        } catch (const std::exception& e) {
            logError("softDeleteById", e.what());
            return false;
        }
    });
}

template<typename T>
QueryResult<T> BaseRepository<T>::createBatch(const std::vector<T>& entities) {
    if (entities.empty()) {
        return QueryResult<T>({});
    }
    
    return executeWithTiming([&]() {
        try {
            auto transaction = db_manager_.beginTransaction();
            std::vector<T> created_entities;
            
            for (const auto& entity : entities) {
                if (!validateEntity(entity)) {
                    transaction->rollback();
                    return QueryResult<T>("Invalid entity in batch");
                }
                
                auto columns = getColumnNames();
                auto values = getInsertValues(entity);
                
                std::string query = buildInsertQuery(columns) + " RETURNING *";
                auto result = transaction->getWork().exec_params(query, values);
                
                if (!result.empty()) {
                    T created_entity = mapRowToEntity(result[0]);
                    created_entities.push_back(created_entity);
                    cacheEntity(created_entity);
                }
            }
            
            transaction->commit();
            return QueryResult<T>(created_entities);
            
        } catch (const std::exception& e) {
            logError("createBatch", e.what());
            return QueryResult<T>(std::string("Batch create failed: ") + e.what());
        }
    });
}

template<typename T>
QueryResult<T> BaseRepository<T>::updateBatch(const std::vector<T>& entities) {
    if (entities.empty()) {
        return QueryResult<T>({});
    }
    
    return executeWithTiming([&]() {
        try {
            auto transaction = db_manager_.beginTransaction();
            std::vector<T> updated_entities;
            
            for (const auto& entity : entities) {
                if (!validateEntity(entity)) {
                    transaction->rollback();
                    return QueryResult<T>("Invalid entity in batch");
                }
                
                auto columns = getColumnNames();
                auto values = getUpdateValues(entity);
                
                std::string where_clause = getIdColumn() + " = $" + std::to_string(values.size() + 1);
                std::string query = buildUpdateQuery(columns, where_clause) + " RETURNING *";
                
                values.push_back(entity.getId());
                
                auto result = transaction->getWork().exec_params(query, values);
                
                if (!result.empty()) {
                    T updated_entity = mapRowToEntity(result[0]);
                    updated_entities.push_back(updated_entity);
                    cacheEntity(updated_entity);
                }
            }
            
            transaction->commit();
            return QueryResult<T>(updated_entities);
            
        } catch (const std::exception& e) {
            logError("updateBatch", e.what());
            return QueryResult<T>(std::string("Batch update failed: ") + e.what());
        }
    });
}

template<typename T>
bool BaseRepository<T>::deleteBatch(const std::vector<std::string>& ids) {
    if (ids.empty()) {
        return true;
    }
    
    return executeWithTiming([&]() {
        try {
            std::ostringstream placeholders;
            for (size_t i = 0; i < ids.size(); ++i) {
                if (i > 0) placeholders << ", ";
                placeholders << "$" << (i + 1);
            }
            
            std::string query = buildDeleteQuery(getIdColumn() + " IN (" + placeholders.str() + ")");
            db_manager_.executeQuery(query, ids);
            
            // Remove from cache
            for (const auto& id : ids) {
                removeCachedEntity(id);
            }
            
            return true;
            
        } catch (const std::exception& e) {
            logError("deleteBatch", e.what());
            return false;
        }
    });
}

template<typename T>
QueryResult<T> BaseRepository<T>::findAll(const PaginationParams& pagination) {
    return executeWithTiming([&]() {
        try {
            std::string query = buildSelectQuery("", 
                                               pagination.getOrderClause(), 
                                               pagination.getLimitClause());
            
            auto result = db_manager_.executeQuery(query);
            
            std::vector<T> entities;
            for (const auto& row : result) {
                entities.push_back(mapRowToEntity(row));
            }
            
            QueryResult<T> query_result(entities);
            
            // Get total count
            auto count_result = db_manager_.executeQuery(buildCountQuery());
            if (!count_result.empty()) {
                query_result.total_count = count_result[0][0].as<int>();
            }
            
            return query_result;
            
        } catch (const std::exception& e) {
            logError("findAll", e.what());
            return QueryResult<T>(std::string("Find all failed: ") + e.what());
        }
    });
}

template<typename T>
QueryResult<T> BaseRepository<T>::findByFilter(const FilterParams& filters, 
                                              const PaginationParams& pagination) {
    return executeWithTiming([&]() {
        try {
            std::string where_clause = filters.buildWhereClause();
            std::string query = buildSelectQuery(where_clause,
                                               pagination.getOrderClause(),
                                               pagination.getLimitClause());
            
            auto params = filters.getParameterValues();
            auto result = db_manager_.executeQuery(query, params);
            
            std::vector<T> entities;
            for (const auto& row : result) {
                entities.push_back(mapRowToEntity(row));
            }
            
            QueryResult<T> query_result(entities);
            
            // Get total count with filters
            auto count_result = db_manager_.executeQuery(buildCountQuery(where_clause), params);
            if (!count_result.empty()) {
                query_result.total_count = count_result[0][0].as<int>();
            }
            
            return query_result;
            
        } catch (const std::exception& e) {
            logError("findByFilter", e.what());
            return QueryResult<T>(std::string("Filter query failed: ") + e.what());
        }
    });
}

template<typename T>
QueryResult<T> BaseRepository<T>::findByQuery(const std::string& custom_query, 
                                             const std::vector<std::string>& params) {
    return executeWithTiming([&]() {
        try {
            auto result = db_manager_.executeQuery(custom_query, params);
            
            std::vector<T> entities;
            for (const auto& row : result) {
                entities.push_back(mapRowToEntity(row));
            }
            
            return QueryResult<T>(entities);
            
        } catch (const std::exception& e) {
            logError("findByQuery", e.what());
            return QueryResult<T>(std::string("Custom query failed: ") + e.what());
        }
    });
}

template<typename T>
int BaseRepository<T>::countAll() {
    return executeWithTiming([&]() {
        try {
            auto result = db_manager_.executeQuery(buildCountQuery());
            return result.empty() ? 0 : result[0][0].as<int>();
        } catch (const std::exception& e) {
            logError("countAll", e.what());
            return 0;
        }
    });
}

template<typename T>
int BaseRepository<T>::countByFilter(const FilterParams& filters) {
    return executeWithTiming([&]() {
        try {
            std::string where_clause = filters.buildWhereClause();
            auto params = filters.getParameterValues();
            
            auto result = db_manager_.executeQuery(buildCountQuery(where_clause), params);
            return result.empty() ? 0 : result[0][0].as<int>();
        } catch (const std::exception& e) {
            logError("countByFilter", e.what());
            return 0;
        }
    });
}

template<typename T>
int BaseRepository<T>::countByQuery(const std::string& custom_query, 
                                   const std::vector<std::string>& params) {
    return executeWithTiming([&]() {
        try {
            auto result = db_manager_.executeQuery(custom_query, params);
            return result.empty() ? 0 : result[0][0].as<int>();
        } catch (const std::exception& e) {
            logError("countByQuery", e.what());
            return 0;
        }
    });
}

template<typename T>
bool BaseRepository<T>::exists(const std::string& id) {
    REPO_VALIDATE_ID(id)
    
    // Check cache first
    if (existsCache(generateCacheKey(id))) {
        return true;
    }
    
    return executeWithTiming([&]() {
        try {
            std::string query = "SELECT EXISTS(SELECT 1 FROM " + table_name_ + 
                              " WHERE " + getIdColumn() + " = $1 AND is_deleted = false)";
            
            auto result = db_manager_.executeQuery(query, {id});
            return !result.empty() && result[0][0].as<bool>();
        } catch (const std::exception& e) {
            logError("exists", e.what());
            return false;
        }
    });
}

template<typename T>
bool BaseRepository<T>::existsByFilter(const FilterParams& filters) {
    return executeWithTiming([&]() {
        try {
            std::string where_clause = filters.buildWhereClause();
            if (!where_clause.empty()) {
                where_clause += " AND ";
            } else {
                where_clause = " WHERE ";
            }
            where_clause += "is_deleted = false";
            
            std::string query = "SELECT EXISTS(SELECT 1 FROM " + table_name_ + where_clause + ")";
            
            auto params = filters.getParameterValues();
            auto result = db_manager_.executeQuery(query, params);
            
            return !result.empty() && result[0][0].as<bool>();
        } catch (const std::exception& e) {
            logError("existsByFilter", e.what());
            return false;
        }
    });
}

template<typename T>
QueryResult<T> BaseRepository<T>::search(const std::string& search_term, 
                                        const std::vector<std::string>& search_fields,
                                        const PaginationParams& pagination) {
    if (search_term.empty() || search_fields.empty()) {
        return QueryResult<T>({});
    }
    
    return executeWithTiming([&]() {
        try {
            FilterParams filters;
            filters.search_term = search_term;
            filters.search_fields = search_fields;
            
            return findByFilter(filters, pagination);
            
        } catch (const std::exception& e) {
            logError("search", e.what());
            return QueryResult<T>(std::string("Search failed: ") + e.what());
        }
    });
}

template<typename T>
QueryResult<T> BaseRepository<T>::createInTransaction(const T& entity, 
                                                     DatabaseManager::Transaction& transaction) {
    REPO_VALIDATE_ENTITY(entity)
    
    try {
        auto columns = getColumnNames();
        auto values = getInsertValues(entity);
        
        std::string query = buildInsertQuery(columns) + " RETURNING *";
        
        auto result = transaction.getWork().exec_params(query, values);
        
        if (result.empty()) {
            return QueryResult<T>("Failed to create entity in transaction");
        }
        
        T created_entity = mapRowToEntity(result[0]);
        return QueryResult<T>({created_entity});
        
    } catch (const std::exception& e) {
        logError("createInTransaction", e.what());
        return QueryResult<T>(std::string("Create in transaction failed: ") + e.what());
    }
}

template<typename T>
QueryResult<T> BaseRepository<T>::updateInTransaction(const T& entity, 
                                                     DatabaseManager::Transaction& transaction) {
    REPO_VALIDATE_ENTITY(entity)
    
    try {
        auto columns = getColumnNames();
        auto values = getUpdateValues(entity);
        
        std::string where_clause = getIdColumn() + " = $" + std::to_string(values.size() + 1);
        std::string query = buildUpdateQuery(columns, where_clause) + " RETURNING *";
        
        values.push_back(entity.getId());
        
        auto result = transaction.getWork().exec_params(query, values);
        
        if (result.empty()) {
            return QueryResult<T>("Failed to update entity in transaction");
        }
        
        T updated_entity = mapRowToEntity(result[0]);
        return QueryResult<T>({updated_entity});
        
    } catch (const std::exception& e) {
        logError("updateInTransaction", e.what());
        return QueryResult<T>(std::string("Update in transaction failed: ") + e.what());
    }
}

template<typename T>
bool BaseRepository<T>::deleteInTransaction(const std::string& id, 
                                           DatabaseManager::Transaction& transaction) {
    REPO_VALIDATE_ID(id)
    
    try {
        std::string query = buildDeleteQuery(getIdColumn() + " = $1");
        transaction.getWork().exec_params(query, {id});
        return true;
    } catch (const std::exception& e) {
        logError("deleteInTransaction", e.what());
        return false;
    }
}

template<typename T>
void BaseRepository<T>::cacheEntity(const T& entity, int ttl_seconds) {
    try {
        std::string key = generateCacheKey(entity.getId());
        db_manager_.setCacheJson(key, entity.toJson(), ttl_seconds);
    } catch (const std::exception& e) {
        logError("cacheEntity", e.what());
    }
}

template<typename T>
std::optional<T> BaseRepository<T>::getCachedEntity(const std::string& id) {
    try {
        std::string key = generateCacheKey(id);
        auto json = db_manager_.getCacheJson(key);
        
        if (!json.empty()) {
            T entity;
            entity.fromJson(json);
            return entity;
        }
    } catch (const std::exception& e) {
        logError("getCachedEntity", e.what());
    }
    
    return std::nullopt;
}

template<typename T>
void BaseRepository<T>::removeCachedEntity(const std::string& id) {
    try {
        std::string key = generateCacheKey(id);
        db_manager_.deleteCache(key);
    } catch (const std::exception& e) {
        logError("removeCachedEntity", e.what());
    }
}

template<typename T>
void BaseRepository<T>::clearEntityCache() {
    try {
        std::string pattern = generateListCacheKey("*");
        db_manager_.clearCache(pattern);
    } catch (const std::exception& e) {
        logError("clearEntityCache", e.what());
    }
}

template<typename T>
std::string BaseRepository<T>::getSelectQuery() const {
    return buildSelectQuery();
}

template<typename T>
std::string BaseRepository<T>::getInsertQuery() const {
    auto columns = getColumnNames();
    return buildInsertQuery(columns);
}

template<typename T>
std::string BaseRepository<T>::getUpdateQuery() const {
    auto columns = getColumnNames();
    return buildUpdateQuery(columns, getIdColumn() + " = $1");
}

template<typename T>
std::string BaseRepository<T>::getDeleteQuery() const {
    return buildDeleteQuery(getIdColumn() + " = $1");
}

template<typename T>
std::string BaseRepository<T>::buildSelectQuery(const std::string& where_clause,
                                               const std::string& order_clause,
                                               const std::string& limit_clause) const {
    std::ostringstream query;
    query << "SELECT * FROM " << table_name_;
    
    if (!where_clause.empty()) {
        query << " " << where_clause;
    }
    
    if (!order_clause.empty()) {
        query << " " << order_clause;
    }
    
    if (!limit_clause.empty()) {
        query << " " << limit_clause;
    }
    
    return query.str();
}

template<typename T>
std::string BaseRepository<T>::buildInsertQuery(const std::vector<std::string>& columns) const {
    std::ostringstream query;
    query << "INSERT INTO " << table_name_ << " (";
    
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) query << ", ";
        query << escapeIdentifier(columns[i]);
    }
    
    query << ") VALUES (";
    query << buildPlaceholders(columns.size());
    query << ")";
    
    return query.str();
}

template<typename T>
std::string BaseRepository<T>::buildUpdateQuery(const std::vector<std::string>& columns,
                                               const std::string& where_clause) const {
    std::ostringstream query;
    query << "UPDATE " << table_name_ << " SET ";
    
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) query << ", ";
        query << escapeIdentifier(columns[i]) << " = $" << (i + 1);
    }
    
    if (!where_clause.empty()) {
        query << " WHERE " << where_clause;
    }
    
    return query.str();
}

template<typename T>
std::string BaseRepository<T>::buildDeleteQuery(const std::string& where_clause) const {
    std::ostringstream query;
    query << "DELETE FROM " << table_name_;
    
    if (!where_clause.empty()) {
        query << " WHERE " << where_clause;
    }
    
    return query.str();
}

template<typename T>
std::string BaseRepository<T>::buildCountQuery(const std::string& where_clause) const {
    std::ostringstream query;
    query << "SELECT COUNT(*) FROM " << table_name_;
    
    if (!where_clause.empty()) {
        query << " " << where_clause;
    }
    
    return query.str();
}

template<typename T>
std::string BaseRepository<T>::escapeIdentifier(const std::string& identifier) const {
    // Simple PostgreSQL identifier escaping
    return "\"" + identifier + "\"";
}

template<typename T>
std::string BaseRepository<T>::buildPlaceholders(int count, int start_index) const {
    std::ostringstream placeholders;
    for (int i = 0; i < count; ++i) {
        if (i > 0) placeholders << ", ";
        placeholders << "$" << (start_index + i);
    }
    return placeholders.str();
}

template<typename T>
void BaseRepository<T>::logQuery(const std::string& query, double duration_ms, bool success) const {
    LOG_DEBUG("Repository[{}] Query: {} ({}ms) - {}", 
              table_name_, query.substr(0, 100), duration_ms, success ? "SUCCESS" : "FAILED");
}

template<typename T>
void BaseRepository<T>::logError(const std::string& operation, const std::string& error) const {
    LOG_ERROR("Repository[{}] Operation '{}' failed: {}", table_name_, operation, error);
}

template<typename T>
void BaseRepository<T>::updateStats(bool success, double duration_ms) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    stats_.total_queries++;
    if (success) {
        stats_.successful_queries++;
    } else {
        stats_.failed_queries++;
    }
    
    // Update average query time
    double total_time = stats_.average_query_time_ms * (stats_.total_queries - 1) + duration_ms;
    stats_.average_query_time_ms = total_time / stats_.total_queries;
    
    stats_.last_query_time = std::chrono::system_clock::now();
}

template<typename T>
std::string BaseRepository<T>::generateCacheKey(const std::string& id) const {
    return "entity:" + table_name_ + ":" + id;
}

template<typename T>
std::string BaseRepository<T>::generateListCacheKey(const std::string& suffix) const {
    return "list:" + table_name_ + ":" + suffix;
}

template<typename T>
bool BaseRepository<T>::validateEntity(const T& entity) const {
    // Basic validation - can be overridden in derived classes
    return !entity.getId().empty();
}

template<typename T>
bool BaseRepository<T>::validateId(const std::string& id) const {
    // Basic ID validation
    return !id.empty() && id.length() == 36; // UUID length
}

template<typename T>
std::vector<std::vector<T>> BaseRepository<T>::chunkEntities(const std::vector<T>& entities, 
                                                            int chunk_size) const {
    std::vector<std::vector<T>> chunks;
    
    for (size_t i = 0; i < entities.size(); i += chunk_size) {
        size_t end = std::min(i + chunk_size, entities.size());
        chunks.emplace_back(entities.begin() + i, entities.begin() + end);
    }
    
    return chunks;
}

template<typename T>
QueryResult<T> BaseRepository<T>::processBatchInChunks(const std::vector<T>& entities,
                                                      std::function<QueryResult<T>(const std::vector<T>&)> processor) const {
    auto chunks = chunkEntities(entities);
    std::vector<T> all_results;
    
    for (const auto& chunk : chunks) {
        auto result = processor(chunk);
        if (!result.success) {
            return result;
        }
        
        all_results.insert(all_results.end(), result.data.begin(), result.data.end());
    }
    
    return QueryResult<T>(all_results);
}

} // namespace healthcare::database