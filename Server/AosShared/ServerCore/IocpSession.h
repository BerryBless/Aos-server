#pragma once
#include <winsock2.h>
#include <mswsock.h>
#include <atomic>
#include <memory>

// ----------------------
// enum   : OperationType - Overlapped 작업 구분
// ----------------------
enum class OperationType {
	Recv,
	Send,
	Accept
};

// ----------------------
// struct : OverlappedEx - Overlapped 확장 구조
// ----------------------
struct OverlappedEx {
	OVERLAPPED overlapped = {};
	OperationType type;
	WSABUF wsaBuf;
	char buffer[4096];
};

// ----------------------
// param   : 없음
// function: IOCP에서 사용하는 세션 베이스 클래스
// return  : 소켓 기반 클라이언트 세션
// ----------------------
class IocpSession {
public:
	IocpSession();
	virtual ~IocpSession();

	void BindSocket(SOCKET socket);
	void Disconnect();

	void PostRecv();
	void PostSend(const char* data, int len);

	SOCKET GetSocket() const;

public:
	virtual void OnRecv(const char* data, int len) = 0;
	virtual void OnSend(int len) = 0;
	virtual void OnDisconnected() = 0;

private:
	SOCKET _socket;
	std::atomic<bool> _connected;
};
