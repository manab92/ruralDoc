package com.medinova.app.presentation.screens.search

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
import com.medinova.app.domain.model.Doctor
import com.medinova.app.presentation.theme.*

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SearchScreen(
    navController: NavHostController
) {
    var searchQuery by remember { mutableStateOf("") }
    var selectedTab by remember { mutableStateOf(0) }
    
    // Sample data
    val doctors = listOf(
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
        ),
        Doctor(
            id = "3",
            name = "Dr. Rumpa",
            specialty = "Cardiologist",
            profileImageUrl = "",
            rating = 4.9f,
            reviewCount = 890,
            experience = 10,
            consultationFee = 250,
            isOnline = true
        )
    )
    
    val filteredDoctors = doctors.filter {
        it.name.contains(searchQuery, ignoreCase = true) ||
        it.specialty.contains(searchQuery, ignoreCase = true)
    }
    
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(MedinovaBackground)
            .padding(16.dp)
    ) {
        // Header
        Row(
            modifier = Modifier.fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically
        ) {
            IconButton(
                onClick = { navController.navigateUp() }
            ) {
                Icon(
                    imageVector = Icons.Default.ArrowBack,
                    contentDescription = "Back"
                )
            }
            
            Text(
                text = "Search Doctors",
                fontSize = 20.sp,
                fontWeight = FontWeight.Bold,
                modifier = Modifier.weight(1f)
            )
            
            IconButton(
                onClick = { /* Filter */ }
            ) {
                Icon(
                    imageVector = Icons.Default.FilterList,
                    contentDescription = "Filter"
                )
            }
        }
        
        Spacer(modifier = Modifier.height(16.dp))
        
        // Search bar
        OutlinedTextField(
            value = searchQuery,
            onValueChange = { searchQuery = it },
            modifier = Modifier.fillMaxWidth(),
            placeholder = { Text("Search doctors, specialties...") },
            leadingIcon = {
                Icon(
                    imageVector = Icons.Default.Search,
                    contentDescription = "Search"
                )
            },
            trailingIcon = {
                if (searchQuery.isNotEmpty()) {
                    IconButton(
                        onClick = { searchQuery = "" }
                    ) {
                        Icon(
                            imageVector = Icons.Default.Clear,
                            contentDescription = "Clear"
                        )
                    }
                }
            },
            shape = RoundedCornerShape(12.dp),
            colors = OutlinedTextFieldDefaults.colors(
                focusedBorderColor = MedinovaPrimary,
                unfocusedBorderColor = MedinovaGray.copy(alpha = 0.3f)
            )
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        // Results
        LazyColumn(
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            if (searchQuery.isEmpty()) {
                item {
                    Text(
                        text = "Popular Specialists",
                        fontSize = 18.sp,
                        fontWeight = FontWeight.SemiBold,
                        color = MedinovaOnBackground
                    )
                }
                
                items(doctors) { doctor ->
                    DoctorSearchCard(doctor = doctor) {
                        // Navigate to doctor profile
                    }
                }
            } else {
                item {
                    Text(
                        text = "Search Results (${filteredDoctors.size})",
                        fontSize = 18.sp,
                        fontWeight = FontWeight.SemiBold,
                        color = MedinovaOnBackground
                    )
                }
                
                if (filteredDoctors.isEmpty()) {
                    item {
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
                                    text = "🔍",
                                    fontSize = 48.sp
                                )
                                Spacer(modifier = Modifier.height(16.dp))
                                Text(
                                    text = "No doctors found",
                                    fontSize = 16.sp,
                                    color = MedinovaGray
                                )
                            }
                        }
                    }
                } else {
                    items(filteredDoctors) { doctor ->
                        DoctorSearchCard(doctor = doctor) {
                            // Navigate to doctor profile
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun DoctorSearchCard(
    doctor: Doctor,
    onDoctorClick: () -> Unit
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
                        Spacer(modifier = Modifier.width(4.dp))
                        Text(
                            text = "Online",
                            fontSize = 12.sp,
                            color = OnlineGreen,
                            fontWeight = FontWeight.Medium
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
                    Spacer(modifier = Modifier.width(8.dp))
                    Text(
                        text = "₹${doctor.consultationFee}",
                        fontSize = 12.sp,
                        color = MedinovaPrimary,
                        fontWeight = FontWeight.SemiBold
                    )
                }
            }
            
            Column {
                IconButton(
                    onClick = { /* Call doctor */ }
                ) {
                    Icon(
                        imageVector = Icons.Default.Phone,
                        contentDescription = "Call",
                        tint = MedinovaPrimary
                    )
                }
                
                IconButton(
                    onClick = { /* Message doctor */ }
                ) {
                    Icon(
                        imageVector = Icons.Default.Message,
                        contentDescription = "Message",
                        tint = MedinovaSecondary
                    )
                }
            }
        }
    }
}