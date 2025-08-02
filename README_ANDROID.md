# Medinova - Online Doctor Consultation Android App

A comprehensive telemedicine Android application built with **Jetpack Compose** and **Modern Android Architecture** that connects patients with healthcare professionals for online consultations.

## 🏥 Features

### Core Functionality
- **🔐 User Authentication** - Secure login and registration
- **👨‍⚕️ Doctor Discovery** - Browse and search doctors by specialty
- **📅 Appointment Booking** - Schedule video/voice/chat consultations
- **💬 Real-time Chat** - Text and voice messaging with doctors
- **📞 Video/Voice Calls** - High-quality video consultations
- **📊 Medical History** - Track appointments and prescriptions
- **💳 Payment Integration** - Secure payment processing
- **🔔 Smart Notifications** - Appointment reminders and updates

### Key Screens Implemented

#### 🎯 Onboarding Flow
- **Splash Screen** - App logo with loading animation
- **Onboarding** - 3-page introduction to app features
- **Authentication** - Login/Register (ready for integration)

#### 🏠 Main Application
- **Home Screen** 
  - Doctor discovery with department categories
  - Top-rated doctors showcase
  - Quick consultation actions
  - Search functionality

- **Search & Discovery**
  - Advanced doctor search with filters
  - Specialty-based browsing
  - Real-time search results
  - Doctor profiles with ratings

- **Appointment Management**
  - Upcoming and past appointments
  - Join consultation calls
  - Reschedule/cancel options
  - Appointment status tracking

- **Chat & Communication**
  - Conversation list with doctors
  - Text and voice messaging support
  - Online status indicators
  - Message history organization

- **User Profile**
  - Personal information management
  - Appointment statistics
  - Settings and preferences
  - Help and support access

## 🛠️ Technology Stack

### Frontend
- **Jetpack Compose** - Modern declarative UI toolkit
- **Material Design 3** - Latest Material Design components
- **Navigation Compose** - Type-safe navigation
- **Compose Animation** - Smooth UI transitions

### Architecture
- **MVVM Pattern** - Clean architecture separation
- **Hilt Dependency Injection** - Efficient dependency management
- **Clean Architecture** - Domain, Data, and Presentation layers

### Networking & Data
- **Retrofit** - REST API communication
- **OkHttp** - HTTP client with logging
- **Gson** - JSON serialization/deserialization
- **Coil** - Image loading and caching

### Real-time Communication
- **WebRTC** - Video/voice calling capabilities
- **Socket.IO** (ready) - Real-time messaging
- **Audio Recording** - Voice message functionality

### Additional Libraries
- **Lottie** - Vector animations
- **Material Date Picker** - Date/time selection
- **Permissions** - Runtime permission handling

## 🎨 Design System

### Color Palette
- **Primary**: `#4A90E2` (Medical Blue)
- **Secondary**: `#50C878` (Medical Green)
- **Background**: `#F8F9FA` (Light Gray)
- **Success**: `#27AE60` (Green)
- **Error**: `#E74C3C` (Red)
- **Warning**: `#F39C12` (Orange)

### Typography
- Modern, readable font hierarchy
- Accessibility-compliant text sizes
- Consistent spacing and alignment

### Components
- Rounded corners (8-16dp) for modern look
- Card-based layouts for content organization
- Consistent icon usage throughout the app
- Intuitive button designs and interactions

## 📱 Screen Specifications

### Home Screen Features
- Department categories with emoji icons
- Doctor cards with ratings and online status
- Quick action buttons for immediate consultation
- Search bar with real-time suggestions

### Appointment Screen Features
- Tab-based navigation (Upcoming/Past)
- Detailed appointment cards with actions
- Status badges and type indicators
- Join call functionality

### Chat Screen Features
- Conversation list with unread counters
- Message type indicators (text/audio/image)
- Online status for doctors
- Time-organized message groups

### Profile Screen Features
- User statistics dashboard
- Comprehensive settings menu
- Quick access to key features
- Logout functionality

## 🚀 Getting Started

### Prerequisites
- Android Studio Arctic Fox or later
- Kotlin 1.9.10+
- Gradle 8.1.4+
- Min SDK: API 24 (Android 7.0)
- Target SDK: API 34 (Android 14)

### Installation

1. **Clone the repository**
```bash
git clone https://github.com/your-repo/medinova-android.git
cd medinova-android
```

2. **Open in Android Studio**
- Open Android Studio
- Select "Open an existing project"
- Navigate to the cloned directory
- Wait for Gradle sync to complete

3. **Build and Run**
```bash
# Via Android Studio
Click "Run" button or press Shift+F10

# Via Command Line
./gradlew assembleDebug
./gradlew installDebug
```

## 📂 Project Structure

```
app/
├── src/main/
│   ├── java/com/medinova/app/
│   │   ├── domain/
│   │   │   └── model/          # Data models
│   │   ├── presentation/
│   │   │   ├── screens/        # UI screens
│   │   │   ├── theme/          # Design system
│   │   │   └── navigation/     # Navigation setup
│   │   └── MedinovaApplication.kt
│   ├── res/
│   │   ├── values/             # Colors, strings, themes
│   │   └── xml/                # Backup and data rules
│   └── AndroidManifest.xml
└── build.gradle.kts
```

## 🔧 Configuration

### API Integration
The app is structured to easily integrate with backend APIs:

```kotlin
// Add your API base URL in build.gradle
buildConfigField("String", "API_BASE_URL", "\"https://your-api.com/\"")
```

### Firebase Setup (Optional)
For push notifications and analytics:
1. Add `google-services.json` to `app/` directory
2. Uncomment Firebase dependencies in `build.gradle.kts`

### WebRTC Configuration
Update WebRTC settings in the app:
```kotlin
// Configure STUN/TURN servers for video calls
val peerConnectionFactory = PeerConnectionFactory.builder()
    .createPeerConnectionFactory()
```

## 🎯 Key Features Implementation

### Doctor Search & Discovery
- Real-time search with filtering
- Department-based categorization
- Rating and review system
- Online status indicators

### Appointment Management
- Multi-type bookings (Video/Voice/Chat)
- Calendar integration ready
- Status tracking and notifications
- Reschedule/cancel functionality

### Communication System
- Text messaging interface
- Voice message recording
- Video call integration
- File sharing capabilities (ready)

### User Experience
- Smooth animations and transitions
- Intuitive navigation patterns
- Accessibility support
- Offline capability (ready for implementation)

## 📊 Sample Data

The app includes comprehensive sample data for demonstration:
- Mock doctor profiles with specialties
- Sample appointment history
- Chat conversations with different message types
- User profile information

## 🔒 Security Features

- Permission handling for camera/microphone
- Secure data transmission ready
- User authentication flow
- Privacy policy compliance

## 🚧 Future Enhancements

### Planned Features
- [ ] Backend API integration
- [ ] Real-time WebSocket communication
- [ ] Push notification system
- [ ] Payment gateway integration
- [ ] Prescription management
- [ ] Medical document upload
- [ ] Offline mode support
- [ ] Multi-language support

### Technical Improvements
- [ ] Unit and UI testing
- [ ] CI/CD pipeline setup
- [ ] Performance optimization
- [ ] Accessibility enhancements
- [ ] Code documentation

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👥 Team

- **UI/UX Design**: Based on modern telemedicine app patterns
- **Android Development**: Jetpack Compose implementation
- **Architecture**: Clean MVVM with Hilt DI

## 📞 Support

For support and questions:
- Create an issue in the repository
- Contact the development team
- Check the documentation

---

**Medinova** - Bringing healthcare to your fingertips 🏥📱