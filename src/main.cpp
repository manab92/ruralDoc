#include <iostream>
#include <memory>
#include <signal.h>
#include <csignal>
#include <crow.h>
#include <nlohmann/json.hpp>

// Utilities
#include "../include/utils/Logger.h"
#include "../include/utils/ConfigManager.h"
#include "../include/utils/ResponseHelper.h"

// Database
#include "../include/database/DatabaseManager.h"

// Middleware
#include "../include/middleware/AuthMiddleware.h"
#include "../include/middleware/LoggingMiddleware.h"
#include "../include/middleware/CorsMiddleware.h"

using namespace healthcare;

// Application singleton
class HealthcareApplication {
public:
    static HealthcareApplication& getInstance() {
        static HealthcareApplication instance;
        return instance;
    }

    bool initialize(const std::string& config_file = "config/app.json") {
        try {
            // Initialize configuration
            if (!utils::GlobalConfig::initialize(config_file)) {
                std::cerr << "Failed to load configuration from: " << config_file << std::endl;
                return false;
            }

            auto& config = utils::GlobalConfig::getInstance();
            
            // Initialize logger
            utils::Logger::getInstance().configure(
                config.getString("logging.level", "INFO"),
                config.getString("logging.file", "healthcare.log"),
                config.getBool("logging.console", true)
            );

            LOG_INFO("========================================");
            LOG_INFO("Healthcare Booking System Starting...");
            LOG_INFO("========================================");

            // Initialize database
            database::DatabaseConfig db_config;
            db_config.host = config.getString("database.host", "localhost");
            db_config.port = config.getInt("database.port", 5432);
            db_config.database = config.getString("database.name", "healthcare_db");
            db_config.username = config.getString("database.username", "postgres");
            db_config.password = config.getString("database.password", "");
            db_config.max_connections = config.getInt("database.max_connections", 10);
            db_config.connection_timeout_seconds = config.getInt("database.timeout", 30);

            database::RedisConfig redis_config;
            redis_config.host = config.getString("redis.host", "localhost");
            redis_config.port = config.getInt("redis.port", 6379);
            redis_config.password = config.getString("redis.password", "");
            redis_config.database = config.getInt("redis.database", 0);

            auto& db_manager = database::DatabaseManager::getInstance();
            db_manager.configure(db_config, redis_config);
            
            if (!db_manager.connect()) {
                LOG_ERROR("Failed to connect to database");
                return false;
            }

            LOG_INFO("Database connection established");

            // Run database migrations if configured
            if (config.getBool("database.auto_migrate", true)) {
                if (!db_manager.migrateDatabase()) {
                    LOG_ERROR("Database migration failed");
                    return false;
                }
                LOG_INFO("Database migration completed");
            }

            // Create Crow application with middleware
            app_ = std::make_unique<crow::App<
                middleware::LoggingMiddleware,
                middleware::CorsMiddleware,
                middleware::AuthMiddleware
            >>();

            // Configure middleware
            configureMiddleware();

            // Register routes
            registerRoutes();

            // Configure server settings
            configureServer();

            LOG_INFO("Application initialized successfully");
            return true;

        } catch (const std::exception& e) {
            std::cerr << "Failed to initialize application: " << e.what() << std::endl;
            LOG_ERROR("Application initialization failed: {}", e.what());
            return false;
        }
    }

    void run() {
        if (!app_) {
            throw std::runtime_error("Application not initialized");
        }

        auto& config = utils::GlobalConfig::getInstance();
        
        // Server configuration
        int port = config.getInt("server.port", 8080);
        std::string host = config.getString("server.host", "0.0.0.0");
        int threads = config.getInt("server.threads", std::thread::hardware_concurrency());

        LOG_INFO("Starting server on {}:{} with {} threads", host, port, threads);
        std::cout << "Healthcare Booking System running on " << host << ":" << port << std::endl;
        std::cout << "API Documentation: http://" << host << ":" << port << "/api/v1/docs" << std::endl;
        std::cout << "Health Check: http://" << host << ":" << port << "/api/v1/health" << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;

        try {
            app_->port(port)
                .bindaddr(host)
                .multithreaded(threads)
                .run();
        } catch (const std::exception& e) {
            LOG_ERROR("Server error: {}", e.what());
            throw;
        }
    }

    void shutdown() {
        LOG_INFO("Initiating graceful shutdown...");
        
        if (app_) {
            app_->stop();
        }

        // Disconnect from database
        try {
            database::DatabaseManager::getInstance().disconnect();
            LOG_INFO("Database disconnected");
        } catch (const std::exception& e) {
            LOG_ERROR("Error during database disconnect: {}", e.what());
        }

        LOG_INFO("Shutdown completed");
        utils::Logger::getInstance().flush();
    }

    bool isRunning() const {
        return app_ != nullptr;
    }

private:
    HealthcareApplication() = default;
    ~HealthcareApplication() {
        shutdown();
    }

    std::unique_ptr<crow::App<
        middleware::LoggingMiddleware,
        middleware::CorsMiddleware,
        middleware::AuthMiddleware
    >> app_;

    void configureMiddleware() {
        auto& config = utils::GlobalConfig::getInstance();

        // Configure logging middleware
        auto& logging_middleware = app_->get_middleware<middleware::LoggingMiddleware>();
        logging_middleware.setLogRequests(config.getBool("logging.requests", true));
        logging_middleware.setLogResponses(config.getBool("logging.responses", true));
        logging_middleware.setLogHeaders(config.getBool("logging.headers", false));
        logging_middleware.setLogBody(config.getBool("logging.body", false));
        logging_middleware.setSlowRequestThreshold(config.getDouble("logging.slow_threshold_ms", 1000.0));

        // Configure CORS middleware
        auto& cors_middleware = app_->get_middleware<middleware::CorsMiddleware>();
        cors_middleware.setAllowedOrigins(config.getStringArray("cors.allowed_origins"));
        cors_middleware.setAllowedMethods({"GET", "POST", "PUT", "DELETE", "OPTIONS", "PATCH"});
        cors_middleware.setAllowedHeaders({
            "Content-Type", "Authorization", "X-Requested-With", 
            "Accept", "Origin", "Cache-Control", "X-File-Name"
        });
        cors_middleware.setAllowCredentials(config.getBool("cors.allow_credentials", true));
        cors_middleware.setMaxAge(config.getInt("cors.max_age", 86400));

        // Configure authentication middleware
        auto& auth_middleware = app_->get_middleware<middleware::AuthMiddleware>();
        auth_middleware.setJwtSecret(config.getString("jwt.secret"));
        auth_middleware.setJwtIssuer(config.getString("jwt.issuer", "healthcare-booking"));
        auth_middleware.setTokenExpiryHours(config.getInt("jwt.expiry_hours", 24));

        // Configure public endpoints
        std::vector<std::string> public_endpoints = {
            "/api/v1/auth/register",
            "/api/v1/auth/login",
            "/api/v1/auth/forgot-password",
            "/api/v1/auth/reset-password",
            "/api/v1/auth/verify-email",
            "/api/v1/health",
            "/api/v1/docs",
            "/api/v1/doctors/search",
            "/api/v1/clinics/search"
        };

        for (const auto& endpoint : public_endpoints) {
            auth_middleware.addPublicEndpoint(endpoint);
        }

        // Configure admin endpoints
        std::vector<std::string> admin_endpoints = {
            "/api/v1/admin",
            "/api/v1/admin/users",
            "/api/v1/admin/doctors",
            "/api/v1/admin/statistics",
            "/api/v1/admin/system"
        };

        for (const auto& endpoint : admin_endpoints) {
            auth_middleware.addAdminEndpoint(endpoint);
        }

        LOG_INFO("Middleware configured successfully");
    }

    void registerRoutes() {
        // Health check endpoint
        CROW_ROUTE((*app_), "/api/v1/health")
        ([&](const crow::request& req) {
            try {
                nlohmann::json health_data;
                health_data["status"] = "healthy";
                health_data["timestamp"] = std::time(nullptr);
                health_data["version"] = "1.0.0";
                health_data["environment"] = utils::GlobalConfig::getInstance().getString("environment", "development");
                
                // Database health check
                auto& db_manager = database::DatabaseManager::getInstance();
                health_data["database"] = db_manager.getHealthStatus();
                
                // System info
                health_data["system"]["uptime"] = std::time(nullptr); // Simplified uptime
                health_data["system"]["memory"] = "N/A"; // Could add actual memory info
                
                bool is_healthy = db_manager.isConnected();
                
                return utils::ResponseHelper::healthCheck(health_data, is_healthy);
            } catch (const std::exception& e) {
                LOG_ERROR("Health check error: {}", e.what());
                nlohmann::json error_data;
                error_data["error"] = e.what();
                return utils::ResponseHelper::healthCheck(error_data, false);
            }
        });

        // API documentation endpoint
        CROW_ROUTE((*app_), "/api/v1/docs")
        ([](const crow::request& req) {
            std::string docs_html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Healthcare Booking System API</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        h1 { color: #2c3e50; }
        .endpoint { background: #f8f9fa; padding: 15px; margin: 10px 0; border-radius: 5px; }
        .method { font-weight: bold; color: #27ae60; }
        .path { font-family: monospace; color: #2980b9; }
    </style>
</head>
<body>
    <h1>Healthcare Booking System API Documentation</h1>
    <p>Welcome to the Healthcare Booking System API. This system provides endpoints for:</p>
    <ul>
        <li>User registration and authentication</li>
        <li>Doctor profile management</li>
        <li>Appointment booking and management</li>
        <li>Prescription management</li>
        <li>Payment processing</li>
        <li>Administrative functions</li>
    </ul>
    
    <h2>Quick Reference</h2>
    <div class="endpoint">
        <span class="method">GET</span> <span class="path">/api/v1/health</span> - System health check
    </div>
    <div class="endpoint">
        <span class="method">POST</span> <span class="path">/api/v1/auth/register</span> - User registration
    </div>
    <div class="endpoint">
        <span class="method">POST</span> <span class="path">/api/v1/auth/login</span> - User login
    </div>
    <div class="endpoint">
        <span class="method">GET</span> <span class="path">/api/v1/doctors/search</span> - Search doctors
    </div>
    
    <p>For complete API documentation, please refer to the docs/api.md file in the repository.</p>
    <p><strong>Base URL:</strong> /api/v1</p>
    <p><strong>Authentication:</strong> Bearer token required for most endpoints</p>
</body>
</html>
            )";
            
            return crow::response(200, docs_html);
        });

        // Root endpoint redirect
        CROW_ROUTE((*app_), "/")
        ([](const crow::request& req) {
            return crow::response(302, "", {{"Location", "/api/v1/docs"}});
        });

        // API info endpoint
        CROW_ROUTE((*app_), "/api/v1")
        ([](const crow::request& req) {
            nlohmann::json api_info;
            api_info["name"] = "Healthcare Booking System API";
            api_info["version"] = "1.0.0";
            api_info["description"] = "REST API for healthcare appointment booking and management";
            api_info["documentation"] = "/api/v1/docs";
            api_info["health_check"] = "/api/v1/health";
            
            return utils::ResponseHelper::success(api_info, "API information retrieved successfully");
        });

        // 404 handler
        CROW_CATCHALL_ROUTE((*app_))
        ([](const crow::request& req) {
            LOG_WARN("404 - Endpoint not found: {} {}", req.method_string(), req.url);
            return utils::ResponseHelper::notFound("Endpoint not found: " + std::string(req.method_string()) + " " + req.url);
        });

        LOG_INFO("Routes registered successfully");
    }

    void configureServer() {
        auto& config = utils::GlobalConfig::getInstance();

        // Server configuration
        app_->server_name("Healthcare Booking System v1.0.0");
        
        // Enable logging if configured
        if (config.getBool("server.enable_logging", true)) {
            app_->loglevel(crow::LogLevel::Info);
        }

        // Configure timeouts and limits
        app_->timeout(config.getInt("server.timeout", 30));
        
        LOG_INFO("Server configured successfully");
    }
};

// Global shutdown flag
volatile std::sig_atomic_t shutdown_requested = 0;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ". Shutting down..." << std::endl;
    shutdown_requested = 1;
    
    try {
        HealthcareApplication::getInstance().shutdown();
    } catch (const std::exception& e) {
        std::cerr << "Error during shutdown: " << e.what() << std::endl;
    }
    
    std::exit(signal);
}

int main(int argc, char* argv[]) {
    try {
        // Set up signal handlers for graceful shutdown
        std::signal(SIGINT, signalHandler);   // Ctrl+C
        std::signal(SIGTERM, signalHandler);  // Termination request
        
        // Parse command line arguments
        std::string config_file = "config/app.json";
        if (argc > 1) {
            config_file = argv[1];
        }

        std::cout << "Healthcare Booking System" << std::endl;
        std::cout << "=========================" << std::endl;
        std::cout << "Configuration: " << config_file << std::endl;

        // Initialize and run application
        auto& app = HealthcareApplication::getInstance();
        
        if (!app.initialize(config_file)) {
            std::cerr << "Failed to initialize application" << std::endl;
            return 1;
        }

        // Run the application
        app.run();

    } catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << std::endl;
        LOG_ERROR("Application error: {}", e.what());
        return 1;
    } catch (...) {
        std::cerr << "Unknown application error occurred" << std::endl;
        LOG_ERROR("Unknown application error occurred");
        return 1;
    }

    return 0;
}