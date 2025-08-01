#pragma once

#include "BaseEntity.h"
#include <string>
#include <chrono>
#include <vector>

namespace healthcare::models {

enum class AppointmentStatus {
    PENDING,
    CONFIRMED,
    IN_PROGRESS,
    COMPLETED,
    CANCELLED,
    NO_SHOW,
    RESCHEDULED
};

enum class AppointmentType {
    ONLINE,
    OFFLINE
};

enum class PaymentStatus {
    PENDING,
    PAID,
    FAILED,
    REFUNDED,
    PARTIALLY_REFUNDED
};

enum class CancellationReason {
    PATIENT_REQUEST,
    DOCTOR_UNAVAILABLE,
    EMERGENCY,
    TECHNICAL_ISSUE,
    WEATHER,
    OTHER
};

struct PaymentInfo {
    std::string payment_id;
    std::string order_id;
    std::string transaction_id;
    double amount;
    std::string currency;
    PaymentStatus status;
    std::string payment_method;  // RAZORPAY, CARD, UPI, etc.
    std::chrono::system_clock::time_point paid_at;
    std::string razorpay_signature;
};

struct CancellationInfo {
    CancellationReason reason;
    std::string description;
    std::chrono::system_clock::time_point cancelled_at;
    std::string cancelled_by_user_id;
    double refund_amount;
    std::string refund_id;
    bool is_refund_processed;
};

struct ConsultationInfo {
    std::string video_call_link;
    std::string meeting_id;
    std::string room_password;
    std::chrono::system_clock::time_point call_started_at;
    std::chrono::system_clock::time_point call_ended_at;
    int duration_minutes;
    std::string recording_url;  // Optional consultation recording
    std::string call_notes;     // Doctor's notes during call
};

class Appointment : public BaseEntity {
public:
    Appointment();
    ~Appointment() override = default;

    // Core appointment details
    const std::string& getUserId() const { return user_id_; }
    const std::string& getDoctorId() const { return doctor_id_; }
    const std::string& getClinicId() const { return clinic_id_; }
    const std::chrono::system_clock::time_point& getAppointmentDate() const { return appointment_date_; }
    const std::chrono::system_clock::time_point& getStartTime() const { return start_time_; }
    const std::chrono::system_clock::time_point& getEndTime() const { return end_time_; }
    AppointmentType getType() const { return type_; }
    AppointmentStatus getStatus() const { return status_; }

    // Patient information
    const std::string& getSymptoms() const { return symptoms_; }
    const std::string& getNotes() const { return notes_; }
    bool isEmergency() const { return is_emergency_; }
    const std::string& getPatientAge() const { return patient_age_; }
    const std::string& getPatientGender() const { return patient_gender_; }

    // Financial details
    double getConsultationFee() const { return consultation_fee_; }
    const PaymentInfo& getPaymentInfo() const { return payment_info_; }

    // Appointment management
    const std::string& getConfirmationCode() const { return confirmation_code_; }
    const std::chrono::system_clock::time_point& getBookedAt() const { return booked_at_; }
    const std::chrono::system_clock::time_point& getConfirmedAt() const { return confirmed_at_; }

    // Consultation details
    const ConsultationInfo& getConsultationInfo() const { return consultation_info_; }
    const CancellationInfo& getCancellationInfo() const { return cancellation_info_; }

    // Follow-up information
    const std::string& getPrescriptionId() const { return prescription_id_; }
    const std::chrono::system_clock::time_point& getFollowUpDate() const { return follow_up_date_; }
    const std::string& getFollowUpNotes() const { return follow_up_notes_; }

    // Setters
    void setUserId(const std::string& user_id) { user_id_ = user_id; }
    void setDoctorId(const std::string& doctor_id) { doctor_id_ = doctor_id; }
    void setClinicId(const std::string& clinic_id) { clinic_id_ = clinic_id; }
    void setAppointmentDate(const std::chrono::system_clock::time_point& date) { appointment_date_ = date; }
    void setStartTime(const std::chrono::system_clock::time_point& start_time) { start_time_ = start_time; }
    void setEndTime(const std::chrono::system_clock::time_point& end_time) { end_time_ = end_time; }
    void setType(AppointmentType type) { type_ = type; }
    void setStatus(AppointmentStatus status) { status_ = status; updateTimestamp(); }
    void setSymptoms(const std::string& symptoms) { symptoms_ = symptoms; }
    void setNotes(const std::string& notes) { notes_ = notes; }
    void setEmergency(bool is_emergency) { is_emergency_ = is_emergency; }
    void setPatientAge(const std::string& age) { patient_age_ = age; }
    void setPatientGender(const std::string& gender) { patient_gender_ = gender; }
    void setConsultationFee(double fee) { consultation_fee_ = fee; }
    void setPaymentInfo(const PaymentInfo& payment_info) { payment_info_ = payment_info; }
    void setConfirmationCode(const std::string& code) { confirmation_code_ = code; }
    void setBookedAt(const std::chrono::system_clock::time_point& booked_at) { booked_at_ = booked_at; }
    void setConfirmedAt(const std::chrono::system_clock::time_point& confirmed_at) { confirmed_at_ = confirmed_at; }
    void setConsultationInfo(const ConsultationInfo& consultation_info) { consultation_info_ = consultation_info; }
    void setCancellationInfo(const CancellationInfo& cancellation_info) { cancellation_info_ = cancellation_info; }
    void setPrescriptionId(const std::string& prescription_id) { prescription_id_ = prescription_id; }
    void setFollowUpDate(const std::chrono::system_clock::time_point& date) { follow_up_date_ = date; }
    void setFollowUpNotes(const std::string& notes) { follow_up_notes_ = notes; }

    // Status management
    void confirmAppointment();
    void startConsultation();
    void completeConsultation();
    void cancelAppointment(CancellationReason reason, const std::string& description, const std::string& cancelled_by);
    void markNoShow();
    void rescheduleAppointment(const std::chrono::system_clock::time_point& new_start_time);

    // Payment operations
    void processPayment(const PaymentInfo& payment_info);
    void markPaymentFailed(const std::string& reason);
    void processRefund(double refund_amount, const std::string& refund_id);

    // Consultation management
    void generateVideoCallLink();
    void startVideoCall();
    void endVideoCall();
    bool isCallActive() const;

    // Utility methods
    bool isPending() const { return status_ == AppointmentStatus::PENDING; }
    bool isConfirmed() const { return status_ == AppointmentStatus::CONFIRMED; }
    bool isCompleted() const { return status_ == AppointmentStatus::COMPLETED; }
    bool isCancelled() const { return status_ == AppointmentStatus::CANCELLED; }
    bool isOnline() const { return type_ == AppointmentType::ONLINE; }
    bool isOffline() const { return type_ == AppointmentType::OFFLINE; }
    bool isPaymentPending() const { return payment_info_.status == PaymentStatus::PENDING; }
    bool isPaymentCompleted() const { return payment_info_.status == PaymentStatus::PAID; }
    bool canBeCancelled() const;
    bool canBeRescheduled() const;
    bool requiresRefund() const;
    
    int getDurationMinutes() const;
    std::chrono::minutes getTimeUntilAppointment() const;
    bool isUpcoming() const;
    bool isPast() const;
    bool isToday() const;

    // Validation
    bool isValidTimeSlot() const;
    bool isValidFutureDate() const;

    // Serialization
    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& json) override;

private:
    // Core details
    std::string user_id_;
    std::string doctor_id_;
    std::string clinic_id_;
    std::chrono::system_clock::time_point appointment_date_;
    std::chrono::system_clock::time_point start_time_;
    std::chrono::system_clock::time_point end_time_;
    AppointmentType type_;
    AppointmentStatus status_;

    // Patient information
    std::string symptoms_;
    std::string notes_;
    bool is_emergency_;
    std::string patient_age_;
    std::string patient_gender_;

    // Financial
    double consultation_fee_;
    PaymentInfo payment_info_;

    // Management
    std::string confirmation_code_;
    std::chrono::system_clock::time_point booked_at_;
    std::chrono::system_clock::time_point confirmed_at_;

    // Consultation and cancellation details
    ConsultationInfo consultation_info_;
    CancellationInfo cancellation_info_;

    // Follow-up
    std::string prescription_id_;
    std::chrono::system_clock::time_point follow_up_date_;
    std::string follow_up_notes_;

    // Helper methods
    std::string generateConfirmationCode();
    std::string generateMeetingId();
};

// Utility functions
std::string appointmentStatusToString(AppointmentStatus status);
AppointmentStatus stringToAppointmentStatus(const std::string& status_str);
std::string appointmentTypeToString(AppointmentType type);
AppointmentType stringToAppointmentType(const std::string& type_str);
std::string paymentStatusToString(PaymentStatus status);
PaymentStatus stringToPaymentStatus(const std::string& status_str);
std::string cancellationReasonToString(CancellationReason reason);
CancellationReason stringToCancellationReason(const std::string& reason_str);

} // namespace healthcare::models