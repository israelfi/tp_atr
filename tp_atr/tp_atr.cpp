// tp_atr.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <direct.h>
#include <conio.h>				//_getch
#include <process.h>

#include "tp_atr.h"
#include "Messages.h"
#include "CheckForError.h"

#define MESSAGE_TYPE_INDEX 6
#define MAX_MESSAGES 100
#define MESSAGE_SIZE 53
#define CRITICAL_ALARM_TYPE 9
#define NON_CRITICAL_ALARM_TYPE 2
#define ALARM_THREADS 2
#define DATA_THREADS 1

typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);	//Casting para terceiro e sexto par�metros da fun��o
                                                    //_beginthreadex
typedef unsigned* CAST_LPDWORD;

// Keyboard inputs
#define	ESC				0x1B
#define S               0x73
#define P               0x70
#define D               0x64
#define A               0x61
#define O               0x6f
#define C               0x63


using namespace std;
using namespace Messages;

string USER_INPUT;
HANDLE hReadCircularList;
HANDLE hWriteCircularList;
HANDLE hReadMutex;
HANDLE hWriteMutex;

// Events regarding the keyboard input
HANDLE hEscEvent;
HANDLE hSEvent;
HANDLE hPEvent;
HANDLE hDEvent;
HANDLE hAEvent;
HANDLE hOEvent;
HANDLE hCEvent;

DWORD dwRet;

char circularList[MAX_MESSAGES][MESSAGE_SIZE];
int readPosition;
int writePosition;
int nTecla;

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

void incrementWritePosition() {
    writePosition = (writePosition + 1) % MAX_MESSAGES;
}

void writeMessage(const char* message) {
    strcpy(circularList[writePosition], message);
    incrementWritePosition();
}

void writeAlarmMessage(int alarmType) {
    WaitForSingleObject(hWriteMutex,NULL);
    WaitForSingleObject(hWriteCircularList, NULL);

    Messages::PIMSMessage alarm(alarmType);

    writeMessage(alarm.getMessage().c_str());

    ReleaseSemaphore(hReadCircularList, 1, NULL);
    ReleaseSemaphore(hWriteMutex, 1, NULL);
}

void writeCriticalAlarmMessage() {
    while (USER_INPUT != "ESC") {
        writeAlarmMessage(CRITICAL_ALARM_TYPE);
    }
}

void writeNonCriticalAlarmMessage() {
    while (USER_INPUT != "ESC") {
        writeAlarmMessage(NON_CRITICAL_ALARM_TYPE);
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


void readKeyboard() {
    /*
    Function that reads the user's input. Available options:
        s: Freezes/Unfreezes SDCD reading task - writeMessage
        p: Freezes/Unfreezes PIMS reading task - writeAlarmMessage
        d: Freezes/Unfreezes data capture task - dataMessageCapture
        a: Freezes/Unfreezes alarm capture task - alarmMessageCapture
        o: Freezes/Unfreezes data exhibition task - ?
        c: Freezes/Unfreezes alarm exhibition task - ?
        ESC: Terminate all tasks
    */

    // !!! Lembrar de checar por erro !!!
    
    // ESC should terminate all tasks, hence, it has automatic reset
    hEscEvent = CreateEvent(NULL, TRUE, FALSE, "EventoEsc");
    // The others have manual reset
    hSEvent = CreateEvent(NULL, FALSE, FALSE, "EventoS");
    hPEvent = CreateEvent(NULL, FALSE, FALSE, "EventoP");
    hDEvent = CreateEvent(NULL, FALSE, FALSE, "EventoD");
    hAEvent = CreateEvent(NULL, FALSE, FALSE, "EventoA");
    hOEvent = CreateEvent(NULL, FALSE, FALSE, "EventoO");
    hCEvent = CreateEvent(NULL, FALSE, FALSE, "EventoC");

    do {
        cout << "Press a key" << endl;
        nTecla = _getch();
        switch (nTecla)
        {
        case S:
            cout << "S" << endl;
            PulseEvent(hSEvent);
            break;
        case P:
            cout << "P" << endl;
            PulseEvent(hPEvent);
            break;
        case D:
            cout << "D" << endl;
            PulseEvent(hDEvent);
            break;
        case A:
            cout << "A" << endl;
            PulseEvent(hAEvent);
            break;
        case O:
            cout << "O" << endl;
            PulseEvent(hOEvent);
            break;
        case C:
            cout << "C" << endl;
            PulseEvent(hOEvent);
            break;
        case ESC:
            cout << "ESC" << endl;
            PulseEvent(hEscEvent);
            break;
        default:
            cout << "Invalid key" << endl;
            break;
        }
    } while (nTecla != ESC);
    cout << "All tasks ended" << endl;

    // Waiting threads to end
    // dwRet = WaitForMultipleObjects(NUM_THREADS,hThreads,TRUE,INFINITE);
    dwRet = WaitForMultipleObjects(0, 0, TRUE, INFINITE);
    //CheckForError(dwRet == WAIT_OBJECT_0);
    CloseHandle(hEscEvent);
}

void createCircularListSemaphores(){
    hReadCircularList = CreateSemaphore(NULL, 0, MAX_MESSAGES,"Readers semaphore");
    CheckForError(hReadCircularList);
    hWriteCircularList = CreateSemaphore(NULL, MAX_MESSAGES, MAX_MESSAGES, "Writers semaphore");
    CheckForError(hWriteCircularList);
    hReadMutex = CreateSemaphore(NULL, 1, 1, "Readers mutex");
    CheckForError(hReadMutex);
    hWriteMutex = CreateSemaphore(NULL, 1, 1, "Writers mutex");
    CheckForError(hWriteMutex);
}

int main()
{
    readKeyboard();

    char CRITICAL_ALARM_HANDLE_INDEX = 0;
    char NON_CRITICAL_ALARM_HANDLE_INDEX = 1;
    char DATA_HANDLE_INDEX = 2;

    STARTUPINFO alarmStartupInfo;
    STARTUPINFO dataStartupInfo;
    PROCESS_INFORMATION alarmProcessInfo;
    PROCESS_INFORMATION dataProcessInfo;

    DWORD dwIdWriteCriticalAlarm;
    DWORD dwIdWriteNonCriticalAlarm;
    DWORD dwIdWriteData;

    HANDLE hThreads[ALARM_THREADS + DATA_THREADS];


    ZeroMemory(&alarmStartupInfo, sizeof(alarmStartupInfo));
    alarmStartupInfo.cb = sizeof(alarmStartupInfo);
    ZeroMemory(&alarmProcessInfo, sizeof(alarmProcessInfo));

    ZeroMemory(&dataStartupInfo, sizeof(dataStartupInfo));
    dataStartupInfo.cb = sizeof(dataStartupInfo);
    ZeroMemory(&dataProcessInfo, sizeof(dataProcessInfo));

    crateProcessOnNewWindow(&alarmStartupInfo, &alarmProcessInfo, "./x64/Debug/show_alarm.exe");
    crateProcessOnNewWindow(&dataStartupInfo, &dataProcessInfo, "./x64/Debug/show_data.exe");

    createCircularListSemaphores();

    hThreads[CRITICAL_ALARM_HANDLE_INDEX] = 
        (HANDLE)_beginthreadex(
                NULL,
                0,
                (CAST_FUNCTION)writeCriticalAlarmMessage,
                (LPVOID)0,
                0,
                (CAST_LPDWORD)&dwIdWriteCriticalAlarm);
    if (hThreads[CRITICAL_ALARM_HANDLE_INDEX])
        printf("Thread Alarme crítico criada com sucesso! Id=%0x\n", dwIdWriteCriticalAlarm);
    else {
        printf("Erro na criacao da thread Alarme crítico! N = %d Erro = %d\n", CRITICAL_ALARM_HANDLE_INDEX, errno);
        exit(0);
    }

    hThreads[NON_CRITICAL_ALARM_HANDLE_INDEX] =
        (HANDLE)_beginthreadex(
            NULL,
            0,
            (CAST_FUNCTION)writeNonCriticalAlarmMessage,
            (LPVOID)0,
            0,
            (CAST_LPDWORD)&dwIdWriteNonCriticalAlarm);
    if (hThreads[NON_CRITICAL_ALARM_HANDLE_INDEX])
        printf("Thread Alarme não crítico criada com sucesso! Id=%0x\n", dwIdWriteNonCriticalAlarm);
    else {
        printf("Erro na criacao da thread Alarme não crítico! N = %d Erro = %d\n", NON_CRITICAL_ALARM_HANDLE_INDEX, errno);
        exit(0);
    }

    hThreads[DATA_HANDLE_INDEX] =
        (HANDLE)_beginthreadex(
            NULL,
            0,
            (CAST_FUNCTION)writeDataMessage,
            (LPVOID)0,
            0,
            (CAST_LPDWORD)&dwIdWriteData);
    if (hThreads[DATA_HANDLE_INDEX])
        printf("Thread Dados criada com sucesso! Id=%0x\n", dwIdWriteData);
    else {
        printf("Erro na criacao da thread Dados! N = %d Erro = %d\n", DATA_HANDLE_INDEX, errno);
        exit(0);
    }

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
