#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

#include <windows.h>
#include <string>
#include <tchar.h>


#define CONNECT_MENU 1
#define SEND_MENU 2
#define PORT 1234

SOCKET connectionSocket;
char* username;
int username_size;

HWND hDisplay;
HWND hUsernameInput;
HWND hConnectButton;
HWND hMessageInput;
HWND hUsernameDisplay;
HWND hSendButton;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void ClientHandler() {
	int msg_size;
	while (true) {
		int result = recv(connectionSocket, (char*)&msg_size, sizeof(int), NULL);
		if (result <= 0) {
			SetWindowTextA(hDisplay, "DISCONNECTED");
			closesocket(connectionSocket);
			return;
		}

		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		result = recv(connectionSocket, msg, msg_size, NULL);
		if (result <= 0) {
			SetWindowTextA(hDisplay, "DISCONNECTED");
			closesocket(connectionSocket);
			delete[] msg;
			return;
		}


		int textLength = GetWindowTextLengthA(hDisplay);
		char* currentText = new char[textLength + 1];
		GetWindowTextA(hDisplay, currentText, textLength + 1);

		if (textLength > 0) {
			std::string combinedText = std::string(currentText) + "\r\n" + msg;
			SetWindowTextA(hDisplay, combinedText.c_str());
		}
		else {
			SetWindowTextA(hDisplay, msg);
		}

		delete[] currentText;
		delete[] msg;

	}
}

void ConnectToServer() {
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (int res = WSAStartup(DLLVersion, &wsaData) != 0) {
		SetWindowText(hDisplay, L"Error: DLL didn't load");
		return;
	}

	SOCKADDR_IN addr;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(PORT);
	addr.sin_family = AF_INET;

	connectionSocket = socket(AF_INET, SOCK_STREAM, NULL);
	if (connect(connectionSocket, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
		SetWindowText(hDisplay, L"Error: Failed connect to server");
		return;
	}
	SetWindowText(hDisplay, L"Connected");

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);	
}

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
	wc.lpszClassName = L"ChatClient";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Failed to register window class.", L"Error", MB_ICONERROR | MB_OK);
		return EXIT_FAILURE;
	}

	hWnd = CreateWindow(wc.lpszClassName, L"CHAT", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 365, 500, NULL, NULL, wc.hInstance, NULL);

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
			hUsernameInput = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 95, 160, 160, 30, hWnd, NULL, NULL, NULL);
			SendMessage(hUsernameInput, EM_SETLIMITTEXT, (WPARAM)32, 0);
			hConnectButton = CreateWindow(L"BUTTON", L"Connect", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 95, 200, 160, 30, hWnd, (HMENU)CONNECT_MENU, NULL, NULL);

		

			break;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case CONNECT_MENU:
					hDisplay = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY, 30, 30, 290, 350, hWnd, NULL, NULL, NULL);
					hMessageInput = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 30, 385, 200, 30, hWnd, NULL, NULL, NULL);
					hUsernameDisplay = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_READONLY, 30, 10, 290, 20, hWnd, NULL, NULL, NULL);
					hSendButton = CreateWindow(L"BUTTON", L"Send", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 235, 385, 85, 30, hWnd, (HMENU)SEND_MENU, NULL, NULL);

					username_size = GetWindowTextLength(hUsernameInput) + 1;
					username = new char[username_size];
					GetWindowTextA(hUsernameInput, username, username_size);
					SetWindowTextA(hUsernameDisplay, username);

					DestroyWindow(hUsernameInput);
					DestroyWindow(hConnectButton);

					ConnectToServer();
					break;
		
				case SEND_MENU:

				
					int msg_size = GetWindowTextLength(hMessageInput) + 1;
					char* msg = new char[msg_size];
					GetWindowTextA(hMessageInput, msg, msg_size);

					std::string full_msg_str = std::string(username) + ": " + std::string(msg);
					int full_msg_size = full_msg_str.length() + 1;
					char* full_msg = new char[full_msg_size];
					strcpy(full_msg, full_msg_str.c_str());

					send(connectionSocket, (char*)&full_msg_size, sizeof(int), NULL);
					send(connectionSocket, full_msg, full_msg_size, NULL);
					SetWindowText(hMessageInput, L"");

					delete[] full_msg;
					delete[] msg;
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