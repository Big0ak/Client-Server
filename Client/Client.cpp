#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "NetworkClient.h"
#include <iostream>
#include <string>
#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

int main(int argc, char* argv[]) {
	if (argc != 6) {
		cout << "Incorrect parameters";
		return 0;
	}

	Client* client = new Client();
	client->initSocketTCP(argv[1], argv[2], argv[5]);
	client->initSocketUDP(argv[1], argv[3]);

	string name = string(argv[4]).substr(string(argv[4]).rfind('\\') + 1);
	client->sendTCP(string(argv[3]) + ";" + name);
	
	Sleep(10);
	client->sendFileUDP(argv[4]);

	delete client;
	return 0;
}