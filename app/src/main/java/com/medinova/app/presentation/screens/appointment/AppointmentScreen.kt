package com.medinova.app.presentation.screens.appointment

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.navigation.NavHostController
import com.medinova.app.domain.model.Appointment
import com.medinova.app.domain.model.AppointmentStatus
import com.medinova.app.domain.model.AppointmentType
import com.medinova.app.presentation.theme.*
import java.text.SimpleDateFormat
import java.util.*

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AppointmentScreen(
    navController: NavHostController
) {
    var selectedTab by remember { mutableStateOf(0) }
    
    // Sample data
    val currentDate = Date()
    val calendar = Calendar.getInstance()
    
    val upcomingAppointments = listOf(
        Appointment(
            id = "1",
            doctorId = "1",
            doctorName = "Dr. Mahbuba Islam",
            doctorImageUrl = "",
            specialty = "Gynecology",
            dateTime = calendar.apply { add(Calendar.DAY_OF_MONTH, 1) }.time,
            duration = 30,
            status = AppointmentStatus.SCHEDULED,
            type = AppointmentType.VIDEO_CALL,
            consultationFee = 199
        ),
        Appointment(
            id = "2",
            doctorId = "2",
            doctorName = "Dr. Kawsar Ahmed",
            doctorImageUrl = "",
            specialty = "Dentist",
            dateTime = calendar.apply { add(Calendar.DAY_OF_MONTH, 2) }.time,
            duration = 30,
            status = AppointmentStatus.SCHEDULED,
            type = AppointmentType.VOICE_CALL,
            consultationFee = 150
        )
    )
    
    calendar.time = currentDate
    val pastAppointments = listOf(
        Appointment(
            id = "3",
            doctorId = "3",
            doctorName = "Dr. Rumpa",
            doctorImageUrl = "",
            specialty = "Cardiologist",
            dateTime = calendar.apply { add(Calendar.DAY_OF_MONTH, -1) }.time,
            duration = 30,
            status = AppointmentStatus.COMPLETED,
            type = AppointmentType.VIDEO_CALL,
            consultationFee = 250
        ),
        Appointment(
            id = "4",
            doctorId = "4",
            doctorName = "Dr. Pakhi",
            doctorImageUrl = "",
            specialty = "Gynecology",
            dateTime = calendar.apply { add(Calendar.DAY_OF_MONTH, -3) }.time,
            duration = 30,
            status = AppointmentStatus.COMPLETED,
            type = AppointmentType.CHAT,
            consultationFee = 199
        )
    )
    
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(MedinovaBackground)
    ) {
        // Header
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .background(Color.White)
                .padding(16.dp)
        ) {
            Text(
                text = "My Appointment",
                fontSize = 20.sp,
                fontWeight = FontWeight.Bold,
                modifier = Modifier.align(Alignment.Center)
            )
        }
        
        // Tab Row
        TabRow(
            selectedTabIndex = selectedTab,
            containerColor = Color.White,
            contentColor = MedinovaPrimary,
            indicator = { tabPositions ->
                TabRowDefaults.Indicator(
                    modifier = Modifier.tabIndicatorOffset(tabPositions[selectedTab]),
                    color = MedinovaPrimary
                )
            }
        ) {
            Tab(
                selected = selectedTab == 0,
                onClick = { selectedTab = 0 },
                text = { Text("Upcoming") }
            )
            Tab(
                selected = selectedTab == 1,
                onClick = { selectedTab = 1 },
                text = { Text("Past") }
            )
        }
        
        // Content
        LazyColumn(
            modifier = Modifier.fillMaxSize(),
            contentPadding = PaddingValues(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            when (selectedTab) {
                0 -> {
                    if (upcomingAppointments.isEmpty()) {
                        item {
                            EmptyAppointmentState(
                                message = "No upcoming appointments",
                                icon = "📅"
                            )
                        }
                    } else {
                        items(upcomingAppointments) { appointment ->
                            AppointmentCard(
                                appointment = appointment,
                                showActions = true,
                                onJoinCall = {
                                    // Navigate to video call
                                },
                                onReschedule = {
                                    // Show reschedule dialog
                                },
                                onCancel = {
                                    // Show cancel dialog
                                }
                            )
                        }
                    }
                }
                1 -> {
                    if (pastAppointments.isEmpty()) {
                        item {
                            EmptyAppointmentState(
                                message = "No past appointments",
                                icon = "📋"
                            )
                        }
                    } else {
                        items(pastAppointments) { appointment ->
                            AppointmentCard(
                                appointment = appointment,
                                showActions = false,
                                onJoinCall = { },
                                onReschedule = { },
                                onCancel = { }
                            )
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun AppointmentCard(
    appointment: Appointment,
    showActions: Boolean,
    onJoinCall: () -> Unit,
    onReschedule: () -> Unit,
    onCancel: () -> Unit
) {
    val dateFormat = SimpleDateFormat("dd MMM, yyyy", Locale.getDefault())
    val timeFormat = SimpleDateFormat("hh:mm a", Locale.getDefault())
    
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(containerColor = Color.White),
        shape = RoundedCornerShape(12.dp),
        elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp)
        ) {
            Row(
                verticalAlignment = Alignment.CenterVertically
            ) {
                // Doctor image placeholder
                Box(
                    modifier = Modifier
                        .size(50.dp)
                        .clip(CircleShape)
                        .background(MedinovaLightGray),
                    contentAlignment = Alignment.Center
                ) {
                    Text(
                        text = appointment.doctorName.first().toString(),
                        fontSize = 20.sp,
                        fontWeight = FontWeight.Bold,
                        color = MedinovaPrimary
                    )
                }
                
                Spacer(modifier = Modifier.width(12.dp))
                
                Column(
                    modifier = Modifier.weight(1f)
                ) {
                    Text(
                        text = appointment.doctorName,
                        fontSize = 16.sp,
                        fontWeight = FontWeight.SemiBold,
                        color = MedinovaOnSurface
                    )
                    
                    Text(
                        text = appointment.specialty,
                        fontSize = 14.sp,
                        color = MedinovaGray
                    )
                }
                
                // Status badge
                val statusColor = when (appointment.status) {
                    AppointmentStatus.SCHEDULED -> MedinovaPrimary
                    AppointmentStatus.ONGOING -> MedinovaWarning
                    AppointmentStatus.COMPLETED -> MedinovaSuccess
                    AppointmentStatus.CANCELLED -> MedinovaError
                }
                
                Box(
                    modifier = Modifier
                        .background(
                            color = statusColor.copy(alpha = 0.1f),
                            shape = RoundedCornerShape(8.dp)
                        )
                        .padding(horizontal = 8.dp, vertical = 4.dp)
                ) {
                    Text(
                        text = appointment.status.name.lowercase().replaceFirstChar { it.uppercase() },
                        fontSize = 12.sp,
                        color = statusColor,
                        fontWeight = FontWeight.Medium
                    )
                }
            }
            
            Spacer(modifier = Modifier.height(12.dp))
            
            // Appointment details
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                AppointmentDetailItem(
                    icon = Icons.Default.CalendarToday,
                    label = "Date",
                    value = dateFormat.format(appointment.dateTime)
                )
                
                AppointmentDetailItem(
                    icon = Icons.Default.AccessTime,
                    label = "Time",
                    value = timeFormat.format(appointment.dateTime)
                )
                
                AppointmentDetailItem(
                    icon = when (appointment.type) {
                        AppointmentType.VIDEO_CALL -> Icons.Default.VideoCall
                        AppointmentType.VOICE_CALL -> Icons.Default.Phone
                        AppointmentType.CHAT -> Icons.Default.Chat
                    },
                    label = "Type",
                    value = when (appointment.type) {
                        AppointmentType.VIDEO_CALL -> "Video"
                        AppointmentType.VOICE_CALL -> "Voice"
                        AppointmentType.CHAT -> "Chat"
                    }
                )
            }
            
            if (showActions && appointment.status == AppointmentStatus.SCHEDULED) {
                Spacer(modifier = Modifier.height(16.dp))
                
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Button(
                        onClick = onJoinCall,
                        modifier = Modifier.weight(1f),
                        colors = ButtonDefaults.buttonColors(containerColor = MedinovaPrimary),
                        shape = RoundedCornerShape(8.dp)
                    ) {
                        Icon(
                            imageVector = when (appointment.type) {
                                AppointmentType.VIDEO_CALL -> Icons.Default.VideoCall
                                AppointmentType.VOICE_CALL -> Icons.Default.Phone
                                AppointmentType.CHAT -> Icons.Default.Chat
                            },
                            contentDescription = "Join",
                            modifier = Modifier.size(16.dp)
                        )
                        Spacer(modifier = Modifier.width(4.dp))
                        Text("Join")
                    }
                    
                    OutlinedButton(
                        onClick = onReschedule,
                        modifier = Modifier.weight(0.7f),
                        colors = ButtonDefaults.outlinedButtonColors(),
                        shape = RoundedCornerShape(8.dp)
                    ) {
                        Text("Reschedule", fontSize = 12.sp)
                    }
                    
                    OutlinedButton(
                        onClick = onCancel,
                        modifier = Modifier.weight(0.5f),
                        colors = ButtonDefaults.outlinedButtonColors(contentColor = MedinovaError),
                        shape = RoundedCornerShape(8.dp)
                    ) {
                        Text("Cancel", fontSize = 12.sp)
                    }
                }
            }
        }
    }
}

@Composable
fun AppointmentDetailItem(
    icon: androidx.compose.ui.graphics.vector.ImageVector,
    label: String,
    value: String
) {
    Column(
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Icon(
            imageVector = icon,
            contentDescription = label,
            tint = MedinovaGray,
            modifier = Modifier.size(16.dp)
        )
        Spacer(modifier = Modifier.height(4.dp))
        Text(
            text = label,
            fontSize = 12.sp,
            color = MedinovaGray
        )
        Text(
            text = value,
            fontSize = 12.sp,
            fontWeight = FontWeight.Medium,
            color = MedinovaOnSurface
        )
    }
}

@Composable
fun EmptyAppointmentState(
    message: String,
    icon: String
) {
    Box(
        modifier = Modifier
            .fillMaxWidth()
            .padding(48.dp),
        contentAlignment = Alignment.Center
    ) {
        Column(
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Text(
                text = icon,
                fontSize = 48.sp
            )
            Spacer(modifier = Modifier.height(16.dp))
            Text(
                text = message,
                fontSize = 16.sp,
                color = MedinovaGray
            )
        }
    }
}