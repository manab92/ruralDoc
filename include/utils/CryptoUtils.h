#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <jwt-cpp/jwt.h>

namespace healthcare::utils {

struct HashResult {
    std::string hash;
    std::string salt;
    bool success;
    
    HashResult(bool success = false) : success(success) {}
    HashResult(const std::string& hash, const std::string& salt) 
        : hash(hash), salt(salt), success(true) {}
};

struct JwtPayload {
    std::string user_id;
    std::string role;
    std::string email;
    std::chrono::system_clock::time_point issued_at;
    std::chrono::system_clock::time_point expires_at;
    std::string issuer;
    std::vector<std::string> permissions;
    std::string session_id;
};

struct EncryptionResult {
    std::string encrypted_data;
    std::string iv;  // Initialization vector
    bool success;
    
    EncryptionResult(bool success = false) : success(success) {}
    EncryptionResult(const std::string& data, const std::string& iv)
        : encrypted_data(data), iv(iv), success(true) {}
};

class CryptoUtils {
public:
    // Password hashing and verification
    static HashResult hashPassword(const std::string& password);
    static HashResult hashPasswordWithSalt(const std::string& password, const std::string& salt);
    static bool verifyPassword(const std::string& password, const std::string& hash, const std::string& salt);
    static std::string generateSalt(size_t length = 32);
    
    // JWT token management
    static std::string generateJwtToken(const JwtPayload& payload, const std::string& secret);
    static JwtPayload verifyJwtToken(const std::string& token, const std::string& secret);
    static bool isJwtTokenValid(const std::string& token, const std::string& secret);
    static bool isJwtTokenExpired(const std::string& token);
    static std::string refreshJwtToken(const std::string& token, const std::string& secret, 
                                      std::chrono::hours new_duration = std::chrono::hours(24));
    
    // Encryption and decryption (AES-256-GCM)
    static EncryptionResult encrypt(const std::string& plaintext, const std::string& key);
    static std::string decrypt(const std::string& encrypted_data, const std::string& iv, const std::string& key);
    static std::string generateEncryptionKey(size_t length = 32);
    
    // Hashing utilities
    static std::string sha256(const std::string& data);
    static std::string sha512(const std::string& data);
    static std::string md5(const std::string& data);
    static std::string hmacSha256(const std::string& data, const std::string& key);
    
    // Random generation
    static std::string generateRandomString(size_t length, bool alphanumeric_only = true);
    static std::string generateUUID();
    static std::string generateSessionId();
    static std::string generateApiKey();
    static std::string generateVerificationCode(size_t length = 6);
    static int generateRandomNumber(int min, int max);
    
    // Secure comparison
    static bool secureCompare(const std::string& a, const std::string& b);
    static bool constantTimeCompare(const void* a, const void* b, size_t length);
    
    // Digital signatures (RSA)
    static std::string generateRSAKeyPair(int key_size = 2048);
    static std::string signData(const std::string& data, const std::string& private_key);
    static bool verifySignature(const std::string& data, const std::string& signature, const std::string& public_key);
    
    // Base64 encoding/decoding
    static std::string base64Encode(const std::string& data);
    static std::string base64Decode(const std::string& encoded_data);
    static std::string urlSafeBase64Encode(const std::string& data);
    static std::string urlSafeBase64Decode(const std::string& encoded_data);
    
    // Hex encoding/decoding
    static std::string hexEncode(const std::string& data);
    static std::string hexDecode(const std::string& hex_data);
    
    // Key derivation
    static std::string deriveKey(const std::string& password, const std::string& salt, 
                                int iterations = 100000, size_t key_length = 32);
    static std::string pbkdf2(const std::string& password, const std::string& salt,
                             int iterations, size_t key_length);
    
    // Secure file operations
    static bool encryptFile(const std::string& input_file, const std::string& output_file, 
                           const std::string& key);
    static bool decryptFile(const std::string& input_file, const std::string& output_file,
                           const std::string& key, const std::string& iv);
    
    // Certificate and key validation
    static bool isValidRSAPrivateKey(const std::string& private_key);
    static bool isValidRSAPublicKey(const std::string& public_key);
    static bool isValidX509Certificate(const std::string& certificate);
    
    // Secure random bytes
    static std::vector<unsigned char> generateRandomBytes(size_t length);
    static bool seedRandomGenerator();
    
    // Payment security
    static std::string generatePaymentSignature(const std::string& order_id, 
                                               const std::string& payment_id,
                                               const std::string& secret);
    static bool verifyPaymentSignature(const std::string& signature,
                                      const std::string& order_id,
                                      const std::string& payment_id,
                                      const std::string& secret);
    
    // Rate limiting and security tokens
    static std::string generateRateLimitToken(const std::string& user_id, 
                                             const std::string& action,
                                             std::chrono::seconds window = std::chrono::seconds(3600));
    static bool verifyRateLimitToken(const std::string& token, const std::string& secret);
    
    // Data masking for logs
    static std::string maskSensitiveData(const std::string& data, 
                                        const std::string& mask_char = "*",
                                        size_t visible_chars = 4);
    static std::string maskEmail(const std::string& email);
    static std::string maskPhoneNumber(const std::string& phone);
    static std::string maskCreditCard(const std::string& card_number);
    
    // Configuration
    static void setJwtDefaultIssuer(const std::string& issuer) { default_jwt_issuer_ = issuer; }
    static void setJwtDefaultExpiration(std::chrono::hours duration) { default_jwt_expiration_ = duration; }
    static void setPasswordHashRounds(int rounds) { password_hash_rounds_ = rounds; }

private:
    // Configuration
    static std::string default_jwt_issuer_;
    static std::chrono::hours default_jwt_expiration_;
    static int password_hash_rounds_;
    
    // Helper methods
    static std::string bytesToHex(const unsigned char* bytes, size_t length);
    static std::vector<unsigned char> hexToBytes(const std::string& hex);
    static std::string opensslErrorString();
    static bool initializeOpenSSL();
    static void cleanupOpenSSL();
    
    // OpenSSL context management
    static std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> createCipherContext();
    static std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> createDigestContext();
    
    // Character sets for random generation
    static const std::string alphanumeric_chars_;
    static const std::string alpha_chars_;
    static const std::string numeric_chars_;
    static const std::string special_chars_;
};

// Convenience macros for common crypto operations
#define HASH_PASSWORD(password) healthcare::utils::CryptoUtils::hashPassword(password)
#define VERIFY_PASSWORD(password, hash, salt) healthcare::utils::CryptoUtils::verifyPassword(password, hash, salt)
#define GENERATE_JWT(payload, secret) healthcare::utils::CryptoUtils::generateJwtToken(payload, secret)
#define VERIFY_JWT(token, secret) healthcare::utils::CryptoUtils::verifyJwtToken(token, secret)
#define GENERATE_UUID() healthcare::utils::CryptoUtils::generateUUID()
#define GENERATE_API_KEY() healthcare::utils::CryptoUtils::generateApiKey()
#define SHA256(data) healthcare::utils::CryptoUtils::sha256(data)
#define MASK_EMAIL(email) healthcare::utils::CryptoUtils::maskEmail(email)

} // namespace healthcare::utils