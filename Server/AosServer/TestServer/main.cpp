// #include "IocpHandle.h"
// #include "IocpCore.h"
// #include "EchoSession.h"
// #include <winsock2.h>
// #include <ws2tcpip.h>
// #include "ServerStat.h"
// #pragma comment(lib, "ws2_32.lib")
// 
// int main() {
// 	WSADATA wsaData;
// 	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
// 		std::cerr << "[WSAStartup 실패] 코드: " << WSAGetLastError() << std::endl;
// 		return -1;
// 	}
// 	ServerStat::Init();
// 	IocpCore core;
// 	core.Initialize(1); // 워커 스레드 4개
// 	core.Run();
// 
// 	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
// 	sockaddr_in addr = {};
// 	addr.sin_family = AF_INET;
// 	addr.sin_addr.s_addr = htonl(INADDR_ANY);
// 	addr.sin_port = htons(9001);
// 
// 	bind(listenSocket, (sockaddr*)&addr, sizeof(addr));
// 	listen(listenSocket, SOMAXCONN);
// 
// 	while (true) {
// 		SOCKET client = accept(listenSocket, nullptr, nullptr);
// 		auto* session = new EchoSession();
// 		session->BindSocket(client);
// 		core.Register(session);
// 		session->PostRecv();
// 	}
// 
// 	ServerStat::Shutdown();
// 	return 0;
// }


// ----------------------
// file   : main.cpp
// function: boost::lockfree::queue 연결 테스트
// ----------------------

#include <iostream>
#include <boost/lockfree/queue.hpp>

int main() {
	// ----------------------
	// function: 큐 생성 (capacity 128)
	// ----------------------
	boost::lockfree::queue<int> queue(128);

	// ----------------------
	// function: 값 푸시
	// ----------------------
	for (int i = 0; i < 5; ++i) {
		if (queue.push(i)) {
			std::cout << "[Producer] pushed: " << i << std::endl;
		}
		else {
			std::cout << "[Producer] failed to push: " << i << std::endl;
		}
	}

	// ----------------------
	// function: 값 팝
	// ----------------------
	int value;
	while (queue.pop(value)) {
		std::cout << "[Consumer] popped: " << value << std::endl;
	}

	return 0;
}
