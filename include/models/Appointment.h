#pragma once

#include <string>
#include <chrono>
#include <memory>
#include "BaseEntity.h"

namespace healthcare {
namespace models {

enum class AppointmentStatus {
    PENDING = 1,
    CONFIRMED = 2,
    CANCELLED = 3,
    COMPLETED = 4,
    NO_SHOW = 5,
    RESCHEDULED = 6
};

enum class AppointmentType {
    ONLINE = 1,
    OFFLINE = 2
};

enum class PaymentStatus {
    PENDING = 1,
    PAID = 2,
    FAILED = 3,
    REFUNDED = 4
};

class Appointment : public BaseEntity {
private:
    std::string user_id_;
    std::string doctor_id_;
    std::string clinic_id_;
    std::chrono::system_clock::time_point appointment_date_;
    std::chrono::system_clock::time_point start_time_;
    std::chrono::system_clock::time_point end_time_;
    AppointmentType type_;
    AppointmentStatus status_;
    double consultation_fee_;
    PaymentStatus payment_status_;
    std::string payment_id_;
    std::string payment_method_;
    std::string symptoms_;
    std::string notes_;
    std::string prescription_id_;
    std::string video_call_link_;
    std::string cancellation_reason_;
    std::chrono::system_clock::time_point cancelled_at_;
    std::string cancelled_by_;  // user_id or doctor_id
    bool is_follow_up_;
    std::string parent_appointment_id_;  // For follow-up appointments
    int reminder_sent_count_;
    std::chrono::system_clock::time_point last_reminder_sent_;

public:
    // Constructors
    Appointment();
    Appointment(const std::string& user_id, const std::string& doctor_id,
                const std::chrono::system_clock::time_point& appointment_date,
                const std::chrono::system_clock::time_point& start_time,
                const std::chrono::system_clock::time_point& end_time,
                AppointmentType type, double consultation_fee);

    // Getters
    const std::string& getUserId() const { return user_id_; }
    const std::string& getDoctorId() const { return doctor_id_; }
    const std::string& getClinicId() const { return clinic_id_; }
    const std::chrono::system_clock::time_point& getAppointmentDate() const { return appointment_date_; }
    const std::chrono::system_clock::time_point& getStartTime() const { return start_time_; }
    const std::chrono::system_clock::time_point& getEndTime() const { return end_time_; }
    AppointmentType getType() const { return type_; }
    AppointmentStatus getStatus() const { return status_; }
    double getConsultationFee() const { return consultation_fee_; }
    PaymentStatus getPaymentStatus() const { return payment_status_; }
    const std::string& getPaymentId() const { return payment_id_; }
    const std::string& getPaymentMethod() const { return payment_method_; }
    const std::string& getSymptoms() const { return symptoms_; }
    const std::string& getNotes() const { return notes_; }
    const std::string& getPrescriptionId() const { return prescription_id_; }
    const std::string& getVideoCallLink() const { return video_call_link_; }
    const std::string& getCancellationReason() const { return cancellation_reason_; }
    const std::chrono::system_clock::time_point& getCancelledAt() const { return cancelled_at_; }
    const std::string& getCancelledBy() const { return cancelled_by_; }
    bool isFollowUp() const { return is_follow_up_; }
    const std::string& getParentAppointmentId() const { return parent_appointment_id_; }
    int getReminderSentCount() const { return reminder_sent_count_; }
    const std::chrono::system_clock::time_point& getLastReminderSent() const { return last_reminder_sent_; }

    // Setters
    void setUserId(const std::string& user_id) { user_id_ = user_id; }
    void setDoctorId(const std::string& doctor_id) { doctor_id_ = doctor_id; }
    void setClinicId(const std::string& clinic_id) { clinic_id_ = clinic_id; }
    void setAppointmentDate(const std::chrono::system_clock::time_point& appointment_date) { appointment_date_ = appointment_date; }
    void setStartTime(const std::chrono::system_clock::time_point& start_time) { start_time_ = start_time; }
    void setEndTime(const std::chrono::system_clock::time_point& end_time) { end_time_ = end_time; }
    void setType(AppointmentType type) { type_ = type; }
    void setStatus(AppointmentStatus status) { status_ = status; }
    void setConsultationFee(double consultation_fee) { consultation_fee_ = consultation_fee; }
    void setPaymentStatus(PaymentStatus payment_status) { payment_status_ = payment_status; }
    void setPaymentId(const std::string& payment_id) { payment_id_ = payment_id; }
    void setPaymentMethod(const std::string& payment_method) { payment_method_ = payment_method; }
    void setSymptoms(const std::string& symptoms) { symptoms_ = symptoms; }
    void setNotes(const std::string& notes) { notes_ = notes; }
    void setPrescriptionId(const std::string& prescription_id) { prescription_id_ = prescription_id; }
    void setVideoCallLink(const std::string& video_call_link) { video_call_link_ = video_call_link; }
    void setCancellationReason(const std::string& cancellation_reason) { cancellation_reason_ = cancellation_reason; }
    void setCancelledAt(const std::chrono::system_clock::time_point& cancelled_at) { cancelled_at_ = cancelled_at; }
    void setCancelledBy(const std::string& cancelled_by) { cancelled_by_ = cancelled_by; }
    void setFollowUp(bool is_follow_up) { is_follow_up_ = is_follow_up; }
    void setParentAppointmentId(const std::string& parent_appointment_id) { parent_appointment_id_ = parent_appointment_id; }
    void setReminderSentCount(int reminder_sent_count) { reminder_sent_count_ = reminder_sent_count; }
    void setLastReminderSent(const std::chrono::system_clock::time_point& last_reminder_sent) { last_reminder_sent_ = last_reminder_sent; }

    // Business methods
    void confirmAppointment();
    void cancelAppointment(const std::string& reason, const std::string& cancelled_by);
    void completeAppointment();
    void markAsNoShow();
    void rescheduleAppointment(const std::chrono::system_clock::time_point& new_start_time,
                              const std::chrono::system_clock::time_point& new_end_time);

    // Payment methods
    void markPaymentPaid(const std::string& payment_id, const std::string& payment_method);
    void markPaymentFailed();
    void refundPayment();

    // Utility methods
    bool canBeCancelled() const;
    bool canBeRescheduled() const;
    bool isUpcoming() const;
    bool isPast() const;
    bool isToday() const;
    std::chrono::minutes getDuration() const;
    bool needsReminder() const;
    void incrementReminderCount();

    // Validation
    bool isValidTimeSlot() const;
    bool isConflictingWith(const Appointment& other) const;

    // Serialization
    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& json) override;
};

} // namespace models
} // namespace healthcare