#ifndef REGWINDOW_H
#define REGWINDOW_H

#include <QDialog>

namespace Ui {
class RegWindow;
}

class RegWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RegWindow(QWidget *parent = nullptr);
    ~RegWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::RegWindow *ui;

    void saveHashToFile(const unsigned char *hash);
};

#endif // REGWINDOW_H
