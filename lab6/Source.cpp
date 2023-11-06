#include <windows.h>
#include <tchar.h>
#include <string>
#include <vector>


struct Account {
    int balance = 0;


    bool Spend(int value) {
        if (value > balance)
            return false;

        balance -= value;
  
        return true;
    }
};

Account account{100000};


HWND hEditResult;
HWND hButtonSpendBalanceA;
HWND hButtonSpendBalanceS;
HANDLE hMutex;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void SpendBalanceA(int* total_spent);
void SpendBalanceS(int* total_spent);
DWORD WINAPI ThreadFuncA(LPVOID lpParam);
DWORD WINAPI ThreadFuncS(LPVOID lpParam);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MSG msg;
	HWND hWnd;
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
	wc.lpszClassName = L"Bank";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Failed to register window class.", L"Error", MB_ICONERROR | MB_OK);
		return EXIT_FAILURE;
	}

	hMutex = CreateMutex(NULL, FALSE, NULL);

	hWnd = CreateWindow(wc.lpszClassName, L"Bank thread", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 300, NULL, NULL, wc.hInstance, NULL);

	if (hWnd == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, L"Failed to create a window.", L"Error", MB_ICONERROR);
		return EXIT_FAILURE;
	}


	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
		case WM_CREATE:
		{
			hEditResult = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_READONLY | WS_BORDER, 20, 20, 400, 80, hWnd, (HMENU)1, NULL, NULL);
			hButtonSpendBalanceA = CreateWindow(L"BUTTON", L"Потратить (-синхронизация)", WS_CHILD | WS_VISIBLE, 20, 150, 200, 30, hWnd, (HMENU)101, NULL, NULL);
			hButtonSpendBalanceS = CreateWindow(L"BUTTON", L"Потратить (+синхронизация)", WS_CHILD | WS_VISIBLE, 20, 200, 200, 30, hWnd, (HMENU)102, NULL, NULL);


			break;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{

				case 101:
				{
					account.balance = 100000;
					SetWindowText(hEditResult, L"");
					int res1=0;
					int res2=0;

					HANDLE hThread1 = CreateThread(NULL, 0, ThreadFuncA, &res1, 0, NULL);
					HANDLE hThread2 = CreateThread(NULL, 0, ThreadFuncA, &res2, 0, NULL);

					WaitForSingleObject(hThread1, INFINITE);
					WaitForSingleObject(hThread2, INFINITE);

					CloseHandle(hThread1);
					CloseHandle(hThread2);

					SetWindowText(hEditResult, (L"[-] Потрачено:" + std::to_wstring(res1 + res2) + L"   Баланс:" + std::to_wstring(account.balance)).c_str());
					break;
				}

				case 102:
				{
					account.balance = 100000;
					SetWindowText(hEditResult, L"");
					int res1 = 0;
					int res2 = 0;

					HANDLE hThread1 = CreateThread(NULL, 0, ThreadFuncS, &res1, 0, NULL);
					HANDLE hThread2 = CreateThread(NULL, 0, ThreadFuncS, &res2, 0, NULL);

					WaitForSingleObject(hThread1, INFINITE);
					WaitForSingleObject(hThread2, INFINITE);

					CloseHandle(hThread1);
					CloseHandle(hThread2);

					SetWindowText(hEditResult, (L"[+] Потрачено:" + std::to_wstring(res1 + res2) + L"   Баланс:" + std::to_wstring(account.balance)).c_str());
					break;
				}

				break;
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
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
	}

	return 0;
}

void SpendBalanceA(int* total_spent) {
	*total_spent = 0;
	for (int i = 0; i < 100000; i++) {
		if(account.Spend(1))
			(*total_spent)++;
	}

}


void SpendBalanceS(int* total_spent) {
	WaitForSingleObject(hMutex, INFINITE);
	*total_spent = 0;
	for (int i = 0; i < 100000; i++) {
		if (account.Spend(1))
			(*total_spent)++;
	}

	ReleaseMutex(hMutex);
}

DWORD WINAPI ThreadFuncA(LPVOID lpParam) {
	int* result = reinterpret_cast<int*>(lpParam);
	SpendBalanceA(result);

	return 0;
}

DWORD WINAPI ThreadFuncS(LPVOID lpParam) {
	int* result = reinterpret_cast<int*>(lpParam);
	SpendBalanceS(result);

	return 0;
}


