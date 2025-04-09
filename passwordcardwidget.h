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


private:
    Ui::PasswordCardWidget *ui;

    PasswordRecord record;
    bool eventFilter(QObject *obj, QEvent *event);
};
