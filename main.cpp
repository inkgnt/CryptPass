#include <QFile>
#include <QApplication>

#include "regwindow.h"
#include "authwindow.h"
#include "mainwindow.h"


const QString HASH_FILE_PATH = "pHash.dat";

bool isHashFileExists()
{
    QFile file(HASH_FILE_PATH);
    return file.exists();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // The purpose of the following branching block is to check for the existence of the master password hash.
    // If it exists, the program proceeds directly to the authorization step.
    // If it does not exist, the user first completes the registration and then the authorization.
    if (!isHashFileExists())
    {
        RegWindow regDialog;
        if (regDialog.exec() == QDialog::Accepted)
        {
            AuthWindow authDialog;
            if (authDialog.exec() != QDialog::Accepted)
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        AuthWindow authDialog;
        if (authDialog.exec() != QDialog::Accepted)
        {
            return 0;
        }
    }

    // After successful authorization, an instance of the main window class is initialized and the main window is opened.
    MainWindow mainWindow;
    mainWindow.show();

    return a.exec();
}
