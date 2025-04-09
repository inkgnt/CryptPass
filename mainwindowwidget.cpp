#include "mainwindowwidget.h"
#include "passwordcardwidget.h"
#include "ui_mainwindowwidget.h"

#include "dialog.h"
#include "passwordform.h"

#include "databasemanager.h"

MainWindowWidget::MainWindowWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindowWidget)
{
    ui->setupUi(this);

    ui->lineEdit->addAction(QIcon(":/icons/icons/search_30dp_FFFFFF_FILL0_wght400_GRAD0_opsz24.svg"), QLineEdit::LeadingPosition);
    loadDataToList("");
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

    loadDataToList("");
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

    loadDataToList("");
}

void MainWindowWidget::loadDataToList(const QString &filter)
{
    ui->listWidget->clear();
    QList<PasswordRecord> records = DatabaseManager::instance().getAllRecords();

    for (PasswordRecord &record : records) {
        if (!(filter.isEmpty() || record.url.contains(filter, Qt::CaseInsensitive)))
            continue;

        auto *widget = new PasswordCardWidget(record, this);
        connect(widget, &PasswordCardWidget::syncRequested, this, &MainWindowWidget::onSyncRequested);

        auto *item = new QListWidgetItem(ui->listWidget);
        item->setData(Qt::UserRole, record.id);
        item->setSizeHint(widget->sizeHint());

        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, widget);
    }
}



void MainWindowWidget::onSyncRequested() {
    loadDataToList("");
}

void MainWindowWidget::on_lineEdit_textChanged(const QString &filter)
{
    loadDataToList(filter);
}

