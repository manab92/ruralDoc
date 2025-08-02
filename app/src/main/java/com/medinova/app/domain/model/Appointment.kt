package com.medinova.app.domain.model

import android.os.Parcelable
import kotlinx.parcelize.Parcelize
import java.util.Date

@Parcelize
data class Appointment(
    val id: String,
    val doctorId: String,
    val doctorName: String,
    val doctorImageUrl: String,
    val specialty: String,
    val dateTime: Date,
    val duration: Int, // in minutes
    val status: AppointmentStatus,
    val type: AppointmentType,
    val consultationFee: Int,
    val symptoms: String = "",
    val prescription: String? = null,
    val recordingUrl: String? = null
) : Parcelable

enum class AppointmentStatus {
    SCHEDULED,
    ONGOING,
    COMPLETED,
    CANCELLED
}

enum class AppointmentType {
    VIDEO_CALL,
    VOICE_CALL,
    CHAT
}