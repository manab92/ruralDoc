package com.medinova.app.presentation.navigation

sealed class Screen(val route: String) {
    object Splash : Screen("splash")
    object Onboarding : Screen("onboarding")
    object Main : Screen("main")
    object Login : Screen("login")
    object Register : Screen("register")
    object Home : Screen("home")
    object Search : Screen("search")
    object Appointment : Screen("appointment")
    object Chat : Screen("chat")
    object Profile : Screen("profile")
    object DoctorProfile : Screen("doctor_profile/{doctorId}") {
        fun createRoute(doctorId: String) = "doctor_profile/$doctorId"
    }
    object BookAppointment : Screen("book_appointment/{doctorId}") {
        fun createRoute(doctorId: String) = "book_appointment/$doctorId"
    }
    object ChatDetail : Screen("chat_detail/{doctorId}") {
        fun createRoute(doctorId: String) = "chat_detail/$doctorId"
    }
    object VideoCall : Screen("video_call/{appointmentId}") {
        fun createRoute(appointmentId: String) = "video_call/$appointmentId"
    }
    object Payment : Screen("payment/{appointmentId}") {
        fun createRoute(appointmentId: String) = "payment/$appointmentId"
    }
}