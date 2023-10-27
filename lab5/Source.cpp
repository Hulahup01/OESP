#include <windows.h>
#include <tchar.h>
#include <string>

HWND hKeyEdit;
HWND hValueEdit;
HWND hListBox;
HWND hCreateButton;
HWND hChangeButton;
HWND hDeleteButton;
HWND hRefreshButton;

void SaveDataToRegistry(HWND hWnd);
void DeleteDataFromRegistry(HWND hWnd);
void RefreshList(HWND hWnd);
void ChangeDataInRegistry(HWND hWnd);
void WriteToEventLog(const std::wstring& message);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


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
	wc.lpszClassName = L"RegistryManager";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Failed to register window class.", L"Error", MB_ICONERROR | MB_OK);
		return EXIT_FAILURE;
	}

	hWnd = CreateWindow(wc.lpszClassName, L"Registry Manager", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 300, NULL, NULL, wc.hInstance, NULL);

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
			hKeyEdit = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER, 20, 20, 200, 30, hWnd, (HMENU)1, NULL, NULL);
			hValueEdit = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER, 20, 60, 200, 30, hWnd, (HMENU)2, NULL, NULL);
			hListBox = CreateWindow(_T("LISTBOX"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_STANDARD, 240, 20, 200, 160, hWnd, (HMENU)3, NULL, NULL);
			hCreateButton = CreateWindow(_T("BUTTON"), _T("Создать ключ"), WS_CHILD | WS_VISIBLE, 20, 100, 200, 30, hWnd, (HMENU)101, NULL, NULL);
			hChangeButton = CreateWindow(_T("BUTTON"), _T("Изменить значение"), WS_CHILD | WS_VISIBLE, 20, 140, 200, 30, hWnd, (HMENU)102, NULL, NULL);
			hDeleteButton = CreateWindow(_T("BUTTON"), _T("Удалить ключ/значение"), WS_CHILD | WS_VISIBLE, 20, 180, 200, 30, hWnd, (HMENU)103, NULL, NULL);
			hRefreshButton = CreateWindow(_T("BUTTON"), _T("Обновить"), WS_CHILD | WS_VISIBLE, 240, 180, 200, 30, hWnd, (HMENU)104, NULL, NULL);
			RefreshList(hWnd);

			break;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case 101:
				{
					SaveDataToRegistry(hWnd);
					break;
				}

				case 102:
				{
					ChangeDataInRegistry(hWnd);
					break;
				}

				case 103:
				{
					DeleteDataFromRegistry(hWnd);
					break;
				}
				break;


				case 104:
				{
					RefreshList(hWnd);
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
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
	}

	return 0;
}


void SaveDataToRegistry(HWND hWnd) {
	TCHAR keyBuffer[256];
	TCHAR valueBuffer[256];
	GetWindowText(hKeyEdit, keyBuffer, 256);
	GetWindowText(hValueEdit, valueBuffer, 256);

	HKEY hKey;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\OESP", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
		if (RegSetValueEx(hKey, keyBuffer, 0, REG_SZ, (LPBYTE)valueBuffer, (lstrlen(valueBuffer) + 1) * sizeof(TCHAR)) == ERROR_SUCCESS) {
			MessageBox(hWnd, L"Ключ и значение успешно созданы в реестре.", L"Успех", MB_ICONINFORMATION);
			TCHAR msg[512];
			_stprintf_s(msg, L"Ключ и значение успешно созданы в реестре: %s", keyBuffer);
			WriteToEventLog(msg);
		}
		else {
			MessageBox(hWnd, L"Не удалось создать значение в реестре.", L"Ошибка", MB_ICONERROR);
		}

		RegCloseKey(hKey);
	}
	else {
		MessageBox(hWnd, L"Не удалось открыть или создать ключ в реестре.", L"Ошибка", MB_ICONERROR);
	}
	RefreshList(hWnd);
}


void RefreshList(HWND hWnd) {

	SendMessage(hListBox, LB_RESETCONTENT, 0, 0);

	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\OESP", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		TCHAR valueName[256];
		DWORD valueNameSize = 256;
		DWORD index = 0;

		while (RegEnumValue(hKey, index, valueName, &valueNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {

			TCHAR value[256];
			DWORD valueSize = 256;
			if (RegQueryValueEx(hKey, valueName, NULL, NULL, (LPBYTE)value, &valueSize) == ERROR_SUCCESS) {
			
				TCHAR listItem[512];
				_stprintf_s(listItem, L"%s  -->  %s", valueName, value);
				SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)listItem);
			}

			valueNameSize = 256;
			index++;
		}

		RegCloseKey(hKey);
	}

}

void DeleteDataFromRegistry(HWND hWnd) {
	TCHAR keyBuffer[256];
	GetWindowText(hKeyEdit, keyBuffer, 256);

	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\OESP", 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
		if (RegDeleteValue(hKey, keyBuffer) == ERROR_SUCCESS) {
			MessageBox(hWnd, L"Ключ и значение успешно удалены из реестра.", L"Успех", MB_ICONINFORMATION);
			TCHAR msg[512];
			_stprintf_s(msg, L"Ключ и значение успешно удалены из реестра: %s", keyBuffer);
			WriteToEventLog(msg);
		}
		else {
			MessageBox(hWnd, L"Не удалось удалить значение из реестра.", L"Ошибка", MB_ICONERROR);
		}

		RegCloseKey(hKey);
	}
	else {
		MessageBox(hWnd, L"Не удалось открыть ключ в реестре.", L"Ошибка", MB_ICONERROR);
	}

	RefreshList(hWnd);
}

void ChangeDataInRegistry(HWND hWnd) {
	TCHAR keyBuffer[256];
	TCHAR valueBuffer[256];
	GetWindowText(hKeyEdit, keyBuffer, 256);
	GetWindowText(hValueEdit, valueBuffer, 256);

	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\OESP", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
		if (RegSetValueEx(hKey, keyBuffer, 0, REG_SZ, (LPBYTE)valueBuffer, (lstrlen(valueBuffer) + 1) * sizeof(TCHAR)) == ERROR_SUCCESS) {
			MessageBox(hWnd, L"Значение успешно изменено в реестре.", L"Успех", MB_ICONINFORMATION);
			TCHAR msg[512];
			_stprintf_s(msg, L"Значение успешно изменено в реестре: %s", keyBuffer);
			WriteToEventLog(msg);
		}
		else {
			MessageBox(hWnd, L"Не удалось изменить значение в реестре.", L"Ошибка", MB_ICONERROR);
		}

		RegCloseKey(hKey);
	}
	else {
		MessageBox(hWnd, L"Не удалось открыть ключ в реестре.", L"Ошибка", MB_ICONERROR);
	}

	RefreshList(hWnd);
}


void WriteToEventLog(const std::wstring& message) {
	HANDLE hEventLog = RegisterEventSource(NULL, L"Registry Manager");

	if (hEventLog) {
		LPCWSTR messageStrings[1];
		messageStrings[0] = message.c_str();

		ReportEvent(hEventLog, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, messageStrings, NULL);

		DeregisterEventSource(hEventLog);
	}
}