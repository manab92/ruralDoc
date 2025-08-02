package com.medinova.app.presentation.screens.chat

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
import com.medinova.app.domain.model.ChatMessage
import com.medinova.app.domain.model.MessageType
import com.medinova.app.presentation.theme.*
import java.text.SimpleDateFormat
import java.util.*

data class ChatConversation(
    val doctorId: String,
    val doctorName: String,
    val doctorImageUrl: String,
    val specialty: String,
    val lastMessage: ChatMessage,
    val unreadCount: Int = 0,
    val isOnline: Boolean = false
)

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ChatScreen(
    navController: NavHostController
) {
    var selectedTab by remember { mutableStateOf(0) }
    var searchQuery by remember { mutableStateOf("") }
    
    // Sample data
    val currentDate = Date()
    val calendar = Calendar.getInstance()
    
    val conversations = listOf(
        ChatConversation(
            doctorId = "1",
            doctorName = "Dr. Mahbuba Islam",
            doctorImageUrl = "",
            specialty = "Gynecology",
            lastMessage = ChatMessage(
                id = "1",
                senderId = "1",
                senderName = "Dr. Mahbuba Islam",
                receiverId = "user",
                content = "Hi, Doctor Pakhi! I am patient. I need your help immediately...!",
                type = MessageType.TEXT,
                timestamp = calendar.apply { add(Calendar.MINUTE, -30) }.time,
                isRead = false
            ),
            unreadCount = 2,
            isOnline = true
        ),
        ChatConversation(
            doctorId = "2",
            doctorName = "Dr. Rumpa",
            doctorImageUrl = "",
            specialty = "Cardiologist",
            lastMessage = ChatMessage(
                id = "2",
                senderId = "user",
                senderName = "User",
                receiverId = "2",
                content = "Thank you for the consultation",
                type = MessageType.TEXT,
                timestamp = calendar.apply { add(Calendar.HOUR, -2) }.time,
                isRead = true
            ),
            unreadCount = 0,
            isOnline = false
        ),
        ChatConversation(
            doctorId = "3",
            doctorName = "Dr. Riya",
            doctorImageUrl = "",
            specialty = "Dermatologist",
            lastMessage = ChatMessage(
                id = "3",
                senderId = "3",
                senderName = "Dr. Riya",
                receiverId = "user",
                content = "Audio message",
                type = MessageType.AUDIO,
                timestamp = calendar.apply { add(Calendar.DAY_OF_MONTH, -1) }.time,
                isRead = true,
                audioDuration = 45
            ),
            unreadCount = 0,
            isOnline = true
        ),
        ChatConversation(
            doctorId = "4",
            doctorName = "Dr. Mahbuba",
            doctorImageUrl = "",
            specialty = "Pediatrics",
            lastMessage = ChatMessage(
                id = "4",
                senderId = "4",
                senderName = "Dr. Mahbuba",
                receiverId = "user",
                content = "How are you feeling today?",
                type = MessageType.TEXT,
                timestamp = calendar.apply { add(Calendar.DAY_OF_MONTH, -1) }.time,
                isRead = true
            ),
            unreadCount = 0,
            isOnline = false
        )
    )
    
    val todayConversations = conversations.filter {
        val today = Calendar.getInstance()
        val messageDay = Calendar.getInstance().apply { time = it.lastMessage.timestamp }
        today.get(Calendar.DAY_OF_YEAR) == messageDay.get(Calendar.DAY_OF_YEAR) &&
        today.get(Calendar.YEAR) == messageDay.get(Calendar.YEAR)
    }
    
    val yesterdayConversations = conversations.filter {
        val yesterday = Calendar.getInstance().apply { add(Calendar.DAY_OF_MONTH, -1) }
        val messageDay = Calendar.getInstance().apply { time = it.lastMessage.timestamp }
        yesterday.get(Calendar.DAY_OF_YEAR) == messageDay.get(Calendar.DAY_OF_YEAR) &&
        yesterday.get(Calendar.YEAR) == messageDay.get(Calendar.YEAR)
    }
    
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
                text = "Message",
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
                text = { Text("Message") }
            )
            Tab(
                selected = selectedTab == 1,
                onClick = { selectedTab = 1 },
                text = { Text("Voice Call") }
            )
            Tab(
                selected = selectedTab == 2,
                onClick = { selectedTab = 2 },
                text = { Text("Video Call") }
            )
        }
        
        when (selectedTab) {
            0 -> {
                // Messages tab content
                Column(
                    modifier = Modifier.fillMaxSize()
                ) {
                    // Search bar
                    OutlinedTextField(
                        value = searchQuery,
                        onValueChange = { searchQuery = it },
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(16.dp),
                        placeholder = { Text("Search") },
                        leadingIcon = {
                            Icon(
                                imageVector = Icons.Default.Search,
                                contentDescription = "Search"
                            )
                        },
                        trailingIcon = {
                            IconButton(
                                onClick = { /* Filter */ }
                            ) {
                                Icon(
                                    imageVector = Icons.Default.FilterList,
                                    contentDescription = "Filter"
                                )
                            }
                        },
                        shape = RoundedCornerShape(12.dp),
                        colors = OutlinedTextFieldDefaults.colors(
                            focusedBorderColor = MedinovaPrimary,
                            unfocusedBorderColor = MedinovaGray.copy(alpha = 0.3f)
                        )
                    )
                    
                    // Conversations
                    LazyColumn(
                        modifier = Modifier.fillMaxSize(),
                        contentPadding = PaddingValues(horizontal = 16.dp)
                    ) {
                        if (todayConversations.isNotEmpty()) {
                            item {
                                Text(
                                    text = "Today",
                                    fontSize = 16.sp,
                                    fontWeight = FontWeight.SemiBold,
                                    color = MedinovaOnBackground,
                                    modifier = Modifier.padding(vertical = 8.dp)
                                )
                            }
                            
                            items(todayConversations) { conversation ->
                                ChatConversationItem(
                                    conversation = conversation,
                                    onClick = {
                                        // Navigate to chat detail
                                    }
                                )
                            }
                        }
                        
                        if (yesterdayConversations.isNotEmpty()) {
                            item {
                                Text(
                                    text = "Yesterday",
                                    fontSize = 16.sp,
                                    fontWeight = FontWeight.SemiBold,
                                    color = MedinovaOnBackground,
                                    modifier = Modifier.padding(vertical = 8.dp)
                                )
                            }
                            
                            items(yesterdayConversations) { conversation ->
                                ChatConversationItem(
                                    conversation = conversation,
                                    onClick = {
                                        // Navigate to chat detail
                                    }
                                )
                            }
                        }
                    }
                }
            }
            1 -> {
                // Voice calls tab
                Box(
                    modifier = Modifier.fillMaxSize(),
                    contentAlignment = Alignment.Center
                ) {
                    Column(
                        horizontalAlignment = Alignment.CenterHorizontally
                    ) {
                        Text(
                            text = "📞",
                            fontSize = 48.sp
                        )
                        Spacer(modifier = Modifier.height(16.dp))
                        Text(
                            text = "No voice calls yet",
                            fontSize = 16.sp,
                            color = MedinovaGray
                        )
                    }
                }
            }
            2 -> {
                // Video calls tab
                Box(
                    modifier = Modifier.fillMaxSize(),
                    contentAlignment = Alignment.Center
                ) {
                    Column(
                        horizontalAlignment = Alignment.CenterHorizontally
                    ) {
                        Text(
                            text = "📹",
                            fontSize = 48.sp
                        )
                        Spacer(modifier = Modifier.height(16.dp))
                        Text(
                            text = "No video calls yet",
                            fontSize = 16.sp,
                            color = MedinovaGray
                        )
                    }
                }
            }
        }
    }
}

@Composable
fun ChatConversationItem(
    conversation: ChatConversation,
    onClick: () -> Unit
) {
    val timeFormat = SimpleDateFormat("hh:mm a", Locale.getDefault())
    
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 4.dp)
            .clickable { onClick() },
        colors = CardDefaults.cardColors(containerColor = Color.White),
        shape = RoundedCornerShape(12.dp),
        elevation = CardDefaults.cardElevation(defaultElevation = 2.dp)
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            // Doctor image with online status
            Box {
                Box(
                    modifier = Modifier
                        .size(50.dp)
                        .clip(CircleShape)
                        .background(MedinovaLightGray),
                    contentAlignment = Alignment.Center
                ) {
                    Text(
                        text = conversation.doctorName.first().toString(),
                        fontSize = 20.sp,
                        fontWeight = FontWeight.Bold,
                        color = MedinovaPrimary
                    )
                }
                
                if (conversation.isOnline) {
                    Box(
                        modifier = Modifier
                            .size(12.dp)
                            .clip(CircleShape)
                            .background(OnlineGreen)
                            .align(Alignment.BottomEnd)
                    )
                }
            }
            
            Spacer(modifier = Modifier.width(12.dp))
            
            Column(
                modifier = Modifier.weight(1f)
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(
                        text = conversation.doctorName,
                        fontSize = 16.sp,
                        fontWeight = FontWeight.SemiBold,
                        color = MedinovaOnSurface
                    )
                    
                    Text(
                        text = timeFormat.format(conversation.lastMessage.timestamp),
                        fontSize = 12.sp,
                        color = MedinovaGray
                    )
                }
                
                Spacer(modifier = Modifier.height(4.dp))
                
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Row(
                        verticalAlignment = Alignment.CenterVertically,
                        modifier = Modifier.weight(1f)
                    ) {
                        when (conversation.lastMessage.type) {
                            MessageType.AUDIO -> {
                                Icon(
                                    imageVector = Icons.Default.Mic,
                                    contentDescription = "Audio message",
                                    tint = MedinovaPrimary,
                                    modifier = Modifier.size(16.dp)
                                )
                                Spacer(modifier = Modifier.width(4.dp))
                                Text(
                                    text = "${conversation.lastMessage.audioDuration ?: 0}s",
                                    fontSize = 14.sp,
                                    color = MedinovaGray,
                                    maxLines = 1
                                )
                            }
                            MessageType.IMAGE -> {
                                Icon(
                                    imageVector = Icons.Default.Image,
                                    contentDescription = "Image message",
                                    tint = MedinovaPrimary,
                                    modifier = Modifier.size(16.dp)
                                )
                                Spacer(modifier = Modifier.width(4.dp))
                                Text(
                                    text = "Photo",
                                    fontSize = 14.sp,
                                    color = MedinovaGray,
                                    maxLines = 1
                                )
                            }
                            else -> {
                                Text(
                                    text = conversation.lastMessage.content,
                                    fontSize = 14.sp,
                                    color = MedinovaGray,
                                    maxLines = 1
                                )
                            }
                        }
                    }
                    
                    if (conversation.unreadCount > 0) {
                        Box(
                            modifier = Modifier
                                .size(20.dp)
                                .clip(CircleShape)
                                .background(MedinovaPrimary),
                            contentAlignment = Alignment.Center
                        ) {
                            Text(
                                text = conversation.unreadCount.toString(),
                                fontSize = 12.sp,
                                color = Color.White,
                                fontWeight = FontWeight.Bold
                            )
                        }
                    }
                }
            }
        }
    }
}