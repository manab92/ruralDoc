package com.medinova.app.presentation.screens.onboarding

import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.pager.HorizontalPager
import androidx.compose.foundation.pager.rememberPagerState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.medinova.app.presentation.theme.MedinovaPrimary
import com.medinova.app.presentation.theme.MedinovaGray
import kotlinx.coroutines.launch

data class OnboardingPage(
    val title: String,
    val description: String,
    val imageRes: Int? = null
)

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun OnboardingScreen(
    onNavigateToMain: () -> Unit
) {
    val pages = listOf(
        OnboardingPage(
            title = "Find the best Doctors in your vicinity!",
            description = "Lorem Ipsum is simply dummy text of the printing and typesetting industry."
        ),
        OnboardingPage(
            title = "Schedule appointments with expert doctors!",
            description = "Lorem Ipsum is simply dummy text of the printing and typesetting industry."
        ),
        OnboardingPage(
            title = "Book face-to-face Appointment!",
            description = "Lorem Ipsum is simply dummy text of the printing and typesetting industry."
        )
    )
    
    val pagerState = rememberPagerState(pageCount = { pages.size })
    val scope = rememberCoroutineScope()
    
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.White)
            .padding(16.dp)
    ) {
        HorizontalPager(
            state = pagerState,
            modifier = Modifier.weight(1f)
        ) { page ->
            OnboardingPageContent(
                page = pages[page],
                pageIndex = page
            )
        }
        
        // Page indicators
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(vertical = 16.dp),
            horizontalArrangement = Arrangement.Center
        ) {
            repeat(pages.size) { index ->
                Box(
                    modifier = Modifier
                        .padding(horizontal = 4.dp)
                        .size(8.dp)
                        .clip(CircleShape)
                        .background(
                            if (pagerState.currentPage == index) MedinovaPrimary
                            else MedinovaGray.copy(alpha = 0.3f)
                        )
                )
            }
        }
        
        // Navigation buttons
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(vertical = 16.dp),
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            if (pagerState.currentPage < pages.size - 1) {
                TextButton(
                    onClick = { onNavigateToMain() }
                ) {
                    Text("Skip", color = MedinovaGray)
                }
                
                Button(
                    onClick = {
                        scope.launch {
                            pagerState.animateScrollToPage(pagerState.currentPage + 1)
                        }
                    },
                    colors = ButtonDefaults.buttonColors(containerColor = MedinovaPrimary),
                    shape = RoundedCornerShape(25.dp),
                    modifier = Modifier.padding(start = 8.dp)
                ) {
                    Text("NEXT", color = Color.White, fontWeight = FontWeight.SemiBold)
                }
            } else {
                Spacer(modifier = Modifier.weight(1f))
                
                Button(
                    onClick = { onNavigateToMain() },
                    colors = ButtonDefaults.buttonColors(containerColor = MedinovaPrimary),
                    shape = RoundedCornerShape(25.dp),
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Text("Get Started", color = Color.White, fontWeight = FontWeight.SemiBold)
                }
            }
        }
    }
}

@Composable
fun OnboardingPageContent(
    page: OnboardingPage,
    pageIndex: Int
) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(horizontal = 16.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        // Placeholder for images - you can replace with actual images
        Box(
            modifier = Modifier
                .size(280.dp)
                .background(
                    color = when (pageIndex) {
                        0 -> Color(0xFFE3F2FD)
                        1 -> Color(0xFFF3E5F5)
                        else -> Color(0xFFE8F5E8)
                    },
                    shape = RoundedCornerShape(20.dp)
                ),
            contentAlignment = Alignment.Center
        ) {
            // Placeholder content based on page
            when (pageIndex) {
                0 -> {
                    // Medical team illustration placeholder
                    Box(
                        modifier = Modifier
                            .size(200.dp)
                            .background(MedinovaPrimary.copy(alpha = 0.1f), CircleShape),
                        contentAlignment = Alignment.Center
                    ) {
                        Text("👨‍⚕️👩‍⚕️", fontSize = 48.sp)
                    }
                }
                1 -> {
                    // Doctor with mask illustration placeholder
                    Box(
                        modifier = Modifier
                            .size(200.dp)
                            .background(MedinovaPrimary.copy(alpha = 0.1f), CircleShape),
                        contentAlignment = Alignment.Center
                    ) {
                        Text("🏥", fontSize = 48.sp)
                    }
                }
                2 -> {
                    // Doctor with thumbs up illustration placeholder
                    Box(
                        modifier = Modifier
                            .size(200.dp)
                            .background(MedinovaPrimary.copy(alpha = 0.1f), CircleShape),
                        contentAlignment = Alignment.Center
                    ) {
                        Text("👍", fontSize = 48.sp)
                    }
                }
            }
        }
        
        Spacer(modifier = Modifier.height(48.dp))
        
        Text(
            text = page.title,
            fontSize = 24.sp,
            fontWeight = FontWeight.Bold,
            textAlign = TextAlign.Center,
            color = Color.Black,
            lineHeight = 32.sp
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        Text(
            text = page.description,
            fontSize = 16.sp,
            textAlign = TextAlign.Center,
            color = MedinovaGray,
            lineHeight = 24.sp
        )
    }
}