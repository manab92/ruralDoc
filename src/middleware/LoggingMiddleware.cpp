#include "../../include/middleware/LoggingMiddleware.h"
#include "../../include/utils/Logger.h"
#include <chrono>
#include <sstream>

namespace healthcare::middleware {

void LoggingMiddleware::before_handle(crow::request& req, crow::response& res, context& ctx) {
    ctx.start_time = std::chrono::high_resolution_clock::now();
    
    // Generate request ID
    ctx.request_id = generateRequestId();
    
    // Log request details
    logRequest(req, ctx.request_id);
    
    // Add request ID to response headers
    res.add_header("X-Request-ID", ctx.request_id);
}

void LoggingMiddleware::after_handle(crow::request& req, crow::response& res, context& ctx) {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - ctx.start_time);
    
    ctx.response_time_ms = duration.count();
    
    // Log response details
    logResponse(req, res, ctx);
    
    // Update statistics
    updateStats(req, res, ctx);
}

void LoggingMiddleware::logRequest(const crow::request& req, const std::string& request_id) {
    nlohmann::json log_data;
    log_data["request_id"] = request_id;
    log_data["method"] = crow::method_name(req.method);
    log_data["url"] = req.url;
    log_data["remote_ip"] = req.remote_ip_address;
    
    // Log headers (excluding sensitive ones)
    nlohmann::json headers;
    for (const auto& [key, value] : req.headers) {
        if (shouldLogHeader(key)) {
            headers[key] = value;
        }
    }
    log_data["headers"] = headers;
    
    // Log query parameters
    if (!req.url_params.keys().empty()) {
        nlohmann::json params;
        for (const auto& key : req.url_params.keys()) {
            params[key] = req.url_params.get(key);
        }
        log_data["query_params"] = params;
    }
    
    // Log body for non-GET requests (with size limit)
    if (req.method != crow::HTTPMethod::Get && !req.body.empty()) {
        if (req.body.length() <= 1024) { // Log small bodies
            log_data["body"] = req.body;
        } else {
            log_data["body_size"] = req.body.length();
            log_data["body_truncated"] = true;
        }
    }
    
    LOG_INFO("Request: {}", log_data.dump());
}

void LoggingMiddleware::logResponse(const crow::request& req, const crow::response& res, const context& ctx) {
    nlohmann::json log_data;
    log_data["request_id"] = ctx.request_id;
    log_data["method"] = crow::method_name(req.method);
    log_data["url"] = req.url;
    log_data["status_code"] = res.code;
    log_data["response_time_ms"] = ctx.response_time_ms;
    
    // Log response headers (excluding sensitive ones)
    nlohmann::json headers;
    for (const auto& [key, value] : res.headers) {
        if (shouldLogHeader(key)) {
            headers[key] = value;
        }
    }
    log_data["response_headers"] = headers;
    
    // Log response size
    log_data["response_size"] = res.body.length();
    
    // Determine log level based on status code
    if (res.code >= 500) {
        LOG_ERROR("Response: {}", log_data.dump());
    } else if (res.code >= 400) {
        LOG_WARN("Response: {}", log_data.dump());
    } else {
        LOG_INFO("Response: {}", log_data.dump());
    }
}

bool LoggingMiddleware::shouldLogHeader(const std::string& header_name) {
    static const std::set<std::string> sensitive_headers = {
        "authorization",
        "cookie",
        "set-cookie",
        "x-api-key",
        "x-auth-token"
    };
    
    std::string lower_header = header_name;
    std::transform(lower_header.begin(), lower_header.end(), lower_header.begin(), ::tolower);
    
    return sensitive_headers.find(lower_header) == sensitive_headers.end();
}

std::string LoggingMiddleware::generateRequestId() {
    static std::atomic<uint64_t> counter{0};
    
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
    
    std::ostringstream oss;
    oss << std::hex << timestamp << "-" << counter.fetch_add(1);
    
    return oss.str();
}

void LoggingMiddleware::updateStats(const crow::request& req, const crow::response& res, const context& ctx) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    stats_.total_requests++;
    stats_.total_response_time_ms += ctx.response_time_ms;
    
    // Update method stats
    stats_.requests_by_method[crow::method_name(req.method)]++;
    
    // Update status code stats
    stats_.requests_by_status[res.code]++;
    
    // Update endpoint stats
    std::string endpoint = extractEndpoint(req.url);
    stats_.requests_by_endpoint[endpoint]++;
    
    // Track slow requests
    if (ctx.response_time_ms > 1000) { // Requests taking more than 1 second
        stats_.slow_requests++;
    }
    
    // Track errors
    if (res.code >= 400) {
        stats_.error_requests++;
    }
}

std::string LoggingMiddleware::extractEndpoint(const std::string& url) {
    // Remove query parameters
    size_t query_pos = url.find('?');
    std::string path = (query_pos != std::string::npos) ? url.substr(0, query_pos) : url;
    
    // Replace IDs with placeholders
    std::regex uuid_regex("[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}");
    path = std::regex_replace(path, uuid_regex, ":id");
    
    std::regex number_regex("/\\d+");
    path = std::regex_replace(path, number_regex, "/:id");
    
    return path;
}

nlohmann::json LoggingMiddleware::getStats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    nlohmann::json stats_json;
    stats_json["total_requests"] = stats_.total_requests;
    stats_json["error_requests"] = stats_.error_requests;
    stats_json["slow_requests"] = stats_.slow_requests;
    
    if (stats_.total_requests > 0) {
        stats_json["average_response_time_ms"] = 
            static_cast<double>(stats_.total_response_time_ms) / stats_.total_requests;
    } else {
        stats_json["average_response_time_ms"] = 0;
    }
    
    stats_json["requests_by_method"] = stats_.requests_by_method;
    stats_json["requests_by_status"] = stats_.requests_by_status;
    
    // Top 10 endpoints
    std::vector<std::pair<std::string, int>> endpoint_pairs(
        stats_.requests_by_endpoint.begin(),
        stats_.requests_by_endpoint.end()
    );
    
    std::sort(endpoint_pairs.begin(), endpoint_pairs.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });
    
    nlohmann::json top_endpoints = nlohmann::json::array();
    for (size_t i = 0; i < std::min(size_t(10), endpoint_pairs.size()); ++i) {
        nlohmann::json endpoint;
        endpoint["path"] = endpoint_pairs[i].first;
        endpoint["count"] = endpoint_pairs[i].second;
        top_endpoints.push_back(endpoint);
    }
    stats_json["top_endpoints"] = top_endpoints;
    
    return stats_json;
}

void LoggingMiddleware::resetStats() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_ = Stats{};
}

} // namespace healthcare::middleware