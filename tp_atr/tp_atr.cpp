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

#define MESSAGE_TYPE_INDEX 7
#define MAX_MESSAGES 100
#define MESSAGE_SIZE 53
#define CRITICAL_ALARM_TYPE 9
#define NON_CRITICAL_ALARM_TYPE 2
#define ALARM_THREADS 3 //Two for creating messages, one for capturing messages
#define DATA_THREADS 2 //One for creating messages, one for capturing messages

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
HANDLE hReadDataCircularList;
HANDLE hReadAlarmCircularList;
HANDLE hWriteCircularList;
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
int dataReadPosition = 0;
int alarmReadPosition = 0;
int writePosition = 0;
int nTecla;

int crateProcessOnNewWindow(STARTUPINFO* startupInfo, PROCESS_INFORMATION* processInfo,LPCSTR filePath) {
    if (!CreateProcess(filePath,   // No module name (use command line)
        NULL,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        TRUE,          // Set handle inheritance to FALSE
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

bool isAlarmMessage(char messageType) {
    if (messageType == '2' || messageType == '9')
        return true;
    else 
        return false;
}

void incrementAlarmReadPosition() {
    alarmReadPosition = (alarmReadPosition + 1) % MAX_MESSAGES;
}

void incrementDataReadPosition() {
    dataReadPosition = (dataReadPosition + 1) % MAX_MESSAGES;
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
            WaitForSingleObject(hReadAlarmCircularList, INFINITE);
            while (!isAlarmMessage(circularList[alarmReadPosition][MESSAGE_TYPE_INDEX])) {
                incrementAlarmReadPosition();
            }
            strcpy(alarmMessage, circularList[alarmReadPosition]);
            ReleaseSemaphore(hWriteCircularList, 1, NULL);
            printf("Mensagem de ALARME capturada com sucesso: - %s\n", alarmMessage);
            incrementAlarmReadPosition();
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
        WaitForSingleObject(hReadDataCircularList, INFINITE);
        while (isAlarmMessage(circularList[dataReadPosition][MESSAGE_TYPE_INDEX])) {
            incrementDataReadPosition();
        }
        strcpy(dataMessage, circularList[dataReadPosition]);
        ReleaseSemaphore(hWriteCircularList, 1, NULL);
        printf("Mensagem de DADO capturada com sucesso: --- %s\n", dataMessage);
        incrementDataReadPosition();
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

            //printf("writeAlarmMessage rolando\n");
            WaitForSingleObject(hWriteMutex, INFINITE);
            WaitForSingleObject(hWriteCircularList, INFINITE);

            Messages::PIMSMessage alarm(alarmType);

            writeMessage(alarm.getMessage().c_str());

            ReleaseSemaphore(hReadAlarmCircularList, 1, NULL);
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
            WaitForSingleObject(hWriteMutex, INFINITE);
            WaitForSingleObject(hWriteCircularList, INFINITE);

            Messages::SDCDMessage data;

            writeMessage(data.getMessage().c_str());

            ReleaseSemaphore(hReadDataCircularList, 1, NULL);
            ReleaseSemaphore(hWriteMutex, 1, NULL);
        }
    } while (nTipoEvento == 0);
    printf("Thread S terminando...\n");
    _endthreadex(0);
}

void closeAllKeyboardHandles() {
    CloseHandle(hEscEvent);
    CloseHandle(hSEvent);
    CloseHandle(hPEvent);
    CloseHandle(hDEvent);
    CloseHandle(hAEvent);
    CloseHandle(hOEvent);
    CloseHandle(hCEvent);
}

void readKeyboard() {
    /*
    Function that reads the user's input. Available options:
        s: Freezes/Unfreezes SDCD reading task - writeDataMessage
        p: Freezes/Unfreezes PIMS reading task - writeAlarmMessage
        d: Freezes/Unfreezes data capture task - dataMessageCapture
        a: Freezes/Unfreezes alarm capture task - alarmMessageCapture
        o: Freezes/Unfreezes data exhibition task - show_data.exe
        c: Freezes/Unfreezes alarm exhibition task - show_alarm.exe
        ESC: Terminate all tasks
    */
    int state[] = { 0, 0, 0, 0, 0, 0 };
    do {
        std::cout << "Press a key\n" << endl;
        nTecla = _getch();
        switch (nTecla)
        {
        case S:
            if (state[0] == 0) {
                std::cout << "S reset - Tarefa de leitura do SDCD bloqueada!\n" << endl;
                ResetEvent(hSEvent);
                state[0] = 1;
            }
            else {
                std::cout << "S set - Tarefa de leitura do SDCD desbloqueada!\n" << endl;
                SetEvent(hSEvent);
                state[0] = 0;
            }
            break;
        case P:
            if (state[1] == 0) {
                std::cout << "P reset - Tarefa de leitura do PIMS bloqueada!\n" << endl;
                ResetEvent(hPEvent);
                state[1] = 1;
            }
            else {
                std::cout << "P set - Tarefa de leitura do SDCD desbloqueada!\n" << endl;
                SetEvent(hPEvent);
                state[1] = 0;
            }
            break;
        case D:
            if (state[2] == 0) {
                std::cout << "D reset - Tarefa de captura de dados do processo bloqueada!\n" << endl;
                ResetEvent(hDEvent);
                state[2] = 1;
            }
            else {
                std::cout << "D set - Tarefa de captura de dados do processo desbloqueada!\n" << endl;
                SetEvent(hDEvent);
                state[2] = 0;
            }
            break;
        case A:
            if (state[3] == 0) {
                std::cout << "A reset - Tarefa de captura de dados de alarmes bloqueada!\n" << endl;
                ResetEvent(hAEvent);
                state[3] = 1;
            }
            else {
                std::cout << "A set - Tarefa de captura de dados de alarmes desbloqueada!\n" << endl;
                SetEvent(hAEvent);
                state[3] = 0;
            }
            break;
        case O:
            if (state[4] == 0) {
                std::cout << "O reset\n" << endl;
                ResetEvent(hOEvent);
                state[4] = 1;
            }
            else {
                std::cout << "O set\n" << endl;
                SetEvent(hOEvent);
                state[4] = 0;
            }
            break;
        case C:
            if (state[5] == 0) {
                std::cout << "C reset\n" << endl;
                ResetEvent(hCEvent);
                state[5] = 1;
            }
            else {
                std::cout << "C set\n" << endl;
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

    // Waiting threads to end
    // dwRet = WaitForMultipleObjects(NUM_THREADS,hThreads,TRUE,INFINITE);
    //CheckForError(dwRet == WAIT_OBJECT_0);
    closeAllKeyboardHandles();
}

void createCircularListSemaphores(){
    hReadDataCircularList = CreateSemaphore(NULL, 0, MAX_MESSAGES, "Data reader semaphore");
    CheckForError(hReadDataCircularList);
    hReadAlarmCircularList = CreateSemaphore(NULL, 0, MAX_MESSAGES, "Alarm readers semaphore");
    CheckForError(hReadAlarmCircularList);
    hWriteCircularList = CreateSemaphore(NULL, MAX_MESSAGES, MAX_MESSAGES, "Writers semaphore");
    CheckForError(hWriteCircularList);
    hWriteMutex = CreateSemaphore(NULL, 1, 1, "Writers mutex");
    CheckForError(hWriteMutex);
}

void closeSemaphoresHandles() {
    CloseHandle(hReadDataCircularList);
    CloseHandle(hWriteCircularList);
    CloseHandle(hWriteMutex);
}

HANDLE createThreadFromHandle(_beginthreadex_proc_type castedFunction, unsigned int* threadAddr) {
    HANDLE hThread =
        (HANDLE)_beginthreadex(
            NULL,
            0,
            castedFunction,
            (LPVOID)0,
            0,
            threadAddr);
    return hThread;
}

int main()
{

    char CRITICAL_ALARM_WRITER_INDEX = 0;
    char NON_CRITICAL_ALARM_WRITER_INDEX = 1;
    char DATA_WRITER_INDEX = 2;
    char ALARM_CAPTURE_INDEX = 3;
    char DATA_CAPTURE_INDEX = 4;

    STARTUPINFO alarmStartupInfo;
    STARTUPINFO dataStartupInfo;
    PROCESS_INFORMATION alarmProcessInfo;
    PROCESS_INFORMATION dataProcessInfo;

    DWORD dwIdWriteCriticalAlarm;
    DWORD dwIdWriteNonCriticalAlarm;
    DWORD dwIdWriteData;
    DWORD dwIdCaptureData;
    DWORD dwIdCaptureAlarm;

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

    hThreads[CRITICAL_ALARM_WRITER_INDEX] = createThreadFromHandle(
        (CAST_FUNCTION)writeCriticalAlarmMessage,
        (CAST_LPDWORD)&dwIdWriteCriticalAlarm);

    if (hThreads[CRITICAL_ALARM_WRITER_INDEX])
        printf("Thread Alarme crítico criada com sucesso! Id=%0x\n", dwIdWriteCriticalAlarm);
    else {
        printf("Erro na criacao da thread Alarme crítico! N = %d Erro = %d\n", CRITICAL_ALARM_WRITER_INDEX, errno);
        exit(0);
    }
    hThreads[NON_CRITICAL_ALARM_WRITER_INDEX] = createThreadFromHandle(
            (CAST_FUNCTION)writeNonCriticalAlarmMessage,
            (CAST_LPDWORD)&dwIdWriteNonCriticalAlarm);

    if (hThreads[NON_CRITICAL_ALARM_WRITER_INDEX])
        printf("Thread Alarme não crítico criada com sucesso! Id=%0x\n", dwIdWriteNonCriticalAlarm);
    else {
        printf("Erro na criacao da thread Alarme não crítico! N = %d Erro = %d\n", NON_CRITICAL_ALARM_WRITER_INDEX, errno);
        exit(0);
    }
    hThreads[DATA_WRITER_INDEX] = createThreadFromHandle(
            (CAST_FUNCTION)writeDataMessage,
            (CAST_LPDWORD)&dwIdWriteData);

    if (hThreads[DATA_WRITER_INDEX])
        printf("Thread Dados criada com sucesso! Id=%0x\n", dwIdWriteData);
    else {
        printf("Erro na criacao da thread Dados! N = %d Erro = %d\n", DATA_WRITER_INDEX, errno);
        exit(0);
    }
    hThreads[ALARM_CAPTURE_INDEX] = createThreadFromHandle(
            (CAST_FUNCTION)alarmMessageCapture,
            (CAST_LPDWORD)&dwIdCaptureAlarm);

    if (hThreads[ALARM_CAPTURE_INDEX])
        printf("Thread Dados criada com sucesso! Id=%0x\n", dwIdCaptureAlarm);
    else {
        printf("Erro na criacao da thread Dados! N = %d Erro = %d\n", ALARM_CAPTURE_INDEX, errno);
        exit(0);
    }

    hThreads[DATA_CAPTURE_INDEX] = createThreadFromHandle(
        (CAST_FUNCTION)dataMessageCapture,
        (CAST_LPDWORD)&dwIdCaptureData);

    if (hThreads[ALARM_CAPTURE_INDEX])
        printf("Thread Dados criada com sucesso! Id=%0x\n", dwIdCaptureData);
    else {
        printf("Erro na criacao da thread Dados! N = %d Erro = %d\n", ALARM_CAPTURE_INDEX, errno);
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
    int threadSize = ALARM_THREADS + DATA_THREADS;
    int i;
    DWORD dwExitCode;
    closeSemaphoresHandles();
    for (i = 0; i < threadSize; ++i) {
        dwRet = WaitForSingleObject(hThreads[i], INFINITE);
        CheckForError(dwRet == WAIT_OBJECT_0);

        GetExitCodeThread(hThreads[i], &dwExitCode);
        printf("thread %d terminou com codigo de saida %d\n", i, dwExitCode);
        CloseHandle(hThreads[i]);    // apaga referência ao objeto
    }
    std::cout << "All tasks ended, press ENTER to exit" << endl;
    getchar();
    return 0;
}
