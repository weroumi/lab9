//checker
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <map>

std::map<QString, QString> cache;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->gridLayoutWidget->setStyleSheet("background-color: rgb(255, 255)");
    txtDirectiories[0] = ui->txtDir_1;
    txtDirectiories[1] = ui->txtDir_2;
    txtDirectiories[2] = ui->txtDir_3;
    txtDirectiories[3] = ui->txtDir_4;
    txtExtencions[0] = ui->txtExtencion_1;
    txtExtencions[1] = ui->txtExtencion_2;
    txtExtencions[2] = ui->txtExtencion_3;
    txtExtencions[3] = ui->txtExtencion_4;
    for(size_t i = 1; i < MAX_NUM_OF_CLIENTS; ++i){
        txtExtencions[i]->hide();
        txtDirectiories[i]->hide();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_cmbNumOfClients_currentTextChanged(const QString &arg1)
{
    for(size_t i = 1; i < MAX_NUM_OF_CLIENTS; ++i){
        txtExtencions[i]->hide();
        txtDirectiories[i]->hide();
    }
    numOfClients = ui->cmbNumOfClients->currentText().toInt();
    for(size_t i = 1; i < numOfClients; ++i){
        txtExtencions[i]->show();
        txtDirectiories[i]->show();
    }
}

DWORD WINAPI cacheThreadMain(LPVOID param){
    while(1){
        Sleep(5000);
        cache.clear();
    }
    return 0;
}
