#include "NetworkClient.h"

void Client::closeSockets() {
	if (SocketTCP != INVALID_SOCKET) {
		shutdown(SocketTCP, 0);
		closesocket(SocketTCP);
	}
	if (SocketUDP != INVALID_SOCKET) {
		shutdown(SocketUDP, 0);
		closesocket(SocketUDP);
	}
	WSACleanup();
}

Client::~Client() {
	closeSockets();
}

void Client::initSocketTCP(char* address, char* port, char* timeout) {
	WSADATA wsaData;

	SocketTCP = INVALID_SOCKET;
	int result;

	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		error("WSAStartup failed: " + result);
	}

	SocketTCP = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SocketTCP == INVALID_SOCKET) {
		error("Socket creation failed");
	}

	SOCKADDR_IN serverTCP;
	ZeroMemory(&serverTCP, sizeof(serverTCP));
	serverTCP.sin_family = AF_INET;
	serverTCP.sin_port = atoi(port);
	inet_pton(AF_INET, address, &serverTCP.sin_addr);

	result = connect(SocketTCP, (SOCKADDR*)&serverTCP, sizeof(serverTCP));
	if (result == SOCKET_ERROR) {
		error("Binding socket failed");
	}
	cout << "Connected!\n";

	int time = atoi(timeout);
	setsockopt(SocketTCP, SOL_SOCKET, SO_RCVTIMEO, (char*)&time, sizeof(time));
}

void Client::initSocketUDP(char* address, char* port) {
	WSADATA wsaData;

	SocketUDP = INVALID_SOCKET;
	int result;

	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		error("WSAStartup failed: " + result);
	}

	SocketUDP = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (SocketUDP == INVALID_SOCKET) {
		error("Socket creation failed");
	}

	ZeroMemory(&serverUDP, sizeof(serverUDP));
	serverUDP.sin_family = AF_INET;
	serverUDP.sin_port = atoi(port);
	inet_pton(AF_INET, address, &serverUDP.sin_addr);
}

int Client::receiveDataTCP(char* recvbuf) {
	int result;
	ZeroMemory(recvbuf, sizeof(recvbuf));
	result = recv(SocketTCP, recvbuf, LEN_DATA, 0);
	if (result == 0) {
		error("Connection closing...");
	}
	return result;
}

void Client::sendTCP(string sendBuffer) {
	int result;
	result = send(SocketTCP, sendBuffer.c_str(), LEN_DATA, 0);
	if (result == SOCKET_ERROR) {
		error("Faild to send data TCP");
	}
}

void Client::sendUDP(char* sendBuffer){
	int result;
	result = sendto(SocketUDP, sendBuffer, LEN_DATA, 0, (SOCKADDR*)&serverUDP, sizeof(serverUDP));
	if (result == SOCKET_ERROR) {
		error("Faild to send data UDP");
	}
}

void Client::sendFileUDP(string path) {
	ifstream file;
	file.open(path, ios::in | ios::binary);
	char* bytes = new char[LEN_DATA];
	ZeroMemory(bytes, sizeof(bytes));

	file.seekg(0, ios::end);
	int lenFile = file.tellg();
	file.seekg(0, ios::beg);

	if (lenFile > MAX_SIZE_FILE) {
		printf("the file size is too large");
		return;
	}

	// передаваемых данных в одном пакете UDP 504 байта
	// последние 8 байат занимает id паакета
	const int LEN_FILE_DATA = LEN_DATA - LEN_ID;

	if (file.is_open()) {
		while (file) {
			ZeroMemory(bytes, sizeof(bytes));
			if (lenFile >= LEN_FILE_DATA)
				file.read(bytes, LEN_FILE_DATA);
			else
				file.read(bytes, lenFile+1);


			// id пакета это размер файла, который осталось переслась
			// благодаря этому можно корректно обрабатывать данные на сервере, не имея какой-либо
			// дополнительной(искусственной) идентификации покетов и
			// не зная заранее размер всего файла до того как начнется начнется отправка самого файла.
			// запись id в последние 8 бит сообщения
			string len_str = to_string(lenFile);
			for (int i = len_str.length(); i >= 0; i--) 
				bytes[i + LEN_FILE_DATA] = len_str[i];

			int proof_id = -1;
			do {				
				sendUDP(bytes); 

				char* proof = new char[LEN_DATA];
				ZeroMemory(proof, sizeof(proof));	
				receiveDataTCP(proof);
				proof_id = atoi(proof);

				//cout << lenFile << " " << proof_id << endl;
			} while (proof_id != lenFile);

			lenFile -= LEN_FILE_DATA;
			ZeroMemory(bytes, sizeof(bytes));
		}
	}
	file.close();
}

void Client::error(string err) {
	printf(err.c_str());
	closeSockets();
	exit(1);
}