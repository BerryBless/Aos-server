#include "pch.h"
#include "IocpSession.h"
#include <iostream>

IocpSession::IocpSession()
	: _socket(INVALID_SOCKET), _connected(false) {
}

IocpSession::~IocpSession() {
	Disconnect();
}

void IocpSession::BindSocket(SOCKET socket) {
	_socket = socket;
	_connected.store(true);
}

void IocpSession::Disconnect() {
	if (_connected.exchange(false)) {
		closesocket(_socket);
		OnDisconnected();
	}
}

SOCKET IocpSession::GetSocket() const {
	return _socket;
}

void IocpSession::PostRecv() {
	if (!_connected) return;

	auto* overlapped = new OverlappedEx();
	overlapped->type = OperationType::Recv;
	overlapped->wsaBuf.buf = overlapped->buffer;
	overlapped->wsaBuf.len = sizeof(overlapped->buffer);

	DWORD flags = 0;
	DWORD recvBytes = 0;

	int ret = WSARecv(
		_socket,
		&overlapped->wsaBuf,
		1,
		&recvBytes,
		&flags,
		(LPWSAOVERLAPPED)overlapped,
		NULL
	);

	if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
		delete overlapped;
		Disconnect();
	}
}

void IocpSession::PostSend(const char* data, int len) {
	if (!_connected) return;

	auto* overlapped = new OverlappedEx();
	overlapped->type = OperationType::Send;
	memcpy(overlapped->buffer, data, len);
	overlapped->wsaBuf.buf = overlapped->buffer;
	overlapped->wsaBuf.len = len;

	DWORD sendBytes = 0;

	int ret = WSASend(
		_socket,
		&overlapped->wsaBuf,
		1,
		&sendBytes,
		0,
		(LPWSAOVERLAPPED)overlapped,
		NULL
	);

	if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
		delete overlapped;
		Disconnect();
	}
}
