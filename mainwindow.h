#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qlineedit.h"
#include <QMainWindow>
#include <Windows.h>
#define MAX_NUM_OF_CLIENTS 4

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_cmbNumOfClients_currentTextChanged(const QString &arg1);

    void on_txtDir_1_selectionChanged();

    void on_txtDir_2_selectionChanged();

    void on_txtDir_3_selectionChanged();

    void on_txtDir_4_selectionChanged();

    void on_btnSearch_clicked();

private:
    Ui::MainWindow *ui;
    QLineEdit* txtDirectiories[MAX_NUM_OF_CLIENTS];
    QLineEdit* txtExtencions[MAX_NUM_OF_CLIENTS];
    size_t numOfClients;
    std::vector<STARTUPINFO> si;
    std::vector<PROCESS_INFORMATION> pi;
};
#endif // MAINWINDOW_H
