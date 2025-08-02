package com.medinova.app.data.repository

import android.content.SharedPreferences
import com.medinova.app.data.remote.*
import com.medinova.app.domain.model.*
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class MedinovaRepository @Inject constructor(
    private val apiService: ApiService,
    private val sharedPreferences: SharedPreferences
) {
    
    // Authentication
    suspend fun login(email: String, password: String): Result<AuthData> {
        return try {
            val response = apiService.login(LoginRequest(email, password))
            if (response.isSuccessful && response.body()?.success == true) {
                val authData = response.body()!!.data!!
                saveAuthData(authData)
                Result.success(authData)
            } else {
                Result.failure(Exception(response.body()?.message ?: "Login failed"))
            }
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
    
    suspend fun register(name: String, email: String, password: String, phone: String): Result<AuthData> {
        return try {
            val response = apiService.register(RegisterRequest(name, email, password, phone))
            if (response.isSuccessful && response.body()?.success == true) {
                val authData = response.body()!!.data!!
                saveAuthData(authData)
                Result.success(authData)
            } else {
                Result.failure(Exception(response.body()?.message ?: "Registration failed"))
            }
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
    
    fun logout() {
        clearAuthData()
    }
    
    fun isLoggedIn(): Boolean {
        return getAccessToken() != null
    }
    
    // Doctors
    suspend fun getDoctors(specialty: String? = null, search: String? = null): Result<List<Doctor>> {
        return try {
            val response = apiService.getDoctors(specialty, search)
            if (response.isSuccessful && response.body()?.success == true) {
                Result.success(response.body()!!.data)
            } else {
                Result.failure(Exception("Failed to fetch doctors"))
            }
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
    
    suspend fun getDoctorById(doctorId: String): Result<Doctor> {
        return try {
            val response = apiService.getDoctorById(doctorId)
            if (response.isSuccessful) {
                Result.success(response.body()!!)
            } else {
                Result.failure(Exception("Doctor not found"))
            }
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
    
    suspend fun getSpecialties(): Result<List<String>> {
        return try {
            val response = apiService.getSpecialties()
            if (response.isSuccessful) {
                Result.success(response.body()!!)
            } else {
                Result.failure(Exception("Failed to fetch specialties"))
            }
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
    
    // Appointments
    suspend fun getAppointments(status: String? = null): Result<List<Appointment>> {
        return try {
            val token = getAuthHeader()
            val response = apiService.getAppointments(token, status)
            if (response.isSuccessful && response.body()?.success == true) {
                Result.success(response.body()!!.data)
            } else {
                Result.failure(Exception("Failed to fetch appointments"))
            }
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
    
    suspend fun createAppointment(
        doctorId: String,
        dateTime: String,
        type: String,
        symptoms: String
    ): Result<Appointment> {
        return try {
            val token = getAuthHeader()
            val request = CreateAppointmentRequest(doctorId, dateTime, type, symptoms)
            val response = apiService.createAppointment(token, request)
            if (response.isSuccessful) {
                Result.success(response.body()!!)
            } else {
                Result.failure(Exception("Failed to create appointment"))
            }
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
    
    suspend fun updateAppointment(
        appointmentId: String,
        dateTime: String? = null,
        status: String? = null
    ): Result<Appointment> {
        return try {
            val token = getAuthHeader()
            val request = UpdateAppointmentRequest(dateTime, status)
            val response = apiService.updateAppointment(token, appointmentId, request)
            if (response.isSuccessful) {
                Result.success(response.body()!!)
            } else {
                Result.failure(Exception("Failed to update appointment"))
            }
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
    
    suspend fun cancelAppointment(appointmentId: String): Result<Unit> {
        return try {
            val token = getAuthHeader()
            val response = apiService.cancelAppointment(token, appointmentId)
            if (response.isSuccessful) {
                Result.success(Unit)
            } else {
                Result.failure(Exception("Failed to cancel appointment"))
            }
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
    
    // Chat
    suspend fun getConversations(): Result<List<ChatConversation>> {
        return try {
            val token = getAuthHeader()
            val response = apiService.getConversations(token)
            if (response.isSuccessful) {
                Result.success(response.body()!!)
            } else {
                Result.failure(Exception("Failed to fetch conversations"))
            }
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
    
    suspend fun getMessages(doctorId: String, page: Int = 1): Result<List<ChatMessage>> {
        return try {
            val token = getAuthHeader()
            val response = apiService.getMessages(token, doctorId, page)
            if (response.isSuccessful && response.body()?.success == true) {
                Result.success(response.body()!!.data)
            } else {
                Result.failure(Exception("Failed to fetch messages"))
            }
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
    
    suspend fun sendMessage(doctorId: String, content: String, type: String = "text"): Result<ChatMessage> {
        return try {
            val token = getAuthHeader()
            val request = SendMessageRequest(doctorId, content, type)
            val response = apiService.sendMessage(token, request)
            if (response.isSuccessful) {
                Result.success(response.body()!!)
            } else {
                Result.failure(Exception("Failed to send message"))
            }
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
    
    // Profile
    suspend fun getProfile(): Result<UserProfile> {
        return try {
            val token = getAuthHeader()
            val response = apiService.getProfile(token)
            if (response.isSuccessful) {
                Result.success(response.body()!!)
            } else {
                Result.failure(Exception("Failed to fetch profile"))
            }
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
    
    suspend fun updateProfile(
        name: String? = null,
        phone: String? = null,
        dateOfBirth: String? = null,
        gender: String? = null
    ): Result<UserProfile> {
        return try {
            val token = getAuthHeader()
            val request = UpdateProfileRequest(name, phone, dateOfBirth, gender)
            val response = apiService.updateProfile(token, request)
            if (response.isSuccessful) {
                Result.success(response.body()!!)
            } else {
                Result.failure(Exception("Failed to update profile"))
            }
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
    
    // Private helper methods
    private fun saveAuthData(authData: AuthData) {
        sharedPreferences.edit().apply {
            putString("access_token", authData.accessToken)
            putString("refresh_token", authData.refreshToken)
            putLong("expires_in", authData.expiresIn)
            putString("user_id", authData.user.id)
            putString("user_name", authData.user.name)
            putString("user_email", authData.user.email)
            apply()
        }
    }
    
    private fun clearAuthData() {
        sharedPreferences.edit().apply {
            remove("access_token")
            remove("refresh_token")
            remove("expires_in")
            remove("user_id")
            remove("user_name")
            remove("user_email")
            apply()
        }
    }
    
    private fun getAccessToken(): String? {
        return sharedPreferences.getString("access_token", null)
    }
    
    private fun getAuthHeader(): String {
        val token = getAccessToken() ?: throw Exception("Not authenticated")
        return "Bearer $token"
    }
}