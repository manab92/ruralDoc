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

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find required packages
find_package(PkgConfig REQUIRED)

# Core dependencies
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# Database dependencies
find_package(PostgreSQL REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

# JSON library
find_package(nlohmann_json REQUIRED)

# Try to find vcpkg packages
find_package(Crow CONFIG QUIET)
find_package(jwt-cpp CONFIG QUIET)
find_package(redis++ CONFIG QUIET)
find_package(hiredis CONFIG QUIET)
find_package(spdlog CONFIG QUIET)

# If vcpkg packages not found, try system packages
if(NOT Crow_FOUND)
    message(WARNING "Crow not found via vcpkg, trying system package")
    find_path(CROW_INCLUDE_DIR crow.h PATHS /usr/include /usr/local/include)
    if(CROW_INCLUDE_DIR)
        add_library(Crow::Crow INTERFACE IMPORTED)
        target_include_directories(Crow::Crow INTERFACE ${CROW_INCLUDE_DIR})
    else()
        message(FATAL_ERROR "Crow framework not found. Please install via vcpkg or system package manager.")
    endif()
endif()

if(NOT spdlog_FOUND)
    message(WARNING "spdlog not found via vcpkg, trying system package")
    find_path(SPDLOG_INCLUDE_DIR spdlog/spdlog.h PATHS /usr/include /usr/local/include)
    if(SPDLOG_INCLUDE_DIR)
        add_library(spdlog::spdlog INTERFACE IMPORTED)
        target_include_directories(spdlog::spdlog INTERFACE ${SPDLOG_INCLUDE_DIR})
    else()
        message(FATAL_ERROR "spdlog not found. Please install via vcpkg or system package manager.")
    endif()
endif()

# Main application source files
set(MAIN_SOURCES
    src/main.cpp
)

# Utility source files
set(UTILITY_SOURCES
    src/utils/Logger.cpp
    src/utils/ConfigManager.cpp
    src/utils/ValidationUtils.cpp
    src/utils/CryptoUtils.cpp
    src/utils/ResponseHelper.cpp
)

# Model source files
set(MODEL_SOURCES
    src/models/BaseEntity.cpp
    src/models/User.cpp
    src/models/Doctor.cpp
    src/models/Clinic.cpp
    src/models/Appointment.cpp
    src/models/Prescription.cpp
)

# Database source files
set(DATABASE_SOURCES
    src/database/DatabaseManager.cpp
    src/database/UserRepository.cpp
)

# Middleware source files
set(MIDDLEWARE_SOURCES
    src/middleware/AuthMiddleware.cpp
    src/middleware/CorsMiddleware.cpp
    src/middleware/LoggingMiddleware.cpp
)

# Service source files
set(SERVICE_SOURCES
    # Services will be added when implemented
)

# Controller source files  
set(CONTROLLER_SOURCES
    # Controllers will be added when implemented
)

# All source files
set(ALL_SOURCES
    ${MAIN_SOURCES}
    ${UTILITY_SOURCES}
    ${MODEL_SOURCES}
    ${DATABASE_SOURCES}
    ${MIDDLEWARE_SOURCES}
    ${SERVICE_SOURCES}
    ${CONTROLLER_SOURCES}
)

# Create main executable
add_executable(${PROJECT_NAME} ${ALL_SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
    CURL::libcurl
    ${PostgreSQL_LIBRARIES}
    ${PQXX_LIBRARIES}
    nlohmann_json::nlohmann_json
)

# Link optional packages if found
if(Crow_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Crow::Crow)
endif()

if(spdlog_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
endif()

if(jwt-cpp_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE jwt-cpp::jwt-cpp)
endif()

if(redis++_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE redis++::redis++)
endif()

if(hiredis_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE hiredis::hiredis)
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${PostgreSQL_INCLUDE_DIRS}
    ${PQXX_INCLUDE_DIRS}
)

# Link directories
target_link_directories(${PROJECT_NAME} PRIVATE
    ${PQXX_LIBRARY_DIRS}
)

# Compiler definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG_BUILD>
    $<$<CONFIG:Release>:RELEASE_BUILD>
    HEALTHCARE_VERSION="${PROJECT_VERSION}"
)

# Create directories for runtime
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/logs)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/uploads)

# Copy configuration files to build directory
configure_file(${CMAKE_SOURCE_DIR}/config/app.json ${CMAKE_BINARY_DIR}/config/app.json COPYONLY)

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Install configuration files
install(FILES config/app.json
    DESTINATION etc/${PROJECT_NAME}
)

# Install documentation
install(FILES README.md docs/api.md
    DESTINATION share/doc/${PROJECT_NAME}
)

# Create log and upload directories in install location
install(DIRECTORY DESTINATION var/log/${PROJECT_NAME})
install(DIRECTORY DESTINATION var/lib/${PROJECT_NAME}/uploads)

# Development helpers
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Enable sanitizers in debug mode
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -fsanitize=address
            -fsanitize=undefined
            -fno-omit-frame-pointer
        )
        target_link_options(${PROJECT_NAME} PRIVATE
            -fsanitize=address
            -fsanitize=undefined
        )
    endif()
endif()

# Testing (optional)
option(BUILD_TESTS "Build test suite" OFF)
if(BUILD_TESTS)
    enable_testing()
    find_package(GTest QUIET)
    
    if(GTest_FOUND)
        set(TEST_SOURCES
            tests/main_test.cpp
            tests/utils/ValidationUtilsTest.cpp
            tests/models/UserTest.cpp
            tests/database/UserRepositoryTest.cpp
        )
        
        add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})
        
        target_link_libraries(${PROJECT_NAME}_tests PRIVATE
            GTest::gtest
            GTest::gtest_main
            # Add other libraries as needed
        )
        
        target_include_directories(${PROJECT_NAME}_tests PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/tests
        )
        
        add_test(NAME unit_tests COMMAND ${PROJECT_NAME}_tests)
    else()
        message(WARNING "GTest not found. Tests will not be built.")
    endif()
endif()

# Custom targets
add_custom_target(format
    COMMAND find ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include -name "*.cpp" -o -name "*.h" | xargs clang-format -i
    COMMENT "Formatting source code"
)

add_custom_target(clean-build
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMENT "Cleaning build files"
)

# Development tools
add_custom_target(run
    COMMAND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running Healthcare Booking System"
)

add_custom_target(debug
    COMMAND gdb ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Debugging Healthcare Booking System"
)

# Build information
message(STATUS "========================================")
message(STATUS "Healthcare Booking System Build Config")
message(STATUS "========================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "PostgreSQL found: ${PostgreSQL_FOUND}")
message(STATUS "Crow found: ${Crow_FOUND}")
message(STATUS "spdlog found: ${spdlog_FOUND}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "========================================")

# Package configuration (CPack)
set(CPACK_PACKAGE_NAME "Healthcare Booking System")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A comprehensive healthcare appointment booking and management system")
set(CPACK_PACKAGE_VENDOR "Healthcare Solutions")
set(CPACK_PACKAGE_CONTACT "support@healthcare-booking.com")

# Package types
set(CPACK_GENERATOR "TGZ;ZIP")

if(UNIX AND NOT APPLE)
    list(APPEND CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libpq5, libssl1.1, libcurl4, postgresql-client")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Healthcare Solutions")
    set(CPACK_DEBIAN_PACKAGE_SECTION "misc")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
endif()

include(CPack)