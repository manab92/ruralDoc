#pragma once

#include <string>
#include <vector>
#include <set>
#include <crow.h>

namespace healthcare::middleware {

class CorsMiddleware : public crow::ILocalMiddleware {
public:
    CorsMiddleware();
    ~CorsMiddleware() = default;

    struct context {
        std::string origin;
        bool is_preflight = false;
        bool is_cors_request = false;
    };

    // Middleware hooks
    void before_handle(crow::request& req, crow::response& res, context& ctx);
    void after_handle(crow::request& req, crow::response& res, context& ctx);

    // Configuration methods
    void setAllowedOrigins(const std::vector<std::string>& origins);
    void addAllowedOrigin(const std::string& origin);
    void removeAllowedOrigin(const std::string& origin);
    void setAllowAllOrigins(bool allow_all) { allow_all_origins_ = allow_all; }
    
    void setAllowedMethods(const std::vector<std::string>& methods);
    void addAllowedMethod(const std::string& method);
    void removeAllowedMethod(const std::string& method);
    
    void setAllowedHeaders(const std::vector<std::string>& headers);
    void addAllowedHeader(const std::string& header);
    void removeAllowedHeader(const std::string& header);
    
    void setExposedHeaders(const std::vector<std::string>& headers);
    void addExposedHeader(const std::string& header);
    void removeExposedHeader(const std::string& header);
    
    void setAllowCredentials(bool allow) { allow_credentials_ = allow; }
    void setMaxAge(int seconds) { max_age_ = seconds; }
    void setPreflightCacheMaxAge(int seconds) { preflight_cache_max_age_ = seconds; }
    
    // Validation methods
    bool isOriginAllowed(const std::string& origin) const;
    bool isMethodAllowed(const std::string& method) const;
    bool isHeaderAllowed(const std::string& header) const;
    
    // Utility methods
    void setDefaultConfiguration();
    void setProductionConfiguration();
    void setDevelopmentConfiguration();
    
    // Statistics
    struct CorsStats {
        long long total_requests = 0;
        long long cors_requests = 0;
        long long preflight_requests = 0;
        long long blocked_requests = 0;
        std::map<std::string, int> origin_requests;
        std::map<std::string, int> method_requests;
    };
    
    CorsStats getStats() const { return stats_; }
    void resetStats() { stats_ = CorsStats{}; }

private:
    // Configuration
    std::set<std::string> allowed_origins_;
    std::set<std::string> allowed_methods_;
    std::set<std::string> allowed_headers_;
    std::set<std::string> exposed_headers_;
    bool allow_all_origins_;
    bool allow_credentials_;
    int max_age_;
    int preflight_cache_max_age_;
    
    // Statistics
    mutable CorsStats stats_;
    mutable std::mutex stats_mutex_;
    
    // Helper methods
    std::string getOrigin(const crow::request& req) const;
    bool isPreflightRequest(const crow::request& req) const;
    bool isCorsRequest(const crow::request& req) const;
    
    void handlePreflightRequest(const crow::request& req, crow::response& res, const std::string& origin);
    void handleSimpleRequest(const crow::request& req, crow::response& res, const std::string& origin);
    
    void addCorsHeaders(crow::response& res, const std::string& origin);
    void addPreflightHeaders(crow::response& res, const crow::request& req, const std::string& origin);
    
    std::string vectorToString(const std::vector<std::string>& vec, const std::string& delimiter = ", ") const;
    std::string setToString(const std::set<std::string>& set, const std::string& delimiter = ", ") const;
    
    void updateStats(const context& ctx) const;
    void logCorsEvent(const std::string& event, const std::string& origin, const std::string& method = "") const;
    
    // Default configurations
    void initializeDefaults();
};

// CORS configuration presets
namespace CorsPresets {
    // Allow all origins with common methods and headers
    CorsMiddleware createPermissive();
    
    // Restrictive CORS for production
    CorsMiddleware createRestrictive(const std::vector<std::string>& allowed_origins);
    
    // Development configuration with localhost allowed
    CorsMiddleware createDevelopment();
    
    // API-specific configuration
    CorsMiddleware createApiOnly();
}

} // namespace healthcare::middleware