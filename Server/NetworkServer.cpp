#include "NetworkServer.h"

void Server::closeSockets() {
	if (ListenSocketTCP != INVALID_SOCKET) {
		shutdown(ListenSocketTCP, 0);
		closesocket(ListenSocketTCP);
	}
	if (ListenSocketUDP != INVALID_SOCKET) {
		shutdown(ListenSocketUDP, 0);
		closesocket(ListenSocketUDP);
	}
	if (ClientSocketTCP != INVALID_SOCKET) {
		shutdown(ClientSocketTCP, 0);
		closesocket(ClientSocketTCP);
	}
	WSACleanup();
}

Server::~Server() {
	closeSockets();
}

void Server::initSocketTCP(char* address, char* port) {
	WSADATA wsaData;

	ListenSocketTCP = INVALID_SOCKET;
	int result;

	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		error("WSAStartup failed: " + result);
	}

	ListenSocketTCP = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocketTCP == INVALID_SOCKET) {
		error("Socket creation failed");
	}

	SOCKADDR_IN serverTCP;
	ZeroMemory(&serverTCP, sizeof(serverTCP));
	serverTCP.sin_family = AF_INET;
	serverTCP.sin_port = atoi(port);
	inet_pton(AF_INET, address, &serverTCP.sin_addr);

	result = bind(ListenSocketTCP, (SOCKADDR*)&serverTCP, sizeof(serverTCP));
	if (result == SOCKET_ERROR) {
		error("Binding socket failed");
	}

	result = listen(ListenSocketTCP, SOMAXCONN);
	if (result == SOCKET_ERROR) {
		error("Listening socket failed");
	}
}

void Server::initSocketUDP(char* address, string port) {
	WSADATA wsaData;
	
	ListenSocketUDP = INVALID_SOCKET;
	int result;

	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		error("WSAStartup failed: " + result);
	}

	ListenSocketUDP = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (ListenSocketUDP == INVALID_SOCKET) {
		error("Socket creation failed");
	}

	SOCKADDR_IN serverUDP;
	ZeroMemory(&serverUDP, sizeof(serverUDP));
	serverUDP.sin_family = AF_INET;
	serverUDP.sin_port = atoi(port.c_str());
	inet_pton(AF_INET, address, &serverUDP.sin_addr);

	result = bind(ListenSocketUDP, (SOCKADDR*)&serverUDP, sizeof(serverUDP));
	if (result == SOCKET_ERROR) {
		error("Binding socket failed");
	}
}

bool Server::acceptNewClientTCP() {
	ClientSocketTCP = accept(ListenSocketTCP, NULL, NULL);
	if (ClientSocketTCP == INVALID_SOCKET) {
		error("Accepting socket failed");
		return false;
	}
	return true;
}

int Server::receiveDataTCP(char* recvbuf) {
	int result;
	ZeroMemory(recvbuf, sizeof(recvbuf));
	result = recv(ClientSocketTCP, recvbuf, LEN_DATA, 0);
	if (result == 0) {
		error("Faild to receiving data TCP");
	}
	return result;
}

void Server::sendTCP(string sendBuffer) {
	int result;
	result = send(ClientSocketTCP, sendBuffer.c_str(), LEN_DATA, 0);
	if (result == SOCKET_ERROR) {
		error("Faild to send data TCP");
	}
}

int Server::receiveDataUDP(char* recvbuf) {
	SOCKADDR_IN clientUDP;
	int result;
	ZeroMemory(recvbuf, sizeof(recvbuf));
	int clientLength = sizeof(clientUDP);
	result = recvfrom(ListenSocketUDP, recvbuf, LEN_DATA, 0, (SOCKADDR*)&clientUDP, &clientLength);
	if (clientLength == SOCKET_ERROR) {
		error("Faild to receiving data UDP");
	}
	return 0;
}

bool Server::receiveFileUDP(string path) {
	ofstream file;
	file.open(path, ios::binary);

	const int LEN_FILE_DATA = LEN_DATA - LEN_ID;

	if (file.is_open()) {

		char* bytes = new char[LEN_DATA];

		if (file.is_open()) {
			int id = -1;
			int id_prev = -1;
			do {
				receiveDataUDP(bytes);
				
				// получение id из полученных данных (послдние 8 байт)
				string id_str;
				for (int i = LEN_FILE_DATA; i < LEN_DATA; i++)
					id_str.push_back(bytes[i]);

				id = atoi(id_str.c_str());

				sendTCP(to_string(id));
				if (id != id_prev) {
					if (id >= LEN_FILE_DATA)
						file.write(bytes, LEN_FILE_DATA);
					else
						file.write(bytes, id);
				}
				
				id_prev = id;
				//cout << "received: " << id << endl;
			} while (id > LEN_FILE_DATA);
		}
		file.close();
	}
	else {
		return false;
	}
	return true;
}

void Server::error(string err){
	printf(err.c_str());
	closeSockets();
	exit(1);
}