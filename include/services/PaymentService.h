#pragma once

#include <string>
#include <memory>
#include <vector>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "../models/Appointment.h"

namespace healthcare {
namespace services {

enum class PaymentMethod {
    RAZORPAY = 1,
    UPI = 2,
    CREDIT_CARD = 3,
    DEBIT_CARD = 4,
    NET_BANKING = 5,
    WALLET = 6
};

enum class PaymentError {
    SUCCESS = 0,
    INVALID_AMOUNT = 1,
    PAYMENT_GATEWAY_ERROR = 2,
    INSUFFICIENT_FUNDS = 3,
    PAYMENT_DECLINED = 4,
    NETWORK_ERROR = 5,
    INVALID_PAYMENT_METHOD = 6,
    PAYMENT_TIMEOUT = 7,
    VERIFICATION_FAILED = 8,
    REFUND_FAILED = 9,
    INVALID_CREDENTIALS = 10,
    APPOINTMENT_NOT_FOUND = 11,
    PAYMENT_ALREADY_PROCESSED = 12,
    REFUND_NOT_ALLOWED = 13
};

struct PaymentRequest {
    std::string appointment_id;
    double amount;
    std::string currency = "INR";
    PaymentMethod method;
    std::string user_id;
    std::string description;
    std::string callback_url;
    std::string cancel_url;
    std::map<std::string, std::string> metadata;
};

struct PaymentResponse {
    PaymentError error;
    std::string message;
    std::string payment_id;
    std::string order_id;
    std::string payment_url;
    std::string status;
    double amount;
    std::string currency;
    std::chrono::system_clock::time_point created_at;
    nlohmann::json gateway_response;
};

struct RefundRequest {
    std::string payment_id;
    std::string appointment_id;
    double amount;
    std::string reason;
    bool is_partial = false;
};

struct RefundResponse {
    PaymentError error;
    std::string message;
    std::string refund_id;
    std::string payment_id;
    double refund_amount;
    std::string status;
    std::chrono::system_clock::time_point processed_at;
};

struct PaymentVerificationRequest {
    std::string payment_id;
    std::string order_id;
    std::string signature;
    std::string appointment_id;
};

class PaymentService {
private:
    std::string razorpay_key_id_;
    std::string razorpay_key_secret_;
    std::string upi_merchant_id_;
    std::string webhook_secret_;
    std::string base_url_;
    bool is_production_;
    int timeout_seconds_;

public:
    PaymentService();
    ~PaymentService() = default;

    // Configuration
    void configure(const std::string& razorpay_key_id, 
                  const std::string& razorpay_key_secret,
                  const std::string& upi_merchant_id,
                  const std::string& webhook_secret,
                  bool is_production = false);

    // Core payment operations
    PaymentResponse createPaymentOrder(const PaymentRequest& request);
    PaymentResponse processPayment(const PaymentRequest& request);
    PaymentResponse verifyPayment(const PaymentVerificationRequest& request);
    RefundResponse initiateRefund(const RefundRequest& request);

    // Payment status operations
    PaymentResponse getPaymentStatus(const std::string& payment_id);
    std::vector<PaymentResponse> getPaymentsByUser(const std::string& user_id, int limit = 50);
    std::vector<PaymentResponse> getPaymentsByAppointment(const std::string& appointment_id);

    // UPI specific operations
    PaymentResponse createUpiPayment(const PaymentRequest& request);
    PaymentResponse verifyUpiPayment(const std::string& upi_transaction_id);
    std::string generateUpiQrCode(const PaymentRequest& request);

    // Webhook handling
    bool handleWebhook(const std::string& payload, const std::string& signature);
    bool processPaymentWebhook(const nlohmann::json& webhook_data);
    bool processRefundWebhook(const nlohmann::json& webhook_data);

    // Refund operations
    RefundResponse processRefund(const std::string& payment_id, double amount, const std::string& reason);
    RefundResponse getRefundStatus(const std::string& refund_id);
    std::vector<RefundResponse> getRefundsByPayment(const std::string& payment_id);

    // Payment validation
    bool validatePaymentAmount(double amount);
    bool validatePaymentMethod(PaymentMethod method);
    bool validateCurrency(const std::string& currency);
    bool isRefundAllowed(const std::string& payment_id);
    bool isPaymentCompleted(const std::string& payment_id);

    // Analytics and reporting
    double getTotalPaymentsByUser(const std::string& user_id, int days = 30);
    double getTotalPaymentsByDoctor(const std::string& doctor_id, int days = 30);
    std::map<std::string, double> getPaymentStatsByMethod(int days = 30);
    std::vector<std::pair<std::chrono::system_clock::time_point, double>> getPaymentTrends(int days = 30);
    double getSuccessRate(int days = 30);
    double getRefundRate(int days = 30);

    // Utility methods
    std::string getPaymentMethodString(PaymentMethod method);
    PaymentMethod getPaymentMethodFromString(const std::string& method_str);
    std::string formatAmount(double amount);
    bool isValidAmount(double amount);

private:
    // HTTP client methods
    std::string makeHttpRequest(const std::string& url, const std::string& method, 
                               const nlohmann::json& data, const std::map<std::string, std::string>& headers);
    std::string makeGetRequest(const std::string& url, const std::map<std::string, std::string>& headers);
    std::string makePostRequest(const std::string& url, const nlohmann::json& data, 
                               const std::map<std::string, std::string>& headers);

    // Razorpay specific methods
    PaymentResponse createRazorpayOrder(const PaymentRequest& request);
    PaymentResponse verifyRazorpayPayment(const PaymentVerificationRequest& request);
    RefundResponse createRazorpayRefund(const RefundRequest& request);
    std::string generateRazorpaySignature(const std::string& data);
    bool verifyRazorpaySignature(const std::string& data, const std::string& signature);

    // UPI specific methods
    PaymentResponse processUpiTransaction(const PaymentRequest& request);
    std::string generateUpiDeepLink(const PaymentRequest& request);
    bool verifyUpiTransaction(const std::string& transaction_id);

    // Helper methods
    std::string generateOrderId();
    std::string generatePaymentId();
    std::string generateRefundId();
    std::string encodeBase64(const std::string& data);
    std::string decodeBase64(const std::string& data);
    std::string hmacSha256(const std::string& data, const std::string& key);
    
    // Error handling
    PaymentError mapHttpErrorToPaymentError(int http_code);
    PaymentError mapRazorpayErrorToPaymentError(const std::string& error_code);
    std::string getErrorMessage(PaymentError error);

    // Validation helpers
    bool validateRazorpayCredentials();
    bool validateUpiCredentials();
    bool validateWebhookSignature(const std::string& payload, const std::string& signature);

    // Database operations
    bool savePaymentRecord(const PaymentResponse& response);
    bool updatePaymentStatus(const std::string& payment_id, const std::string& status);
    bool saveRefundRecord(const RefundResponse& response);
    std::optional<PaymentResponse> getPaymentFromDb(const std::string& payment_id);

    // Logging and monitoring
    void logPaymentActivity(const std::string& activity, const nlohmann::json& data);
    void logPaymentError(const std::string& error, const nlohmann::json& context);
    void sendPaymentNotification(const std::string& user_id, const PaymentResponse& response);
};

// Utility functions
namespace PaymentUtils {
    std::string formatCurrency(double amount, const std::string& currency = "INR");
    bool isValidIndianMobile(const std::string& mobile);
    bool isValidEmail(const std::string& email);
    std::string sanitizeAmount(const std::string& amount_str);
    double parseAmount(const std::string& amount_str);
}

} // namespace services
} // namespace healthcare