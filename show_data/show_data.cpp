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

struct subMessage {
public:
	int beginPosition;
	int endPosition;

	subMessage(int begin, int end) {
		beginPosition = begin;
		endPosition = end;
	}
};

void incrementDataMessagePosition() {
	dataMessage += sizeof(char) * DATA_MESSAGE_SIZE;
}

void setDataMessage() {
	dataMessage = (char*)MapViewOfFile(
		hSharedMemory,
		FILE_MAP_WRITE,		// Direitos de acesso: leitura e escrita
		0,					// dwOffsetHigh
		0,					// dwOffset Low
		DATA_MESSAGE_SIZE * CIRCULAR_DISK_MAX_MESSAGES);			// N�mero de bytes a serem mapeados
}

void printDataOnScreen() {
	subMessage nseqPosition(0,5);
	subMessage tagPosition(10, 17);
	subMessage valorPosition(20, 27);
	subMessage uePosition(29, 36);
	subMessage modoPosition(38, 38);
	subMessage hourPosition(40, 51);

	printf("NSEQ: ");
	for (int i = nseqPosition.beginPosition; i <= nseqPosition.endPosition; i++) {
		printf("%c", dataMessage[i]);
	}
	printf(" HORA: ");
	for (int i = hourPosition.beginPosition; i <= hourPosition.endPosition; i++) {
		printf("%c", dataMessage[i]);
	}
	printf(" TAG: ");
	for (int i = tagPosition.beginPosition; i <= tagPosition.endPosition; i++) {
		printf("%c", dataMessage[i]);
	}
	printf(" VALOR: ");
	for (int i = valorPosition.beginPosition; i <= valorPosition.endPosition; i++) {
		printf("%c", dataMessage[i]);
	}
	printf(" UE: ");
	for (int i = uePosition.beginPosition; i <= uePosition.endPosition; i++) {
		printf("%c", dataMessage[i]);
	}
	printf(" MODO: ");
	for (int i = modoPosition.beginPosition; i <= modoPosition.endPosition; i++) {
		printf("%c", dataMessage[i]);
	}
	printf("\n");
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
	int actualPosition = 0;

	hSharedMemory = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		FALSE,				// Handle herd�vel
		L"Data messages Shared Memory");			// lpName
	CheckForError(hSharedMemory);

	setDataMessage();
	

	do {
		ret = WaitForMultipleObjects(2, Events, FALSE, 0);
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
			break;
		case 1:
			if (WaitForSingleObject(hReceiveData, 0) == WAIT_OBJECT_0) {
				if (dataMessage != "") {
					printDataOnScreen();
				}
				else {
					printf("sem mensagens na fila\n");
				}
				ReleaseSemaphore(hSendData, 1, NULL);
				if (actualPosition < CIRCULAR_DISK_MAX_MESSAGES) {
					actualPosition++;
					incrementDataMessagePosition();
				}
				else {
					actualPosition = 0;
					setDataMessage();
				}
			}
			break;
		case -1:
			printf("---- Tarefa de exibicao de dados bloqueada! ----\n\n");
			WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			printf("---- Tarefa de exibicao de dados desbloqueada! ----\n\n");
			break;
		}
	} while (!exit);

	printf("Thread O terminando...\n");
	CloseHandle(hOEvent);
	CloseHandle(hEscEvent);
	CloseHandle(hSendData);
	CloseHandle(hReceiveData);
	return 0;
}