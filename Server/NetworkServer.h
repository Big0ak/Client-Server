#pragma once
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <string>
#include <fstream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>

using namespace std;

class Server {
public:
	~Server();

	void closeSockets();

	void initSocketTCP(char* address, char* port);
	bool acceptNewClientTCP();
	int receiveDataTCP(char* recvbuf);
	void sendTCP(string sendBuffer);
	
	void initSocketUDP(char* address, string port);
	int receiveDataUDP(char* recvbuf);
	bool receiveFileUDP(string path);

private:
	const int LEN_DATA = 512;
	const int LEN_ID = 8;

	SOCKET ListenSocketTCP, ListenSocketUDP;
	SOCKET ClientSocketTCP;

	void error(string err);
};
