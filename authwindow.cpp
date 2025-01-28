#include <QMessageBox>
#include <QFile>
#include <openssl/evp.h>
#include <openssl/sha.h>

#include "authwindow.h"
#include "ui_authwindow.h"

unsigned char AuthWindow::key[32];
AuthWindow::AuthWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AuthWindow)
{
    ui->setupUi(this);
}

AuthWindow::~AuthWindow()
{
    delete ui;
}

void AuthWindow::on_pushButton_clicked()
{
    QString password = ui->lineEdit->text();
    if (password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Password cannot be empty!");
        return;
    }

    unsigned char savedHash[32];
    QFile file("pHash.dat");
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    file.read(reinterpret_cast<char *>(savedHash), 32);
    file.close();


    QByteArray reversedPassword = password.toUtf8();
    std::reverse(reversedPassword.begin(), reversedPassword.end());

    // Code for reuse:
    unsigned char sha256Hash[SHA256_DIGEST_LENGTH];
    if (SHA256(reinterpret_cast<const unsigned char*>(reversedPassword.constData()), reversedPassword.length(), sha256Hash) == NULL) {
        return;
    }

    unsigned char salt[16];
    memcpy(salt, sha256Hash, 16);

    unsigned char hash[32];
    if (!PKCS5_PBKDF2_HMAC(password.toUtf8().constData(), password.length(),
                           salt, sizeof(salt), 100000, EVP_sha256(),
                           sizeof(hash), hash)) {
        return;
    }
    //

    if (memcmp(savedHash, hash, 32) == 0) {
        QMessageBox::information(this, "Authorization", "Authorization successful!");

        if (!EVP_PBE_scrypt(password.toUtf8().constData(), password.length(), salt, sizeof(salt),
                            16384, 8, 1, 32*1024*1024 , key, 32)) {
            QMessageBox::warning(this, "Error", "Enexpexted error!");
            return;
        }

        accept();
    } else {
        ui->lineEdit->clear();
        QMessageBox::warning(this, "Error", "Wrong password!");
    }
}

