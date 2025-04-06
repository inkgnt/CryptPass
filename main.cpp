#include <QFile>
#include <QApplication>
#include "mainapp.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainApp w;

    w.show();
    return a.exec();
}
