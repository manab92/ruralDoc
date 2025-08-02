# 🔗 Android-C++ Backend Integration Guide

This guide explains how to integrate the **Android Jetpack Compose frontend** with the existing **C++ Crow backend**.

## 🏗️ **Architecture Overview**

```
┌─────────────────────┐    HTTP/REST API    ┌─────────────────────┐
│                     │◄───────────────────►│                     │
│   Android Frontend  │                     │   C++ Crow Backend  │
│   (Jetpack Compose) │                     │   (Healthcare API)  │
│                     │                     │                     │
└─────────────────────┘                     └─────────────────────┘
         │                                            │
         │                                            │
         ▼                                            ▼
┌─────────────────────┐                     ┌─────────────────────┐
│  Local Storage      │                     │  PostgreSQL         │
│  - SharedPrefs      │                     │  - User Data        │
│  - Auth Tokens      │                     │  - Appointments     │
│  - Cache            │                     │  - Doctor Info      │
└─────────────────────┘                     └─────────────────────┘
```

## ✅ **Current Status**

### **✓ Android App Ready:**
- Complete UI implementation
- API service interfaces created
- Repository pattern implemented
- Hilt dependency injection configured
- Authentication flow prepared

### **✓ C++ Backend Available:**
- Crow web framework running
- PostgreSQL database setup
- JWT authentication
- RESTful API endpoints
- CORS middleware ready

## 🔌 **Integration Steps**

### **Step 1: Configure Backend URL**

Update the base URL in `NetworkModule.kt`:

```kotlin
private fun getBaseUrl(): String {
    // Local development (emulator)
    return "http://10.0.2.2:8080/"
    
    // Local development (device on same network)
    // return "http://192.168.1.XXX:8080/"
    
    // Production
    // return "https://your-domain.com/"
}
```

### **Step 2: Verify C++ Backend Endpoints**

Ensure your C++ backend has these endpoints (check `src/main.cpp`):

```cpp
// Authentication
app.route("/api/auth/register").methods("POST"_method)
app.route("/api/auth/login").methods("POST"_method)
app.route("/api/auth/refresh").methods("POST"_method)

// Doctors
app.route("/api/doctors").methods("GET"_method)
app.route("/api/doctors/<string>").methods("GET"_method)
app.route("/api/doctors/specialties").methods("GET"_method)

// Appointments
app.route("/api/appointments").methods("GET"_method, "POST"_method)
app.route("/api/appointments/<string>").methods("PUT"_method, "DELETE"_method)

// Chat
app.route("/api/chat/conversations").methods("GET"_method)
app.route("/api/chat/messages/<string>").methods("GET"_method)
app.route("/api/chat/messages").methods("POST"_method)

// Profile
app.route("/api/profile").methods("GET"_method, "PUT"_method)
```

### **Step 3: Update Android Build Configuration**

Add network security config in `AndroidManifest.xml`:

```xml
<application
    android:networkSecurityConfig="@xml/network_security_config"
    ...>
```

Create `res/xml/network_security_config.xml`:

```xml
<?xml version="1.0" encoding="utf-8"?>
<network-security-config>
    <domain-config cleartextTrafficPermitted="true">
        <domain includeSubdomains="true">10.0.2.2</domain>
        <domain includeSubdomains="true">192.168.1.0/24</domain>
        <domain includeSubdomains="true">localhost</domain>
    </domain-config>
</network-security-config>
```

### **Step 4: Start C++ Backend**

```bash
# Build C++ backend
mkdir build && cd build
cmake ..
make

# Run the server
./healthcare_booking_system

# Should see: "Healthcare Booking System Starting..."
# Server running on: http://localhost:8080
```

### **Step 5: Test Integration**

1. **Start C++ backend** on port 8080
2. **Run Android app** in emulator/device
3. **Test API connectivity**:
   - Registration/Login flows
   - Doctor search
   - Appointment booking
   - Chat functionality

## 📊 **API Response Format**

Ensure your C++ backend returns JSON in this format:

```json
// Success Response
{
    "success": true,
    "message": "Operation successful",
    "data": { ... }
}

// Error Response
{
    "success": false,
    "message": "Error description",
    "error": "ERROR_CODE"
}
```

## 🔐 **Authentication Flow**

```sequence
Android App -> C++ Backend: POST /api/auth/login
C++ Backend -> Database: Verify credentials
Database -> C++ Backend: User data
C++ Backend -> Android App: JWT tokens
Android App -> Android App: Store tokens
Android App -> C++ Backend: API calls with Bearer token
```

## 🛠️ **Required C++ Backend Modifications**

### **1. Add CORS Headers**

```cpp
// In your C++ backend
app.route("/api/<path>").methods("OPTIONS"_method)
([](const crow::request& req) {
    crow::response res;
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    return res;
});
```

### **2. JSON Response Helper**

```cpp
crow::json::wvalue createSuccessResponse(const crow::json::wvalue& data) {
    crow::json::wvalue response;
    response["success"] = true;
    response["data"] = data;
    return response;
}

crow::json::wvalue createErrorResponse(const std::string& message) {
    crow::json::wvalue response;
    response["success"] = false;
    response["message"] = message;
    return response;
}
```

### **3. Authentication Middleware**

```cpp
struct AuthMiddleware {
    struct context {
        std::string user_id;
        bool is_authenticated = false;
    };

    void before_handle(crow::request& req, crow::response& res, context& ctx) {
        auto auth_header = req.get_header_value("Authorization");
        if (auth_header.empty() || !auth_header.starts_with("Bearer ")) {
            res.code = 401;
            res.body = createErrorResponse("Missing or invalid authorization header");
            res.end();
            return;
        }

        std::string token = auth_header.substr(7); // Remove "Bearer "
        // Verify JWT token here
        // Set ctx.user_id and ctx.is_authenticated
    }
};
```

## 🔄 **Real-time Features (Optional)**

### **WebSocket for Chat (Future Enhancement)**

```cpp
// C++ Backend - WebSocket endpoint
app.websocket("/ws/chat")
    .onopen([&](crow::websocket::connection& conn) {
        // Handle connection
    })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
        // Handle disconnection
    })
    .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
        // Handle messages
    });
```

```kotlin
// Android - WebSocket client (add to build.gradle)
implementation "org.java-websocket:Java-WebSocket:1.5.3"
```

## 🧪 **Testing Integration**

### **1. API Testing with curl**

```bash
# Test backend directly
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"email":"test@example.com","password":"password"}'

# Test with Android app
# Check Android Studio Logcat for HTTP requests/responses
```

### **2. Network Debugging**

```kotlin
// Add to NetworkModule.kt for debugging
val loggingInterceptor = HttpLoggingInterceptor().apply {
    level = HttpLoggingInterceptor.Level.BODY
}
```

### **3. Database Verification**

```sql
-- Check data in PostgreSQL
SELECT * FROM users;
SELECT * FROM doctors;
SELECT * FROM appointments;
```

## 📱 **Development Workflow**

1. **Start C++ backend**: `./healthcare_booking_system`
2. **Open Android Studio**: Import the project
3. **Run on emulator**: Or physical device
4. **Monitor logs**: Check both C++ and Android logs
5. **Test features**: Registration → Login → Browse → Book

## 🐛 **Common Issues & Solutions**

### **Connection Refused**
- ✅ Check C++ backend is running
- ✅ Verify port 8080 is available
- ✅ Use correct IP (10.0.2.2 for emulator)

### **CORS Errors**
- ✅ Add CORS headers in C++ backend
- ✅ Handle OPTIONS requests

### **Authentication Failures**
- ✅ Check JWT token format
- ✅ Verify Authorization header
- ✅ Check token expiration

### **JSON Parsing Errors**
- ✅ Match response format exactly
- ✅ Check field names and types
- ✅ Handle null values properly

## 🚀 **Deployment Considerations**

### **Production Setup**
1. **Backend**: Deploy C++ server to cloud (AWS, Azure, GCP)
2. **Database**: Use managed PostgreSQL
3. **SSL**: Enable HTTPS with certificates
4. **Monitoring**: Add logging and metrics
5. **Scaling**: Load balancing for high traffic

### **Android Release**
1. **ProGuard**: Configure code obfuscation
2. **Signing**: Setup release keystore
3. **Play Store**: Prepare for submission
4. **Updates**: Setup CI/CD pipeline

## 🎯 **Next Steps**

1. ✅ **Basic Integration**: Connect Android to C++ API
2. ✅ **Authentication**: Login/register flows
3. ✅ **Core Features**: Doctors, appointments, chat
4. ⏳ **Real-time Chat**: WebSocket implementation
5. ⏳ **Push Notifications**: Firebase integration
6. ⏳ **Payment Gateway**: Stripe/PayPal integration
7. ⏳ **Video Calls**: WebRTC implementation

---

## 📞 **Support**

If you encounter issues during integration:
1. Check C++ backend logs
2. Monitor Android Logcat
3. Verify network connectivity
4. Test API endpoints directly
5. Review this guide step-by-step

**The Android frontend is fully prepared and ready to connect to your C++ backend!** 🎉