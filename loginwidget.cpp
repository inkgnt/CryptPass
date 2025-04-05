#include "loginwidget.h"
#include "ui_loginwidget.h"
#include <QFile>

#include <QMessageBox>
#include "crypto.h"


LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    ui->setupUi(this);
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

void LoginWidget::on_pushButton_clicked()
{
    QString password = ui->lineEdit->text();
    if (password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Password cannot be empty!");
        return;
    }

    std::vector<unsigned char> storedSalt;
    std::vector<unsigned char> storedHash;

    loadHashAndSaltFromFile(storedSalt, storedHash);

    if (memcmp(generatePBKDF2Hash(password, storedSalt).data(), storedHash.data(), storedHash.size()) == 0) {
        auto newSalt = generateSalt();
        auto newHash = generatePBKDF2Hash(password, newSalt);

        saveHashAndSaltToFile(newSalt, newHash);

        emit loginSuccess();
    } else {
        QMessageBox::warning(this, "Error", "Password is wrong!");
        ui->lineEdit->text();
    }
}

