#include "loginwidget.h"
#include "ui_loginwidget.h"
#include <QFile>

#include <QMessageBox>
#include "crypto.h"
#include "keymanager.h"

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

auto toHexString = [](const std::vector<unsigned char>& data) -> QString {
    return QByteArray(reinterpret_cast<const char*>(data.data()), static_cast<int>(data.size())).toHex(' ');
};

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

        KeyManager::instance().setKey(generateScryptKey(password, storedSalt));

        ui->lineEdit->clear();
        emit loginSuccess();
    } else {
        QMessageBox::warning(this, "Error", "Password is wrong!");
        ui->lineEdit->clear();
    }
}

