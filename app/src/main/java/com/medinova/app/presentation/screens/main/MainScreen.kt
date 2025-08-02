package com.medinova.app.presentation.screens.main

import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.unit.dp
import androidx.navigation.NavHostController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController
import com.medinova.app.presentation.screens.home.HomeScreen
import com.medinova.app.presentation.screens.search.SearchScreen
import com.medinova.app.presentation.screens.appointment.AppointmentScreen
import com.medinova.app.presentation.screens.chat.ChatScreen
import com.medinova.app.presentation.screens.profile.ProfileScreen
import com.medinova.app.presentation.theme.MedinovaPrimary

data class BottomNavItem(
    val title: String,
    val icon: ImageVector,
    val route: String
)

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MainScreen() {
    val navController = rememberNavController()
    
    val bottomNavItems = listOf(
        BottomNavItem("Home", Icons.Default.Home, "home"),
        BottomNavItem("Search", Icons.Default.Search, "search"),
        BottomNavItem("Appointment", Icons.Default.CalendarToday, "appointment"),
        BottomNavItem("Chat", Icons.Default.ChatBubble, "chat"),
        BottomNavItem("Profile", Icons.Default.Person, "profile")
    )
    
    Scaffold(
        bottomBar = {
            NavigationBar(
                containerColor = MaterialTheme.colorScheme.surface,
                tonalElevation = 8.dp
            ) {
                val navBackStackEntry by navController.currentBackStackEntryAsState()
                val currentRoute = navBackStackEntry?.destination?.route
                
                bottomNavItems.forEach { item ->
                    NavigationBarItem(
                        icon = {
                            Icon(
                                imageVector = item.icon,
                                contentDescription = item.title
                            )
                        },
                        label = { Text(item.title) },
                        selected = currentRoute == item.route,
                        onClick = {
                            navController.navigate(item.route) {
                                popUpTo(navController.graph.startDestinationId)
                                launchSingleTop = true
                            }
                        },
                        colors = NavigationBarItemDefaults.colors(
                            selectedIconColor = MedinovaPrimary,
                            selectedTextColor = MedinovaPrimary,
                            indicatorColor = MedinovaPrimary.copy(alpha = 0.1f)
                        )
                    )
                }
            }
        }
    ) { paddingValues ->
        NavHost(
            navController = navController,
            startDestination = "home",
            modifier = Modifier.padding(paddingValues)
        ) {
            composable("home") {
                HomeScreen(navController = navController)
            }
            composable("search") {
                SearchScreen(navController = navController)
            }
            composable("appointment") {
                AppointmentScreen(navController = navController)
            }
            composable("chat") {
                ChatScreen(navController = navController)
            }
            composable("profile") {
                ProfileScreen(navController = navController)
            }
        }
    }
}