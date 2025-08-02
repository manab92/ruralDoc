package com.medinova.app.presentation.screens.splash

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.medinova.app.presentation.theme.MedinovaPrimary
import kotlinx.coroutines.delay

@Composable
fun SplashScreen(
    onNavigateToOnboarding: () -> Unit,
    onNavigateToMain: () -> Unit
) {
    var showLoadingDots by remember { mutableStateOf(0) }
    
    LaunchedEffect(Unit) {
        // Animate loading dots
        repeat(3) {
            delay(500)
            showLoadingDots = (showLoadingDots + 1) % 4
        }
        
        // Navigate after delay (check if user is logged in)
        delay(1000)
        // For now, always go to onboarding
        // In real app, check SharedPreferences for login status
        onNavigateToOnboarding()
    }
    
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(MedinovaPrimary),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        // Logo placeholder (circle with M)
        Box(
            modifier = Modifier
                .size(120.dp)
                .clip(CircleShape)
                .background(Color.White),
            contentAlignment = Alignment.Center
        ) {
            Text(
                text = "M",
                fontSize = 48.sp,
                fontWeight = FontWeight.Bold,
                color = MedinovaPrimary
            )
        }
        
        Spacer(modifier = Modifier.height(24.dp))
        
        Text(
            text = "Medinova",
            fontSize = 32.sp,
            fontWeight = FontWeight.Bold,
            color = Color.White
        )
        
        Spacer(modifier = Modifier.height(8.dp))
        
        Text(
            text = "Your Health, Our Priority",
            fontSize = 16.sp,
            color = Color.White.copy(alpha = 0.8f),
            textAlign = TextAlign.Center
        )
        
        Spacer(modifier = Modifier.height(48.dp))
        
        // Loading dots
        Row(
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            repeat(3) { index ->
                Box(
                    modifier = Modifier
                        .size(8.dp)
                        .clip(CircleShape)
                        .background(
                            if (showLoadingDots == index) Color.White 
                            else Color.White.copy(alpha = 0.3f)
                        )
                )
            }
        }
    }
}