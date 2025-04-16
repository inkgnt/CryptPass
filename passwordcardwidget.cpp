#include "passwordcardwidget.h"
#include "ui_passwordcardwidget.h"
#include <QMouseEvent>
#include "crypto.h"
#include "keymanager.h"
#include <QListWidget>

PasswordCardWidget::PasswordCardWidget(PasswordRecord& record, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PasswordCardWidget)
    , record(record)
{
    ui->setupUi(this);
    ui->pushButton->setStyleSheet("QPushButton { color: palette(window-text); }");
    ui->urlLabel->setText("🌐: " + record.url);
    ui->loginLabel->setText("👤: " + QString(8, QChar(0x25CF)));
    ui->passwordLabel->setText("🔒: " + QString(8, QChar(0x25CF)));

    ui->urlLabel->installEventFilter(this);
    ui->loginLabel->installEventFilter(this);
    ui->passwordLabel->installEventFilter(this);
}

PasswordCardWidget::~PasswordCardWidget()
{
    delete ui;
}

bool PasswordCardWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {

        if (obj == ui->urlLabel || obj == ui->loginLabel || obj == ui->passwordLabel) {
            QWidget::mousePressEvent(static_cast<QMouseEvent *>(event));
            return false;
        }
    }

    return QWidget::eventFilter(obj, event);
}

void PasswordCardWidget::on_pushButton_clicked()
{

    if (flag) {
        std::vector<unsigned char> ivLogin; ivLogin.assign(record.login.begin(), record.login.begin() + 16);
        std::vector<unsigned char> ivPass; ivPass.assign(record.password.begin(), record.password.begin() + 16);
        std::vector<unsigned char> cipherLogin; cipherLogin.assign(record.login.begin() + 16, record.login.end());
        std::vector<unsigned char> cipherPass; cipherPass.assign(record.password.begin() + 16, record.password.end());

        auto plainLogin = decryptAES256(cipherLogin, KeyManager::instance().getKey(), ivLogin);
        auto plainPass = decryptAES256(cipherPass, KeyManager::instance().getKey(), ivPass);

        QString loginString = QString::fromUtf8(reinterpret_cast<const char*>(plainLogin.data()), plainLogin.size());
        QString passString = QString::fromUtf8(reinterpret_cast<const char*>(plainPass.data()), plainPass.size());

        ui->loginLabel->setText("👤: " + loginString);
        ui->passwordLabel->setText("🔒: " + passString);
        flag = false;
    }
    else {
        ui->loginLabel->setText("👤: " + QString(8, QChar(0x25CF)));
        ui->passwordLabel->setText("🔒: " + QString(8, QChar(0x25CF)));
        flag = true;
    }
}

QListWidget* findListWidget(QWidget* start) {
    QWidget *p = start;
    while (p) {
        if (auto lw = qobject_cast<QListWidget*>(p))
            return lw;
        p = p->parentWidget();
    }
    return nullptr;
}

void PasswordCardWidget::on_pushButton_2_clicked()
{
    QListWidget *listWidget = findListWidget(this);
    if (!listWidget)
    {
        return;
    }

    QListWidgetItem *foundItem = nullptr;

    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem *item = listWidget->item(i);
        if (listWidget->itemWidget(item) == this) {
            foundItem = item;
            break;
        }
    }

    if (foundItem) {
        int row = listWidget->row(foundItem);
        listWidget->takeItem(row);
        delete foundItem;

        DatabaseManager::instance().deleteRecord(record.id);
    }

    emit syncRequested();
}


