#include "../../include/utils/Logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <iomanip>
#include <sstream>

namespace healthcare::utils {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::initialize(const LogConfig& config) {
    config_ = config;
    
    try {
        std::vector<spdlog::sink_ptr> sinks;
        
        // Console sink
        if (config_.enable_console) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(spdlog::level::from_str(config_.log_level));
            console_sink->set_pattern(config_.pattern);
            sinks.push_back(console_sink);
        }
        
        // File sink
        if (config_.enable_file) {
            spdlog::sink_ptr file_sink;
            
            if (config_.use_daily_rotation) {
                file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
                    config_.file_path, 
                    config_.rotation_hour, 
                    config_.rotation_minute
                );
            } else {
                file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                    config_.file_path,
                    config_.max_file_size,
                    config_.max_files
                );
            }
            
            file_sink->set_level(spdlog::level::from_str(config_.log_level));
            file_sink->set_pattern(config_.pattern);
            sinks.push_back(file_sink);
        }
        
        // Create logger
        logger_ = std::make_shared<spdlog::logger>("healthcare", sinks.begin(), sinks.end());
        logger_->set_level(spdlog::level::from_str(config_.log_level));
        logger_->flush_on(spdlog::level::err);
        
        // Register as default logger
        spdlog::set_default_logger(logger_);
        
        // Set flush interval
        spdlog::flush_every(std::chrono::seconds(config_.flush_interval_seconds));
        
        info("Logger initialized with level: {}", config_.log_level);
        
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
    }
}

void Logger::setLogLevel(const std::string& level) {
    if (logger_) {
        logger_->set_level(spdlog::level::from_str(level));
        config_.log_level = level;
        info("Log level changed to: {}", level);
    }
}

void Logger::enableFileLogging(const std::string& file_path) {
    config_.enable_file = true;
    config_.file_path = file_path;
    initialize(config_); // Reinitialize with new config
}

void Logger::disableFileLogging() {
    config_.enable_file = false;
    initialize(config_); // Reinitialize with new config
}

void Logger::enableConsoleLogging() {
    config_.enable_console = true;
    initialize(config_); // Reinitialize with new config
}

void Logger::disableConsoleLogging() {
    config_.enable_console = false;
    initialize(config_); // Reinitialize with new config
}

void Logger::flush() {
    if (logger_) {
        logger_->flush();
    }
}

void Logger::logWithContext(const std::string& level, const std::string& message, const nlohmann::json& context) {
    if (!logger_) return;
    
    nlohmann::json log_entry;
    log_entry["message"] = message;
    log_entry["context"] = context;
    log_entry["timestamp"] = getCurrentTimestamp();
    
    auto level_enum = spdlog::level::from_str(level);
    logger_->log(level_enum, log_entry.dump());
}

void Logger::logRequest(const std::string& method, const std::string& path, 
                       const std::string& client_ip, const nlohmann::json& headers) {
    nlohmann::json context;
    context["type"] = "request";
    context["method"] = method;
    context["path"] = path;
    context["client_ip"] = client_ip;
    context["headers"] = headers;
    
    info("HTTP Request: {} {}", method, path);
    logWithContext("debug", "Request details", context);
}

void Logger::logResponse(const std::string& method, const std::string& path, 
                        int status_code, double response_time_ms) {
    nlohmann::json context;
    context["type"] = "response";
    context["method"] = method;
    context["path"] = path;
    context["status_code"] = status_code;
    context["response_time_ms"] = response_time_ms;
    
    std::string level = (status_code >= 500) ? "error" : 
                       (status_code >= 400) ? "warn" : "info";
    
    logWithContext(level, 
        "HTTP Response: " + method + " " + path + " - " + std::to_string(status_code), 
        context);
}

void Logger::logError(const std::string& error_type, const std::string& message, 
                     const nlohmann::json& details) {
    nlohmann::json context;
    context["type"] = "error";
    context["error_type"] = error_type;
    context["details"] = details;
    context["stack_trace"] = getStackTrace();
    
    error("{}: {}", error_type, message);
    logWithContext("error", "Error details", context);
}

void Logger::logException(const std::exception& e, const nlohmann::json& context) {
    nlohmann::json error_context = context;
    error_context["exception_type"] = typeid(e).name();
    error_context["exception_message"] = e.what();
    
    logError("Exception", e.what(), error_context);
}

void Logger::logBusinessEvent(const std::string& event_type, const std::string& entity_type,
                             const std::string& entity_id, const nlohmann::json& details) {
    nlohmann::json context;
    context["type"] = "business_event";
    context["event_type"] = event_type;
    context["entity_type"] = entity_type;
    context["entity_id"] = entity_id;
    context["details"] = details;
    
    info("Business Event: {} - {} {}", event_type, entity_type, entity_id);
    logWithContext("info", "Business event details", context);
}

void Logger::logPerformance(const std::string& operation, double duration_ms, 
                           const nlohmann::json& metrics) {
    nlohmann::json context;
    context["type"] = "performance";
    context["operation"] = operation;
    context["duration_ms"] = duration_ms;
    context["metrics"] = metrics;
    
    if (duration_ms > 1000) { // Log as warning if operation takes more than 1 second
        warn("Slow operation: {} took {}ms", operation, duration_ms);
    } else {
        debug("Performance: {} completed in {}ms", operation, duration_ms);
    }
    
    logWithContext("debug", "Performance metrics", context);
}

void Logger::logDatabase(const std::string& query, double execution_time_ms, bool success) {
    nlohmann::json context;
    context["type"] = "database";
    context["query"] = query.substr(0, 200); // Truncate long queries
    context["execution_time_ms"] = execution_time_ms;
    context["success"] = success;
    
    if (!success) {
        error("Database query failed: {}", query.substr(0, 50));
    } else if (execution_time_ms > 100) {
        warn("Slow query ({}ms): {}", execution_time_ms, query.substr(0, 50));
    } else {
        debug("Database query executed in {}ms", execution_time_ms);
    }
    
    logWithContext("debug", "Database query details", context);
}

void Logger::logPayment(const std::string& transaction_id, const std::string& status,
                       double amount, const std::string& currency, const nlohmann::json& details) {
    nlohmann::json context;
    context["type"] = "payment";
    context["transaction_id"] = transaction_id;
    context["status"] = status;
    context["amount"] = amount;
    context["currency"] = currency;
    context["details"] = details;
    
    info("Payment Transaction: {} - {} {} {}", transaction_id, status, amount, currency);
    logWithContext("info", "Payment details", context);
}

void Logger::logAppointment(const std::string& appointment_id, const std::string& action,
                           const nlohmann::json& details) {
    nlohmann::json context;
    context["type"] = "appointment";
    context["appointment_id"] = appointment_id;
    context["action"] = action;
    context["details"] = details;
    
    info("Appointment {}: {}", action, appointment_id);
    logWithContext("info", "Appointment details", context);
}

std::string Logger::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string Logger::getStackTrace() const {
    // In a real implementation, this would use platform-specific
    // stack trace functionality (e.g., backtrace on Linux)
    return "Stack trace not available";
}

} // namespace healthcare::utils