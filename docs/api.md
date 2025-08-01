# Healthcare Booking System API Documentation

## Overview

The Healthcare Booking System provides REST APIs for three main modules:
- **User App**: Patient registration, booking, payments
- **Doctor App**: Schedule management, consultations, prescriptions  
- **Admin Panel**: System management, analytics, monitoring

**Base URL**: `https://api.healthcarebooking.com/api/v1`
**Authentication**: JWT Bearer Token
**Content-Type**: `application/json`

## Authentication

### Register User
```http
POST /auth/register
```

**Request Body:**
```json
{
  "email": "user@example.com",
  "password": "SecurePass123!",
  "first_name": "John",
  "last_name": "Doe",
  "phone_number": "+91-9876543210",
  "role": "USER",
  "gender": "MALE",
  "date_of_birth": "1990-05-15",
  "address": "123 Main St",
  "city": "Mumbai",
  "state": "Maharashtra",
  "pincode": "400001"
}
```

**Response (201):**
```json
{
  "success": true,
  "message": "User registered successfully",
  "data": {
    "user_id": "uuid-here",
    "email": "user@example.com",
    "jwt_token": "eyJhbGciOiJIUzI1NiIs...",
    "refresh_token": "refresh-token-here"
  }
}
```

### Login User
```http
POST /auth/login
```

**Request Body:**
```json
{
  "email": "user@example.com",
  "password": "SecurePass123!",
  "fcm_token": "fcm-token-for-notifications"
}
```

**Response (200):**
```json
{
  "success": true,
  "message": "Login successful",
  "data": {
    "user": {
      "id": "uuid-here",
      "email": "user@example.com",
      "first_name": "John",
      "last_name": "Doe",
      "role": "USER",
      "is_verified": true
    },
    "jwt_token": "eyJhbGciOiJIUzI1NiIs...",
    "refresh_token": "refresh-token-here"
  }
}
```

## User Management

### Get User Profile
```http
GET /users/{user_id}
Authorization: Bearer <jwt_token>
```

**Response (200):**
```json
{
  "success": true,
  "data": {
    "id": "uuid-here",
    "email": "user@example.com",
    "first_name": "John",
    "last_name": "Doe",
    "phone_number": "+91-9876543210",
    "role": "USER",
    "city": "Mumbai",
    "state": "Maharashtra",
    "is_verified": true,
    "created_at": "2024-01-15T10:30:00Z"
  }
}
```

### Update User Profile
```http
PUT /users/{user_id}
Authorization: Bearer <jwt_token>
```

**Request Body:**
```json
{
  "first_name": "John",
  "last_name": "Smith",
  "phone_number": "+91-9876543210",
  "address": "456 New Street",
  "city": "Delhi"
}
```

## Doctor Management

### Search Available Doctors
```http
GET /doctors/search?specialization=cardiology&city=mumbai&date=2024-02-15&type=ONLINE
```

**Response (200):**
```json
{
  "success": true,
  "data": [
    {
      "id": "doctor-uuid",
      "first_name": "Dr. Sarah",
      "last_name": "Wilson",
      "specializations": [
        {
          "id": "spec-uuid",
          "name": "Cardiology",
          "description": "Heart specialist"
        }
      ],
      "years_of_experience": 15,
      "consultation_fee": 800.0,
      "rating": 4.8,
      "total_reviews": 324,
      "clinic": {
        "id": "clinic-uuid",
        "name": "Heart Care Clinic",
        "address": "123 Medical Street, Mumbai"
      },
      "next_available_slots": [
        {
          "start_time": "2024-02-15T09:00:00Z",
          "end_time": "2024-02-15T09:30:00Z",
          "is_available": true
        }
      ]
    }
  ],
  "pagination": {
    "page": 1,
    "page_size": 20,
    "total_pages": 5,
    "total_count": 89
  }
}
```

### Get Doctor Availability
```http
GET /doctors/{doctor_id}/availability?start_date=2024-02-15&end_date=2024-02-20
Authorization: Bearer <jwt_token>
```

**Response (200):**
```json
{
  "success": true,
  "data": [
    {
      "date": "2024-02-15",
      "slots": [
        {
          "start_time": "2024-02-15T09:00:00Z",
          "end_time": "2024-02-15T09:30:00Z",
          "is_available": true,
          "consultation_fee": 800.0
        },
        {
          "start_time": "2024-02-15T09:30:00Z",
          "end_time": "2024-02-15T10:00:00Z",
          "is_available": false,
          "consultation_fee": 800.0
        }
      ]
    }
  ]
}
```

## Appointment Booking

### Book Appointment
```http
POST /appointments
Authorization: Bearer <jwt_token>
```

**Request Body:**
```json
{
  "doctor_id": "doctor-uuid",
  "clinic_id": "clinic-uuid",
  "appointment_date": "2024-02-15",
  "start_time": "2024-02-15T09:00:00Z",
  "type": "ONLINE",
  "symptoms": "Chest pain and difficulty breathing",
  "notes": "Patient has history of heart issues",
  "is_emergency": false
}
```

**Response (201):**
```json
{
  "success": true,
  "message": "Appointment booked successfully",
  "data": {
    "appointment": {
      "id": "appointment-uuid",
      "user_id": "user-uuid",
      "doctor_id": "doctor-uuid",
      "clinic_id": "clinic-uuid",
      "appointment_date": "2024-02-15",
      "start_time": "2024-02-15T09:00:00Z",
      "end_time": "2024-02-15T09:30:00Z",
      "type": "ONLINE",
      "status": "PENDING",
      "consultation_fee": 800.0,
      "payment_status": "PENDING"
    },
    "payment_url": "https://razorpay.com/payment/pay_xyz123"
  }
}
```

### Get User Appointments
```http
GET /users/{user_id}/appointments?status=CONFIRMED&page=1&page_size=10
Authorization: Bearer <jwt_token>
```

**Response (200):**
```json
{
  "success": true,
  "data": [
    {
      "id": "appointment-uuid",
      "doctor": {
        "id": "doctor-uuid",
        "name": "Dr. Sarah Wilson",
        "specialization": "Cardiology"
      },
      "clinic": {
        "id": "clinic-uuid",
        "name": "Heart Care Clinic",
        "address": "123 Medical Street, Mumbai"
      },
      "appointment_date": "2024-02-15",
      "start_time": "2024-02-15T09:00:00Z",
      "end_time": "2024-02-15T09:30:00Z",
      "type": "ONLINE",
      "status": "CONFIRMED",
      "consultation_fee": 800.0,
      "payment_status": "PAID",
      "video_call_link": "https://meet.healthcare.com/room/abc123"
    }
  ]
}
```

### Cancel Appointment
```http
DELETE /appointments/{appointment_id}
Authorization: Bearer <jwt_token>
```

**Request Body:**
```json
{
  "reason": "Emergency came up, need to reschedule"
}
```

**Response (200):**
```json
{
  "success": true,
  "message": "Appointment cancelled successfully",
  "data": {
    "appointment_id": "appointment-uuid",
    "status": "CANCELLED",
    "refund_amount": 720.0,
    "refund_id": "refund-uuid"
  }
}
```

## Payment Integration

### Process Payment
```http
POST /appointments/{appointment_id}/payment
Authorization: Bearer <jwt_token>
```

**Request Body:**
```json
{
  "payment_method": "RAZORPAY",
  "payment_id": "pay_xyz123",
  "order_id": "order_abc456",
  "signature": "signature_hash"
}
```

**Response (200):**
```json
{
  "success": true,
  "message": "Payment processed successfully",
  "data": {
    "payment_id": "pay_xyz123",
    "amount": 800.0,
    "currency": "INR",
    "status": "PAID",
    "appointment_status": "CONFIRMED"
  }
}
```

### Verify Payment
```http
GET /payments/{payment_id}/verify
Authorization: Bearer <jwt_token>
```

**Response (200):**
```json
{
  "success": true,
  "data": {
    "payment_id": "pay_xyz123",
    "appointment_id": "appointment-uuid",
    "amount": 800.0,
    "status": "VERIFIED",
    "verified_at": "2024-02-15T08:45:00Z"
  }
}
```

## Doctor APIs

### Get Doctor Schedule
```http
GET /doctors/{doctor_id}/schedule?date=2024-02-15
Authorization: Bearer <jwt_token>
```

**Response (200):**
```json
{
  "success": true,
  "data": {
    "date": "2024-02-15",
    "appointments": [
      {
        "id": "appointment-uuid",
        "patient": {
          "id": "user-uuid",
          "name": "John Doe",
          "age": 34,
          "phone": "+91-9876543210"
        },
        "start_time": "2024-02-15T09:00:00Z",
        "end_time": "2024-02-15T09:30:00Z",
        "type": "ONLINE",
        "status": "CONFIRMED",
        "symptoms": "Chest pain and difficulty breathing"
      }
    ],
    "total_appointments": 8,
    "available_slots": 4
  }
}
```

### Update Doctor Availability
```http
PUT /doctors/{doctor_id}/availability
Authorization: Bearer <jwt_token>
```

**Request Body:**
```json
{
  "date": "2024-02-15",
  "slots": [
    {
      "start_time": "2024-02-15T09:00:00Z",
      "end_time": "2024-02-15T17:00:00Z",
      "is_available": true,
      "break_times": [
        {
          "start_time": "2024-02-15T13:00:00Z",
          "end_time": "2024-02-15T14:00:00Z"
        }
      ]
    }
  ]
}
```

### Upload Prescription
```http
POST /appointments/{appointment_id}/prescription
Authorization: Bearer <jwt_token>
Content-Type: multipart/form-data
```

**Request Body:**
```json
{
  "diagnosis": "Mild hypertension",
  "medicines": [
    {
      "name": "Amlodipine",
      "dosage": "5mg",
      "frequency": "Once daily",
      "duration_days": 30,
      "instructions": "Take after breakfast"
    }
  ],
  "follow_up_date": "2024-03-15",
  "notes": "Monitor blood pressure regularly"
}
```

## Admin APIs

### Get System Statistics
```http
GET /admin/statistics
Authorization: Bearer <jwt_token>
```

**Response (200):**
```json
{
  "success": true,
  "data": {
    "users": {
      "total": 15420,
      "active": 12350,
      "new_this_month": 1250
    },
    "doctors": {
      "total": 850,
      "verified": 820,
      "new_this_month": 45
    },
    "appointments": {
      "total": 45600,
      "completed": 42300,
      "today": 156
    },
    "revenue": {
      "total": 2456000.0,
      "this_month": 245600.0,
      "currency": "INR"
    }
  }
}
```

### Manage Users
```http
GET /admin/users?role=DOCTOR&status=PENDING&page=1
Authorization: Bearer <jwt_token>
```

**Response (200):**
```json
{
  "success": true,
  "data": [
    {
      "id": "user-uuid",
      "email": "doctor@example.com",
      "name": "Dr. John Smith",
      "role": "DOCTOR",
      "status": "PENDING_VERIFICATION",
      "registration_date": "2024-02-10T10:30:00Z",
      "documents": [
        {
          "type": "medical_license",
          "url": "https://storage.com/license.pdf",
          "verified": false
        }
      ]
    }
  ]
}
```

## Error Responses

All endpoints return consistent error responses:

```json
{
  "success": false,
  "error": {
    "code": "VALIDATION_ERROR",
    "message": "Invalid input data",
    "details": [
      {
        "field": "email",
        "message": "Invalid email format"
      }
    ]
  },
  "timestamp": "2024-02-15T10:30:00Z"
}
```

### Common Error Codes
- `VALIDATION_ERROR` - Invalid input data
- `AUTHENTICATION_ERROR` - Invalid or missing token
- `AUTHORIZATION_ERROR` - Insufficient permissions
- `NOT_FOUND` - Resource not found
- `CONFLICT` - Resource already exists
- `PAYMENT_ERROR` - Payment processing failed
- `BOOKING_ERROR` - Appointment booking failed
- `SERVER_ERROR` - Internal server error

## Rate Limiting

- **Public endpoints**: 100 requests per minute
- **Authenticated endpoints**: 1000 requests per minute
- **Admin endpoints**: 500 requests per minute

Rate limit headers are included in responses:
```
X-RateLimit-Limit: 1000
X-RateLimit-Remaining: 995
X-RateLimit-Reset: 1644937200
```

## Webhooks

### Payment Webhook
```http
POST /webhooks/payment
```

**Request Body:**
```json
{
  "event": "payment.captured",
  "payload": {
    "payment_id": "pay_xyz123",
    "order_id": "order_abc456",
    "amount": 80000,
    "currency": "INR",
    "status": "captured"
  }
}
```

### Appointment Reminder Webhook
```http
POST /webhooks/appointment-reminder
```

**Request Body:**
```json
{
  "event": "appointment.reminder",
  "payload": {
    "appointment_id": "appointment-uuid",
    "user_id": "user-uuid",
    "doctor_id": "doctor-uuid",
    "reminder_type": "24_hour",
    "appointment_time": "2024-02-15T09:00:00Z"
  }
}
```

## SDK Integration

### Android SDK Example
```java
// Initialize SDK
HealthcareBookingSDK sdk = new HealthcareBookingSDK("your_api_key");

// Login user
sdk.auth().login("user@example.com", "password", new AuthCallback() {
    @Override
    public void onSuccess(User user, String token) {
        // Handle successful login
    }
    
    @Override
    public void onError(ApiError error) {
        // Handle error
    }
});

// Book appointment
BookingRequest request = new BookingRequest()
    .setDoctorId("doctor-uuid")
    .setDateTime("2024-02-15T09:00:00Z")
    .setType(AppointmentType.ONLINE);

sdk.booking().bookAppointment(request, new BookingCallback() {
    @Override
    public void onSuccess(Appointment appointment) {
        // Handle successful booking
    }
});
```

This API documentation provides comprehensive coverage of all endpoints in the healthcare booking system, including request/response formats, authentication, error handling, and integration examples.