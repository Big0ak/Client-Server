#pragma once
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <string>
#include <fstream>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

class Client {
public:
	~Client();

	void closeSockets();

	void initSocketTCP(char* address, char* port, char* timeout);
	int receiveDataTCP(char* recvbuf);
	void sendTCP(string sendBuffer);

	void initSocketUDP(char* address, char* port);
	void sendFileUDP(string path);
	void sendUDP(char* sendBuffer);

private:
	const int LEN_DATA = 512;
	const int LEN_ID = 8;
	const int MAX_SIZE_FILE = 99999999; // ~95 Ìבאיע

	SOCKET SocketTCP, SocketUDP;
	SOCKADDR_IN serverUDP;

	void error(string err);
};
