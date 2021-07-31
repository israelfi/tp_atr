// tp_atr.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <direct.h>
#include "tp_atr.h"
#include "Messages.h"
using namespace std;
using namespace Messages;

int crateProcessOnNewWindow(STARTUPINFO* startupInfo, PROCESS_INFORMATION* processInfo,LPCSTR filePath) {
    if (!CreateProcess(filePath,   // No module name (use command line)
        NULL,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE,              // No creation flags
        NULL,           // Use parent's environment block
        "./x64/Debug",           // Use parent's starting directory 
        startupInfo,            // Pointer to STARTUPINFO structure
        processInfo)           // Pointer to PROCESS_INFORMATION structure
        )
    {
        printf("CreateProcess failed (%d).\n", GetLastError());
        return -1;
    }
    return 0;
}

void scratch_main() {
    char m[52];
    PIMSMessage m1(2);
    m1.getCharMessage(m);
    for (int i = 0; i < 31; i++) {
        cout << m[i];
    };
    getchar();
    return;
}

int main()
{
    //STARTUPINFO alarmStartupInfo;
    //STARTUPINFO dataStartupInfo;
    //PROCESS_INFORMATION alarmProcessInfo;
    //PROCESS_INFORMATION dataProcessInfo;


    //ZeroMemory(&alarmStartupInfo, sizeof(alarmStartupInfo));
    //alarmStartupInfo.cb = sizeof(alarmStartupInfo);
    //ZeroMemory(&alarmProcessInfo, sizeof(alarmProcessInfo));

    //ZeroMemory(&dataStartupInfo, sizeof(dataStartupInfo));
    //dataStartupInfo.cb = sizeof(dataStartupInfo);
    //ZeroMemory(&dataProcessInfo, sizeof(dataProcessInfo));

    //crateProcessOnNewWindow(&alarmStartupInfo, &alarmProcessInfo, "./x64/Debug/show_alarm.exe");
    //crateProcessOnNewWindow(&dataStartupInfo, &dataProcessInfo, "./x64/Debug/show_data.exe");

    //// Wait until child process exits.
    //WaitForSingleObject(alarmProcessInfo.hProcess, INFINITE);
    //WaitForSingleObject(dataProcessInfo.hProcess, INFINITE);

    //// Close process and thread handles. 
    //CloseHandle(alarmProcessInfo.hProcess);
    //CloseHandle(dataProcessInfo.hProcess);
    //CloseHandle(alarmProcessInfo.hThread);
    //CloseHandle(dataProcessInfo.hThread);
    //getchar();
    scratch_main();
    return 0;
}

// Executar programa: Ctrl + F5 ou Menu Depurar > Iniciar Sem Depuração
// Depurar programa: F5 ou menu Depurar > Iniciar Depuração

// Dicas para Começar: 
//   1. Use a janela do Gerenciador de Soluções para adicionar/gerenciar arquivos
//   2. Use a janela do Team Explorer para conectar-se ao controle do código-fonte
//   3. Use a janela de Saída para ver mensagens de saída do build e outras mensagens
//   4. Use a janela Lista de Erros para exibir erros
//   5. Ir Para o Projeto > Adicionar Novo Item para criar novos arquivos de código, ou Projeto > Adicionar Item Existente para adicionar arquivos de código existentes ao projeto
//   6. No futuro, para abrir este projeto novamente, vá para Arquivo > Abrir > Projeto e selecione o arquivo. sln
