#include "mainwindowwidget.h"
#include "passwordcardwidget.h"
#include "ui_mainwindowwidget.h"

#include "keymanager.h"
#include "dialog.h"
#include "passwordform.h"

#include "databasemanager.h"

MainWindowWidget::MainWindowWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindowWidget)
{
    ui->setupUi(this);

    loadDataToList();
}

MainWindowWidget::~MainWindowWidget()
{
    delete ui;
}

void MainWindowWidget::on_pushButton_clicked()
{
    auto pw = new PasswordForm;
    auto d = new Dialog(pw, this);
    d->exec();

    loadDataToList();
}

void MainWindowWidget::on_pushButton_2_clicked()
{
    QListWidgetItem *selectedItem = ui->listWidget->currentItem();
    if (!selectedItem) return;

    int id = selectedItem->data(Qt::UserRole).toInt();

    if (DatabaseManager::instance().deleteRecord(id)) {
        int row = ui->listWidget->row(selectedItem);
        QListWidgetItem *item = ui->listWidget->takeItem(row);
        delete item;
    } else {
        qWarning() << "Failed to delete record with ID:" << id;
    }

    loadDataToList();
}

void MainWindowWidget::loadDataToList()
{
    ui->listWidget->clear();
    QList<PasswordRecord> records = DatabaseManager::instance().getAllRecords();

    for (PasswordRecord& record : records) {
        auto *widget = new PasswordCardWidget(record, this);
        auto *item = new QListWidgetItem(ui->listWidget);
        item->setData(Qt::UserRole, record.id);

        item->setSizeHint(widget->sizeHint());

        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, widget);
    }
}

