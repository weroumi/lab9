#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <map>
#include <QDir>
#include <QFileDialog>
#include <vector>
#include <QMessageBox>

#define BUFSIZE 512


std::map<QString, QString> cache;
HANDLE pipeServer;

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
    numOfClients = ui->cmbNumOfClients->currentText().toInt();
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

DWORD WINAPI InstanceThread(LPVOID lpvParam){
    HANDLE hHeap      = GetProcessHeap();
    TCHAR* pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(TCHAR));
    TCHAR* pchReply   = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(TCHAR));

    DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
    BOOL fSuccess = FALSE;
    HANDLE hPipe  = NULL;

    // Додаткові перевірочки, бо потік запуститься, навіть, якщо буде помилка з пайпами

    if (lpvParam == NULL)
    {
        QString sMsg = "ERROR - Pipe Server Failure: \n";
        sMsg += "   InstanceThread got an unexpected NULL value in lpvParam.\n";
        sMsg += "   InstanceThread exitting.\n";
        QMessageBox msg;
        msg.setText(sMsg);
        msg.exec();
        if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
        if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
        return (DWORD)-1;
    }

    if (pchRequest == NULL)
    {
        QString sMsg = "\nERROR - Pipe Server Failure:\n";
        sMsg += "   InstanceThread got an unexpected NULL heap allocation.\n";
        sMsg += "   InstanceThread exitting.\n";
        QMessageBox msg;
        msg.setText(sMsg);
        msg.exec();
        if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
        return (DWORD)-1;
    }

    if (pchReply == NULL)
    {
        QString sMsg = "\nERROR - Pipe Server Failure:\n";
        sMsg += "   InstanceThread got an unexpected NULL heap allocation.\n";
        sMsg += "   InstanceThread exitting.\n";
        QMessageBox msg;
        msg.setText(sMsg);
        msg.exec();
        if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
        return (DWORD)-1;
    }

    hPipe = (HANDLE) lpvParam;

    // Зациклуємось, доки не буде завершено читання
    while(1){
        // Читання запиту клієнта з пайпу. Максимальна довжина запиту = BUFSIZE (на разі 512).
        fSuccess = ReadFile(hPipe, pchRequest, BUFSIZE*sizeof(TCHAR), &cbBytesRead, NULL);
        if (!fSuccess || cbBytesRead == 0)
        {
            if (GetLastError() == ERROR_BROKEN_PIPE)
            {
                QMessageBox msg;
                QString sMsg = "InstanceThread: client disconnected.";
                msg.setText(sMsg);
                msg.exec();
            }
            else
            {
                QMessageBox msg;
                QString sMsg = "InstanceThread ReadFile failed, GLE=" + QString::number(GetLastError());
                msg.setText(sMsg);
                msg.exec();
            }
            break;
         }

        // Обробка запиту
        QString QDirFilter_temp = QString::fromStdWString(pchRequest);
        QStringList QDirFilter = QDirFilter_temp.split("*");
        QDirFilter[1] = "*" + QDirFilter[1];
        //ну тут короче у тебе свій код по пошуку а я просто шось присвою
        QDir directory = QDirFilter[0];
        QStringList extencionFilter;
        extencionFilter << QDirFilter[1];
        QFileInfoList list =  directory.entryInfoList(extencionFilter, QDir::Files);
        QString dirInfo = "";
        int totalFilesSize = 0;
        for(int i = 0; i < list.size(); ++i){
            totalFilesSize += list[i].size();
            dirInfo += "\n" + list[i].fileName() + "\n\tCreated: " + list[i].birthTime().toString();
        }
        dirInfo = "Size: " + QString::number(totalFilesSize) + " bytes" + dirInfo;
        dirInfo.toWCharArray(pchReply);
        cbReplyBytes = (lstrlen(pchReply)+1)*sizeof(TCHAR);

        // Надсилаємо відповідь клієнту через піпе
        fSuccess = WriteFile(hPipe, pchReply, cbReplyBytes, &cbWritten, NULL);

        if (!fSuccess || cbReplyBytes != cbWritten)
        {
            QMessageBox msg;
            QString sMsg = "InstanceThread ReadFile failed, GLE=" + QString::number(GetLastError());
            msg.setText(sMsg);
            msg.exec();
            break;
        }
    }

    // треба флаш буфер піпу, щоб усі дані були записані у файл і нормально зчитані клієнтом
    FlushFileBuffers(hPipe);

    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    HeapFree(hHeap, 0, pchRequest);
    HeapFree(hHeap, 0, pchReply);

    return 1;
}

DWORD WINAPI serverThreadMain(LPVOID param){
    BOOL   fConnected = FALSE;
    DWORD  dwThreadId = 0;
    HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;
    LPCTSTR lpszPipename = TEXT("\\\\.\\Pipe\\myPipe");

    // цикл створює екземпляр піпу і чекає коли підключиться клієнт
    // після підкл. клієнта створюємо потік що забезпечує комунікацію
    // таким чином, якщо буде кілька клієнтів - сервер зможе їх також "обслужити"
    for (;;)
    {
        hPipe = CreateNamedPipe(
         lpszPipename,             // ім'я піпу
         PIPE_ACCESS_DUPLEX,       // read/write access
         PIPE_TYPE_MESSAGE |       // message type pipe
         PIPE_READMODE_MESSAGE |   // message-read mode
         PIPE_WAIT,                // blocking mode
         PIPE_UNLIMITED_INSTANCES, // максимальна к-ть клієнтів
         BUFSIZE,                  // output buffer size
         BUFSIZE,                  // input buffer size
         0,                        // client time-out
         NULL);                    // default security attribute

        if (hPipe == INVALID_HANDLE_VALUE)
        {
            QMessageBox msg;
            QString sMsg = "CreateNamedPipe failed, GLE=" + QString::number(GetLastError());
            msg.setText(sMsg);
            msg.exec();
            return -1;
        }

        // чекаємо коли під'єднається клієнт
        fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (fConnected)
        {
        // створюємо потік спеціально для обробки запиту цього клієнта
            hThread = CreateThread(NULL, 0, InstanceThread, (LPVOID) hPipe, 0, &dwThreadId);
            if (hThread == NULL)
            {
                QMessageBox msg;
                QString sMsg = "CreateThread failed, GLE=" + QString::number(GetLastError());
                msg.setText(sMsg);
                msg.exec();
                return -1;
            }
            else CloseHandle(hThread);
         }
         else
         // Клієнт не може підключитись, вирубаємо пайп
         CloseHandle(hPipe);
    }
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

void MainWindow::on_btnSearch_clicked()
{
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&serverThreadMain, NULL, 0, 0);
    for(int i = 0; i < numOfClients; ++i){
        std::wstring dir = txtDirectiories[i]->text().toStdWString();
        std::wstring extencion = txtExtencions[i]->text().toStdWString();
        std::wstring path = L"C:\\Users\\Admin\\Documents\\Project2\\Debug\\client.exe";
        std::wstring args = path + L" " + dir + L" " + extencion;
        STARTUPINFO si_t;
        PROCESS_INFORMATION pi_t;
        si.push_back(si_t);
        pi.push_back(pi_t);
        ZeroMemory(&si[i], sizeof(si[i]));
        si[i].cb = sizeof(si[i]);
        ZeroMemory(&pi[i], sizeof(pi[i]));
        CreateProcess(NULL,&args[0],NULL,NULL,true,CREATE_NEW_CONSOLE,NULL,NULL,&si[i],&pi[i]);
    }
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
}

