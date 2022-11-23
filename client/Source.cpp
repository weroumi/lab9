#include<Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <conio.h>

#define BUFSIZE 512
#define STDC_WANT_LIB_EXT11

int main(int argc, char* argv[]){
	
	// ��䳿 �������� � mainwindow.cpp
	// EVENT_MODIFY_STATE - ��� �������� ������ �� ��������� SetEvent()
	// SYNCHRONIZE - ��� ���� ��������� ��������� �� ����
	HANDLE ghWriteEvent = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, TEXT("WriteEvent"));
	HANDLE ghReadEvent = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, TEXT("ReadEvent"));

	if (ghWriteEvent == NULL || ghReadEvent == NULL){
		printf("OpenEvent failed (%d)\n", GetLastError());
		return -1;
	}


	char* dirFilter = argv[1];
	char* extencion = argv[2];
	strcat_s(dirFilter, 256, extencion);
	HANDLE hPipe;
	TCHAR lpvMessage[BUFSIZE] = L"Default message from client.";
	TCHAR  chBuf[BUFSIZE];
	BOOL   fSuccess = FALSE;
	DWORD  cbRead, cbToWrite, cbWritten, dwMode;
	TCHAR lpszPipename[BUFSIZE] = L"\\\\.\\Pipe\\myPipe";

	if(argc > 1)
		swprintf(lpvMessage, BUFSIZE, L"%hs", dirFilter);

	// ��������� ���, ������ �� �����, ���� ����� 

	while (1) {
		hPipe = CreateFile(
			lpszPipename,   // ��'� ��� 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE,
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // �������� 
			0,              // default attributes 
			NULL);          // no template file 

		// ����� � �����, ���� ��������� �� ���  

		if (hPipe != INVALID_HANDLE_VALUE)
			break;

		// Exit if an error other than ERROR_PIPE_BUSY occurs

		if (GetLastError() != ERROR_PIPE_BUSY){
			_tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
			return -1;
		}

		// ���� �� ���������� ����� ������ ������ 20 ��� 

		if (!WaitNamedPipe(lpszPipename, 20000)){
			printf("Could not open pipe: 20 second wait timed out.");
			return -1;
		}
	}

	// ���� ���������; ������������ ������ ������� � ���� 

	dwMode = PIPE_READMODE_MESSAGE;
	fSuccess = SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);    
	if (!fSuccess){
		_tprintf(TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError());
		return -1;
	}

	// ��������� ����� ������� �� �������� ������� 
	
	cbToWrite = (lstrlen(lpvMessage) + 1) * sizeof(TCHAR);
	_tprintf(TEXT("Sending %d byte message: \"%s\"\n"), cbToWrite, lpvMessage);

	fSuccess = WriteFile(hPipe, lpvMessage, cbToWrite, &cbWritten, NULL);                   

	if (!fSuccess){
		_tprintf(TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError());
		return -1;
	}

	// Set ghWriteEvent to signaled

	if (!SetEvent(ghWriteEvent)){
		printf("SetEvent failed (%d)\n", GetLastError());
		return -1;
	}

	printf("\nMessage sent to server, receiving reply as follows:\n");

	DWORD dwWaitResult = WaitForSingleObject(ghReadEvent, INFINITE);

	if (dwWaitResult != WAIT_OBJECT_0) {
		printf("Wait error (%d)\n", GetLastError());
		return -1;
	}

	do{
		// ���������� � ����� 
		fSuccess = ReadFile(hPipe, chBuf, BUFSIZE * sizeof(TCHAR), &cbRead, NULL);    
		if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
			break;

		_tprintf(TEXT("\"%s\"\n"), chBuf);
	} while (!fSuccess);  // repeat loop if ERROR_MORE_DATA 

	if (!fSuccess)
	{
		_tprintf(TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError());
		return -1;
	}

	printf("\n<End of message, press ENTER to terminate connection and exit>");
	_getch();

	CloseHandle(hPipe);
	return 0;
}