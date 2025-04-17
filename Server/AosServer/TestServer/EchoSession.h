#pragma once
#include "IocpSession.h"
#include <iostream>

class EchoSession : public IocpSession {
protected:
	void OnRecv(const char* data, int len) override {
		std::cout << "[Recv] " << std::string(data, len) << std::endl;
		PostSend(data, len); // ���� ������ �״�� ����
	}

	void OnSend(int len) override {
		std::cout << "[Send] " << len << " bytes sent" << std::endl;
	}

	void OnDisconnected() override {
		std::cout << "[Disconnect] Ŭ���̾�Ʈ ���� ����" << std::endl;
	}
};
