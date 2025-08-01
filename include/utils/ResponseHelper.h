#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <crow/http_response.h>
#include <chrono>

namespace healthcare::utils {

enum class ErrorCode {
    // General errors
    SUCCESS = 0,
    INTERNAL_SERVER_ERROR = 1000,
    INVALID_REQUEST = 1001,
    MISSING_PARAMETER = 1002,
    INVALID_PARAMETER = 1003,
    
    // Authentication errors
    AUTHENTICATION_ERROR = 2000,
    INVALID_TOKEN = 2001,
    EXPIRED_TOKEN = 2002,
    MISSING_TOKEN = 2003,
    INVALID_CREDENTIALS = 2004,
    ACCOUNT_LOCKED = 2005,
    
    // Authorization errors
    AUTHORIZATION_ERROR = 3000,
    INSUFFICIENT_PERMISSIONS = 3001,
    ACCESS_DENIED = 3002,
    ROLE_NOT_ALLOWED = 3003,
    
    // Validation errors
    VALIDATION_ERROR = 4000,
    INVALID_EMAIL = 4001,
    INVALID_PHONE = 4002,
    INVALID_PASSWORD = 4003,
    INVALID_DATE = 4004,
    INVALID_TIME = 4005,
    
    // Resource errors
    NOT_FOUND = 5000,
    USER_NOT_FOUND = 5001,
    DOCTOR_NOT_FOUND = 5002,
    APPOINTMENT_NOT_FOUND = 5003,
    CLINIC_NOT_FOUND = 5004,
    PRESCRIPTION_NOT_FOUND = 5005,
    
    // Business logic errors
    CONFLICT = 6000,
    USER_ALREADY_EXISTS = 6001,
    EMAIL_ALREADY_EXISTS = 6002,
    APPOINTMENT_CONFLICT = 6003,
    DOCTOR_NOT_AVAILABLE = 6004,
    SLOT_NOT_AVAILABLE = 6005,
    
    // Payment errors
    PAYMENT_ERROR = 7000,
    PAYMENT_FAILED = 7001,
    INSUFFICIENT_FUNDS = 7002,
    PAYMENT_ALREADY_PROCESSED = 7003,
    REFUND_FAILED = 7004,
    INVALID_PAYMENT_METHOD = 7005,
    
    // Booking errors
    BOOKING_ERROR = 8000,
    BOOKING_NOT_ALLOWED = 8001,
    BOOKING_TIME_PASSED = 8002,
    BOOKING_LIMIT_EXCEEDED = 8003,
    CANCELLATION_NOT_ALLOWED = 8004,
    
    // System errors
    DATABASE_ERROR = 9000,
    REDIS_ERROR = 9001,
    EXTERNAL_SERVICE_ERROR = 9002,
    FILE_UPLOAD_ERROR = 9003,
    EMAIL_SERVICE_ERROR = 9004,
    SMS_SERVICE_ERROR = 9005,
    
    // Rate limiting
    RATE_LIMIT_EXCEEDED = 10000,
    TOO_MANY_REQUESTS = 10001
};

struct ApiResponse {
    bool success;
    ErrorCode error_code;
    std::string message;
    nlohmann::json data;
    nlohmann::json error_details;
    std::string timestamp;
    std::string request_id;
    
    ApiResponse(bool success = true, ErrorCode code = ErrorCode::SUCCESS)
        : success(success), error_code(code) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        timestamp = std::to_string(time_t);
    }
};

struct PaginationInfo {
    int page;
    int page_size;
    int total_pages;
    int total_count;
    bool has_next;
    bool has_previous;
    
    PaginationInfo(int page = 1, int page_size = 20, int total_count = 0)
        : page(page), page_size(page_size), total_count(total_count) {
        total_pages = (total_count + page_size - 1) / page_size;
        has_next = page < total_pages;
        has_previous = page > 1;
    }
    
    nlohmann::json toJson() const {
        return nlohmann::json{
            {"page", page},
            {"page_size", page_size},
            {"total_pages", total_pages},
            {"total_count", total_count},
            {"has_next", has_next},
            {"has_previous", has_previous}
        };
    }
};

class ResponseHelper {
public:
    // Success responses
    static crow::response success(const nlohmann::json& data = {}, 
                                 const std::string& message = "Operation successful",
                                 const std::string& request_id = "");
    
    static crow::response created(const nlohmann::json& data = {},
                                 const std::string& message = "Resource created successfully",
                                 const std::string& request_id = "");
    
    static crow::response accepted(const nlohmann::json& data = {},
                                  const std::string& message = "Request accepted",
                                  const std::string& request_id = "");
    
    static crow::response noContent(const std::string& request_id = "");
    
    // Success with pagination
    static crow::response successWithPagination(const nlohmann::json& data,
                                               const PaginationInfo& pagination,
                                               const std::string& message = "Data retrieved successfully",
                                               const std::string& request_id = "");
    
    // Error responses
    static crow::response error(ErrorCode error_code,
                               const std::string& message,
                               const nlohmann::json& details = {},
                               const std::string& request_id = "");
    
    static crow::response validationError(const std::vector<std::string>& errors,
                                         const std::string& message = "Validation failed",
                                         const std::string& request_id = "");
    
    static crow::response validationError(const std::string& field,
                                         const std::string& error_message,
                                         const std::string& request_id = "");
    
    // Common HTTP error responses
    static crow::response badRequest(const std::string& message = "Bad request",
                                    const nlohmann::json& details = {},
                                    const std::string& request_id = "");
    
    static crow::response unauthorized(const std::string& message = "Unauthorized",
                                      const std::string& request_id = "");
    
    static crow::response forbidden(const std::string& message = "Forbidden",
                                   const std::string& request_id = "");
    
    static crow::response notFound(const std::string& message = "Resource not found",
                                  const std::string& request_id = "");
    
    static crow::response conflict(const std::string& message = "Resource conflict",
                                  const nlohmann::json& details = {},
                                  const std::string& request_id = "");
    
    static crow::response tooManyRequests(const std::string& message = "Too many requests",
                                         const std::string& request_id = "");
    
    static crow::response internalServerError(const std::string& message = "Internal server error",
                                             const std::string& request_id = "");
    
    static crow::response serviceUnavailable(const std::string& message = "Service unavailable",
                                            const std::string& request_id = "");
    
    // Business logic specific responses
    static crow::response userNotFound(const std::string& user_id = "",
                                      const std::string& request_id = "");
    
    static crow::response doctorNotFound(const std::string& doctor_id = "",
                                        const std::string& request_id = "");
    
    static crow::response appointmentNotFound(const std::string& appointment_id = "",
                                             const std::string& request_id = "");
    
    static crow::response appointmentConflict(const std::string& message = "Appointment slot is not available",
                                             const std::string& request_id = "");
    
    static crow::response paymentFailed(const std::string& payment_id = "",
                                       const std::string& reason = "",
                                       const std::string& request_id = "");
    
    static crow::response invalidCredentials(const std::string& request_id = "");
    
    static crow::response accountLocked(const std::string& request_id = "");
    
    static crow::response tokenExpired(const std::string& request_id = "");
    
    // Custom responses with specific error codes
    static crow::response customError(ErrorCode error_code,
                                     int http_status,
                                     const std::string& message,
                                     const nlohmann::json& details = {},
                                     const std::string& request_id = "");
    
    // Health check response
    static crow::response healthCheck(const nlohmann::json& health_data,
                                     bool is_healthy = true,
                                     const std::string& request_id = "");
    
    // File upload responses
    static crow::response fileUploadSuccess(const std::string& file_url,
                                           const std::string& file_id = "",
                                           const std::string& request_id = "");
    
    static crow::response fileUploadError(const std::string& reason,
                                         const std::string& request_id = "");
    
    // Utility methods
    static std::string generateRequestId();
    static std::string errorCodeToString(ErrorCode code);
    static int errorCodeToHttpStatus(ErrorCode code);
    static nlohmann::json createErrorDetails(const std::string& field, const std::string& message);
    static nlohmann::json createErrorDetails(const std::vector<std::pair<std::string, std::string>>& field_errors);
    
    // Response formatting
    static nlohmann::json formatSuccessResponse(const nlohmann::json& data,
                                               const std::string& message,
                                               const std::string& request_id = "");
    
    static nlohmann::json formatErrorResponse(ErrorCode error_code,
                                             const std::string& message,
                                             const nlohmann::json& details = {},
                                             const std::string& request_id = "");
    
    // Headers management
    static void addCorsHeaders(crow::response& response);
    static void addSecurityHeaders(crow::response& response);
    static void addCacheHeaders(crow::response& response, int max_age_seconds = 0);
    static void addPaginationHeaders(crow::response& response, const PaginationInfo& pagination);
    
    // Content type helpers
    static crow::response jsonResponse(const nlohmann::json& json, int status_code = 200);
    static crow::response textResponse(const std::string& text, int status_code = 200);
    static crow::response htmlResponse(const std::string& html, int status_code = 200);
    
    // Response timing
    static void startRequestTimer(const std::string& request_id);
    static double getRequestDuration(const std::string& request_id);
    static void addTimingHeaders(crow::response& response, const std::string& request_id);

private:
    static crow::response createResponse(const ApiResponse& api_response, int http_status);
    static std::string getCurrentTimestamp();
    static std::map<std::string, std::chrono::high_resolution_clock::time_point> request_timers_;
};

// Convenience macros for common responses
#define RESPONSE_SUCCESS(data) healthcare::utils::ResponseHelper::success(data)
#define RESPONSE_CREATED(data) healthcare::utils::ResponseHelper::created(data)
#define RESPONSE_ERROR(code, message) healthcare::utils::ResponseHelper::error(code, message)
#define RESPONSE_BAD_REQUEST(message) healthcare::utils::ResponseHelper::badRequest(message)
#define RESPONSE_UNAUTHORIZED() healthcare::utils::ResponseHelper::unauthorized()
#define RESPONSE_FORBIDDEN() healthcare::utils::ResponseHelper::forbidden()
#define RESPONSE_NOT_FOUND(message) healthcare::utils::ResponseHelper::notFound(message)
#define RESPONSE_CONFLICT(message) healthcare::utils::ResponseHelper::conflict(message)
#define RESPONSE_INTERNAL_ERROR() healthcare::utils::ResponseHelper::internalServerError()

} // namespace healthcare::utils