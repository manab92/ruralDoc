package com.medinova.app.domain.model

import android.os.Parcelable
import kotlinx.parcelize.Parcelize

@Parcelize
data class Doctor(
    val id: String,
    val name: String,
    val specialty: String,
    val profileImageUrl: String,
    val rating: Float,
    val reviewCount: Int,
    val experience: Int,
    val consultationFee: Int,
    val isOnline: Boolean = false,
    val nextAvailableSlot: String? = null,
    val about: String = "",
    val education: List<String> = emptyList(),
    val achievements: List<String> = emptyList()
) : Parcelable