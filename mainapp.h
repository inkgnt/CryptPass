#pragma once

#include <QMainWindow>
#include <QStackedWidget>

#include "registerwidget.h"
#include "loginwidget.h"
#include "mainwindowwidget.h"

namespace Ui {
class MainApp;
}

class MainApp : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainApp(QWidget *parent = nullptr);
    ~MainApp();

private slots:
    void onRegistrationComplete();
    void onLoginSuccess();

private:
    Ui::MainApp *ui;

    QStackedWidget *stack;
    RegisterWidget *registerWidget;
    LoginWidget *loginWidget;
    MainWindowWidget *mainWindowWidget;

    void setWidget();
};
