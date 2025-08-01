#include "../../include/models/Appointment.h"
#include "../../include/utils/CryptoUtils.h"
#include <random>
#include <sstream>
#include <iomanip>

namespace healthcare::models {

Appointment::Appointment() 
    : BaseEntity(),
      type_(AppointmentType::OFFLINE),
      status_(AppointmentStatus::PENDING),
      is_emergency_(false),
      consultation_fee_(0.0) {
    booked_at_ = std::chrono::system_clock::now();
    confirmation_code_ = generateConfirmationCode();
}

void Appointment::confirmAppointment() {
    status_ = AppointmentStatus::CONFIRMED;
    confirmed_at_ = std::chrono::system_clock::now();
    updateTimestamp();
}

void Appointment::startConsultation() {
    status_ = AppointmentStatus::IN_PROGRESS;
    consultation_info_.call_started_at = std::chrono::system_clock::now();
    updateTimestamp();
}

void Appointment::completeConsultation() {
    status_ = AppointmentStatus::COMPLETED;
    consultation_info_.call_ended_at = std::chrono::system_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::minutes>(
        consultation_info_.call_ended_at - consultation_info_.call_started_at
    );
    consultation_info_.duration_minutes = static_cast<int>(duration.count());
    
    updateTimestamp();
}

void Appointment::cancelAppointment(CancellationReason reason, const std::string& description, const std::string& cancelled_by) {
    status_ = AppointmentStatus::CANCELLED;
    
    cancellation_info_.reason = reason;
    cancellation_info_.description = description;
    cancellation_info_.cancelled_at = std::chrono::system_clock::now();
    cancellation_info_.cancelled_by_user_id = cancelled_by;
    
    updateTimestamp();
}

void Appointment::markNoShow() {
    status_ = AppointmentStatus::NO_SHOW;
    updateTimestamp();
}

void Appointment::rescheduleAppointment(const std::chrono::system_clock::time_point& new_start_time) {
    status_ = AppointmentStatus::RESCHEDULED;
    start_time_ = new_start_time;
    
    // Calculate new end time based on duration
    auto duration = end_time_ - start_time_;
    end_time_ = new_start_time + duration;
    
    updateTimestamp();
}

void Appointment::processPayment(const PaymentInfo& payment_info) {
    payment_info_ = payment_info;
    updateTimestamp();
}

void Appointment::markPaymentFailed(const std::string& reason) {
    payment_info_.status = PaymentStatus::FAILED;
    updateTimestamp();
}

void Appointment::processRefund(double refund_amount, const std::string& refund_id) {
    cancellation_info_.refund_amount = refund_amount;
    cancellation_info_.refund_id = refund_id;
    cancellation_info_.is_refund_processed = true;
    payment_info_.status = PaymentStatus::REFUNDED;
    updateTimestamp();
}

void Appointment::generateVideoCallLink() {
    if (type_ == AppointmentType::ONLINE) {
        consultation_info_.meeting_id = generateMeetingId();
        consultation_info_.room_password = utils::CryptoUtils::generateRandomString(8);
        consultation_info_.video_call_link = "https://meet.healthcare.com/room/" + consultation_info_.meeting_id;
        updateTimestamp();
    }
}

void Appointment::startVideoCall() {
    consultation_info_.call_started_at = std::chrono::system_clock::now();
    updateTimestamp();
}

void Appointment::endVideoCall() {
    consultation_info_.call_ended_at = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::minutes>(
        consultation_info_.call_ended_at - consultation_info_.call_started_at
    );
    consultation_info_.duration_minutes = static_cast<int>(duration.count());
    updateTimestamp();
}

bool Appointment::isCallActive() const {
    return consultation_info_.call_started_at.time_since_epoch().count() > 0 &&
           consultation_info_.call_ended_at.time_since_epoch().count() == 0;
}

bool Appointment::canBeCancelled() const {
    if (status_ == AppointmentStatus::COMPLETED || 
        status_ == AppointmentStatus::CANCELLED ||
        status_ == AppointmentStatus::NO_SHOW) {
        return false;
    }
    
    // Check if appointment is in the past
    if (start_time_ < std::chrono::system_clock::now()) {
        return false;
    }
    
    return true;
}

bool Appointment::canBeRescheduled() const {
    if (status_ == AppointmentStatus::COMPLETED || 
        status_ == AppointmentStatus::CANCELLED ||
        status_ == AppointmentStatus::NO_SHOW ||
        status_ == AppointmentStatus::IN_PROGRESS) {
        return false;
    }
    
    // Check if appointment is too close (less than 2 hours)
    auto now = std::chrono::system_clock::now();
    auto time_until = std::chrono::duration_cast<std::chrono::hours>(start_time_ - now);
    
    return time_until.count() >= 2;
}

bool Appointment::requiresRefund() const {
    return status_ == AppointmentStatus::CANCELLED && 
           payment_info_.status == PaymentStatus::PAID &&
           !cancellation_info_.is_refund_processed;
}

int Appointment::getDurationMinutes() const {
    auto duration = std::chrono::duration_cast<std::chrono::minutes>(end_time_ - start_time_);
    return static_cast<int>(duration.count());
}

std::chrono::minutes Appointment::getTimeUntilAppointment() const {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::minutes>(start_time_ - now);
}

bool Appointment::isUpcoming() const {
    return start_time_ > std::chrono::system_clock::now() &&
           (status_ == AppointmentStatus::PENDING || status_ == AppointmentStatus::CONFIRMED);
}

bool Appointment::isPast() const {
    return end_time_ < std::chrono::system_clock::now();
}

bool Appointment::isToday() const {
    auto now = std::chrono::system_clock::now();
    auto today = std::chrono::system_clock::to_time_t(now);
    auto appointment_day = std::chrono::system_clock::to_time_t(appointment_date_);
    
    std::tm* now_tm = std::localtime(&today);
    std::tm* appointment_tm = std::localtime(&appointment_day);
    
    return now_tm->tm_year == appointment_tm->tm_year &&
           now_tm->tm_mon == appointment_tm->tm_mon &&
           now_tm->tm_mday == appointment_tm->tm_mday;
}

bool Appointment::isValidTimeSlot() const {
    return start_time_ < end_time_ && getDurationMinutes() >= 15;
}

bool Appointment::isValidFutureDate() const {
    return start_time_ > std::chrono::system_clock::now();
}

nlohmann::json Appointment::toJson() const {
    nlohmann::json json;
    
    // Base entity fields
    json["id"] = getId();
    json["created_at"] = std::chrono::system_clock::to_time_t(getCreatedAt());
    json["updated_at"] = std::chrono::system_clock::to_time_t(getUpdatedAt());
    json["is_deleted"] = isDeleted();
    
    // Core appointment fields
    json["user_id"] = user_id_;
    json["doctor_id"] = doctor_id_;
    json["clinic_id"] = clinic_id_;
    json["appointment_date"] = std::chrono::system_clock::to_time_t(appointment_date_);
    json["start_time"] = std::chrono::system_clock::to_time_t(start_time_);
    json["end_time"] = std::chrono::system_clock::to_time_t(end_time_);
    json["type"] = appointmentTypeToString(type_);
    json["status"] = appointmentStatusToString(status_);
    
    // Patient information
    json["symptoms"] = symptoms_;
    json["notes"] = notes_;
    json["is_emergency"] = is_emergency_;
    json["patient_age"] = patient_age_;
    json["patient_gender"] = patient_gender_;
    
    // Financial
    json["consultation_fee"] = consultation_fee_;
    
    // Payment info
    nlohmann::json payment_json;
    payment_json["payment_id"] = payment_info_.payment_id;
    payment_json["order_id"] = payment_info_.order_id;
    payment_json["transaction_id"] = payment_info_.transaction_id;
    payment_json["amount"] = payment_info_.amount;
    payment_json["currency"] = payment_info_.currency;
    payment_json["status"] = paymentStatusToString(payment_info_.status);
    payment_json["payment_method"] = payment_info_.payment_method;
    payment_json["paid_at"] = std::chrono::system_clock::to_time_t(payment_info_.paid_at);
    payment_json["razorpay_signature"] = payment_info_.razorpay_signature;
    json["payment_info"] = payment_json;
    
    // Management
    json["confirmation_code"] = confirmation_code_;
    json["booked_at"] = std::chrono::system_clock::to_time_t(booked_at_);
    json["confirmed_at"] = std::chrono::system_clock::to_time_t(confirmed_at_);
    
    // Consultation info
    nlohmann::json consultation_json;
    consultation_json["video_call_link"] = consultation_info_.video_call_link;
    consultation_json["meeting_id"] = consultation_info_.meeting_id;
    consultation_json["room_password"] = consultation_info_.room_password;
    consultation_json["call_started_at"] = std::chrono::system_clock::to_time_t(consultation_info_.call_started_at);
    consultation_json["call_ended_at"] = std::chrono::system_clock::to_time_t(consultation_info_.call_ended_at);
    consultation_json["duration_minutes"] = consultation_info_.duration_minutes;
    consultation_json["recording_url"] = consultation_info_.recording_url;
    consultation_json["call_notes"] = consultation_info_.call_notes;
    json["consultation_info"] = consultation_json;
    
    // Cancellation info
    if (status_ == AppointmentStatus::CANCELLED) {
        nlohmann::json cancellation_json;
        cancellation_json["reason"] = cancellationReasonToString(cancellation_info_.reason);
        cancellation_json["description"] = cancellation_info_.description;
        cancellation_json["cancelled_at"] = std::chrono::system_clock::to_time_t(cancellation_info_.cancelled_at);
        cancellation_json["cancelled_by_user_id"] = cancellation_info_.cancelled_by_user_id;
        cancellation_json["refund_amount"] = cancellation_info_.refund_amount;
        cancellation_json["refund_id"] = cancellation_info_.refund_id;
        cancellation_json["is_refund_processed"] = cancellation_info_.is_refund_processed;
        json["cancellation_info"] = cancellation_json;
    }
    
    // Follow-up
    json["prescription_id"] = prescription_id_;
    json["follow_up_date"] = std::chrono::system_clock::to_time_t(follow_up_date_);
    json["follow_up_notes"] = follow_up_notes_;
    
    return json;
}

void Appointment::fromJson(const nlohmann::json& json) {
    // Base entity fields
    if (json.contains("id")) setId(json["id"].get<std::string>());
    if (json.contains("created_at")) {
        auto timestamp = json["created_at"].get<std::time_t>();
        setCreatedAt(std::chrono::system_clock::from_time_t(timestamp));
    }
    if (json.contains("updated_at")) {
        auto timestamp = json["updated_at"].get<std::time_t>();
        setUpdatedAt(std::chrono::system_clock::from_time_t(timestamp));
    }
    if (json.contains("is_deleted")) setDeleted(json["is_deleted"].get<bool>());
    
    // Core appointment fields
    if (json.contains("user_id")) user_id_ = json["user_id"].get<std::string>();
    if (json.contains("doctor_id")) doctor_id_ = json["doctor_id"].get<std::string>();
    if (json.contains("clinic_id")) clinic_id_ = json["clinic_id"].get<std::string>();
    if (json.contains("appointment_date")) {
        appointment_date_ = std::chrono::system_clock::from_time_t(json["appointment_date"].get<std::time_t>());
    }
    if (json.contains("start_time")) {
        start_time_ = std::chrono::system_clock::from_time_t(json["start_time"].get<std::time_t>());
    }
    if (json.contains("end_time")) {
        end_time_ = std::chrono::system_clock::from_time_t(json["end_time"].get<std::time_t>());
    }
    if (json.contains("type")) type_ = stringToAppointmentType(json["type"].get<std::string>());
    if (json.contains("status")) status_ = stringToAppointmentStatus(json["status"].get<std::string>());
    
    // Patient information
    if (json.contains("symptoms")) symptoms_ = json["symptoms"].get<std::string>();
    if (json.contains("notes")) notes_ = json["notes"].get<std::string>();
    if (json.contains("is_emergency")) is_emergency_ = json["is_emergency"].get<bool>();
    if (json.contains("patient_age")) patient_age_ = json["patient_age"].get<std::string>();
    if (json.contains("patient_gender")) patient_gender_ = json["patient_gender"].get<std::string>();
    
    // Financial
    if (json.contains("consultation_fee")) consultation_fee_ = json["consultation_fee"].get<double>();
    
    // Payment info
    if (json.contains("payment_info")) {
        auto payment_json = json["payment_info"];
        payment_info_.payment_id = payment_json.value("payment_id", "");
        payment_info_.order_id = payment_json.value("order_id", "");
        payment_info_.transaction_id = payment_json.value("transaction_id", "");
        payment_info_.amount = payment_json.value("amount", 0.0);
        payment_info_.currency = payment_json.value("currency", "INR");
        payment_info_.status = stringToPaymentStatus(payment_json.value("status", "PENDING"));
        payment_info_.payment_method = payment_json.value("payment_method", "");
        if (payment_json.contains("paid_at")) {
            payment_info_.paid_at = std::chrono::system_clock::from_time_t(payment_json["paid_at"].get<std::time_t>());
        }
        payment_info_.razorpay_signature = payment_json.value("razorpay_signature", "");
    }
    
    // Management
    if (json.contains("confirmation_code")) confirmation_code_ = json["confirmation_code"].get<std::string>();
    if (json.contains("booked_at")) {
        booked_at_ = std::chrono::system_clock::from_time_t(json["booked_at"].get<std::time_t>());
    }
    if (json.contains("confirmed_at")) {
        confirmed_at_ = std::chrono::system_clock::from_time_t(json["confirmed_at"].get<std::time_t>());
    }
    
    // Consultation info
    if (json.contains("consultation_info")) {
        auto consultation_json = json["consultation_info"];
        consultation_info_.video_call_link = consultation_json.value("video_call_link", "");
        consultation_info_.meeting_id = consultation_json.value("meeting_id", "");
        consultation_info_.room_password = consultation_json.value("room_password", "");
        if (consultation_json.contains("call_started_at")) {
            consultation_info_.call_started_at = std::chrono::system_clock::from_time_t(
                consultation_json["call_started_at"].get<std::time_t>()
            );
        }
        if (consultation_json.contains("call_ended_at")) {
            consultation_info_.call_ended_at = std::chrono::system_clock::from_time_t(
                consultation_json["call_ended_at"].get<std::time_t>()
            );
        }
        consultation_info_.duration_minutes = consultation_json.value("duration_minutes", 0);
        consultation_info_.recording_url = consultation_json.value("recording_url", "");
        consultation_info_.call_notes = consultation_json.value("call_notes", "");
    }
    
    // Cancellation info
    if (json.contains("cancellation_info")) {
        auto cancellation_json = json["cancellation_info"];
        cancellation_info_.reason = stringToCancellationReason(cancellation_json.value("reason", "OTHER"));
        cancellation_info_.description = cancellation_json.value("description", "");
        if (cancellation_json.contains("cancelled_at")) {
            cancellation_info_.cancelled_at = std::chrono::system_clock::from_time_t(
                cancellation_json["cancelled_at"].get<std::time_t>()
            );
        }
        cancellation_info_.cancelled_by_user_id = cancellation_json.value("cancelled_by_user_id", "");
        cancellation_info_.refund_amount = cancellation_json.value("refund_amount", 0.0);
        cancellation_info_.refund_id = cancellation_json.value("refund_id", "");
        cancellation_info_.is_refund_processed = cancellation_json.value("is_refund_processed", false);
    }
    
    // Follow-up
    if (json.contains("prescription_id")) prescription_id_ = json["prescription_id"].get<std::string>();
    if (json.contains("follow_up_date")) {
        follow_up_date_ = std::chrono::system_clock::from_time_t(json["follow_up_date"].get<std::time_t>());
    }
    if (json.contains("follow_up_notes")) follow_up_notes_ = json["follow_up_notes"].get<std::string>();
}

std::string Appointment::generateConfirmationCode() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100000, 999999);
    return "APT" + std::to_string(dis(gen));
}

std::string Appointment::generateMeetingId() {
    return utils::CryptoUtils::generateRandomString(10, true);
}

// Utility functions
std::string appointmentStatusToString(AppointmentStatus status) {
    switch (status) {
        case AppointmentStatus::PENDING: return "PENDING";
        case AppointmentStatus::CONFIRMED: return "CONFIRMED";
        case AppointmentStatus::IN_PROGRESS: return "IN_PROGRESS";
        case AppointmentStatus::COMPLETED: return "COMPLETED";
        case AppointmentStatus::CANCELLED: return "CANCELLED";
        case AppointmentStatus::NO_SHOW: return "NO_SHOW";
        case AppointmentStatus::RESCHEDULED: return "RESCHEDULED";
        default: return "PENDING";
    }
}

AppointmentStatus stringToAppointmentStatus(const std::string& status_str) {
    if (status_str == "CONFIRMED") return AppointmentStatus::CONFIRMED;
    if (status_str == "IN_PROGRESS") return AppointmentStatus::IN_PROGRESS;
    if (status_str == "COMPLETED") return AppointmentStatus::COMPLETED;
    if (status_str == "CANCELLED") return AppointmentStatus::CANCELLED;
    if (status_str == "NO_SHOW") return AppointmentStatus::NO_SHOW;
    if (status_str == "RESCHEDULED") return AppointmentStatus::RESCHEDULED;
    return AppointmentStatus::PENDING;
}

std::string appointmentTypeToString(AppointmentType type) {
    switch (type) {
        case AppointmentType::ONLINE: return "ONLINE";
        case AppointmentType::OFFLINE: return "OFFLINE";
        default: return "OFFLINE";
    }
}

AppointmentType stringToAppointmentType(const std::string& type_str) {
    if (type_str == "ONLINE") return AppointmentType::ONLINE;
    return AppointmentType::OFFLINE;
}

std::string paymentStatusToString(PaymentStatus status) {
    switch (status) {
        case PaymentStatus::PENDING: return "PENDING";
        case PaymentStatus::PAID: return "PAID";
        case PaymentStatus::FAILED: return "FAILED";
        case PaymentStatus::REFUNDED: return "REFUNDED";
        case PaymentStatus::PARTIALLY_REFUNDED: return "PARTIALLY_REFUNDED";
        default: return "PENDING";
    }
}

PaymentStatus stringToPaymentStatus(const std::string& status_str) {
    if (status_str == "PAID") return PaymentStatus::PAID;
    if (status_str == "FAILED") return PaymentStatus::FAILED;
    if (status_str == "REFUNDED") return PaymentStatus::REFUNDED;
    if (status_str == "PARTIALLY_REFUNDED") return PaymentStatus::PARTIALLY_REFUNDED;
    return PaymentStatus::PENDING;
}

std::string cancellationReasonToString(CancellationReason reason) {
    switch (reason) {
        case CancellationReason::PATIENT_REQUEST: return "PATIENT_REQUEST";
        case CancellationReason::DOCTOR_UNAVAILABLE: return "DOCTOR_UNAVAILABLE";
        case CancellationReason::EMERGENCY: return "EMERGENCY";
        case CancellationReason::TECHNICAL_ISSUE: return "TECHNICAL_ISSUE";
        case CancellationReason::WEATHER: return "WEATHER";
        case CancellationReason::OTHER: return "OTHER";
        default: return "OTHER";
    }
}

CancellationReason stringToCancellationReason(const std::string& reason_str) {
    if (reason_str == "PATIENT_REQUEST") return CancellationReason::PATIENT_REQUEST;
    if (reason_str == "DOCTOR_UNAVAILABLE") return CancellationReason::DOCTOR_UNAVAILABLE;
    if (reason_str == "EMERGENCY") return CancellationReason::EMERGENCY;
    if (reason_str == "TECHNICAL_ISSUE") return CancellationReason::TECHNICAL_ISSUE;
    if (reason_str == "WEATHER") return CancellationReason::WEATHER;
    return CancellationReason::OTHER;
}

} // namespace healthcare::models