#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <tlhelp32.h>
#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <process.h>
#include <Psapi.h>

#define ID_TERMINATE 1001
#define ID_UPDATE 1002
#define ID_STOP_UPDATING 1003
#define ID_START_UPDATING 1004
#define ID_PROCESS_LIST 1005
#define UPDATE_TIME 1000

HANDLE g_hThread = NULL;
HANDLE g_hThreadMutex = NULL;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void UpdateProcessList(HWND hwnd);
void TerminateProcces(HWND hwnd);
void StartUpdateThread(HWND hwnd);
void StopUpdateThread();


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    MSG msg;
    HWND hwnd;
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = L"TaskManager";
    wc.lpszMenuName = NULL;
    wc.style = CS_HREDRAW | CS_VREDRAW;

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Failed to register window class.", L"Error", MB_ICONERROR | MB_OK);
        return EXIT_FAILURE;
    }

    hwnd = CreateWindow(wc.lpszClassName, L"Task Manager", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 800, NULL, NULL, wc.hInstance, NULL);

    if (hwnd == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, L"Failed to create a window.", L"Error", MB_ICONERROR);
        return EXIT_FAILURE;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg)
    {
    case WM_CREATE:
    {
        HWND hUpdateButton = CreateWindow(L"BUTTON", L"Update", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 10, 10, 120, 30, hwnd, (HMENU)ID_UPDATE, NULL, NULL);
        HWND hStartUpdating = CreateWindow(L"BUTTON", L"Start updating", WS_VISIBLE | WS_CHILD, 10, 50, 120, 30, hwnd, (HMENU)ID_START_UPDATING, NULL, NULL);
        HWND hStopUpdating = CreateWindow(L"BUTTON", L"Stop updating", WS_VISIBLE | WS_CHILD, 10, 90, 120, 30, hwnd, (HMENU)ID_STOP_UPDATING, NULL, NULL);
        HWND hTerminateButton = CreateWindow(L"BUTTON", L"Terminate", WS_VISIBLE | WS_CHILD, 10, 130, 120, 30, hwnd, (HMENU)ID_TERMINATE, NULL, NULL);
        HWND hProcessList = CreateWindow(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_DISABLENOSCROLL | LBS_NOTIFY, 150, 10, 400, 700, hwnd, (HMENU)ID_PROCESS_LIST, NULL, NULL);

        StartUpdateThread(hwnd);
        g_hThreadMutex = CreateMutex(NULL, FALSE, NULL);

        break;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case ID_STOP_UPDATING:
        {
            StopUpdateThread();
            break;
        }

        case ID_START_UPDATING:
        {
            StartUpdateThread(hwnd);
            break;
        }

        case ID_UPDATE:
        {
            UpdateProcessList(hwnd);
            break;
        }

        case ID_TERMINATE:
        {
            TerminateProcces(hwnd);
            break;
        }
        }
        break;
    }

    case WM_DESTROY:
    {
        PostQuitMessage(0);
        break;
    }

    default:
    {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    }
    return 0;
}


DWORD WINAPI UpdateProcessListThread(LPVOID lpParam) {
    while (true)
    {
        UpdateProcessList((HWND)lpParam);
        Sleep(UPDATE_TIME);
    }
    return 0;
}


void StartUpdateThread(HWND hwnd) {
    WaitForSingleObject(g_hThreadMutex, INFINITE);

    if (g_hThread == NULL)
    {
        g_hThread = CreateThread(NULL, 0, UpdateProcessListThread, (LPVOID)hwnd, 0, NULL);
    }

    ReleaseMutex(g_hThreadMutex);
}


void StopUpdateThread() {
    WaitForSingleObject(g_hThreadMutex, INFINITE);

    if (g_hThread != NULL) {
        TerminateThread(g_hThread, 0);
        CloseHandle(g_hThread);
        g_hThread = NULL;
    }

    ReleaseMutex(g_hThreadMutex);
}


void TerminateProcces(HWND hwnd) {

    HWND listBox = GetDlgItem(hwnd, ID_PROCESS_LIST);
    int selectedIndex = SendMessage(listBox, LB_GETCURSEL, 0, 0);
    if (selectedIndex == LB_ERR) {
        MessageBox(hwnd, L"No process selected.", L"Error", MB_ICONERROR);
        return;
    }


    TCHAR processName[MAX_PATH];
    SendMessage(listBox, LB_GETTEXT, selectedIndex, (LPARAM)processName);

    TCHAR* spacePos = _tcschr(processName, ' ');
    if (spacePos != nullptr) {
        *spacePos = '\0';
    }

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        MessageBox(hwnd, L"Error creating process snapshot.", L"Error", MB_ICONERROR);
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    DWORD targetPID = 0;
    bool processFound = false;

    if (Process32First(hProcessSnap, &pe32)) {
        do {
            if (_tcscmp(pe32.szExeFile, processName) == 0) {
                targetPID = pe32.th32ProcessID;
                processFound = true;
                break;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }

    CloseHandle(hProcessSnap);

    if (!processFound) {
        MessageBox(hwnd, L"Process not found.", L"Error", MB_ICONERROR);
        return;
    }

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, targetPID);
    if (hProcess == NULL) {
        MessageBox(hwnd, L"Error opening process.", L"Error", MB_ICONERROR);
        return;
    }

    if (TerminateProcess(hProcess, 0)) {
        MessageBox(hwnd, L"Process terminated successfully.", L"Success", MB_ICONINFORMATION);
    }
    else {
        MessageBox(hwnd, L"Error terminating process.", L"Error", MB_ICONERROR);
    }

    CloseHandle(hProcess);
}


void UpdateProcessList(HWND hwnd) {

    // Очистка listBox
    HWND listBox = GetDlgItem(hwnd, ID_PROCESS_LIST);
    SendMessage(listBox, LB_RESETCONTENT, 0, 0);

    // Создание снимка (TH32CS_SNAPPROCESS включает в снимок все процессы в системе)
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        MessageBox(hwnd, L"Error creating process snapshot.", L"Error", MB_ICONERROR);
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hProcessSnap, &pe32)) {
        do {
            // Открытие процесса
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);

            if (hProcess != NULL) {
                PROCESS_MEMORY_COUNTERS pmc;
                if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                    TCHAR buffer[512];
                    _stprintf_s(buffer, _T("%s - Memory Usage: %I64u KB"), pe32.szExeFile, pmc.WorkingSetSize / (1024));
                    SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)buffer);

                    //InvalidateRect(listBox, NULL, TRUE);
                }

                CloseHandle(hProcess);
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }

    CloseHandle(hProcessSnap);
}