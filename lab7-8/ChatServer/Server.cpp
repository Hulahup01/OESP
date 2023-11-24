#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

#include <WinSock2.h>
#include <iostream>
#include <string>

#define PORT 1234

SOCKET Connections[100];
int Counter = 0;

void ClientHandler(int index) {
	int msg_size;
	while (true) {
		int result = recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		if (result <= 0) {
			std::cout << "Client disconnected" << std::endl;
			closesocket(Connections[index]);
			return; 
		}

		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		result = recv(Connections[index], msg, msg_size, NULL);
		if (result <= 0) {
			std::cout << "Client disconnected" << std::endl;
			closesocket(Connections[index]);			
			delete[] msg;
			return;
		}

		for (int i = 0; i < Counter; i++) {
			send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
			send(Connections[i], msg, msg_size, NULL);
		}
		delete[] msg;
	}
}

int main(int argc, char* argv[]) {
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (int res = WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error: DLL didn't load" << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(PORT);
	addr.sin_family = AF_INET;

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	if (sListen == 0) {
		std::cout << "Error: Socket error" << std::endl;
		exit(sListen);
	}

	if (int res = bind(sListen, (SOCKADDR*)&addr, sizeof(addr)) == -1) {
		std::cout << "Error: Bind error" << std::endl;
		exit(res);
	}

	if (int res = listen(sListen, SOMAXCONN) != 0) {
		std::cout << "Error: Listen Error" << std::endl;
		exit(res);
	}

	SOCKET newConnection;
	int sizeOfAddr = sizeof(addr);
	for (int i = 0; i < 100; i++) {
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeOfAddr);

		if (newConnection == 0) {
			std::cout << "Error: Invalid socket" << std::endl;
		}
		else {
			std::string msg = "===============CHAT===============";
			int msg_size = msg.size();
			send(newConnection, (char*)&msg_size, sizeof(int), NULL);
			send(newConnection, msg.c_str(), msg_size, NULL);

			std::cout << "Client Connected" << std::endl;
			Connections[i] = newConnection;
			Counter++;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);
		}
	}

	system("pause");

	return 0;
}

