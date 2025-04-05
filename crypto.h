#pragma once

#include <vector>
#include <QByteArray>
#include <QString>
#include <QFile>

const int SALT_LENGTH = 16;
const int HASH_LENGTH = 32;
const int ITERATIONS = 10000;
const QString HASH_FILE_PATH = "pHash.dat";

inline bool isHashFileExists() {
    QFile file(HASH_FILE_PATH);
    return file.exists();
}

std::vector<unsigned char> encryptAES256(const std::vector<unsigned char>& plaintext, const unsigned char* key, const unsigned char* iv);
std::vector<unsigned char> decryptAES256(const std::vector<unsigned char>& ciphertext, const unsigned char* key, const unsigned char* iv);

bool loadHashAndSaltFromFile(std::vector<unsigned char> &storedSalt, std::vector<unsigned char> &storedHash);
void saveHashAndSaltToFile(const std::vector<unsigned char> &Salt, const std::vector<unsigned char> &Hash);

std::vector<unsigned char> generatePBKDF2Hash(const QString &password, const std::vector<unsigned char> &salt);
std::vector<unsigned char> generateSalt();
