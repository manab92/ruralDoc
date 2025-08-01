#pragma once

#include <string>
#include <memory>
#include <vector>
#include <optional>
#include "../models/Appointment.h"
#include "../models/Doctor.h"
#include "../models/User.h"
#include "../database/AppointmentRepository.h"
#include "../database/DoctorRepository.h"
#include "../database/UserRepository.h"
#include "PaymentService.h"
#include "NotificationService.h"

namespace healthcare {
namespace services {

struct BookingRequest {
    std::string user_id;
    std::string doctor_id;
    std::string clinic_id;
    std::chrono::system_clock::time_point preferred_date;
    std::chrono::system_clock::time_point preferred_start_time;
    models::AppointmentType type;
    std::string symptoms;
    std::string notes;
    bool is_emergency = false;
    bool is_follow_up = false;
    std::string parent_appointment_id;  // For follow-ups
};

struct RescheduleRequest {
    std::string appointment_id;
    std::chrono::system_clock::time_point new_start_time;
    std::chrono::system_clock::time_point new_end_time;
    std::string reason;
};

struct CancellationRequest {
    std::string appointment_id;
    std::string reason;
    std::string cancelled_by;  // user_id or doctor_id
};

enum class BookingError {
    SUCCESS = 0,
    DOCTOR_NOT_FOUND = 1,
    USER_NOT_FOUND = 2,
    CLINIC_NOT_FOUND = 3,
    DOCTOR_NOT_AVAILABLE = 4,
    TIME_SLOT_OCCUPIED = 5,
    INVALID_TIME_SLOT = 6,
    PAYMENT_FAILED = 7,
    BOOKING_CONFLICT = 8,
    APPOINTMENT_NOT_FOUND = 9,
    UNAUTHORIZED_ACCESS = 10,
    CANNOT_CANCEL = 11,
    CANNOT_RESCHEDULE = 12,
    CLINIC_CLOSED = 13,
    DOCTOR_NOT_VERIFIED = 14,
    INSUFFICIENT_BALANCE = 15,
    EMERGENCY_BOOKING_FAILED = 16,
    FOLLOW_UP_NOT_ALLOWED = 17,
    VALIDATION_ERROR = 18,
    DATABASE_ERROR = 19
};

struct BookingResult {
    BookingError error;
    std::string message;
    std::unique_ptr<models::Appointment> appointment;
    std::string payment_url;  // For payment gateway
};

struct AvailabilitySlot {
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    bool is_available;
    double consultation_fee;
    std::string doctor_id;
    std::string clinic_id;
};

class BookingService {
private:
    std::unique_ptr<database::AppointmentRepository> appointment_repository_;
    std::unique_ptr<database::DoctorRepository> doctor_repository_;
    std::unique_ptr<database::UserRepository> user_repository_;
    std::unique_ptr<PaymentService> payment_service_;
    std::unique_ptr<NotificationService> notification_service_;

public:
    BookingService();
    ~BookingService() = default;

    // Core Booking Operations
    BookingResult bookAppointment(const BookingRequest& request);
    BookingResult rescheduleAppointment(const RescheduleRequest& request);
    BookingResult cancelAppointment(const CancellationRequest& request);
    BookingResult confirmAppointment(const std::string& appointment_id);

    // Appointment Management
    std::optional<std::unique_ptr<models::Appointment>> getAppointmentById(const std::string& appointment_id);
    std::vector<std::unique_ptr<models::Appointment>> getUserAppointments(const std::string& user_id, 
                                                                         models::AppointmentStatus status = models::AppointmentStatus::PENDING);
    std::vector<std::unique_ptr<models::Appointment>> getDoctorAppointments(const std::string& doctor_id,
                                                                           const std::chrono::system_clock::time_point& date);
    std::vector<std::unique_ptr<models::Appointment>> getClinicAppointments(const std::string& clinic_id,
                                                                           const std::chrono::system_clock::time_point& date);

    // Availability Management
    std::vector<AvailabilitySlot> getDoctorAvailability(const std::string& doctor_id,
                                                       const std::chrono::system_clock::time_point& start_date,
                                                       const std::chrono::system_clock::time_point& end_date);
    std::vector<AvailabilitySlot> getClinicAvailability(const std::string& clinic_id,
                                                       const std::chrono::system_clock::time_point& date);
    bool isDoctorAvailable(const std::string& doctor_id,
                          const std::chrono::system_clock::time_point& start_time,
                          const std::chrono::system_clock::time_point& end_time);
    bool isTimeSlotAvailable(const std::string& doctor_id,
                           const std::chrono::system_clock::time_point& start_time,
                           const std::chrono::system_clock::time_point& end_time);

    // Search and Discovery
    std::vector<std::unique_ptr<models::Doctor>> searchAvailableDoctors(const std::string& specialization,
                                                                       const std::string& city,
                                                                       const std::chrono::system_clock::time_point& preferred_date,
                                                                       models::ConsultationType type = models::ConsultationType::BOTH);
    std::vector<std::unique_ptr<models::Doctor>> getNearbyDoctors(double latitude, double longitude, 
                                                                 double radius_km = 10.0);
    std::vector<AvailabilitySlot> getNextAvailableSlots(const std::string& doctor_id, int slot_count = 5);

    // Emergency Booking
    BookingResult bookEmergencyAppointment(const BookingRequest& request);
    std::vector<std::unique_ptr<models::Doctor>> getEmergencyAvailableDoctors(const std::string& city);

    // Follow-up Appointments
    BookingResult bookFollowUpAppointment(const std::string& parent_appointment_id,
                                        const std::chrono::system_clock::time_point& preferred_date);
    bool isFollowUpAllowed(const std::string& parent_appointment_id);

    // Appointment Status Management
    bool markAppointmentCompleted(const std::string& appointment_id);
    bool markAppointmentNoShow(const std::string& appointment_id);
    bool startAppointment(const std::string& appointment_id);  // For online consultations

    // Payment Integration
    BookingResult processPayment(const std::string& appointment_id, const std::string& payment_method);
    bool refundPayment(const std::string& appointment_id);
    bool verifyPayment(const std::string& appointment_id, const std::string& payment_id);

    // Notifications and Reminders
    bool sendAppointmentReminder(const std::string& appointment_id);
    bool sendConfirmationNotification(const std::string& appointment_id);
    bool sendCancellationNotification(const std::string& appointment_id);
    bool sendRescheduleNotification(const std::string& appointment_id);

    // Analytics and Reporting
    std::map<std::string, int> getBookingStatsByDoctor(const std::string& doctor_id, int days = 30);
    std::map<std::string, int> getBookingStatsByClinic(const std::string& clinic_id, int days = 30);
    std::vector<std::pair<std::chrono::system_clock::time_point, int>> getBookingTrends(int days = 30);
    double getAverageBookingValue(const std::string& doctor_id, int days = 30);
    int getCancellationRate(const std::string& doctor_id, int days = 30);

    // Queue Management
    std::vector<std::unique_ptr<models::Appointment>> getAppointmentQueue(const std::string& doctor_id,
                                                                         const std::chrono::system_clock::time_point& date);
    int getQueuePosition(const std::string& appointment_id);
    std::chrono::minutes getEstimatedWaitTime(const std::string& appointment_id);

    // Validation and Business Rules
    bool validateBookingRequest(const BookingRequest& request);
    bool canUserBookAppointment(const std::string& user_id);
    bool isDoctorAcceptingBookings(const std::string& doctor_id);
    bool isClinicOperational(const std::string& clinic_id, 
                           const std::chrono::system_clock::time_point& time);

private:
    // Helper methods
    std::chrono::system_clock::time_point calculateEndTime(const std::chrono::system_clock::time_point& start_time,
                                                          const models::Doctor& doctor);
    bool hasTimeConflict(const std::string& doctor_id,
                        const std::chrono::system_clock::time_point& start_time,
                        const std::chrono::system_clock::time_point& end_time,
                        const std::string& exclude_appointment_id = "");
    double calculateConsultationFee(const models::Doctor& doctor, models::AppointmentType type);
    bool isWithinBookingWindow(const std::chrono::system_clock::time_point& appointment_time);
    bool isValidTimeSlot(const std::chrono::system_clock::time_point& start_time,
                        const std::chrono::system_clock::time_point& end_time);
    std::string generateVideoCallLink(const std::string& appointment_id);
    void logBookingActivity(const std::string& user_id, const std::string& activity);
    bool checkUserBookingLimits(const std::string& user_id);
    void updateDoctorAvailability(const std::string& doctor_id,
                                 const std::chrono::system_clock::time_point& start_time,
                                 const std::chrono::system_clock::time_point& end_time,
                                 bool is_available);
};

} // namespace services
} // namespace healthcare