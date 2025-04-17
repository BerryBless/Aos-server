#pragma once
#include "IocpSession.h"
#include <iostream>

class EchoSession : public IocpSession {
protected:
	void OnRecv(const char* data, int len) override {
		//std::cout << "[Recv] " << std::string(data, len) << std::endl;
		PostSend(data, len); // 받은 데이터 그대로 전송
	}


	void OnRecvPacket(const char* data, int len) {
		// 받은 패킷을 그대로 다시 보내기 (에코)
		PostSend(data, len);
	}

	void OnDisconnected() override {
		std::cout << "[Disconnect] 클라이언트 연결 종료" << std::endl;
	}
};
