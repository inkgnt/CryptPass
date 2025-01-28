#ifndef CRYPTO_H
#define CRYPTO_H

#include <QByteArray>

QByteArray encryptAES256(const QByteArray& plaintext, const unsigned char* key, const unsigned char* iv);
QByteArray decryptAES256(const QByteArray& ciphertext, const unsigned char* key, const unsigned char* iv);

#endif // CRYPT_H
