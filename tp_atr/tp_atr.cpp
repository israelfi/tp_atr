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
    HANDLE Events[2] = { hAEvent, hEscEvent };
    DWORD ret;
    int nTipoEvento;

    do {
        Sleep(250);
        ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
        nTipoEvento = ret - WAIT_OBJECT_0;
        if (nTipoEvento == 0) {
            SetEvent(hAEvent);
            WaitForSingleObject(hReadMutex, NULL);
            WaitForSingleObject(hReadCircularList, NULL);
            if (isAlarmMessage(circularList[readPosition])) {
                strcpy(alarmMessage, circularList[readPosition]);
                printf(alarmMessage);
                incrementReadPosition();
                ReleaseSemaphore(hWriteCircularList, 1, NULL);
            }
            else {
                ReleaseSemaphore(hReadCircularList, 1, NULL);
            }
            ReleaseSemaphore(hReadMutex, 1, NULL);
        }
    } while (nTipoEvento == 0);
    printf("Thread A terminando...\n");
    
}

void dataMessageCapture() {
    char dataMessage[MESSAGE_SIZE];
    HANDLE Events[2] = { hDEvent, hEscEvent };
    DWORD ret;
    int nTipoEvento;

    do {
        Sleep(250);
        ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
        nTipoEvento = ret - WAIT_OBJECT_0;

        SetEvent(hDEvent);
        WaitForSingleObject(hReadMutex, NULL);
        WaitForSingleObject(hReadCircularList, NULL);

        if (!isAlarmMessage(circularList[readPosition])) {
            strcpy(dataMessage, circularList[readPosition]);
            printf(dataMessage);
            incrementReadPosition();
            ReleaseSemaphore(hWriteCircularList, 1, NULL);
        }
        else {
            ReleaseSemaphore(hReadCircularList, 1, NULL);
        }
        
    } while (nTipoEvento == 0);
    printf("Thread D terminando...\n");
}

void incrementWritePosition() {
    writePosition = (writePosition + 1) % MAX_MESSAGES;
}

void writeMessage(const char* message) {
    strcpy(circularList[writePosition], message);
    incrementWritePosition();
}

void writeAlarmMessage(int alarmType) {
    HANDLE Events[2] = { hPEvent, hEscEvent };
    DWORD ret;
    int nTipoEvento;

    do {
        Sleep(250);
        ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
        nTipoEvento = ret - WAIT_OBJECT_0;
        if (nTipoEvento == 0) {
            //printf("Dps Wait writeAlarmMessage: %d\n", nTipoEvento);

            SetEvent(hPEvent);

            printf("writeAlarmMessage rolando\n");
            WaitForSingleObject(hWriteMutex, NULL);
            WaitForSingleObject(hWriteCircularList, NULL);

            Messages::PIMSMessage alarm(alarmType);

            writeMessage(alarm.getMessage().c_str());

            ReleaseSemaphore(hReadCircularList, 1, NULL);
            ReleaseSemaphore(hWriteMutex, 1, NULL);

        }
    } while (nTipoEvento == 0);

    printf("Thread P terminando...\n");
}

void writeCriticalAlarmMessage() {

    writeAlarmMessage(CRITICAL_ALARM_TYPE);        
    printf("Thread P Critico terminando...\n");
    _endthreadex(0);
}

void writeNonCriticalAlarmMessage() {

    writeAlarmMessage(NON_CRITICAL_ALARM_TYPE);
    printf("Thread P Nao Critico terminando...\n");
    _endthreadex(0);

}

void writeDataMessage() {
    HANDLE Events[2] = { hSEvent, hEscEvent };
    DWORD ret;
    int nTipoEvento;

    do {
        Sleep(250);
        ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
        nTipoEvento = ret - WAIT_OBJECT_0;
        if (nTipoEvento == 0) {
            SetEvent(hSEvent);
            WaitForSingleObject(hWriteMutex, NULL);
            WaitForSingleObject(hWriteCircularList, NULL);

            Messages::SDCDMessage data;

            writeMessage(data.getMessage().c_str());

            ReleaseSemaphore(hReadCircularList, 1, NULL);
            ReleaseSemaphore(hWriteMutex, 1, NULL);
        }
    } while (nTipoEvento == 0);
    printf("Thread S terminando...\n");
    _endthreadex(0);
}


void readKeyboard() {
    /*
    Function that reads the user's input. Available options:
        s: Freezes/Unfreezes SDCD reading task - writeDataMessage
        p: Freezes/Unfreezes PIMS reading task - writeAlarmMessage
        d: Freezes/Unfreezes data capture task - dataMessageCapture
        a: Freezes/Unfreezes alarm capture task - alarmMessageCapture
        o: Freezes/Unfreezes data exhibition task - ?
        c: Freezes/Unfreezes alarm exhibition task - ?
        ESC: Terminate all tasks
    */
    int state[] = { 0, 0, 0, 0, 0, 0 };
    do {
        std::cout << "Press a key" << endl;
        nTecla = _getch();
        switch (nTecla)
        {
        case S:
            if (state[0] == 0) {
                std::cout << "S reset" << endl;
                ResetEvent(hSEvent);
                state[0] = 1;
            }
            else {
                std::cout << "S set" << endl;
                SetEvent(hSEvent);
                state[0] = 0;
            }
            break;
        case P:
            if (state[1] == 0) {
                std::cout << "P reset" << endl;
                ResetEvent(hPEvent);
                state[1] = 1;
            }
            else {
                std::cout << "P set" << endl;
                SetEvent(hPEvent);
                state[1] = 0;
            }
            break;
        case D:
            if (state[2] == 0) {
                std::cout << "D reset" << endl;
                ResetEvent(hDEvent);
                state[2] = 1;
            }
            else {
                std::cout << "D set" << endl;
                SetEvent(hDEvent);
                state[2] = 0;
            }
            break;
        case A:
            if (state[3] == 0) {
                std::cout << "A reset" << endl;
                ResetEvent(hAEvent);
                state[3] = 1;
            }
            else {
                std::cout << "A set" << endl;
                SetEvent(hAEvent);
                state[3] = 0;
            }
            break;
        case O:
            if (state[4] == 0) {
                std::cout << "O reset" << endl;
                ResetEvent(hOEvent);
                state[4] = 1;
            }
            else {
                std::cout << "O set" << endl;
                SetEvent(hOEvent);
                state[4] = 0;
            }
            break;
        case C:
            if (state[5] == 0) {
                std::cout << "C reset" << endl;
                ResetEvent(hCEvent);
                state[5] = 1;
            }
            else {
                std::cout << "C set" << endl;
                SetEvent(hCEvent);
                state[5] = 0;
            }
            break;
        case ESC:
            std::cout << "ESC" << endl;
            SetEvent(hEscEvent);
            break;
        default:
            std::cout << "Invalid key" << endl;
            break;
        }
    } while (nTecla != ESC);
    std::cout << "All tasks ended" << endl;

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

    // ESC should terminate all tasks, hence, it has automatic reset
    hEscEvent = CreateEvent(NULL, TRUE, FALSE, "EventoEsc");
    CheckForError(hEscEvent);

    // The others have manual reset
    hSEvent = CreateEvent(NULL, FALSE, TRUE, "EventoS");
    CheckForError(hSEvent);
    hPEvent = CreateEvent(NULL, FALSE, TRUE, "EventoP");
    CheckForError(hPEvent);
    hDEvent = CreateEvent(NULL, FALSE, TRUE, "EventoD");
    CheckForError(hDEvent);
    hAEvent = CreateEvent(NULL, FALSE, TRUE, "EventoA");
    CheckForError(hAEvent);
    hOEvent = CreateEvent(NULL, FALSE, TRUE, "EventoO");
    CheckForError(hOEvent);
    hCEvent = CreateEvent(NULL, FALSE, TRUE, "EventoC");
    CheckForError(hCEvent);


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

    readKeyboard();

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
