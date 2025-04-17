#pragma once
#include <winsock2.h>
#include <mswsock.h>
#include <atomic>
#include <memory>
#include <queue>
#include <mutex>
#include "RecvBuffer.h"

// ----------------------
// enum   : OperationType - Overlapped �۾� ����
// ----------------------
enum class OperationType {
	Recv,
	Send,
	Accept
};

// ----------------------
// struct : OverlappedEx - Overlapped Ȯ�� ����
// ----------------------
struct OverlappedEx {
	OVERLAPPED overlapped = {};
	OperationType type;
	WSABUF wsaBuf;
	char buffer[4096];
};

// ----------------------
// param   : ����
// function: IOCP���� ����ϴ� ���� ���̽� Ŭ����
// return  : ���� ��� Ŭ���̾�Ʈ ����
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
	// function: IOCP �̺�Ʈ �ڵ鷯 (���� ȣ���, �ܺο��� ���� ȣ�� ����)
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
