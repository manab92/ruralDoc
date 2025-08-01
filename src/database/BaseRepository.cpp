#include "../../include/database/BaseRepository.h"
#include <sstream>
#include <iomanip>

namespace healthcare::database {

std::string FilterParams::buildWhereClause() const {
    std::vector<std::string> conditions;
    int param_index = 1;
    
    // String filters
    for (const auto& [field, value] : string_filters) {
        if (!value.empty()) {
            conditions.push_back(field + " = $" + std::to_string(param_index++));
        }
    }
    
    // Integer filters
    for (const auto& [field, value] : int_filters) {
        conditions.push_back(field + " = $" + std::to_string(param_index++));
    }
    
    // Boolean filters
    for (const auto& [field, value] : bool_filters) {
        conditions.push_back(field + " = $" + std::to_string(param_index++));
    }
    
    // Date filters
    for (const auto& [field, value] : date_filters) {
        conditions.push_back(field + " >= $" + std::to_string(param_index++));
    }
    
    // Search term
    if (!search_term.empty() && !search_fields.empty()) {
        std::vector<std::string> search_conditions;
        for (const auto& field : search_fields) {
            search_conditions.push_back(field + " ILIKE $" + std::to_string(param_index));
        }
        param_index++;
        conditions.push_back("(" + std::string(search_conditions.begin(), search_conditions.end()) + ")");
    }
    
    if (conditions.empty()) {
        return "";
    }
    
    std::ostringstream where_clause;
    where_clause << " WHERE ";
    for (size_t i = 0; i < conditions.size(); ++i) {
        if (i > 0) where_clause << " AND ";
        where_clause << conditions[i];
    }
    
    return where_clause.str();
}

std::vector<std::string> FilterParams::getParameterValues() const {
    std::vector<std::string> params;
    
    // String filters
    for (const auto& [field, value] : string_filters) {
        if (!value.empty()) {
            params.push_back(value);
        }
    }
    
    // Integer filters
    for (const auto& [field, value] : int_filters) {
        params.push_back(std::to_string(value));
    }
    
    // Boolean filters
    for (const auto& [field, value] : bool_filters) {
        params.push_back(value ? "true" : "false");
    }
    
    // Date filters
    for (const auto& [field, value] : date_filters) {
        auto time_t = std::chrono::system_clock::to_time_t(value);
        std::tm* tm = std::localtime(&time_t);
        std::ostringstream oss;
        oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
        params.push_back(oss.str());
    }
    
    // Search term
    if (!search_term.empty() && !search_fields.empty()) {
        params.push_back("%" + search_term + "%");
    }
    
    return params;
}

} // namespace healthcare::database