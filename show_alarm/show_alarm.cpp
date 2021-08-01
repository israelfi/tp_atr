#include <stdio.h>
#include <iostream>
#include <string>
#include <windows.h>


int main() {
	HANDLE hCEvent;
	HANDLE hEscEvent;

	hCEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"EventoC");
	hEscEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"EventoEsc");

	HANDLE Events[2] = { hCEvent, hEscEvent };
	DWORD ret, retEsc;
	int nTipoEvento;
	bool show_once = TRUE;

	do {
		Sleep(1000);
		ret = WaitForMultipleObjects(2, Events, FALSE, 0);
		nTipoEvento = ret - WAIT_OBJECT_0;
		retEsc = WaitForSingleObject(hEscEvent, 0);

		if (nTipoEvento == 0) {
			SetEvent(hCEvent);
			printf("---- Mensagens de alarme! ----\n\n");
			show_once = TRUE;
		}
		else if (show_once){
			printf("---- Tarefa de exibicao de alarmes bloqueada! ----\n\n");
			show_once = FALSE;
		}
	} while (retEsc != 0);

	printf("Thread C terminando...\n");
	char x;
	std::cin >> x;
}