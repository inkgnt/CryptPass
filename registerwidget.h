#pragma once

#include <QWidget>

namespace Ui {
class RegisterWidget;
}

class RegisterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWidget(QWidget *parent = nullptr);
    ~RegisterWidget();

signals:
    void registerSuccess();

private slots:
    void onSUBMITbtnClicked();

private:
    Ui::RegisterWidget *ui;
};

