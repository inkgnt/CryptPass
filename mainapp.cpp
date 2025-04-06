#include "mainapp.h"
#include "ui_mainapp.h"
#include "crypto.h"
#include "keymanager.h"

MainApp::MainApp(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainApp)
{
    ui->setupUi(this);

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    registerWidget = new RegisterWidget;
    loginWidget = new LoginWidget;

    connect(registerWidget, &RegisterWidget::registerSuccess, this, &MainApp::onRegistrationComplete);
    connect(loginWidget, &LoginWidget::loginSuccess, this, &MainApp::onLoginSuccess);
    connect(&KeyManager::instance(), &KeyManager::keyCleared, this, &MainApp::onKeyCleared);

    stack->addWidget(registerWidget);
    stack->addWidget(loginWidget);

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
    mainWindowWidget = new MainWindowWidget;
    stack->addWidget(mainWindowWidget);
    stack->setCurrentWidget(mainWindowWidget);
}

void MainApp::onKeyCleared() {
    stack->setCurrentWidget(loginWidget);
}
