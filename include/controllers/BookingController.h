#pragma once

#include <crow.h>
#include <nlohmann/json.hpp>
#include "../services/BookingService.h"
#include "../middleware/AuthMiddleware.h"
#include "../utils/ResponseHelper.h"

namespace healthcare {
namespace controllers {

class BookingController {
private:
    std::unique_ptr<services::BookingService> booking_service_;
    std::unique_ptr<middleware::AuthMiddleware> auth_middleware_;

public:
    BookingController();
    ~BookingController() = default;

    // Route registration
    void registerRoutes(crow::Crow<crow::CookieParser, middleware::AuthMiddleware>& app);

    // Core booking operations
    crow::response book_appointment(const crow::request& req);
    crow::response reschedule_appointment(const crow::request& req, const std::string& appointment_id);
    crow::response cancel_appointment(const crow::request& req, const std::string& appointment_id);
    crow::response confirm_appointment(const crow::request& req, const std::string& appointment_id);

    // Appointment management
    crow::response get_appointment(const crow::request& req, const std::string& appointment_id);
    crow::response get_user_appointments(const crow::request& req, const std::string& user_id);
    crow::response get_doctor_appointments(const crow::request& req, const std::string& doctor_id);
    crow::response get_clinic_appointments(const crow::request& req, const std::string& clinic_id);

    // Availability and search
    crow::response get_doctor_availability(const crow::request& req, const std::string& doctor_id);
    crow::response get_clinic_availability(const crow::request& req, const std::string& clinic_id);
    crow::response search_available_doctors(const crow::request& req);
    crow::response get_nearby_doctors(const crow::request& req);
    crow::response get_next_available_slots(const crow::request& req, const std::string& doctor_id);

    // Emergency booking
    crow::response book_emergency_appointment(const crow::request& req);
    crow::response get_emergency_doctors(const crow::request& req);

    // Follow-up appointments
    crow::response book_follow_up(const crow::request& req, const std::string& parent_appointment_id);
    crow::response check_follow_up_eligibility(const crow::request& req, const std::string& appointment_id);

    // Appointment status management
    crow::response mark_appointment_completed(const crow::request& req, const std::string& appointment_id);
    crow::response mark_appointment_no_show(const crow::request& req, const std::string& appointment_id);
    crow::response start_appointment(const crow::request& req, const std::string& appointment_id);

    // Payment integration
    crow::response process_payment(const crow::request& req, const std::string& appointment_id);
    crow::response verify_payment(const crow::request& req, const std::string& appointment_id);
    crow::response refund_payment(const crow::request& req, const std::string& appointment_id);

    // Queue management
    crow::response get_appointment_queue(const crow::request& req, const std::string& doctor_id);
    crow::response get_queue_position(const crow::request& req, const std::string& appointment_id);
    crow::response get_estimated_wait_time(const crow::request& req, const std::string& appointment_id);

    // Analytics and reporting
    crow::response get_booking_stats_by_doctor(const crow::request& req, const std::string& doctor_id);
    crow::response get_booking_stats_by_clinic(const crow::request& req, const std::string& clinic_id);
    crow::response get_booking_trends(const crow::request& req);
    crow::response get_cancellation_rate(const crow::request& req, const std::string& doctor_id);

    // Notifications
    crow::response send_appointment_reminder(const crow::request& req, const std::string& appointment_id);

private:
    // Helper methods
    nlohmann::json appointmentToJson(const models::Appointment& appointment);
    nlohmann::json availabilitySlotToJson(const services::AvailabilitySlot& slot);
    services::BookingRequest parseBookingRequest(const nlohmann::json& json);
    services::RescheduleRequest parseRescheduleRequest(const nlohmann::json& json);
    services::CancellationRequest parseCancellationRequest(const nlohmann::json& json);

    // Validation helpers
    bool validateBookingInput(const nlohmann::json& json, std::string& error_message);
    bool validateRescheduleInput(const nlohmann::json& json, std::string& error_message);
    bool validateCancellationInput(const nlohmann::json& json, std::string& error_message);
    bool validatePaymentInput(const nlohmann::json& json, std::string& error_message);

    // Authentication helpers
    bool isUserAuthorized(const crow::request& req, const std::string& user_id);
    bool isDoctorAuthorized(const crow::request& req, const std::string& doctor_id);
    bool isAdminUser(const crow::request& req);
    std::string getUserIdFromToken(const crow::request& req);
    models::UserRole getUserRoleFromToken(const crow::request& req);

    // Permission checks
    bool canAccessAppointment(const crow::request& req, const std::string& appointment_id);
    bool canModifyAppointment(const crow::request& req, const std::string& appointment_id);
    bool canViewDoctorSchedule(const crow::request& req, const std::string& doctor_id);
    bool canViewClinicSchedule(const crow::request& req, const std::string& clinic_id);

    // Response helpers
    crow::response createSuccessResponse(const nlohmann::json& data);
    crow::response createErrorResponse(const std::string& error, int status_code = 400);
    crow::response createBookingServiceErrorResponse(services::BookingError error, const std::string& message);

    // Query parameter helpers
    std::chrono::system_clock::time_point parseDateTime(const std::string& datetime_str);
    models::AppointmentStatus parseAppointmentStatus(const std::string& status_str);
    models::AppointmentType parseAppointmentType(const std::string& type_str);
    int parseIntParam(const std::string& param, int default_value = 0);
    double parseDoubleParam(const std::string& param, double default_value = 0.0);

    // Filter helpers
    std::string buildDateFilter(const std::string& start_date, const std::string& end_date);
    std::string buildStatusFilter(const std::string& status);
    std::string buildTypeFilter(const std::string& type);
};

} // namespace controllers
} // namespace healthcare