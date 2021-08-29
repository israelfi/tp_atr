#include <stdio.h>
#include <iostream>
#include <string>
#include <windows.h>

#include "CheckForError.h"

#define ALARM_MESSAGE_SIZE 31


int main() {
	HANDLE hCEvent;
	HANDLE hEscEvent;

	HANDLE hMailslot;
	HANDLE hEventMailslot;

	hCEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"EventoC");
	hEscEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"EventoEsc");

	hEventMailslot = OpenEvent(EVENT_MODIFY_STATE, FALSE, L"EventoMailslot");

	HANDLE Events[2] = { hCEvent, hEscEvent };
	DWORD ret, retEsc;
	int nTipoEvento;
	BOOL show_once = TRUE;

	BOOL bStatus;
	char MsgBuffer[ALARM_MESSAGE_SIZE];
	DWORD dwBytesLidos;

	hMailslot = CreateMailslot(
		L"\\\\.\\mailslot\\MyMailslot",
		0,
		MAILSLOT_WAIT_FOREVER,
		NULL);

	// Mailslot ready to receive messages
	SetEvent(hEventMailslot);

	//if (hMailslot != INVALID_HANDLE_VALUE) {
	//	do {

	//		/*DWORD MaxMsgSize;
	//		DWORD NextMsgSize;
	//		DWORD MsgCont;
	//		DWORD Timeout;

	//		bStatus = GetMailslotInfo(hMailslot, &MaxMsgSize, &NextMsgSize, &MsgCont, &Timeout);
	//		printf("NextMsgSize= %d\n", MaxMsgSize);*/

	//		bStatus = ReadFile(hMailslot, MsgBuffer, sizeof(char) * ALARM_MESSAGE_SIZE, &dwBytesLidos, NULL);
	//		for (int i = 0; i < ALARM_MESSAGE_SIZE; i++) {
	//			printf("%c", MsgBuffer[i]);
	//		}
	//		printf("\n");

	//		//printf("%s - teste\n", MsgBuffer);

	//	} while (1);
	//}

	if (hMailslot != INVALID_HANDLE_VALUE) {
		do {
			Sleep(1000);
			ret = WaitForMultipleObjects(2, Events, FALSE, 0);
			nTipoEvento = ret - WAIT_OBJECT_0;
			retEsc = WaitForSingleObject(hEscEvent, 0);

			if (nTipoEvento == 0) {
				SetEvent(hCEvent);

				bStatus = ReadFile(hMailslot, MsgBuffer, sizeof(char) * ALARM_MESSAGE_SIZE, &dwBytesLidos, NULL);
				for (int i = 0; i < ALARM_MESSAGE_SIZE; i++) {
					printf("%c", MsgBuffer[i]);
				}
				printf("\n");
				show_once = TRUE;
			}
			else if (show_once) {
				printf("---- Tarefa de exibicao de alarmes bloqueada! ----\n\n");
				show_once = FALSE;
			}
		} while (retEsc != 0);
	}
	
	printf("Thread C terminando...\n");
	return 0;
}