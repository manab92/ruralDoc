#include "../../include/utils/ResponseHelper.h"

namespace healthcare::utils {

crow::response ResponseHelper::success(const nlohmann::json& data, const std::string& message) {
    ApiResponse response(true, ErrorCode::SUCCESS);
    response.message = message;
    response.data = data;
    
    crow::response res(200);
    res.set_header("Content-Type", "application/json");
    res.body = response.toJson().dump();
    
    return res;
}

crow::response ResponseHelper::created(const nlohmann::json& data, const std::string& message) {
    ApiResponse response(true, ErrorCode::SUCCESS);
    response.message = message.empty() ? "Resource created successfully" : message;
    response.data = data;
    
    crow::response res(201);
    res.set_header("Content-Type", "application/json");
    res.body = response.toJson().dump();
    
    return res;
}

crow::response ResponseHelper::noContent() {
    crow::response res(204);
    return res;
}

crow::response ResponseHelper::successWithPagination(const nlohmann::json& data, 
                                                   const PaginationInfo& pagination,
                                                   const std::string& message) {
    ApiResponse response(true, ErrorCode::SUCCESS);
    response.message = message;
    response.data = data;
    response.pagination = pagination.toJson();
    
    crow::response res(200);
    res.set_header("Content-Type", "application/json");
    res.body = response.toJson().dump();
    
    return res;
}

crow::response ResponseHelper::error(ErrorCode code, const std::string& message, 
                                   const nlohmann::json& details) {
    ApiResponse response(false, code);
    response.message = message;
    response.error_details = details;
    
    int status_code = getHttpStatusCode(code);
    
    crow::response res(status_code);
    res.set_header("Content-Type", "application/json");
    res.body = response.toJson().dump();
    
    return res;
}

crow::response ResponseHelper::validationError(const std::vector<std::string>& errors) {
    nlohmann::json error_details;
    error_details["validation_errors"] = errors;
    
    return error(ErrorCode::VALIDATION_ERROR, "Validation failed", error_details);
}

crow::response ResponseHelper::badRequest(const std::string& message) {
    return error(ErrorCode::BAD_REQUEST, message);
}

crow::response ResponseHelper::unauthorized(const std::string& message) {
    return error(ErrorCode::UNAUTHORIZED, 
                message.empty() ? "Authentication required" : message);
}

crow::response ResponseHelper::forbidden(const std::string& message) {
    return error(ErrorCode::FORBIDDEN, 
                message.empty() ? "Access denied" : message);
}

crow::response ResponseHelper::notFound(const std::string& resource) {
    std::string message = resource.empty() ? "Resource not found" : resource + " not found";
    return error(ErrorCode::NOT_FOUND, message);
}

crow::response ResponseHelper::conflict(const std::string& message) {
    return error(ErrorCode::CONFLICT, message);
}

crow::response ResponseHelper::tooManyRequests(int retry_after_seconds) {
    nlohmann::json details;
    details["retry_after"] = retry_after_seconds;
    
    crow::response res = error(ErrorCode::TOO_MANY_REQUESTS, 
                              "Rate limit exceeded", details);
    res.set_header("Retry-After", std::to_string(retry_after_seconds));
    
    return res;
}

crow::response ResponseHelper::internalServerError(const std::string& message) {
    return error(ErrorCode::INTERNAL_SERVER_ERROR, 
                message.empty() ? "Internal server error" : message);
}

crow::response ResponseHelper::serviceUnavailable(const std::string& message) {
    return error(ErrorCode::SERVICE_UNAVAILABLE, 
                message.empty() ? "Service temporarily unavailable" : message);
}

crow::response ResponseHelper::userAlreadyExists(const std::string& field) {
    std::string message = "User with this " + field + " already exists";
    nlohmann::json details;
    details["field"] = field;
    
    return error(ErrorCode::USER_ALREADY_EXISTS, message, details);
}

crow::response ResponseHelper::invalidCredentials() {
    return error(ErrorCode::INVALID_CREDENTIALS, "Invalid email or password");
}

crow::response ResponseHelper::emailNotVerified() {
    return error(ErrorCode::EMAIL_NOT_VERIFIED, 
                "Please verify your email address before logging in");
}

crow::response ResponseHelper::invalidToken(const std::string& token_type) {
    std::string message = "Invalid " + token_type + " token";
    return error(ErrorCode::INVALID_TOKEN, message);
}

crow::response ResponseHelper::tokenExpired(const std::string& token_type) {
    std::string message = token_type + " token has expired";
    return error(ErrorCode::TOKEN_EXPIRED, message);
}

crow::response ResponseHelper::insufficientPermissions(const std::string& action) {
    std::string message = action.empty() ? 
        "Insufficient permissions" : 
        "Insufficient permissions to " + action;
    
    return error(ErrorCode::INSUFFICIENT_PERMISSIONS, message);
}

crow::response ResponseHelper::appointmentNotAvailable(const std::string& reason) {
    std::string message = "Appointment slot not available";
    if (!reason.empty()) {
        message += ": " + reason;
    }
    
    return error(ErrorCode::APPOINTMENT_NOT_AVAILABLE, message);
}

crow::response ResponseHelper::appointmentCancellationFailed(const std::string& reason) {
    std::string message = "Cannot cancel appointment";
    if (!reason.empty()) {
        message += ": " + reason;
    }
    
    return error(ErrorCode::APPOINTMENT_CANCELLATION_FAILED, message);
}

crow::response ResponseHelper::paymentFailed(const std::string& reason) {
    std::string message = "Payment processing failed";
    if (!reason.empty()) {
        message += ": " + reason;
    }
    
    nlohmann::json details;
    details["reason"] = reason;
    
    return error(ErrorCode::PAYMENT_FAILED, message, details);
}

crow::response ResponseHelper::paymentRequired(double amount, const std::string& currency) {
    nlohmann::json details;
    details["amount"] = amount;
    details["currency"] = currency;
    
    return error(ErrorCode::PAYMENT_REQUIRED, 
                "Payment required to proceed", details);
}

crow::response ResponseHelper::refundFailed(const std::string& reason) {
    std::string message = "Refund processing failed";
    if (!reason.empty()) {
        message += ": " + reason;
    }
    
    return error(ErrorCode::REFUND_FAILED, message);
}

crow::response ResponseHelper::doctorNotAvailable(const std::string& doctor_name) {
    std::string message = doctor_name.empty() ? 
        "Doctor is not available" : 
        doctor_name + " is not available";
    
    return error(ErrorCode::DOCTOR_NOT_AVAILABLE, message);
}

crow::response ResponseHelper::clinicNotOperational(const std::string& clinic_name) {
    std::string message = clinic_name.empty() ? 
        "Clinic is not operational" : 
        clinic_name + " is not operational";
    
    return error(ErrorCode::CLINIC_NOT_OPERATIONAL, message);
}

crow::response ResponseHelper::prescriptionInvalid(const std::string& reason) {
    std::string message = "Invalid prescription";
    if (!reason.empty()) {
        message += ": " + reason;
    }
    
    return error(ErrorCode::PRESCRIPTION_INVALID, message);
}

crow::response ResponseHelper::prescriptionExpired() {
    return error(ErrorCode::PRESCRIPTION_EXPIRED, "Prescription has expired");
}

int ResponseHelper::getHttpStatusCode(ErrorCode code) {
    switch (code) {
        case ErrorCode::SUCCESS:
            return 200;
        
        case ErrorCode::BAD_REQUEST:
        case ErrorCode::VALIDATION_ERROR:
            return 400;
        
        case ErrorCode::UNAUTHORIZED:
        case ErrorCode::INVALID_CREDENTIALS:
        case ErrorCode::EMAIL_NOT_VERIFIED:
        case ErrorCode::INVALID_TOKEN:
        case ErrorCode::TOKEN_EXPIRED:
            return 401;
        
        case ErrorCode::FORBIDDEN:
        case ErrorCode::INSUFFICIENT_PERMISSIONS:
            return 403;
        
        case ErrorCode::NOT_FOUND:
            return 404;
        
        case ErrorCode::CONFLICT:
        case ErrorCode::USER_ALREADY_EXISTS:
        case ErrorCode::APPOINTMENT_NOT_AVAILABLE:
            return 409;
        
        case ErrorCode::TOO_MANY_REQUESTS:
            return 429;
        
        case ErrorCode::INTERNAL_SERVER_ERROR:
        case ErrorCode::DATABASE_ERROR:
            return 500;
        
        case ErrorCode::SERVICE_UNAVAILABLE:
        case ErrorCode::DOCTOR_NOT_AVAILABLE:
        case ErrorCode::CLINIC_NOT_OPERATIONAL:
            return 503;
        
        case ErrorCode::PAYMENT_REQUIRED:
            return 402;
        
        case ErrorCode::PAYMENT_FAILED:
        case ErrorCode::REFUND_FAILED:
        case ErrorCode::APPOINTMENT_CANCELLATION_FAILED:
        case ErrorCode::PRESCRIPTION_INVALID:
        case ErrorCode::PRESCRIPTION_EXPIRED:
            return 422; // Unprocessable Entity
        
        default:
            return 500;
    }
}

std::string ResponseHelper::getErrorMessage(ErrorCode code) {
    switch (code) {
        case ErrorCode::SUCCESS:
            return "Success";
        case ErrorCode::BAD_REQUEST:
            return "Bad Request";
        case ErrorCode::UNAUTHORIZED:
            return "Unauthorized";
        case ErrorCode::FORBIDDEN:
            return "Forbidden";
        case ErrorCode::NOT_FOUND:
            return "Not Found";
        case ErrorCode::CONFLICT:
            return "Conflict";
        case ErrorCode::VALIDATION_ERROR:
            return "Validation Error";
        case ErrorCode::TOO_MANY_REQUESTS:
            return "Too Many Requests";
        case ErrorCode::INTERNAL_SERVER_ERROR:
            return "Internal Server Error";
        case ErrorCode::SERVICE_UNAVAILABLE:
            return "Service Unavailable";
        case ErrorCode::DATABASE_ERROR:
            return "Database Error";
        case ErrorCode::USER_ALREADY_EXISTS:
            return "User Already Exists";
        case ErrorCode::INVALID_CREDENTIALS:
            return "Invalid Credentials";
        case ErrorCode::EMAIL_NOT_VERIFIED:
            return "Email Not Verified";
        case ErrorCode::INVALID_TOKEN:
            return "Invalid Token";
        case ErrorCode::TOKEN_EXPIRED:
            return "Token Expired";
        case ErrorCode::INSUFFICIENT_PERMISSIONS:
            return "Insufficient Permissions";
        case ErrorCode::APPOINTMENT_NOT_AVAILABLE:
            return "Appointment Not Available";
        case ErrorCode::APPOINTMENT_CANCELLATION_FAILED:
            return "Appointment Cancellation Failed";
        case ErrorCode::PAYMENT_FAILED:
            return "Payment Failed";
        case ErrorCode::PAYMENT_REQUIRED:
            return "Payment Required";
        case ErrorCode::REFUND_FAILED:
            return "Refund Failed";
        case ErrorCode::DOCTOR_NOT_AVAILABLE:
            return "Doctor Not Available";
        case ErrorCode::CLINIC_NOT_OPERATIONAL:
            return "Clinic Not Operational";
        case ErrorCode::PRESCRIPTION_INVALID:
            return "Prescription Invalid";
        case ErrorCode::PRESCRIPTION_EXPIRED:
            return "Prescription Expired";
        default:
            return "Unknown Error";
    }
}

nlohmann::json ResponseHelper::ApiResponse::toJson() const {
    nlohmann::json json;
    json["success"] = success;
    json["timestamp"] = timestamp;
    
    if (!message.empty()) {
        json["message"] = message;
    }
    
    if (!data.empty()) {
        json["data"] = data;
    }
    
    if (!success) {
        json["error"] = {
            {"code", static_cast<int>(error_code)},
            {"type", getErrorMessage(error_code)}
        };
        
        if (!error_details.empty()) {
            json["error"]["details"] = error_details;
        }
    }
    
    if (!pagination.empty()) {
        json["pagination"] = pagination;
    }
    
    if (!metadata.empty()) {
        json["metadata"] = metadata;
    }
    
    return json;
}

} // namespace healthcare::utils