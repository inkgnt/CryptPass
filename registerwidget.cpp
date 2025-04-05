#include "registerwidget.h"
#include "ui_registerwidget.h"

#include <QMessageBox>
#include "crypto.h"

RegisterWidget::RegisterWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RegisterWidget)
{
    ui->setupUi(this);
}

RegisterWidget::~RegisterWidget()
{
    delete ui;
}

void RegisterWidget::on_pushButton_clicked()
{
    QString password = ui->lineEdit->text();
    QString confirmPassword = ui->lineEdit_2->text();

    if (password.isEmpty() || confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "Error", "All fields must be filled in!");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "Error", "The passwords do not match!");
        return;
    }

    auto salt = generateSalt();
    auto hash = generatePBKDF2Hash(password, salt);

    saveHashAndSaltToFile(salt, hash);

    emit registerSuccess();
}

