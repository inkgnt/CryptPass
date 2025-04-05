#include "mainapp.h"
#include "ui_mainapp.h"
#include "crypto.h"

MainApp::MainApp(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainApp)
{
    ui->setupUi(this);

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    registerWidget = new RegisterWidget;
    loginWidget = new LoginWidget;
    mainWindowWidget = new MainWindowWidget;

    connect(registerWidget, &RegisterWidget::registerSuccess, this, &MainApp::onRegistrationComplete);
    connect(loginWidget, &LoginWidget::loginSuccess, this, &MainApp::onLoginSuccess);

    stack->addWidget(registerWidget);
    stack->addWidget(loginWidget);
    stack->addWidget(mainWindowWidget);

    setWidget();
}

MainApp::~MainApp()
{
    delete ui;
}

void MainApp::setWidget() {
    if (isHashFileExists()) {
        stack->setCurrentWidget(loginWidget);
    }
    else {
        stack->setCurrentWidget(registerWidget);
    }
}

void MainApp::onRegistrationComplete() {
    stack->setCurrentWidget(loginWidget);
}

void MainApp::onLoginSuccess() {
    stack->setCurrentWidget(mainWindowWidget);
}
