#include "pch.h"
#include "IocpHandle.h"
#include "IocpCore.h"
#include "EchoSession.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int main() {
	// Winsock �ʱ�ȭ
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// ������ ���� ����
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9001);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(listenSocket, (sockaddr*)&addr, sizeof(addr));
	listen(listenSocket, SOMAXCONN);

	std::cout << "[Listen] ��Ʈ 9001 ��� ��..." << std::endl;

	// IOCP Core ����
	IocpCore core;
	core.Initialize(2);
	core.Run();

	while (true) {
		SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
		std::cout << "[Accept] Ŭ���̾�Ʈ �����" << std::endl;

		auto* session = new EchoSession();
		session->BindSocket(clientSocket);

		if (!core.Register(session)) {
			std::cerr << "[Error] IOCP Register ����" << std::endl;
			delete session;
			continue;
		}

		session->PostRecv();
	}

	closesocket(listenSocket);
	WSACleanup();
	return 0;
}
