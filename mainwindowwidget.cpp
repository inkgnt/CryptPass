#include "mainwindowwidget.h"
#include "ui_mainwindowwidget.h"

#include "keymanager.h"
#include "databasemanager.h"

MainWindowWidget::MainWindowWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindowWidget)
{
    ui->setupUi(this);

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

