#pragma once

#include <string>
#include <chrono>
#include <crow.h>
#include "../utils/Logger.h"

namespace healthcare::middleware {

enum class LogLevel {
    NONE,
    ERROR,
    WARN,
    INFO,
    DEBUG,
    TRACE
};

struct RequestInfo {
    std::string method;
    std::string path;
    std::string query_string;
    std::string user_agent;
    std::string client_ip;
    std::string user_id;
    size_t content_length;
    std::string content_type;
    std::chrono::high_resolution_clock::time_point start_time;
};

struct ResponseInfo {
    int status_code;
    size_t content_length;
    std::string content_type;
    std::chrono::high_resolution_clock::time_point end_time;
    double duration_ms;
};

class LoggingMiddleware : public crow::ILocalMiddleware {
public:
    LoggingMiddleware();
    ~LoggingMiddleware() = default;

    struct context {
        RequestInfo request_info;
        ResponseInfo response_info;
        std::string request_id;
        nlohmann::json custom_data;
        bool should_log = true;
    };

    // Middleware hooks
    void before_handle(crow::request& req, crow::response& res, context& ctx);
    void after_handle(crow::request& req, crow::response& res, context& ctx);

    // Configuration
    void setLogLevel(LogLevel level) { log_level_ = level; }
    void setLogRequests(bool log_requests) { log_requests_ = log_requests; }
    void setLogResponses(bool log_responses) { log_responses_ = log_responses; }
    void setLogHeaders(bool log_headers) { log_headers_ = log_headers; }
    void setLogBody(bool log_body) { log_body_ = log_body; }
    void setLogSensitiveData(bool log_sensitive) { log_sensitive_data_ = log_sensitive; }
    
    // Performance logging
    void setPerformanceThreshold(double threshold_ms) { performance_threshold_ms_ = threshold_ms; }
    void setLogSlowRequests(bool log_slow) { log_slow_requests_ = log_slow; }
    void setSlowRequestThreshold(double threshold_ms) { slow_request_threshold_ms_ = threshold_ms; }
    
    // Filtering
    void addIgnoredPath(const std::string& path);
    void removeIgnoredPath(const std::string& path);
    void addSensitiveHeader(const std::string& header);
    void addSensitiveParam(const std::string& param);
    
    // Custom logging
    void logCustomEvent(const std::string& event, const nlohmann::json& data = {});
    void logError(const std::string& message, const std::exception& ex);
    void logSecurityEvent(const std::string& event, const std::string& details = "");
    
    // Format configuration
    void setLogFormat(const std::string& format) { log_format_ = format; }
    void setTimestampFormat(const std::string& format) { timestamp_format_ = format; }
    void setIncludeRequestId(bool include) { include_request_id_ = include; }
    
    // Statistics
    struct LogStats {
        long long total_requests = 0;
        long long error_requests = 0;
        long long slow_requests = 0;
        double average_response_time_ms = 0.0;
        double max_response_time_ms = 0.0;
        double min_response_time_ms = 0.0;
        std::map<int, int> status_code_counts;
        std::map<std::string, int> endpoint_counts;
        std::map<std::string, double> endpoint_avg_times;
        std::chrono::system_clock::time_point last_request_time;
    };
    
    LogStats getStats() const { return stats_; }
    void resetStats() { stats_ = LogStats{}; }
    
    // Health monitoring
    bool isHealthy() const;
    nlohmann::json getHealthStatus() const;

private:
    // Configuration
    LogLevel log_level_;
    bool log_requests_;
    bool log_responses_;
    bool log_headers_;
    bool log_body_;
    bool log_sensitive_data_;
    bool log_slow_requests_;
    double performance_threshold_ms_;
    double slow_request_threshold_ms_;
    std::string log_format_;
    std::string timestamp_format_;
    bool include_request_id_;
    
    // Filtering
    std::set<std::string> ignored_paths_;
    std::set<std::string> sensitive_headers_;
    std::set<std::string> sensitive_params_;
    
    // Statistics
    mutable LogStats stats_;
    mutable std::mutex stats_mutex_;
    
    // Helper methods
    std::string generateRequestId() const;
    std::string extractClientIp(const crow::request& req) const;
    std::string extractUserId(const crow::request& req) const;
    
    bool shouldIgnorePath(const std::string& path) const;
    bool shouldLogLevel(LogLevel level) const;
    
    void logRequest(const RequestInfo& request_info, const std::string& request_id);
    void logResponse(const RequestInfo& request_info, const ResponseInfo& response_info, const std::string& request_id);
    void logSlowRequest(const RequestInfo& request_info, const ResponseInfo& response_info, const std::string& request_id);
    void logPerformanceMetrics(const RequestInfo& request_info, const ResponseInfo& response_info);
    
    // Sanitization
    std::string sanitizeHeaders(const crow::ci_map& headers) const;
    std::string sanitizeQueryString(const std::string& query_string) const;
    std::string sanitizeBody(const std::string& body, const std::string& content_type) const;
    std::string maskSensitiveData(const std::string& data) const;
    
    // Formatting
    std::string formatLogMessage(const std::string& template_str, const RequestInfo& req_info, 
                                const ResponseInfo& res_info, const std::string& request_id) const;
    std::string formatTimestamp(const std::chrono::system_clock::time_point& time_point) const;
    std::string formatDuration(double duration_ms) const;
    
    // JSON helpers
    nlohmann::json requestToJson(const RequestInfo& request_info, const std::string& request_id) const;
    nlohmann::json responseToJson(const ResponseInfo& response_info) const;
    nlohmann::json createLogEntry(const RequestInfo& request_info, const ResponseInfo& response_info, 
                                 const std::string& request_id) const;
    
    // Statistics updates
    void updateStats(const ResponseInfo& response_info, const std::string& endpoint) const;
    void updateEndpointStats(const std::string& endpoint, double duration_ms) const;
    
    // Health checks
    bool isResponseTimeHealthy() const;
    bool isErrorRateHealthy() const;
    
    // Default configurations
    void initializeDefaults();
    void setupSensitiveHeaders();
    void setupSensitiveParams();
};

// Logging format templates
namespace LogFormats {
    const std::string COMMON = "%timestamp% [%level%] %method% %path% %status% %duration%ms - %user_id%";
    const std::string COMBINED = "%timestamp% %client_ip% - %user_id% \"%method% %path% %protocol%\" %status% %content_length% \"%user_agent%\" %duration%ms";
    const std::string JSON = "json"; // Special format for JSON output
    const std::string CUSTOM = "%timestamp% [%request_id%] %method% %path% %status% %duration%ms %client_ip% %user_id%";
}

// Utility functions
std::string logLevelToString(LogLevel level);
LogLevel stringToLogLevel(const std::string& level_str);

} // namespace healthcare::middleware