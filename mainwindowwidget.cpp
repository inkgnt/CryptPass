#include "mainwindowwidget.h"
#include "ui_mainwindowwidget.h"

#include "keymanager.h"
#include "databasemanager.h"
#include <QSqlTableModel>

MainWindowWidget::MainWindowWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindowWidget)
{
    ui->setupUi(this);
    auto model = new QSqlTableModel(this, QSqlDatabase::database("passDB"));
    model->setTable("passwords");
    model->select();
    ui->tableView->setModel(model);
}

MainWindowWidget::~MainWindowWidget()
{
    delete ui;
}

void MainWindowWidget::on_pushButton_clicked()
{

}


void MainWindowWidget::on_pushButton_2_clicked()
{

}

