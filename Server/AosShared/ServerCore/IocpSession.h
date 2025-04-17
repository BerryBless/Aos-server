#pragma once
#include <winsock2.h>
#include <mswsock.h>
#include <atomic>
#include <memory>
#include <queue>
#include <mutex>
#include "RecvBuffer.h"

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
	// ----------------------
	// function: IOCP 이벤트 핸들러 (내부 호출용, 외부에서 직접 호출 금지)
	// ----------------------
	void HandleSend(int len);
	void HandleRecv(const char* data, int len);

public:
	virtual void OnRecv(const char* data, int len) = 0;
	virtual void OnRecvPacket(const char* data, int len) = 0;
	virtual void OnSendComplete(int len) {};
	virtual void OnDisconnected() = 0;

protected:
	// SendQueue
	std::queue<std::vector<char>> _sendQueue;
	std::mutex _sendLock;
	bool _sending = false;

	// RecvBuffer
	RecvBuffer _recvBuffer;
protected:
	void SendInternal(const std::vector<char>& packet);


private:
	SOCKET _socket;
	std::atomic<bool> _connected;
};
