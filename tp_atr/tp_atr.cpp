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

#define MESSAGE_TYPE_INDEX 6
#define MAX_MESSAGES 100
#define MESSAGE_SIZE 52
#define CRITICAL_ALARM_TYPE 9
#define NON_CRITICAL_ALARM_TYPE 2

using namespace std;
using namespace Messages;

string USER_INPUT;
HANDLE hReadCircularList;
HANDLE hWriteCircularList;
HANDLE hReadMutex;
HANDLE hWriteMutex;
char circularList[MAX_MESSAGES][MESSAGE_SIZE];
int readPosition;
int writePosition;

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

bool isAlarmMessage(char* message) {
    char messageType = message[MESSAGE_TYPE_INDEX];
    if (messageType == '2' || messageType == '9')
        return true;
    else 
        return false;
}

void incrementReadPosition() {
    readPosition = (readPosition + 1) % MAX_MESSAGES;
}

void alarmMessageCapture() {
    char alarmMessage[MESSAGE_SIZE];
    while (USER_INPUT != "ESC") {
        WaitForSingleObject(hReadMutex, NULL);
        WaitForSingleObject(hReadCircularList,NULL);
        if (isAlarmMessage(circularList[readPosition])) {
            strcpy(alarmMessage,circularList[readPosition]);
            printf(alarmMessage);
            incrementReadPosition();
            ReleaseSemaphore(hWriteCircularList, 1, NULL);
        }
        else {
            ReleaseSemaphore(hReadCircularList, 1, NULL);
        }
        ReleaseSemaphore(hReadMutex,1, NULL);
    }
}

void dataMessageCapture() {
    char dataMessage[MESSAGE_SIZE];
    while (USER_INPUT != "ESC") {
        WaitForSingleObject(hReadMutex, NULL);
        WaitForSingleObject(hReadCircularList, NULL);

        if (!isAlarmMessage(circularList[readPosition])) {
            strcpy(dataMessage, circularList[readPosition]);
            printf(dataMessage);
            incrementReadPosition();
            ReleaseSemaphore(hWriteCircularList,1,NULL);
        }
        else {
            ReleaseSemaphore(hReadCircularList, 1, NULL);
        }
    }
}

void writeMessage(const char* message) {
    strcpy(circularList[writePosition], message);
}

void writeAlarmMessage(int alarmType) {
    while (USER_INPUT != "ESC") {
        WaitForSingleObject(hWriteMutex,NULL);
        WaitForSingleObject(hWriteCircularList, NULL);

        Messages::PIMSMessage alarm(alarmType);

        writeMessage(alarm.getMessage().c_str());

        ReleaseSemaphore(hReadCircularList, 1, NULL);
        ReleaseSemaphore(hWriteMutex, 1, NULL);
    }
}

void writeDataMessage() {
    while (USER_INPUT != "ESC") {
        WaitForSingleObject(hWriteMutex, NULL);
        WaitForSingleObject(hWriteCircularList, NULL);

        Messages::SDCDMessage data;

        writeMessage(data.getMessage().c_str());

        ReleaseSemaphore(hReadCircularList, 1, NULL);
        ReleaseSemaphore(hWriteMutex, 1, NULL);
    }
}

int main()
{
    STARTUPINFO alarmStartupInfo;
    STARTUPINFO dataStartupInfo;
    PROCESS_INFORMATION alarmProcessInfo;
    PROCESS_INFORMATION dataProcessInfo;


    ZeroMemory(&alarmStartupInfo, sizeof(alarmStartupInfo));
    alarmStartupInfo.cb = sizeof(alarmStartupInfo);
    ZeroMemory(&alarmProcessInfo, sizeof(alarmProcessInfo));

    ZeroMemory(&dataStartupInfo, sizeof(dataStartupInfo));
    dataStartupInfo.cb = sizeof(dataStartupInfo);
    ZeroMemory(&dataProcessInfo, sizeof(dataProcessInfo));

    crateProcessOnNewWindow(&alarmStartupInfo, &alarmProcessInfo, "./x64/Debug/show_alarm.exe");
    crateProcessOnNewWindow(&dataStartupInfo, &dataProcessInfo, "./x64/Debug/show_data.exe");

    // Wait until child process exits.
    WaitForSingleObject(alarmProcessInfo.hProcess, INFINITE);
    WaitForSingleObject(dataProcessInfo.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(alarmProcessInfo.hProcess);
    CloseHandle(dataProcessInfo.hProcess);
    CloseHandle(alarmProcessInfo.hThread);
    CloseHandle(dataProcessInfo.hThread);
    getchar();
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
