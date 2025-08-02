package com.medinova.app.presentation.screens.home

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.LazyRow
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
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.navigation.NavHostController
import coil.compose.AsyncImage
import com.medinova.app.domain.model.Doctor
import com.medinova.app.presentation.theme.*

data class DepartmentItem(
    val name: String,
    val icon: String,
    val color: Color
)

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun HomeScreen(
    navController: NavHostController
) {
    // Sample data - in real app this would come from ViewModel
    val departments = listOf(
        DepartmentItem("Headache", "🧠", Color(0xFF6C63FF)),
        DepartmentItem("Sexology", "❤️", Color(0xFFFF6B9D)),
        DepartmentItem("Pregnancy", "🤱", Color(0xFF4ECDC4)),
        DepartmentItem("Child Health", "👶", Color(0xFFFFD93D)),
        DepartmentItem("Chest Pain", "🫁", Color(0xFFFF8A65)),
        DepartmentItem("Hair Loss", "💇", Color(0xFF81C784)),
        DepartmentItem("Diabetes", "🩺", Color(0xFF9575CD)),
        DepartmentItem("Kidney", "🫘", Color(0xFFAED581))
    )
    
    val topDoctors = listOf(
        Doctor(
            id = "1",
            name = "Dr. Mahbuba Islam",
            specialty = "Gynecology",
            profileImageUrl = "",
            rating = 5.0f,
            reviewCount = 2050,
            experience = 7,
            consultationFee = 199,
            isOnline = true
        ),
        Doctor(
            id = "2",
            name = "Dr. Kawsar Ahmed",
            specialty = "Dentist",
            profileImageUrl = "",
            rating = 4.8f,
            reviewCount = 1520,
            experience = 5,
            consultationFee = 150,
            isOnline = false
        )
    )
    
    LazyColumn(
        modifier = Modifier
            .fillMaxSize()
            .background(MedinovaBackground),
        contentPadding = PaddingValues(16.dp),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        // Header
        item {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Column {
                    Text(
                        text = "Good Morning! 👋",
                        fontSize = 16.sp,
                        color = MedinovaGray
                    )
                    Text(
                        text = "Find Your Doctor",
                        fontSize = 24.sp,
                        fontWeight = FontWeight.Bold,
                        color = MedinovaOnBackground
                    )
                }
                
                Icon(
                    imageVector = Icons.Default.Notifications,
                    contentDescription = "Notifications",
                    tint = MedinovaGray,
                    modifier = Modifier
                        .size(24.dp)
                        .clickable { /* Handle notification click */ }
                )
            }
        }
        
        // Search bar
        item {
            OutlinedTextField(
                value = "",
                onValueChange = { },
                modifier = Modifier.fillMaxWidth(),
                placeholder = { Text("Search doctors, diseases...") },
                leadingIcon = {
                    Icon(
                        imageVector = Icons.Default.Search,
                        contentDescription = "Search"
                    )
                },
                shape = RoundedCornerShape(12.dp),
                colors = OutlinedTextFieldDefaults.colors(
                    focusedBorderColor = MedinovaPrimary,
                    unfocusedBorderColor = MedinovaGray.copy(alpha = 0.3f)
                )
            )
        }
        
        // Quick Actions
        item {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                QuickActionCard(
                    title = "Consult a Doctor",
                    icon = Icons.Default.VideoCall,
                    color = MedinovaPrimary,
                    modifier = Modifier.weight(1f)
                ) {
                    // Navigate to consultation
                }
                
                QuickActionCard(
                    title = "Find Specialist",
                    icon = Icons.Default.Person,
                    color = MedinovaSecondary,
                    modifier = Modifier.weight(1f)
                ) {
                    navController.navigate("search")
                }
            }
        }
        
        // Departments section
        item {
            Text(
                text = "Choose a department or symptom",
                fontSize = 18.sp,
                fontWeight = FontWeight.SemiBold,
                color = MedinovaOnBackground
            )
        }
        
        item {
            LazyRow(
                horizontalArrangement = Arrangement.spacedBy(12.dp),
                contentPadding = PaddingValues(horizontal = 4.dp)
            ) {
                items(departments.chunked(4)) { departmentChunk ->
                    Column(
                        verticalArrangement = Arrangement.spacedBy(8.dp)
                    ) {
                        departmentChunk.forEach { department ->
                            DepartmentCard(department = department) {
                                // Navigate to department doctors
                            }
                        }
                    }
                }
            }
        }
        
        // Top Doctors section
        item {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = "Top Doctor",
                    fontSize = 18.sp,
                    fontWeight = FontWeight.SemiBold,
                    color = MedinovaOnBackground
                )
                
                TextButton(
                    onClick = { navController.navigate("search") }
                ) {
                    Text("See All", color = MedinovaPrimary)
                }
            }
        }
        
        items(topDoctors) { doctor ->
            DoctorCard(
                doctor = doctor,
                onDoctorClick = {
                    // Navigate to doctor profile
                },
                onCallClick = {
                    // Start call
                }
            )
        }
    }
}

@Composable
fun QuickActionCard(
    title: String,
    icon: ImageVector,
    color: Color,
    modifier: Modifier = Modifier,
    onClick: () -> Unit
) {
    Card(
        modifier = modifier
            .height(80.dp)
            .clickable { onClick() },
        colors = CardDefaults.cardColors(containerColor = color),
        shape = RoundedCornerShape(12.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(12.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            Icon(
                imageVector = icon,
                contentDescription = title,
                tint = Color.White,
                modifier = Modifier.size(24.dp)
            )
            Spacer(modifier = Modifier.height(4.dp))
            Text(
                text = title,
                color = Color.White,
                fontSize = 12.sp,
                fontWeight = FontWeight.Medium
            )
        }
    }
}

@Composable
fun DepartmentCard(
    department: DepartmentItem,
    onClick: () -> Unit
) {
    Card(
        modifier = Modifier
            .size(80.dp)
            .clickable { onClick() },
        colors = CardDefaults.cardColors(containerColor = Color.White),
        shape = RoundedCornerShape(12.dp),
        elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(8.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            Text(
                text = department.icon,
                fontSize = 24.sp
            )
            Spacer(modifier = Modifier.height(4.dp))
            Text(
                text = department.name,
                fontSize = 10.sp,
                fontWeight = FontWeight.Medium,
                color = MedinovaOnSurface
            )
        }
    }
}

@Composable
fun DoctorCard(
    doctor: Doctor,
    onDoctorClick: () -> Unit,
    onCallClick: () -> Unit
) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { onDoctorClick() },
        colors = CardDefaults.cardColors(containerColor = Color.White),
        shape = RoundedCornerShape(12.dp),
        elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            // Doctor image placeholder
            Box(
                modifier = Modifier
                    .size(60.dp)
                    .clip(CircleShape)
                    .background(MedinovaLightGray),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = doctor.name.first().toString(),
                    fontSize = 24.sp,
                    fontWeight = FontWeight.Bold,
                    color = MedinovaPrimary
                )
            }
            
            Spacer(modifier = Modifier.width(12.dp))
            
            Column(
                modifier = Modifier.weight(1f)
            ) {
                Row(
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(
                        text = doctor.name,
                        fontSize = 16.sp,
                        fontWeight = FontWeight.SemiBold,
                        color = MedinovaOnSurface
                    )
                    
                    if (doctor.isOnline) {
                        Spacer(modifier = Modifier.width(8.dp))
                        Box(
                            modifier = Modifier
                                .size(8.dp)
                                .clip(CircleShape)
                                .background(OnlineGreen)
                        )
                    }
                }
                
                Text(
                    text = doctor.specialty,
                    fontSize = 14.sp,
                    color = MedinovaGray
                )
                
                Row(
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Icon(
                        imageVector = Icons.Default.Star,
                        contentDescription = "Rating",
                        tint = Color(0xFFFFD700),
                        modifier = Modifier.size(16.dp)
                    )
                    Text(
                        text = "${doctor.rating}",
                        fontSize = 12.sp,
                        color = MedinovaGray
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text(
                        text = "${doctor.experience} Years",
                        fontSize = 12.sp,
                        color = MedinovaGray
                    )
                }
            }
            
            IconButton(
                onClick = onCallClick
            ) {
                Icon(
                    imageVector = Icons.Default.Phone,
                    contentDescription = "Call",
                    tint = MedinovaPrimary
                )
            }
        }
    }
}