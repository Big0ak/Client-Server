#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "NetworkServer.h"
#include <iostream>
#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <direct.h>

using namespace std;

int main(int argc, char* argv[]) {
	if (argc != 4) {
		cout << "Incorrect parameters";
		return 0;
	}

	_mkdir(argv[3]);

	// сервер работает всегда в ожидании клиентов
	while (true) {
		Server* server = new Server();
		server->initSocketTCP(argv[1], argv[2]);
		if (server->acceptNewClientTCP()) {
			std::cout << "Client Connected!\n";
		}

		char recvBuffer[512];
		ZeroMemory(recvBuffer, 512);
		server->receiveDataTCP(recvBuffer);
		cout << "Received data TCP: " << recvBuffer << endl;

		string recv(recvBuffer);
		int pos = recv.find(";");

		string port = recv.substr(0, pos);
		string nameFile = recv.substr(pos + 1);

		server->initSocketUDP(argv[1], port);

		string path = argv[3];
		if (server->receiveFileUDP(path + '\\' + nameFile)) {
			cout << "Filer received" << endl;
		}
		else {
			cout << "Faild to received file" << endl;
		}
		delete server;
	}
	
	return 0;
}