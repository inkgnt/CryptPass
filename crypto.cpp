#include "crypto.h"

#include <memory>

#include <openssl/evp.h>
#include <openssl/aes.h>

std::vector<unsigned char> encryptAES256(const std::vector<unsigned char>& plaintext, const unsigned char* key, const unsigned char* iv)
{
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx(EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free); //added RAII

    if (!ctx) {
        return {};
    }

    std::vector<unsigned char> ciphertext(plaintext.size() + AES_BLOCK_SIZE);

    int len = 0;
    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, key, iv) != 1) {
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

std::vector<unsigned char> decryptAES256(const std::vector<unsigned char>& ciphertext, const unsigned char* key, const unsigned char* iv)
{
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx(EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free); //added RAII

    if (!ctx) {
        return {};
    }

    std::vector<unsigned char> plaintext(ciphertext.size());

    int len = 0;
    if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, key, iv) != 1) {
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
