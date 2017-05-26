#include <fstream>
#include <windows.h>
#include "message.h"

using namespace std;

int main(int argc, char *argv[])
{
	Message* messages;
	int command;
	int messageNum;
	char name[10];
	char text[20];
	bool isFree;

	HANDLE hMutex;
	HANDLE hWriteEvent;
	HANDLE hReadEvent;
	HANDLE handles[2];

	hMutex = OpenMutex(SYNCHRONIZE, FALSE, "ReceiverMutex");

	printf("%s", "Enter your name (max 10 symbols):\n");
	scanf("%s", name);

	while (true)
	{
		printf("Enter command code(1 - write, 0 - exit):\n");
		scanf("%d", &command);

		WaitForSingleObject(hMutex, INFINITE);

		isFree = false;
		ifstream fin(argv[1]);
		fin >> messageNum;

		if (command != 1)
		{
			break;
		}
		else
		{
			messages = new Message[messageNum];
			printf("Enter message text (max 20 symbols):\n");
			scanf("%s", text);

			for (int i = 0; i < messageNum; i++)
			{
				fin.read((char*)&messages[i], sizeof(Message));
				if (messages[i].free)
				{
					isFree = true;
				}
			}
			fin.close();

			if (!isFree)
			{
				hReadEvent = CreateEvent(NULL, NULL, FALSE, "Read");
				handles[0] = hReadEvent;
				handles[1] = hMutex;
				ReleaseMutex(hMutex);
				WaitForMultipleObjects(2, handles, TRUE, INFINITE);
				ifstream fin(argv[1]);
				fin >> messageNum;
				for (int i = 0; i < messageNum; i++)
				{
					fin.read((char*)&messages[i], sizeof(Message));
				}
				fin.close();
				CloseHandle(hReadEvent);
			}

			for (int i = 0; i < messageNum; i++)
			{
				if (messages[i].free)
				{
					sprintf(messages[i].name, "%s", name);
					sprintf(messages[i].text, "%s", text);
					messages[i].free = false;

					hWriteEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, "Write");
					SetEvent(hWriteEvent);

					break;
				}
			}

			ofstream fout(argv[1]);
			fout << messageNum;
			for (int i = 0; i < messageNum; i++)
			{
				fout.write((char*)&messages[i], sizeof(Message));
			}
			fout.close();
		}
		ReleaseMutex(hMutex);
	}

	CloseHandle(hMutex);

	return 0;
}