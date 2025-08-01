#include "../../include/utils/CryptoUtils.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <bcrypt/BCrypt.hpp>
#include <jwt-cpp/jwt.h>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>

namespace healthcare::utils {

// Static member initialization
std::string CryptoUtils::default_jwt_issuer_ = "healthcare-booking";
std::chrono::hours CryptoUtils::default_jwt_expiration_ = std::chrono::hours(24);
int CryptoUtils::password_hash_rounds_ = 10;

CryptoUtils::PasswordHashResult CryptoUtils::hashPassword(const std::string& password) {
    PasswordHashResult result;
    
    try {
        // Generate salt
        result.salt = BCrypt::generateSalt(password_hash_rounds_);
        
        // Hash password with salt
        result.hash = BCrypt::generateHash(password, result.salt);
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error = std::string("Password hashing failed: ") + e.what();
    }
    
    return result;
}

bool CryptoUtils::verifyPassword(const std::string& password, const std::string& hash, const std::string& salt) {
    try {
        return BCrypt::validatePassword(password, hash);
    } catch (const std::exception&) {
        return false;
    }
}

CryptoUtils::JwtResult CryptoUtils::generateJwtToken(const nlohmann::json& payload,
                                                    const std::string& secret,
                                                    std::chrono::hours expiration) {
    JwtResult result;
    
    try {
        auto now = std::chrono::system_clock::now();
        auto exp = now + expiration;
        
        auto token = jwt::create()
            .set_issuer(default_jwt_issuer_)
            .set_type("JWT")
            .set_issued_at(now)
            .set_expires_at(exp)
            .set_not_before(now);
        
        // Add custom claims from payload
        for (auto& [key, value] : payload.items()) {
            if (value.is_string()) {
                token.set_payload_claim(key, jwt::claim(value.get<std::string>()));
            } else if (value.is_number_integer()) {
                token.set_payload_claim(key, jwt::claim(value.get<int>()));
            } else if (value.is_boolean()) {
                token.set_payload_claim(key, jwt::claim(value.get<bool>()));
            }
        }
        
        result.token = token.sign(jwt::algorithm::hs256{secret});
        result.expires_at = exp;
        result.valid = true;
        
    } catch (const std::exception& e) {
        result.valid = false;
        result.error = std::string("JWT generation failed: ") + e.what();
    }
    
    return result;
}

CryptoUtils::JwtResult CryptoUtils::verifyJwtToken(const std::string& token, const std::string& secret) {
    JwtResult result;
    
    try {
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret})
            .with_issuer(default_jwt_issuer_);
        
        auto decoded = jwt::decode(token);
        verifier.verify(decoded);
        
        // Extract claims
        for (auto& [key, value] : decoded.get_payload_claims()) {
            if (value.get_type() == jwt::json::type::string) {
                result.claims[key] = value.as_string();
            } else if (value.get_type() == jwt::json::type::integer) {
                result.claims[key] = std::to_string(value.as_int());
            } else if (value.get_type() == jwt::json::type::boolean) {
                result.claims[key] = value.as_bool() ? "true" : "false";
            }
        }
        
        result.expires_at = decoded.get_expires_at();
        result.valid = true;
        
    } catch (const jwt::token_verification_exception& e) {
        result.valid = false;
        result.error = std::string("JWT verification failed: ") + e.what();
    } catch (const std::exception& e) {
        result.valid = false;
        result.error = std::string("JWT parsing failed: ") + e.what();
    }
    
    return result;
}

CryptoUtils::AesResult CryptoUtils::encryptAES(const std::string& plaintext, const std::string& key) {
    AesResult result;
    
    try {
        // Ensure key is 32 bytes for AES-256
        std::string aes_key = key;
        aes_key.resize(32, '\0');
        
        // Generate random IV
        unsigned char iv[AES_BLOCK_SIZE];
        RAND_bytes(iv, AES_BLOCK_SIZE);
        result.iv = std::string(reinterpret_cast<char*>(iv), AES_BLOCK_SIZE);
        
        // Create cipher context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) throw std::runtime_error("Failed to create cipher context");
        
        // Initialize encryption
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                              reinterpret_cast<const unsigned char*>(aes_key.c_str()),
                              iv) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize encryption");
        }
        
        // Encrypt data
        std::vector<unsigned char> ciphertext(plaintext.length() + AES_BLOCK_SIZE);
        int len;
        int ciphertext_len;
        
        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                             reinterpret_cast<const unsigned char*>(plaintext.c_str()),
                             plaintext.length()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to encrypt data");
        }
        ciphertext_len = len;
        
        // Finalize encryption
        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to finalize encryption");
        }
        ciphertext_len += len;
        
        EVP_CIPHER_CTX_free(ctx);
        
        result.data = std::string(reinterpret_cast<char*>(ciphertext.data()), ciphertext_len);
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error = std::string("AES encryption failed: ") + e.what();
    }
    
    return result;
}

CryptoUtils::AesResult CryptoUtils::decryptAES(const std::string& ciphertext, const std::string& key, const std::string& iv) {
    AesResult result;
    
    try {
        // Ensure key is 32 bytes for AES-256
        std::string aes_key = key;
        aes_key.resize(32, '\0');
        
        // Create cipher context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) throw std::runtime_error("Failed to create cipher context");
        
        // Initialize decryption
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                              reinterpret_cast<const unsigned char*>(aes_key.c_str()),
                              reinterpret_cast<const unsigned char*>(iv.c_str())) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize decryption");
        }
        
        // Decrypt data
        std::vector<unsigned char> plaintext(ciphertext.length() + AES_BLOCK_SIZE);
        int len;
        int plaintext_len;
        
        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len,
                             reinterpret_cast<const unsigned char*>(ciphertext.c_str()),
                             ciphertext.length()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to decrypt data");
        }
        plaintext_len = len;
        
        // Finalize decryption
        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to finalize decryption");
        }
        plaintext_len += len;
        
        EVP_CIPHER_CTX_free(ctx);
        
        result.data = std::string(reinterpret_cast<char*>(plaintext.data()), plaintext_len);
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error = std::string("AES decryption failed: ") + e.what();
    }
    
    return result;
}

std::string CryptoUtils::sha256(const std::string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), hash);
    
    return bytesToHex(hash, SHA256_DIGEST_LENGTH);
}

std::string CryptoUtils::sha512(const std::string& data) {
    unsigned char hash[SHA512_DIGEST_LENGTH];
    SHA512(reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), hash);
    
    return bytesToHex(hash, SHA512_DIGEST_LENGTH);
}

std::string CryptoUtils::md5(const std::string& data) {
    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), hash);
    
    return bytesToHex(hash, MD5_DIGEST_LENGTH);
}

std::string CryptoUtils::hmacSha256(const std::string& data, const std::string& key) {
    unsigned char* digest = HMAC(EVP_sha256(), key.c_str(), key.length(),
                                reinterpret_cast<const unsigned char*>(data.c_str()),
                                data.length(), nullptr, nullptr);
    
    return bytesToHex(digest, SHA256_DIGEST_LENGTH);
}

std::string CryptoUtils::generateRandomString(int length, bool alphanumeric_only) {
    static const std::string alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static const std::string all_chars = alphanum + "!@#$%^&*()_+-=[]{}|;:,.<>?";
    
    const std::string& charset = alphanumeric_only ? alphanum : all_chars;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, charset.length() - 1);
    
    std::string result;
    result.reserve(length);
    
    for (int i = 0; i < length; ++i) {
        result += charset[dis(gen)];
    }
    
    return result;
}

std::string CryptoUtils::generateUUID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);
    
    std::stringstream ss;
    ss << std::hex;
    
    // Generate 128 random bits
    for (int i = 0; i < 8; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; i++) ss << dis(gen);
    ss << "-4"; // Version 4 UUID
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    ss << dis2(gen); // Variant
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; i++) ss << dis(gen);
    
    return ss.str();
}

bool CryptoUtils::secureCompare(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return false;
    }
    
    volatile unsigned char result = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        result |= a[i] ^ b[i];
    }
    
    return result == 0;
}

CryptoUtils::SignatureResult CryptoUtils::signDataRSA(const std::string& data, const std::string& private_key_pem) {
    SignatureResult result;
    
    try {
        // Create BIO for private key
        BIO* bio = BIO_new_mem_buf(private_key_pem.c_str(), -1);
        if (!bio) throw std::runtime_error("Failed to create BIO");
        
        // Read private key
        EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        
        if (!pkey) throw std::runtime_error("Failed to read private key");
        
        // Create signing context
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (!ctx) {
            EVP_PKEY_free(pkey);
            throw std::runtime_error("Failed to create signing context");
        }
        
        // Initialize signing
        if (EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) != 1) {
            EVP_MD_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            throw std::runtime_error("Failed to initialize signing");
        }
        
        // Update with data
        if (EVP_DigestSignUpdate(ctx, data.c_str(), data.length()) != 1) {
            EVP_MD_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            throw std::runtime_error("Failed to update signing");
        }
        
        // Get signature length
        size_t sig_len;
        if (EVP_DigestSignFinal(ctx, nullptr, &sig_len) != 1) {
            EVP_MD_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            throw std::runtime_error("Failed to get signature length");
        }
        
        // Sign data
        std::vector<unsigned char> signature(sig_len);
        if (EVP_DigestSignFinal(ctx, signature.data(), &sig_len) != 1) {
            EVP_MD_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            throw std::runtime_error("Failed to sign data");
        }
        
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        
        result.signature = base64Encode(std::string(reinterpret_cast<char*>(signature.data()), sig_len));
        result.algorithm = "RSA-SHA256";
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error = std::string("RSA signing failed: ") + e.what();
    }
    
    return result;
}

bool CryptoUtils::verifySignatureRSA(const std::string& data, const std::string& signature_base64, 
                                    const std::string& public_key_pem) {
    try {
        // Decode signature from base64
        std::string signature = base64Decode(signature_base64);
        
        // Create BIO for public key
        BIO* bio = BIO_new_mem_buf(public_key_pem.c_str(), -1);
        if (!bio) return false;
        
        // Read public key
        EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        
        if (!pkey) return false;
        
        // Create verification context
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (!ctx) {
            EVP_PKEY_free(pkey);
            return false;
        }
        
        // Initialize verification
        if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) != 1) {
            EVP_MD_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            return false;
        }
        
        // Update with data
        if (EVP_DigestVerifyUpdate(ctx, data.c_str(), data.length()) != 1) {
            EVP_MD_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            return false;
        }
        
        // Verify signature
        int result = EVP_DigestVerifyFinal(ctx, 
            reinterpret_cast<const unsigned char*>(signature.c_str()), 
            signature.length());
        
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        
        return result == 1;
        
    } catch (const std::exception&) {
        return false;
    }
}

std::string CryptoUtils::base64Encode(const std::string& data) {
    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    
    bio = BIO_push(b64, bio);
    BIO_write(bio, data.c_str(), data.length());
    BIO_flush(bio);
    
    BUF_MEM* buffer_ptr;
    BIO_get_mem_ptr(bio, &buffer_ptr);
    
    std::string result(buffer_ptr->data, buffer_ptr->length);
    BIO_free_all(bio);
    
    return result;
}

std::string CryptoUtils::base64Decode(const std::string& encoded) {
    BIO* bio = BIO_new_mem_buf(encoded.c_str(), -1);
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    
    bio = BIO_push(b64, bio);
    
    std::vector<char> buffer(encoded.length());
    int decoded_length = BIO_read(bio, buffer.data(), encoded.length());
    BIO_free_all(bio);
    
    return std::string(buffer.data(), decoded_length);
}

std::string CryptoUtils::hexEncode(const std::string& data) {
    return bytesToHex(reinterpret_cast<const unsigned char*>(data.c_str()), data.length());
}

std::string CryptoUtils::hexDecode(const std::string& hex) {
    std::string result;
    result.reserve(hex.length() / 2);
    
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byte = hex.substr(i, 2);
        result += static_cast<char>(std::stoi(byte, nullptr, 16));
    }
    
    return result;
}

std::string CryptoUtils::deriveKey(const std::string& password, const std::string& salt, int iterations, int key_length) {
    std::vector<unsigned char> derived_key(key_length);
    
    PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                      reinterpret_cast<const unsigned char*>(salt.c_str()), salt.length(),
                      iterations, EVP_sha256(), key_length, derived_key.data());
    
    return std::string(reinterpret_cast<char*>(derived_key.data()), key_length);
}

CryptoUtils::FileHashResult CryptoUtils::hashFile(const std::string& file_path, const std::string& algorithm) {
    FileHashResult result;
    
    try {
        std::ifstream file(file_path, std::ios::binary);
        if (!file) throw std::runtime_error("Failed to open file");
        
        const EVP_MD* md = nullptr;
        if (algorithm == "SHA256") md = EVP_sha256();
        else if (algorithm == "SHA512") md = EVP_sha512();
        else if (algorithm == "MD5") md = EVP_md5();
        else throw std::runtime_error("Unsupported algorithm");
        
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (!ctx) throw std::runtime_error("Failed to create hash context");
        
        if (EVP_DigestInit_ex(ctx, md, nullptr) != 1) {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize hash");
        }
        
        char buffer[8192];
        while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
            if (EVP_DigestUpdate(ctx, buffer, file.gcount()) != 1) {
                EVP_MD_CTX_free(ctx);
                throw std::runtime_error("Failed to update hash");
            }
        }
        
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hash_len;
        if (EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1) {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("Failed to finalize hash");
        }
        
        EVP_MD_CTX_free(ctx);
        
        result.hash = bytesToHex(hash, hash_len);
        result.algorithm = algorithm;
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error = std::string("File hashing failed: ") + e.what();
    }
    
    return result;
}

bool CryptoUtils::encryptFile(const std::string& input_path, const std::string& output_path, 
                             const std::string& key) {
    try {
        std::ifstream input(input_path, std::ios::binary);
        if (!input) return false;
        
        // Read file content
        std::string content((std::istreambuf_iterator<char>(input)),
                           std::istreambuf_iterator<char>());
        input.close();
        
        // Encrypt content
        auto encrypted = encryptAES(content, key);
        if (!encrypted.success) return false;
        
        // Write encrypted content
        std::ofstream output(output_path, std::ios::binary);
        if (!output) return false;
        
        // Write IV first
        output.write(encrypted.iv.c_str(), encrypted.iv.length());
        // Write encrypted data
        output.write(encrypted.data.c_str(), encrypted.data.length());
        output.close();
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

bool CryptoUtils::decryptFile(const std::string& input_path, const std::string& output_path, 
                             const std::string& key) {
    try {
        std::ifstream input(input_path, std::ios::binary);
        if (!input) return false;
        
        // Read IV
        char iv[AES_BLOCK_SIZE];
        input.read(iv, AES_BLOCK_SIZE);
        std::string iv_str(iv, AES_BLOCK_SIZE);
        
        // Read encrypted content
        std::string encrypted((std::istreambuf_iterator<char>(input)),
                             std::istreambuf_iterator<char>());
        input.close();
        
        // Decrypt content
        auto decrypted = decryptAES(encrypted, key, iv_str);
        if (!decrypted.success) return false;
        
        // Write decrypted content
        std::ofstream output(output_path, std::ios::binary);
        if (!output) return false;
        
        output.write(decrypted.data.c_str(), decrypted.data.length());
        output.close();
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

bool CryptoUtils::validateCertificate(const std::string& cert_pem) {
    try {
        BIO* bio = BIO_new_mem_buf(cert_pem.c_str(), -1);
        if (!bio) return false;
        
        X509* cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        
        if (!cert) return false;
        
        // Check if certificate is currently valid
        ASN1_TIME* not_before = X509_get_notBefore(cert);
        ASN1_TIME* not_after = X509_get_notAfter(cert);
        
        int day, sec;
        ASN1_TIME_diff(&day, &sec, not_before, nullptr);
        bool valid = (day > 0 || (day == 0 && sec > 0));
        
        ASN1_TIME_diff(&day, &sec, nullptr, not_after);
        valid = valid && (day > 0 || (day == 0 && sec > 0));
        
        X509_free(cert);
        return valid;
        
    } catch (const std::exception&) {
        return false;
    }
}

bool CryptoUtils::validatePrivateKey(const std::string& key_pem) {
    try {
        BIO* bio = BIO_new_mem_buf(key_pem.c_str(), -1);
        if (!bio) return false;
        
        EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        
        if (!pkey) return false;
        
        EVP_PKEY_free(pkey);
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

std::string CryptoUtils::generatePaymentHash(const nlohmann::json& payment_data, const std::string& secret) {
    // Create canonical string from payment data
    std::string canonical;
    
    // Sort keys for consistent ordering
    std::vector<std::string> keys;
    for (auto& [key, value] : payment_data.items()) {
        keys.push_back(key);
    }
    std::sort(keys.begin(), keys.end());
    
    // Build canonical string
    for (const auto& key : keys) {
        if (!canonical.empty()) canonical += "|";
        
        if (payment_data[key].is_string()) {
            canonical += payment_data[key].get<std::string>();
        } else {
            canonical += payment_data[key].dump();
        }
    }
    
    // Add secret
    canonical += "|" + secret;
    
    // Generate hash
    return sha512(canonical);
}

bool CryptoUtils::verifyPaymentSignature(const std::string& signature, const nlohmann::json& payment_data, 
                                        const std::string& secret) {
    std::string expected_signature = generatePaymentHash(payment_data, secret);
    return secureCompare(signature, expected_signature);
}

std::string CryptoUtils::generateRateLimitToken(const std::string& identifier, 
                                              const std::chrono::system_clock::time_point& window_start) {
    auto window_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        window_start.time_since_epoch()
    ).count();
    
    std::string data = identifier + ":" + std::to_string(window_ms);
    return sha256(data);
}

std::string CryptoUtils::maskSensitiveData(const std::string& data, int visible_chars) {
    if (data.length() <= visible_chars * 2) {
        return std::string(data.length(), '*');
    }
    
    std::string masked = data.substr(0, visible_chars);
    masked += std::string(data.length() - visible_chars * 2, '*');
    masked += data.substr(data.length() - visible_chars);
    
    return masked;
}

std::string CryptoUtils::bytesToHex(const unsigned char* bytes, size_t length) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<unsigned int>(bytes[i]);
    }
    
    return ss.str();
}

} // namespace healthcare::utils