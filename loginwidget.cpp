#include "loginwidget.h"
#include "ui_loginwidget.h"
#include <QFile>

#include <QMessageBox>
#include "crypto.h"
#include "keymanager.h"

inline bool isDarkTheme(QWidget* w) {
    return w->palette().color(QPalette::Window).lightness() < 128;
}

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    ui->setupUi(this);

    QString show = isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark";

    ui->PSWDlineEdit->setEchoMode(QLineEdit::Password);
    toggleAction = new QAction(QIcon(show), "Show/Hide", ui->PSWDlineEdit);
    ui->PSWDlineEdit->addAction(toggleAction, QLineEdit::TrailingPosition);

    connect(toggleAction, &QAction::triggered, this, [this]() {
        isPasswordVisible = !isPasswordVisible;

        ui->PSWDlineEdit->setEchoMode(isPasswordVisible ? QLineEdit::Normal : QLineEdit::Password);
        toggleAction->setIcon(QIcon(isPasswordVisible
                                        ? isDarkTheme(this) ? ":/icons/light/icon_hide_light" : ":/icons/dark/icon_hide_dark"
                                        : isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark"));
    });

    connect(ui->LOGINbtn, &QPushButton::clicked, this, &LoginWidget::onLOGINbtnClicked);
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

void LoginWidget::onLOGINbtnClicked()
{
    QString password = ui->PSWDlineEdit->text();
    if (password.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Password cannot be empty!");
        return;
    }

    std::vector<uint8_t> storedSalt;
    std::vector<uint8_t> storedHash;

    loadHashAndSaltFromFile(storedSalt, storedHash);

    if (memcmp(generatePBKDF2Hash(password, storedSalt).data(), storedHash.data(), storedHash.size()) == 0)
    {
        KeyManager::instance().setKey(generateScryptKey(password, storedSalt));

        ui->PSWDlineEdit->clear();
        emit loginSuccess();
    }
    else
    {
        QMessageBox::warning(this, "Error", "Password is wrong!");
        ui->PSWDlineEdit->clear();
    }
}

void LoginWidget::onThemeChanged() {
    QString show = isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark";
    QString hide = isDarkTheme(this) ? ":/icons/light/icon_hide_light" : ":/icons/dark/icon_hide_dark";

    toggleAction->setIcon(QIcon(isPasswordVisible ? hide : show));
}
