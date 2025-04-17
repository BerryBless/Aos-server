#include "pch.h"
#include "IocpHandle.h"
#include "IocpCore.h"
#include "EchoSession.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int main() {
	// Winsock 초기화
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// 리스닝 소켓 생성
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9001);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(listenSocket, (sockaddr*)&addr, sizeof(addr));
	listen(listenSocket, SOMAXCONN);

	std::cout << "[Listen] 포트 9001 대기 중..." << std::endl;

	// IOCP Core 시작
	IocpCore core;
	core.Initialize(2);
	core.Run();

	while (true) {
		SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
		std::cout << "[Accept] 클라이언트 연결됨" << std::endl;

		auto* session = new EchoSession();
		session->BindSocket(clientSocket);

		if (!core.Register(session)) {
			std::cerr << "[Error] IOCP Register 실패" << std::endl;
			delete session;
			continue;
		}

		session->PostRecv();
	}

	closesocket(listenSocket);
	WSACleanup();
	return 0;
}
