#include <stdio.h>
#include <iostream>
#include <string>
#include <windows.h>
#include <conio.h>	

#include "CheckForError.h"

#define ALARM_MESSAGE_SIZE 31
#define NSEQ_POSITION_BEGINS 0
#define NSEQ_POSITION_ENDS 5
#define TIME_POSITION_BEGINS 23
#define TIME_POSITION_ENDS 30
#define ID_POSITION_STARTS 9
#define ID_POSITION_ENDS 12
#define DEGREE_POSITION_STARTS 14
#define DEGREE_POSITION_ENDS 15
#define PREV_POSITION_STARTS 17
#define PREV_POSITION_ENDS 21
#define WHITE   FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE
#define HLGREEN FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define HLRED   FOREGROUND_RED   | FOREGROUND_INTENSITY


int main() {
	HANDLE hCEvent;
	HANDLE hEscEvent;

	HANDLE hMailslot;
	HANDLE hEventMailslot;
	HANDLE hOut;	// Console output

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

	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		printf("Erro ao obter handle para a saída da console\n");

	hMailslot = CreateMailslot(
		L"\\\\.\\mailslot\\MyMailslot",
		0,
		MAILSLOT_WAIT_FOREVER,
		NULL);

	// Mailslot ready to receive messages
	SetEvent(hEventMailslot);

	if (hMailslot != INVALID_HANDLE_VALUE) {
		do {
			ret = WaitForMultipleObjects(2, Events, FALSE, 0);
			nTipoEvento = ret - WAIT_OBJECT_0;
			retEsc = WaitForSingleObject(hEscEvent, 0);

			if (nTipoEvento == 0) {
				SetEvent(hCEvent);

				bStatus = ReadFile(hMailslot, MsgBuffer, sizeof(char) * ALARM_MESSAGE_SIZE, &dwBytesLidos, NULL);
				if (MsgBuffer[7]=='9') {
					SetConsoleTextAttribute(hOut, HLRED);
				}
				else {
					SetConsoleTextAttribute(hOut, WHITE);
				}
				// HH:MM:SS NSEQ: ###### ID ALARME: #### GRAU: ## PREV: ##### 
				for (int i = TIME_POSITION_BEGINS; i <= TIME_POSITION_ENDS; i++) {
					printf("%c", MsgBuffer[i]);
				}
				printf(" NSEQ: ");
				for (int i = NSEQ_POSITION_BEGINS; i <= NSEQ_POSITION_ENDS; i++) {
					printf("%c", MsgBuffer[i]);
				}
				printf(" ID ALARME: ");
				for (int i = ID_POSITION_STARTS; i <= ID_POSITION_ENDS; i++) {
					printf("%c", MsgBuffer[i]);
				}
				printf(" GRAU: ");
				for (int i = DEGREE_POSITION_STARTS; i <= DEGREE_POSITION_ENDS; i++) {
					printf("%c", MsgBuffer[i]);
				}
				printf(" PREV: ");
				for (int i = PREV_POSITION_STARTS; i <= PREV_POSITION_ENDS; i++) {
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