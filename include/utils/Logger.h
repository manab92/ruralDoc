#pragma once

#include <string>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/file_sinks.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <nlohmann/json.hpp>

namespace healthcare::utils {

enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL
};

class Logger {
public:
    static Logger& getInstance();
    
    // Configuration
    void configure(const std::string& level = "INFO", 
                  const std::string& log_file = "healthcare.log",
                  bool enable_console = true,
                  const std::string& pattern = "[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
    
    // Basic logging methods
    void trace(const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);
    
    // Formatted logging
    template<typename... Args>
    void trace(const std::string& format, Args&&... args) {
        if (logger_) logger_->trace(format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void debug(const std::string& format, Args&&... args) {
        if (logger_) logger_->debug(format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void info(const std::string& format, Args&&... args) {
        if (logger_) logger_->info(format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void warn(const std::string& format, Args&&... args) {
        if (logger_) logger_->warn(format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void error(const std::string& format, Args&&... args) {
        if (logger_) logger_->error(format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void critical(const std::string& format, Args&&... args) {
        if (logger_) logger_->critical(format, std::forward<Args>(args)...);
    }
    
    // Structured logging with context
    void logWithContext(LogLevel level, const std::string& message, 
                       const nlohmann::json& context = {});
    
    // Request/Response logging
    void logRequest(const std::string& method, const std::string& endpoint, 
                   const std::string& user_id = "", const nlohmann::json& params = {});
    void logResponse(const std::string& endpoint, int status_code, 
                    double duration_ms, const std::string& user_id = "");
    
    // Error logging with stack trace
    void logError(const std::string& message, const std::exception& ex, 
                 const nlohmann::json& context = {});
    
    // Business logic logging
    void logUserAction(const std::string& user_id, const std::string& action, 
                      const nlohmann::json& details = {});
    void logSystemEvent(const std::string& event, const nlohmann::json& details = {});
    void logSecurityEvent(const std::string& event, const std::string& user_id = "", 
                         const nlohmann::json& details = {});
    
    // Performance logging
    void logPerformance(const std::string& operation, double duration_ms, 
                       const nlohmann::json& metrics = {});
    
    // Database logging
    void logDatabaseQuery(const std::string& query, double duration_ms, 
                         int affected_rows = 0);
    void logDatabaseError(const std::string& query, const std::string& error);
    
    // Payment logging
    void logPaymentEvent(const std::string& event, const std::string& payment_id,
                        const std::string& user_id, double amount, 
                        const nlohmann::json& details = {});
    
    // Appointment logging
    void logAppointmentEvent(const std::string& event, const std::string& appointment_id,
                           const std::string& user_id, const std::string& doctor_id,
                           const nlohmann::json& details = {});
    
    // Utility methods
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;
    void flush();
    bool isLevelEnabled(LogLevel level) const;
    
    // Health check
    bool isHealthy() const;
    nlohmann::json getLoggerStats() const;

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    std::shared_ptr<spdlog::logger> logger_;
    LogLevel current_level_ = LogLevel::INFO;
    std::string log_file_path_;
    bool console_enabled_ = true;
    
    // Helper methods
    std::string logLevelToString(LogLevel level) const;
    LogLevel stringToLogLevel(const std::string& level) const;
    nlohmann::json createLogContext(const std::string& event_type) const;
};

// Convenience macros for common logging patterns
#define LOG_TRACE(...) healthcare::utils::Logger::getInstance().trace(__VA_ARGS__)
#define LOG_DEBUG(...) healthcare::utils::Logger::getInstance().debug(__VA_ARGS__)
#define LOG_INFO(...) healthcare::utils::Logger::getInstance().info(__VA_ARGS__)
#define LOG_WARN(...) healthcare::utils::Logger::getInstance().warn(__VA_ARGS__)
#define LOG_ERROR(...) healthcare::utils::Logger::getInstance().error(__VA_ARGS__)
#define LOG_CRITICAL(...) healthcare::utils::Logger::getInstance().critical(__VA_ARGS__)

// Context logging macros
#define LOG_REQUEST(method, endpoint, user_id) \
    healthcare::utils::Logger::getInstance().logRequest(method, endpoint, user_id)

#define LOG_RESPONSE(endpoint, status, duration, user_id) \
    healthcare::utils::Logger::getInstance().logResponse(endpoint, status, duration, user_id)

#define LOG_USER_ACTION(user_id, action, details) \
    healthcare::utils::Logger::getInstance().logUserAction(user_id, action, details)

#define LOG_SECURITY_EVENT(event, user_id, details) \
    healthcare::utils::Logger::getInstance().logSecurityEvent(event, user_id, details)

} // namespace healthcare::utils