#include <windows.h>
#include <cmath>
#include <string>


HWND hAEdit, hBEdit, hCEdit, hResultEdit, hButtonSolve, hButtonChangeColor, hButton;
HINSTANCE hInstance; 


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam);


HHOOK hKeyboardHook = NULL;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"QuadraticEquationSolver";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex)) {
        MessageBox(NULL, L"Не удалось зарегистрировать класс окна.", L"Ошибка", MB_ICONERROR);
        return 1;
    }

  
    HWND hWnd = CreateWindow(L"QuadraticEquationSolver", L"Решение квадратного уравнения",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, NULL, NULL, hInstance, NULL);

    if (!hWnd) {
        MessageBox(NULL, L"Не удалось создать окно.", L"Ошибка", MB_ICONERROR);
        return 1;
    }

    CreateWindow(L"static", L"coeff a", WS_VISIBLE | WS_CHILD, 20, 5, 100, 25, hWnd, NULL, NULL, NULL);
    CreateWindow(L"static", L"coeff b", WS_VISIBLE | WS_CHILD, 20, 45, 100, 25, hWnd, NULL, NULL, NULL);
    CreateWindow(L"static", L"coeff c", WS_VISIBLE | WS_CHILD, 20, 85, 100, 25, hWnd, NULL, NULL, NULL);

 
    hAEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        20, 20, 100, 25, hWnd, NULL, hInstance, NULL);
    hBEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        20, 60, 100, 25, hWnd, NULL, hInstance, NULL);
    hCEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        20, 100, 100, 25, hWnd, NULL, hInstance, NULL);

    hButton = CreateWindow(L"BUTTON", L"Решить", WS_CHILD | WS_VISIBLE,
        20, 140, 100, 30, hWnd, (HMENU)1, hInstance, NULL);

  
    hResultEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_READONLY,
        150, 20, 200, 150, hWnd, NULL, hInstance, NULL);

    hButtonSolve = CreateWindow(L"BUTTON", L"Решить", WS_CHILD | WS_VISIBLE,
        20, 140, 100, 30, hWnd, (HMENU)1, hInstance, NULL);

    hButtonChangeColor = CreateWindow(L"BUTTON", L"Изменить цвет фона", WS_CHILD | WS_VISIBLE,
        180, 180, 150, 30, hWnd, (HMENU)2, hInstance, NULL);

    hInstance = hInst;

  
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

   
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook, hInstance, 0);

    if (!hKeyboardHook) {
        MessageBox(NULL, L"Не удалось установить глобальный хук клавиш.", L"Ошибка", MB_ICONERROR);
        return 1;
    }

  
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

  
    UnhookWindowsHookEx(hKeyboardHook);

    return (int)msg.wParam;
}

void ChangeWindowBackgroundColor(HWND hWnd) {
    static COLORREF customColor = RGB(255, 255, 255); 

    CHOOSECOLOR cc = { sizeof(CHOOSECOLOR) };
    cc.hwndOwner = hWnd;
    cc.lpCustColors = &customColor;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (ChooseColor(&cc)) {
        customColor = cc.rgbResult;
        HBRUSH hBrush = CreateSolidBrush(customColor);
        SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);
        InvalidateRect(hWnd, NULL, TRUE);
        RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE);
    }
}


LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
        if (kbStruct->vkCode == 'B') {
         
            ChangeWindowBackgroundColor(GetForegroundWindow());
        }
    }
    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}



void SolveQuadraticEquation(HWND hWnd) {
    wchar_t aBuffer[100], bBuffer[100], cBuffer[100];
    GetWindowText(hAEdit, aBuffer, sizeof(aBuffer));
    GetWindowText(hBEdit, bBuffer, sizeof(bBuffer));
    GetWindowText(hCEdit, cBuffer, sizeof(cBuffer));

    double a = _wtof(aBuffer);
    double b = _wtof(bBuffer);
    double c = _wtof(cBuffer);

    double discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        SetWindowText(hResultEdit, L"Нет действительных корней");
    }
    else if (discriminant == 0) {
        double root = -b / (2 * a);
        SetWindowText(hResultEdit, std::to_wstring(root).c_str());
    }
    else {
        double root1 = (-b + sqrt(discriminant)) / (2 * a);
        double root2 = (-b - sqrt(discriminant)) / (2 * a);
        SetWindowText(hResultEdit, (std::to_wstring(root1) + L", " + std::to_wstring(root2)).c_str());
    }
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
        
            SolveQuadraticEquation(hWnd);
        }
        else if (LOWORD(wParam) == 2) {
         
            ChangeWindowBackgroundColor(hWnd);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
    return 0;
}