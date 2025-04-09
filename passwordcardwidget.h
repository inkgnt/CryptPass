#pragma once

#include <QWidget>
#include "databasemanager.h"

namespace Ui {
class PasswordCardWidget;
}

class PasswordCardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PasswordCardWidget(PasswordRecord& record, QWidget *parent = nullptr);

    ~PasswordCardWidget();

signals:
    void syncRequested();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::PasswordCardWidget *ui;

    PasswordRecord record;
    bool eventFilter(QObject *obj, QEvent *event);
};
