cmake_minimum_required(VERSION 3.16)
project(healthcare_booking_system VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configuration
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()

# Find required packages
find_package(PkgConfig REQUIRED)

# PostgreSQL
find_package(PostgreSQL REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

# OpenSSL
find_package(OpenSSL REQUIRED)

# CURL
find_package(CURL REQUIRED)

# Threads
find_package(Threads REQUIRED)

# nlohmann_json
find_package(nlohmann_json REQUIRED)

# vcpkg packages
find_package(Crow CONFIG REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)
find_package(redis++ CONFIG REQUIRED)
find_package(hiredis CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${PostgreSQL_INCLUDE_DIRS})
include_directories(${PQXX_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.cpp
    
    # Models
    src/models/BaseEntity.cpp
    src/models/User.cpp
    src/models/Doctor.cpp
    src/models/Clinic.cpp
    src/models/Appointment.cpp
    src/models/Prescription.cpp
    
    # Database
    src/database/DatabaseManager.cpp
    src/database/UserRepository.cpp
    src/database/DoctorRepository.cpp
    src/database/ClinicRepository.cpp
    src/database/AppointmentRepository.cpp
    src/database/PrescriptionRepository.cpp
    
    # Services
    src/services/UserService.cpp
    src/services/DoctorService.cpp
    src/services/BookingService.cpp
    src/services/PaymentService.cpp
    src/services/NotificationService.cpp
    src/services/ValidationService.cpp
    
    # Controllers
    src/controllers/UserController.cpp
    src/controllers/DoctorController.cpp
    src/controllers/BookingController.cpp
    src/controllers/AdminController.cpp
    
    # Middleware
    src/middleware/AuthMiddleware.cpp
    src/middleware/LoggingMiddleware.cpp
    src/middleware/CorsMiddleware.cpp
    
    # Utilities
    src/utils/Logger.cpp
    src/utils/ConfigManager.cpp
    src/utils/ResponseHelper.cpp
    src/utils/ValidationUtils.cpp
    src/utils/DateTimeUtils.cpp
    src/utils/CryptoUtils.cpp
    src/utils/FileUtils.cpp
)

# Header files
set(HEADERS
    # Models
    include/models/BaseEntity.h
    include/models/User.h
    include/models/Doctor.h
    include/models/Clinic.h
    include/models/Appointment.h
    include/models/Prescription.h
    
    # Database
    include/database/DatabaseManager.h
    include/database/BaseRepository.h
    include/database/UserRepository.h
    include/database/DoctorRepository.h
    include/database/ClinicRepository.h
    include/database/AppointmentRepository.h
    include/database/PrescriptionRepository.h
    
    # Services
    include/services/UserService.h
    include/services/DoctorService.h
    include/services/BookingService.h
    include/services/PaymentService.h
    include/services/NotificationService.h
    include/services/ValidationService.h
    
    # Controllers
    include/controllers/UserController.h
    include/controllers/DoctorController.h
    include/controllers/BookingController.h
    include/controllers/AdminController.h
    
    # Middleware
    include/middleware/AuthMiddleware.h
    include/middleware/LoggingMiddleware.h
    include/middleware/CorsMiddleware.h
    
    # Utilities
    include/utils/Logger.h
    include/utils/ConfigManager.h
    include/utils/ResponseHelper.h
    include/utils/ValidationUtils.h
    include/utils/DateTimeUtils.h
    include/utils/CryptoUtils.h
    include/utils/FileUtils.h
    
    # Configuration
    include/config/AppConfig.h
)

# Create main executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Crow::Crow
    jwt-cpp::jwt-cpp
    redis++::redis++
    hiredis::hiredis
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    ${PostgreSQL_LIBRARIES}
    ${PQXX_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
    CURL::libcurl
    Threads::Threads
)

# Compiler definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    CROW_ENABLE_SSL
    CROW_USE_BOOST=0
    $<$<CONFIG:Debug>:DEBUG_BUILD>
    $<$<CONFIG:Release>:RELEASE_BUILD>
)

# Link directories
target_link_directories(${PROJECT_NAME} PRIVATE
    ${PQXX_LIBRARY_DIRS}
)

# Testing
enable_testing()

# Test sources
set(TEST_SOURCES
    tests/main_test.cpp
    tests/models/UserTest.cpp
    tests/models/DoctorTest.cpp
    tests/models/AppointmentTest.cpp
    tests/services/UserServiceTest.cpp
    tests/services/BookingServiceTest.cpp
    tests/services/PaymentServiceTest.cpp
    tests/database/UserRepositoryTest.cpp
    tests/database/AppointmentRepositoryTest.cpp
    tests/utils/ValidationUtilsTest.cpp
)

# Create test executable
add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})

# Link test libraries
target_link_libraries(${PROJECT_NAME}_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    Crow::Crow
    jwt-cpp::jwt-cpp
    redis++::redis++
    hiredis::hiredis
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    ${PostgreSQL_LIBRARIES}
    ${PQXX_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
    CURL::libcurl
    Threads::Threads
)

# Include test directories
target_include_directories(${PROJECT_NAME}_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests
)

# Add tests
add_test(NAME unit_tests COMMAND ${PROJECT_NAME}_tests)

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Install configuration files
install(DIRECTORY config/
    DESTINATION etc/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.json"
)

# Install documentation
install(DIRECTORY docs/
    DESTINATION share/doc/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.md" PATTERN "*.html"
)

# CPack configuration
set(CPACK_PACKAGE_NAME "Healthcare Booking System")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Healthcare booking and management platform")
set(CPACK_PACKAGE_VENDOR "Healthcare Solutions Inc.")
set(CPACK_PACKAGE_CONTACT "info@healthcaresolutions.com")

# Package types
set(CPACK_GENERATOR "DEB;RPM;TGZ")

# Debian package settings
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libpq5, libssl1.1, libcurl4, redis-server, postgresql")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Healthcare Solutions Inc.")
set(CPACK_DEBIAN_PACKAGE_SECTION "misc")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

# RPM package settings
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Internet")
set(CPACK_RPM_PACKAGE_REQUIRES "postgresql-libs, openssl-libs, libcurl, redis")

include(CPack)

# Custom targets
add_custom_target(format
    COMMAND find ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include -name "*.cpp" -o -name "*.h" | xargs clang-format -i
    COMMENT "Formatting source code"
)

add_custom_target(lint
    COMMAND find ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include -name "*.cpp" -o -name "*.h" | xargs cpplint
    COMMENT "Running code linter"
)

add_custom_target(docs
    COMMAND doxygen Doxyfile
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating documentation"
)

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")