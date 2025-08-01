#include <iostream>
#include <memory>
#include <signal.h>
#include <crow.h>
#include <nlohmann/json.hpp>

// Configuration
#include "../include/config/AppConfig.h"

// Database
#include "../include/database/DatabaseManager.h"

// Services
#include "../include/services/UserService.h"
#include "../include/services/BookingService.h"
#include "../include/services/PaymentService.h"
#include "../include/services/NotificationService.h"

// Controllers
#include "../include/controllers/UserController.h"
#include "../include/controllers/BookingController.h"
#include "../include/controllers/DoctorController.h"
#include "../include/controllers/AdminController.h"

// Middleware
#include "../include/middleware/AuthMiddleware.h"
#include "../include/middleware/LoggingMiddleware.h"
#include "../include/middleware/CorsMiddleware.h"

// Utilities
#include "../include/utils/Logger.h"
#include "../include/utils/ConfigManager.h"

using namespace healthcare;

// Global application state
std::unique_ptr<crow::Crow<crow::CookieParser, middleware::AuthMiddleware>> app;
bool is_shutting_down = false;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ". Initiating graceful shutdown..." << std::endl;
    is_shutting_down = true;
    
    if (app) {
        app->stop();
    }
    
    // Cleanup database connections
    database::DatabaseManager::getInstance().disconnect();
    
    std::cout << "Shutdown complete." << std::endl;
    exit(0);
}

// Initialize application components
bool initializeApplication() {
    try {
        // Load configuration
        auto config_manager = std::make_unique<utils::ConfigManager>();
        if (!config_manager->loadConfig("config/app.json")) {
            std::cerr << "Failed to load application configuration" << std::endl;
            return false;
        }

        // Initialize logger
        utils::Logger::getInstance().configure(
            config_manager->getString("logging.level", "INFO"),
            config_manager->getString("logging.file", "healthcare.log"),
            config_manager->getBool("logging.console", true)
        );

        utils::Logger::getInstance().info("Starting Healthcare Booking System...");

        // Initialize database
        auto& db_manager = database::DatabaseManager::getInstance();
        
        database::DatabaseConfig db_config;
        db_config.host = config_manager->getString("database.host", "localhost");
        db_config.port = config_manager->getInt("database.port", 5432);
        db_config.database = config_manager->getString("database.name", "healthcare_db");
        db_config.username = config_manager->getString("database.username", "postgres");
        db_config.password = config_manager->getString("database.password", "");
        db_config.max_connections = config_manager->getInt("database.max_connections", 10);

        database::RedisConfig redis_config;
        redis_config.host = config_manager->getString("redis.host", "localhost");
        redis_config.port = config_manager->getInt("redis.port", 6379);
        redis_config.password = config_manager->getString("redis.password", "");
        redis_config.database = config_manager->getInt("redis.database", 0);

        db_manager.configure(db_config, redis_config);
        
        if (!db_manager.connect()) {
            std::cerr << "Failed to connect to database" << std::endl;
            return false;
        }

        utils::Logger::getInstance().info("Database connection established");

        // Create database tables if needed
        if (config_manager->getBool("database.auto_migrate", true)) {
            if (!db_manager.migrateDatabase()) {
                std::cerr << "Database migration failed" << std::endl;
                return false;
            }
            utils::Logger::getInstance().info("Database migration completed");
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize application: " << e.what() << std::endl;
        return false;
    }
}

// Configure middleware
void configureMiddleware(crow::Crow<crow::CookieParser, middleware::AuthMiddleware>& app, 
                        const utils::ConfigManager& config) {
    // CORS middleware
    auto cors_middleware = std::make_unique<middleware::CorsMiddleware>();
    cors_middleware->setAllowedOrigins(config.getStringArray("cors.allowed_origins"));
    cors_middleware->setAllowedMethods({"GET", "POST", "PUT", "DELETE", "OPTIONS"});
    cors_middleware->setAllowedHeaders({"Content-Type", "Authorization", "X-Requested-With"});

    // Authentication middleware
    auto auth_middleware = std::get<1>(app.middlewares);
    auth_middleware.setJwtSecret(config.getString("jwt.secret"));
    auth_middleware.setJwtIssuer(config.getString("jwt.issuer", "healthcare-booking"));
    auth_middleware.setTokenExpiryHours(config.getInt("jwt.expiry_hours", 24));

    // Add public endpoints
    auth_middleware.addPublicEndpoint("/api/v1/auth/register");
    auth_middleware.addPublicEndpoint("/api/v1/auth/login");
    auth_middleware.addPublicEndpoint("/api/v1/auth/forgot-password");
    auth_middleware.addPublicEndpoint("/api/v1/auth/reset-password");
    auth_middleware.addPublicEndpoint("/api/v1/health");
    auth_middleware.addPublicEndpoint("/api/v1/docs");

    // Add admin endpoints
    auth_middleware.addAdminEndpoint("/api/v1/admin");
    auth_middleware.addAdminEndpoint("/api/v1/users/admin");
    auth_middleware.addAdminEndpoint("/api/v1/statistics");
}

// Register API routes
void registerRoutes(crow::Crow<crow::CookieParser, middleware::AuthMiddleware>& app) {
    // Health check endpoint
    CROW_ROUTE(app, "/api/v1/health")
    ([&](const crow::request& req) {
        nlohmann::json response;
        response["status"] = "healthy";
        response["timestamp"] = std::time(nullptr);
        response["version"] = "1.0.0";
        response["database"] = database::DatabaseManager::getInstance().getHealthStatus();
        
        return crow::response(200, response.dump());
    });

    // API documentation endpoint
    CROW_ROUTE(app, "/api/v1/docs")
    ([](const crow::request& req) {
        return crow::load_text("docs/api.html");
    });

    // Register controller routes
    auto user_controller = std::make_unique<controllers::UserController>();
    user_controller->registerRoutes(app);

    auto booking_controller = std::make_unique<controllers::BookingController>();
    booking_controller->registerRoutes(app);

    auto doctor_controller = std::make_unique<controllers::DoctorController>();
    doctor_controller->registerRoutes(app);

    auto admin_controller = std::make_unique<controllers::AdminController>();
    admin_controller->registerRoutes(app);

    utils::Logger::getInstance().info("API routes registered successfully");
}

// Configure application settings
void configureApp(crow::Crow<crow::CookieParser, middleware::AuthMiddleware>& app,
                 const utils::ConfigManager& config) {
    // Set server configuration
    app.port(config.getInt("server.port", 8080))
       .multithreaded(config.getInt("server.threads", 4))
       .server_name("Healthcare Booking System")
       .bindaddr(config.getString("server.host", "0.0.0.0"));

    // Set request limits
    app.get_context<crow::CookieParser>(app).set_cookie_max_age(3600);
    
    // Enable logging
    if (config.getBool("server.enable_logging", true)) {
        app.loglevel(crow::LogLevel::Info);
    }

    utils::Logger::getInstance().info("Application configured successfully");
}

int main() {
    try {
        // Set up signal handlers
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);

        std::cout << "Healthcare Booking System starting..." << std::endl;

        // Initialize application
        if (!initializeApplication()) {
            std::cerr << "Application initialization failed" << std::endl;
            return 1;
        }

        // Load configuration
        auto config_manager = std::make_unique<utils::ConfigManager>();
        config_manager->loadConfig("config/app.json");

        // Create Crow application with middleware
        app = std::make_unique<crow::Crow<crow::CookieParser, middleware::AuthMiddleware>>();

        // Configure middleware
        configureMiddleware(*app, *config_manager);

        // Configure application settings
        configureApp(*app, *config_manager);

        // Register routes
        registerRoutes(*app);

        // Start server
        int port = config_manager->getInt("server.port", 8080);
        std::string host = config_manager->getString("server.host", "0.0.0.0");

        std::cout << "Server starting on " << host << ":" << port << std::endl;
        utils::Logger::getInstance().info("Server starting on " + host + ":" + std::to_string(port));

        // Run the server
        app->run();

    } catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << std::endl;
        utils::Logger::getInstance().error("Application error: " + std::string(e.what()));
        return 1;
    }

    return 0;
}