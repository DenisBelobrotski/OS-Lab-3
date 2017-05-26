#include <fstream>
#include <windows.h>
#include "message.h"

using namespace std;

int main()
{
	char senderPath[] = "Sender.exe";
	char* lpszCommandLine = new char[500];
	char* containerFilePath = new char[260];
	int messagesNum;
	int processesNum;
	int command;
	bool isEmpty;
	Message message;
	Message* messages;

	STARTUPINFO* si;
	PROCESS_INFORMATION* pi;

	HANDLE hMutex;
	HANDLE hReadEvent;
	HANDLE hWriteEvent;
	HANDLE handles[2];

	printf("%s", "Enter name of container file:\n");
	scanf("%s", containerFilePath);
	printf("%s", "Enter number of messages:\n");
	scanf("%d", &messagesNum);

	ofstream fout(containerFilePath);
	sprintf(lpszCommandLine, "%s %s", senderPath, containerFilePath);
	fout << messagesNum;
	message.free = true;
	sprintf(message.name, "%s", "");
	sprintf(message.text, "%s", "");
	for (int i = 0; i < messagesNum; i++)
		fout.write((char*)&message, sizeof(Message));
	fout.close();

	printf("%s", "Enter number of processes Sender:\n");
	scanf("%d", &processesNum);

	si = new STARTUPINFO[processesNum];
	pi = new PROCESS_INFORMATION[processesNum];
	for (int i = 0; i < processesNum; i++)
	{
		ZeroMemory(&si[i], sizeof(STARTUPINFO));
		si[i].cb = sizeof(STARTUPINFO);
		CreateProcess(NULL, lpszCommandLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si[i], &pi[i]);
	}

	hMutex = CreateMutex(NULL, FALSE, "ReceiverMutex");

	while (true)
	{
		printf("Enter command code(1 - read, 0 - exit):\n");
		scanf("%d", &command);
		WaitForSingleObject(hMutex, INFINITE);

		if (command != 1) {
			break;
		}
		else
		{
			isEmpty = true;
			ifstream fin(containerFilePath);
			fin >> messagesNum;
			messages = new Message[messagesNum];
			for (int i = 0; i < messagesNum; i++)
			{
				fin.read((char*)&messages[i], sizeof(Message));
				if (!messages[i].free) {
					isEmpty = false;
				}
			}
			fin.close();
			if (isEmpty)
			{
				hWriteEvent = CreateEvent(NULL, NULL, FALSE, "Write");
				handles[0] = hWriteEvent;
				handles[1] = hMutex;
				ReleaseMutex(hMutex);
				WaitForMultipleObjects(2, handles, TRUE, INFINITE);
				ifstream fin(containerFilePath);
				fin >> messagesNum;
				for (int i = 0; i < messagesNum; i++)
				{
					fin.read((char*)&messages[i], sizeof(Message));
				}
				fin.close();
				CloseHandle(hWriteEvent);
			}
			for (int i = 0; i < messagesNum; i++)
			{
				if (!messages[i].free) {
					printf("Readed message:\n%s\n", messages[i].text);
					messages[i].free = true;

					hReadEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, "Read");
					SetEvent(hReadEvent);

					break;
				}
			}
			ofstream fout(containerFilePath);
			fout << messagesNum;
			for (int i = 0; i < messagesNum; i++)
			{
				fout.write((char*)&messages[i], sizeof(Message));
			}
			fout.close();
		}
		ReleaseMutex(hMutex);
	}

	for (int i = 0; i < processesNum; i++)
	{
		CloseHandle(pi[i].hThread);
		CloseHandle(pi[i].hProcess);
	}
	CloseHandle(hMutex);

	return 0;
}