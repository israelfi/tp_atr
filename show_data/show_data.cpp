#include <stdio.h>
#include <iostream>
#include <string>
#include <windows.h>
#include "CheckForError.h"
using namespace std;

#define CIRCULAR_DISK_MAX_MESSAGES 200
#define DATA_MESSAGE_SIZE 53

char* dataMessage;

HANDLE hSharedMemory;

void getData() {
	dataMessage = (char*)MapViewOfFile(
		hSharedMemory,
		FILE_MAP_WRITE,		// Direitos de acesso: leitura e escrita
		0,					// dwOffsetHigh
		0,					// dwOffset Low
		DATA_MESSAGE_SIZE * CIRCULAR_DISK_MAX_MESSAGES);			// N�mero de bytes a serem mapeados
}

int main() {
	HANDLE hOEvent;
	HANDLE hEscEvent;
	HANDLE hSendData;
	HANDLE hReceiveData;

	hOEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"EventoO");
	CheckForError(hOEvent);
	hEscEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"EventoEsc");
	if (hEscEvent == NULL) {
		printf("ERROR ON OPEN HESC EVENT");
	}
	CheckForError(hEscEvent);
	hSendData = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, L"Data sent to shared memory");
	CheckForError(hSendData);
	hReceiveData = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, L"Data received from shared memory");
	CheckForError(hReceiveData);


	HANDLE Events[2] = { hEscEvent, hOEvent };
	DWORD ret;
	int nTipoEvento;
	bool show_once = TRUE;
	bool exit = false;

	hSharedMemory = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		FALSE,				// Handle herd�vel
		L"Data messages Shared Memory");			// lpName
	CheckForError(hSharedMemory);

	do {
		ret = WaitForMultipleObjects(2, Events, FALSE, 1);
		if (ret == WAIT_TIMEOUT) {
			nTipoEvento = -1;
		}
		else {
			nTipoEvento = ret - WAIT_OBJECT_0;
		}

		switch (nTipoEvento)
		{
		case 0:
			exit = true;
		case 1:
			if (WaitForSingleObject(hReceiveData, 0) == WAIT_OBJECT_0) {
				getData();
				if (dataMessage != "") {
					printf("Mensagem de dado ---- %s\n", dataMessage);
				}
				else {
					printf("sem mensagens na fila\n");
					;
				}
				ReleaseSemaphore(hSendData, 1, NULL);
			}
		}
	} while (!exit);

	printf("Thread O terminando...\n");
	return 0;
}