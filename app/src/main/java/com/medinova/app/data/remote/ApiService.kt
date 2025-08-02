package com.medinova.app.data.remote

import com.medinova.app.domain.model.Doctor
import com.medinova.app.domain.model.Appointment
import com.medinova.app.domain.model.ChatMessage
import retrofit2.Response
import retrofit2.http.*

interface ApiService {
    
    // Authentication endpoints (matches C++ backend)
    @POST("api/auth/register")
    suspend fun register(@Body request: RegisterRequest): Response<AuthResponse>
    
    @POST("api/auth/login") 
    suspend fun login(@Body request: LoginRequest): Response<AuthResponse>
    
    @POST("api/auth/refresh")
    suspend fun refreshToken(@Body request: RefreshTokenRequest): Response<AuthResponse>
    
    // Doctor endpoints
    @GET("api/doctors")
    suspend fun getDoctors(
        @Query("specialty") specialty: String? = null,
        @Query("search") search: String? = null,
        @Query("page") page: Int = 1,
        @Query("limit") limit: Int = 20
    ): Response<DoctorsResponse>
    
    @GET("api/doctors/{id}")
    suspend fun getDoctorById(@Path("id") doctorId: String): Response<Doctor>
    
    @GET("api/doctors/specialties")
    suspend fun getSpecialties(): Response<List<String>>
    
    // Appointment endpoints  
    @GET("api/appointments")
    suspend fun getAppointments(
        @Header("Authorization") token: String,
        @Query("status") status: String? = null
    ): Response<AppointmentsResponse>
    
    @POST("api/appointments")
    suspend fun createAppointment(
        @Header("Authorization") token: String,
        @Body request: CreateAppointmentRequest
    ): Response<Appointment>
    
    @PUT("api/appointments/{id}")
    suspend fun updateAppointment(
        @Header("Authorization") token: String,
        @Path("id") appointmentId: String,
        @Body request: UpdateAppointmentRequest
    ): Response<Appointment>
    
    @DELETE("api/appointments/{id}")
    suspend fun cancelAppointment(
        @Header("Authorization") token: String,
        @Path("id") appointmentId: String
    ): Response<Unit>
    
    // Chat endpoints
    @GET("api/chat/conversations")
    suspend fun getConversations(
        @Header("Authorization") token: String
    ): Response<List<ChatConversation>>
    
    @GET("api/chat/messages/{doctorId}")
    suspend fun getMessages(
        @Header("Authorization") token: String,
        @Path("doctorId") doctorId: String,
        @Query("page") page: Int = 1
    ): Response<MessagesResponse>
    
    @POST("api/chat/messages")
    suspend fun sendMessage(
        @Header("Authorization") token: String,
        @Body request: SendMessageRequest
    ): Response<ChatMessage>
    
    // Profile endpoints
    @GET("api/profile")
    suspend fun getProfile(
        @Header("Authorization") token: String
    ): Response<UserProfile>
    
    @PUT("api/profile")
    suspend fun updateProfile(
        @Header("Authorization") token: String,
        @Body request: UpdateProfileRequest
    ): Response<UserProfile>
}

// Data classes for API requests/responses
data class RegisterRequest(
    val name: String,
    val email: String,
    val password: String,
    val phone: String
)

data class LoginRequest(
    val email: String,
    val password: String
)

data class RefreshTokenRequest(
    val refreshToken: String
)

data class AuthResponse(
    val success: Boolean,
    val message: String,
    val data: AuthData? = null
)

data class AuthData(
    val user: UserProfile,
    val accessToken: String,
    val refreshToken: String,
    val expiresIn: Long
)

data class DoctorsResponse(
    val success: Boolean,
    val data: List<Doctor>,
    val totalCount: Int,
    val page: Int,
    val totalPages: Int
)

data class AppointmentsResponse(
    val success: Boolean,
    val data: List<Appointment>
)

data class CreateAppointmentRequest(
    val doctorId: String,
    val dateTime: String, // ISO format
    val type: String, // "video", "voice", "chat"
    val symptoms: String
)

data class UpdateAppointmentRequest(
    val dateTime: String? = null,
    val status: String? = null
)

data class ChatConversation(
    val doctorId: String,
    val doctorName: String,
    val lastMessage: ChatMessage,
    val unreadCount: Int
)

data class MessagesResponse(
    val success: Boolean,
    val data: List<ChatMessage>,
    val page: Int,
    val totalPages: Int
)

data class SendMessageRequest(
    val doctorId: String,
    val content: String,
    val type: String = "text"
)

data class UserProfile(
    val id: String,
    val name: String,
    val email: String,
    val phone: String,
    val avatar: String? = null,
    val dateOfBirth: String? = null,
    val gender: String? = null
)

data class UpdateProfileRequest(
    val name: String? = null,
    val phone: String? = null,
    val dateOfBirth: String? = null,
    val gender: String? = null
)