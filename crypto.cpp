#include "crypto.h"

#include <memory>

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

std::vector<unsigned char> encryptAES256(const std::vector<unsigned char>& plaintext, const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv)
{
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx(EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free); //added RAII

    if (!ctx) {
        return {};
    }

    std::vector<unsigned char> ciphertext(plaintext.size() + AES_BLOCK_SIZE);

    int len = 0;
    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        return {};
    }

    if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) {
        return {};
    }

    int ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + len, &len) != 1) {
        return {};
    }

    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);

    return ciphertext;
}

std::vector<unsigned char> decryptAES256(const std::vector<unsigned char>& ciphertext, const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv)
{
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx(EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free); //added RAII

    if (!ctx) {
        return {};
    }

    std::vector<unsigned char> plaintext(ciphertext.size());

    int len = 0;
    if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        return {};
    }

    if (EVP_DecryptUpdate(ctx.get(), plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) {
        return {};
    }

    int plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx.get(), plaintext.data() + len, &len) != 1) {
        return {};
    }

    plaintext_len += len;
    plaintext.resize(plaintext_len);

    return plaintext;
}


bool loadHashAndSaltFromFile(std::vector<unsigned char> &storedSalt, std::vector<unsigned char> &storedHash)
{
    QFile file(HASH_FILE_PATH);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QDataStream in(&file);
    QByteArray saltData, hashData;
    in >> saltData >> hashData;

    if (saltData.size() != SALT_LENGTH || hashData.size() != HASH_LENGTH) {
        return false;
    }

    storedSalt.assign(saltData.begin(), saltData.end());
    storedHash.assign(hashData.begin(), hashData.end());

    return true;
}

void saveHashAndSaltToFile(const std::vector<unsigned char> &salt, const std::vector<unsigned char> &hash)
{
    QFile file(HASH_FILE_PATH);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    QDataStream out(&file);
    out << QByteArray(reinterpret_cast<const char*>(salt.data()), salt.size());
    out << QByteArray(reinterpret_cast<const char*>(hash.data()), hash.size());
}

std::vector<unsigned char> generatePBKDF2Hash(const QString &password, const std::vector<unsigned char> &salt)
{
    std::vector<unsigned char> hash(HASH_LENGTH);

    int success = PKCS5_PBKDF2_HMAC(
        password.toUtf8().constData(),          // password
        password.toUtf8().length(),             // password length
        salt.data(),                            // salt
        salt.size(),                            // salt length
        ITERATIONS,                             // num iterations
        EVP_sha256(),                           // hash func
        HASH_LENGTH,                            // hash length
        hash.data()                             // out buffer
        );

    if (success != 1) {
        return {};
    }

    return hash;
}

std::vector<unsigned char> generateScryptKey(const QString &password, const std::vector<unsigned char> &salt)
{
    std::vector<unsigned char> key(HASH_LENGTH);

    if (salt.empty())
        return {};

    int success = EVP_PBE_scrypt(
        password.toUtf8().constData(),       // password
        password.toUtf8().length(),          // password length
        salt.data(),                         // salt
        salt.size(),                         // salt length
        SCRYPT_N,                            //
        SCRYPT_r,                            //
        SCRYPT_p,                            //
        SCRYPT_MAXMEM,                       //
        key.data(),                          // out buffer
        key.size()                           // key length
        );

    if (success != 1)
        return {};


    return key;
}



std::vector<unsigned char> generateSalt()
{
    std::vector<unsigned char> salt(SALT_LENGTH);

    if (RAND_bytes(salt.data(), SALT_LENGTH) != 1) {
        throw std::runtime_error("Error while generating IV with OpenSSL");
    }

    return salt;
}

std::vector<unsigned char> generateIV()
{
    std::vector<unsigned char> IV(AES_BLOCK_SIZE);
    if (RAND_bytes(IV.data(), AES_BLOCK_SIZE) != 1) {
        throw std::runtime_error("Error while generating IV with OpenSSL");
    }

    return IV;
}

