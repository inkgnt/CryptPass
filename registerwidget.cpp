#include "registerwidget.h"
#include "ui_registerwidget.h"

#include <QMessageBox>
#include "crypto.h"

inline bool isDarkTheme(QWidget* w) {
    return w->palette().color(QPalette::Window).lightness() < 128;
}

RegisterWidget::RegisterWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RegisterWidget)
{
    ui->setupUi(this);
    ui->PSWDlineEdit->setEchoMode(QLineEdit::Password);
    ui->PSWDREPEATlineEdit->setEchoMode(QLineEdit::Password);

    connect(ui->SUBMITbtn, &QPushButton::clicked, this, &RegisterWidget::onSUBMITbtnClicked);
}

RegisterWidget::~RegisterWidget()
{
    delete ui;
}

void RegisterWidget::onSUBMITbtnClicked()
{
    QString password = ui->PSWDlineEdit->text();
    QString confirmPassword = ui->PSWDREPEATlineEdit->text();

    if (password.isEmpty() || confirmPassword.isEmpty())
    {
        QMessageBox::warning(this, "Error", "All fields must be filled in!");
        return;
    }

    if (password != confirmPassword)
    {
        QMessageBox::warning(this, "Error", "The passwords do not match!");
        return;
    }

    auto salt = generateSalt();
    auto hash = generatePBKDF2Hash(password, salt);

    saveHashAndSaltToFile(salt, hash);

    ui->PSWDlineEdit->clear();
    ui->PSWDREPEATlineEdit->clear();

    emit registerSuccess();
}

