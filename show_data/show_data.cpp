#include <stdio.h>
#include <iostream>
#include <string>
#include <windows.h>


int main() {
	HANDLE hOEvent;
	HANDLE hEscEvent;

	hOEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"EventoO");
	hEscEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"EventoEsc");

	HANDLE Events[2] = { hOEvent, hEscEvent };
	DWORD ret, retEsc;
	int nTipoEvento;
	bool show_once = TRUE;

	do {
		Sleep(1000);
		ret = WaitForMultipleObjects(2, Events, FALSE, 0);
		nTipoEvento = ret - WAIT_OBJECT_0;
		retEsc = WaitForSingleObject(hEscEvent, 0);

		if (nTipoEvento == 0) {
			SetEvent(hOEvent);
			printf("---- Mensagens de dados do processo! ----\n\n");
			show_once = TRUE;
		}
		else if (show_once) {
			printf("---- Tarefa de exibicao o de dados de processo bloqueada! ----\n\n");
			show_once = FALSE;
		}
	} while (retEsc != 0);

	printf("Thread O terminando...\n");
	return 0;
}