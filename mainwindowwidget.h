#pragma once

#include <QWidget>

namespace Ui {
class MainWindowWidget;
}

class MainWindowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindowWidget(QWidget *parent = nullptr);
    ~MainWindowWidget();

signals:
    void lockRequested();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void onSyncRequested();

    void on_lineEdit_textChanged(const QString &arg1);

    void on_pushButton_4_clicked();

private:
    Ui::MainWindowWidget *ui;

    void loadDataToList(const QString& filter);
};
