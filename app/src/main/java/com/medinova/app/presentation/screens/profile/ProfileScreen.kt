package com.medinova.app.presentation.screens.profile

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
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
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.navigation.NavHostController
import com.medinova.app.presentation.theme.*

data class ProfileMenuItem(
    val title: String,
    val icon: ImageVector,
    val onClick: () -> Unit
)

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ProfileScreen(
    navController: NavHostController
) {
    val profileMenuItems = listOf(
        ProfileMenuItem(
            title = "Personal Information",
            icon = Icons.Default.Person,
            onClick = { /* Navigate to personal info */ }
        ),
        ProfileMenuItem(
            title = "My Appointment",
            icon = Icons.Default.CalendarToday,
            onClick = { navController.navigate("appointment") }
        ),
        ProfileMenuItem(
            title = "History",
            icon = Icons.Default.History,
            onClick = { /* Navigate to history */ }
        ),
        ProfileMenuItem(
            title = "People",
            icon = Icons.Default.Group,
            onClick = { /* Navigate to people */ }
        ),
        ProfileMenuItem(
            title = "Notification",
            icon = Icons.Default.Notifications,
            onClick = { /* Navigate to notifications */ }
        ),
        ProfileMenuItem(
            title = "Reminder",
            icon = Icons.Default.Alarm,
            onClick = { /* Navigate to reminders */ }
        ),
        ProfileMenuItem(
            title = "Contact Doctor",
            icon = Icons.Default.ContactPhone,
            onClick = { /* Navigate to contact */ }
        ),
        ProfileMenuItem(
            title = "Chatting",
            icon = Icons.Default.Chat,
            onClick = { navController.navigate("chat") }
        ),
        ProfileMenuItem(
            title = "Settings",
            icon = Icons.Default.Settings,
            onClick = { /* Navigate to settings */ }
        ),
        ProfileMenuItem(
            title = "Help & Support",
            icon = Icons.Default.Help,
            onClick = { /* Navigate to help */ }
        ),
        ProfileMenuItem(
            title = "Privacy Policy",
            icon = Icons.Default.Policy,
            onClick = { /* Navigate to privacy */ }
        ),
        ProfileMenuItem(
            title = "Logout",
            icon = Icons.Default.Logout,
            onClick = { /* Handle logout */ }
        )
    )
    
    LazyColumn(
        modifier = Modifier
            .fillMaxSize()
            .background(MedinovaBackground),
        contentPadding = PaddingValues(16.dp),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        // Profile Header
        item {
            ProfileHeader()
        }
        
        // Menu Items
        items(profileMenuItems.size) { index ->
            val item = profileMenuItems[index]
            ProfileMenuCard(
                title = item.title,
                icon = item.icon,
                onClick = item.onClick,
                isLogout = item.title == "Logout"
            )
        }
        
        // App Version
        item {
            Text(
                text = "Version 1.0.0",
                fontSize = 12.sp,
                color = MedinovaGray,
                modifier = Modifier.fillMaxWidth()
            )
        }
    }
}

@Composable
fun ProfileHeader() {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(containerColor = Color.White),
        shape = RoundedCornerShape(12.dp),
        elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(20.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            // Profile Image
            Box(
                modifier = Modifier
                    .size(80.dp)
                    .clip(CircleShape)
                    .background(MedinovaLightGray),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = "JD",
                    fontSize = 28.sp,
                    fontWeight = FontWeight.Bold,
                    color = MedinovaPrimary
                )
            }
            
            Spacer(modifier = Modifier.height(12.dp))
            
            Text(
                text = "John Doe",
                fontSize = 20.sp,
                fontWeight = FontWeight.Bold,
                color = MedinovaOnSurface
            )
            
            Text(
                text = "john.doe@email.com",
                fontSize = 14.sp,
                color = MedinovaGray
            )
            
            Spacer(modifier = Modifier.height(16.dp))
            
            // Stats Row
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceEvenly
            ) {
                ProfileStat(
                    title = "Appointments",
                    value = "12"
                )
                
                Divider(
                    modifier = Modifier
                        .height(40.dp)
                        .width(1.dp),
                    color = MedinovaGray.copy(alpha = 0.3f)
                )
                
                ProfileStat(
                    title = "Consultations",
                    value = "8"
                )
                
                Divider(
                    modifier = Modifier
                        .height(40.dp)
                        .width(1.dp),
                    color = MedinovaGray.copy(alpha = 0.3f)
                )
                
                ProfileStat(
                    title = "Reviews",
                    value = "5"
                )
            }
        }
    }
}

@Composable
fun ProfileStat(
    title: String,
    value: String
) {
    Column(
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Text(
            text = value,
            fontSize = 18.sp,
            fontWeight = FontWeight.Bold,
            color = MedinovaPrimary
        )
        Text(
            text = title,
            fontSize = 12.sp,
            color = MedinovaGray
        )
    }
}

@Composable
fun ProfileMenuCard(
    title: String,
    icon: ImageVector,
    onClick: () -> Unit,
    isLogout: Boolean = false
) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { onClick() },
        colors = CardDefaults.cardColors(
            containerColor = if (isLogout) MedinovaError.copy(alpha = 0.1f) else Color.White
        ),
        shape = RoundedCornerShape(12.dp),
        elevation = CardDefaults.cardElevation(defaultElevation = 2.dp)
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Box(
                modifier = Modifier
                    .size(40.dp)
                    .clip(CircleShape)
                    .background(
                        if (isLogout) MedinovaError.copy(alpha = 0.1f)
                        else MedinovaPrimary.copy(alpha = 0.1f)
                    ),
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    imageVector = icon,
                    contentDescription = title,
                    tint = if (isLogout) MedinovaError else MedinovaPrimary,
                    modifier = Modifier.size(20.dp)
                )
            }
            
            Spacer(modifier = Modifier.width(16.dp))
            
            Text(
                text = title,
                fontSize = 16.sp,
                fontWeight = FontWeight.Medium,
                color = if (isLogout) MedinovaError else MedinovaOnSurface,
                modifier = Modifier.weight(1f)
            )
            
            Icon(
                imageVector = Icons.Default.ArrowForwardIos,
                contentDescription = "Navigate",
                tint = MedinovaGray,
                modifier = Modifier.size(16.dp)
            )
        }
    }
}