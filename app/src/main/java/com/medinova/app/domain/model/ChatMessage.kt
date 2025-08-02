package com.medinova.app.domain.model

import android.os.Parcelable
import kotlinx.parcelize.Parcelize
import java.util.Date

@Parcelize
data class ChatMessage(
    val id: String,
    val senderId: String,
    val senderName: String,
    val receiverId: String,
    val content: String,
    val type: MessageType,
    val timestamp: Date,
    val isRead: Boolean = false,
    val audioUrl: String? = null,
    val audioDuration: Int? = null, // in seconds
    val imageUrl: String? = null
) : Parcelable

enum class MessageType {
    TEXT,
    AUDIO,
    IMAGE,
    SYSTEM
}