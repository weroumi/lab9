//checker
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <map>
#include <QDir>
#include <QFileDialog>
#include <vector>

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

struct serverThredArgs{
    QString dirName;
    QString extencionFilter;
};

std::vector<serverThredArgs> params;

DWORD WINAPI serverThreadMain(LPVOID param){
    serverThredArgs* pParam = (serverThredArgs*) param;
    QDir directory = pParam->dirName;
    QStringList extencionFilter;
    extencionFilter << pParam->extencionFilter;
    QFileInfoList list =  directory.entryInfoList(extencionFilter, QDir::Files);
    QString dirInfo = "";
    int totalFilesSize = 0;
    for(int i = 0; i < list.size(); ++i){
        totalFilesSize += list[i].size();
        dirInfo += "\n" + list[i].fileName() + "\n\tCreated: " + list[i].birthTime().toString();
    }
    dirInfo = "Size: " + QString::number(totalFilesSize) + " bytes" + dirInfo;
    return 0;
}

void MainWindow::on_txtDir_1_selectionChanged()
{
    ui->txtDir_1->setText(QFileDialog::getExistingDirectory(this, "Select directory"));
}


void MainWindow::on_txtDir_2_selectionChanged()
{
    ui->txtDir_2->setText(QFileDialog::getExistingDirectory(this, "Select directory"));
}


void MainWindow::on_txtDir_3_selectionChanged()
{
    ui->txtDir_3->setText(QFileDialog::getExistingDirectory(this, "Select directory"));
}


void MainWindow::on_txtDir_4_selectionChanged()
{
    ui->txtDir_4->setText(QFileDialog::getExistingDirectory(this, "Select directory"));
}
