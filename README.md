# Healthcare Booking System - Low Level Design

## 📋 System Overview

A comprehensive healthcare booking platform with three main modules:
- **User Android App**: Book consultations, make payments, view prescriptions
- **Doctor Android App**: Manage availability, conduct consultations, upload prescriptions  
- **Admin Web Panel**: Manage doctors, clinics, monitor appointments

## 🏗️ Architecture Overview

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   User App      │    │   Doctor App    │    │   Admin Panel   │
│   (Android)     │    │   (Android)     │    │     (Web)       │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
                    ┌─────────────────────┐
                    │   REST API Gateway  │
                    │     (C++ Backend)   │
                    └─────────────────────┘
                                 │
                    ┌─────────────────────┐
                    │   Service Layer     │
                    │   - UserService     │
                    │   - DoctorService   │
                    │   - BookingService  │
                    │   - PaymentService  │
                    └─────────────────────┘
                                 │
                    ┌─────────────────────┐
                    │   Database Layer    │
                    │   - PostgreSQL      │
                    │   - Redis (Cache)   │
                    └─────────────────────┘
```

## 🛠️ Technology Stack

- **Backend**: C++17/20 with Crow/Drogon framework
- **Database**: PostgreSQL with libpqxx
- **Cache**: Redis with hiredis
- **Authentication**: JWT tokens
- **Payment**: Razorpay Integration
- **Build System**: CMake
- **Testing**: Google Test

## 📁 Project Structure

```
healthcare-booking-system/
├── src/
│   ├── models/          # Data models/entities
│   ├── services/        # Business logic layer
│   ├── controllers/     # REST API controllers
│   ├── database/        # Database abstraction layer
│   ├── middleware/      # Authentication, logging
│   ├── utils/           # Utility functions
│   └── main.cpp         # Application entry point
├── include/             # Header files
├── tests/              # Unit and integration tests
├── config/             # Configuration files
├── docker/             # Docker configurations
├── docs/               # API documentation
├── CMakeLists.txt      # Build configuration
└── README.md
```

## 🚀 Quick Start

### Prerequisites
```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install -y cmake g++ libpq-dev libssl-dev libcurl4-openssl-dev
sudo apt-get install -y redis-server postgresql postgresql-contrib

# Install vcpkg for package management
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
```

### Build & Run
```bash
# Clone repository
git clone <repository-url>
cd healthcare-booking-system

# Configure build
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
make -j$(nproc)

# Run
./healthcare_booking_system
```

## 📊 Database Schema

See `docs/database-schema.md` for detailed database design.

## 🔗 API Documentation

- **User APIs**: `/api/v1/users/*`
- **Doctor APIs**: `/api/v1/doctors/*`  
- **Booking APIs**: `/api/v1/bookings/*`
- **Admin APIs**: `/api/v1/admin/*`

Detailed API documentation available at `/docs/api.md`

## 🔐 Security Features

- JWT-based authentication
- Role-based access control (User, Doctor, Admin)
- Request rate limiting
- Input validation and sanitization
- HTTPS enforcement

## 🧪 Testing

```bash
# Run all tests
cd build
make test

# Run specific test suite
./tests/unit_tests
./tests/integration_tests
```

## 📝 License

This project is licensed under the MIT License - see the LICENSE file for details.