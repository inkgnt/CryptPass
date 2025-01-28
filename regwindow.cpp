#include <QMessageBox>
#include <QFile>
#include <openssl/evp.h>
#include <openssl/sha.h>

#include "regwindow.h"
#include "ui_regwindow.h"

RegWindow::RegWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegWindow)
{
    ui->setupUi(this);
}

RegWindow::~RegWindow()
{
    delete ui;
}

void RegWindow::on_pushButton_clicked()
{
    QString password = ui->lineEdit->text();
    QString confirmPassword = ui->lineEdit_2->text();


    if (password.isEmpty() || confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "All fields must be filled in!");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "Ошибка", "The passwords do not match!");
        return;
    }

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
    saveHashToFile(hash);
    accept();
}

void RegWindow::saveHashToFile(const unsigned char *hash) {
    QFile file("pHash.dat");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(reinterpret_cast<const char *>(hash), 32);
        file.close();
    }
}
