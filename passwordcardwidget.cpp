#include "passwordcardwidget.h"
#include "ui_passwordcardwidget.h"
#include <QMouseEvent>
PasswordCardWidget::PasswordCardWidget(PasswordRecord& record, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PasswordCardWidget)
    , record(record)
{
    ui->setupUi(this);

    ui->urlLabel->setText("🌐 " + record.url);
    ui->loginLabel->setText("👤 " + QString(8, QChar(0x25CF)));
    ui->passwordLabel->setText("🔒 " + QString(8, QChar(0x25CF)));

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
