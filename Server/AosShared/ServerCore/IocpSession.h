#pragma once
#include <winsock2.h>
#include <mswsock.h>
#include <atomic>
#include <memory>

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
	virtual void OnRecv(const char* data, int len) = 0;
	virtual void OnSend(int len) = 0;
	virtual void OnDisconnected() = 0;

private:
	SOCKET _socket;
	std::atomic<bool> _connected;
};
