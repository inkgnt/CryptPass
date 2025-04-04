#include "loginwidget.h"
#include "ui_loginwidget.h"
#include <QFile>

#include <QMessageBox>
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

    //далее я должен сравнить хэш введенного пароля с хэшем пароля из файла
    //функция извлекает соль хэш, в качестве параметра получает введенный пароль
    //генерит хэш с указанной солью из введенного пароля - сравнивает, если успех
    //генерит новую соль, из введенного пароля и новой соли новый хэш и сохраняет с перезаписью файла
    //получается можно создать одну функцию
    //которая принимает на вход старую соль, старый хэш и введенный пароль
}

