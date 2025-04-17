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
	if (ret == SOCKET_ERROR) {
		int err = WSAGetLastError();
		//std::cerr << "WSARecv ����: " << err << std::endl;
	}

	if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
		delete overlapped;
		Disconnect();
	}
}

void IocpSession::PostSend(const char* data, int len) {
	if (!_connected) return;

	std::vector<char> packet(data, data + len);
	bool sendNow = false;

	{
		std::lock_guard<std::mutex> lock(_sendLock);

		_sendQueue.push(std::move(packet));

		// ���� �۽� ���� �ƴϸ� �ٷ� ������
		if (!_sending) {
			_sending = true;
			sendNow = true;
		}
	}

	if (sendNow) {
		std::lock_guard<std::mutex> lock(_sendLock);
		if (!_sendQueue.empty()) {
			SendInternal(_sendQueue.front());
		}
	}
}


void IocpSession::SendInternal(const std::vector<char>& packet) {
	auto* overlapped = new OverlappedEx();
	overlapped->type = OperationType::Send;
	memcpy(overlapped->buffer, packet.data(), packet.size());
	overlapped->wsaBuf.buf = overlapped->buffer;
	overlapped->wsaBuf.len = static_cast<ULONG>(packet.size());

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


void IocpSession::HandleRecv(const char* data, int len) {
	if (!_recvBuffer.Write(data, len)) {
		Disconnect();
		return;
	}

	while (true) {
		// �ּ� ��Ŷ ��� ���� (��: 2����Ʈ ���� ����)
		if (_recvBuffer.GetStoredSize() < 2)
			break;

		// ������� ��ü ���� ���� (��: uint16_t)
		uint16_t packetSize = *(uint16_t*)_recvBuffer.Peek();
		if (!_recvBuffer.HasCompletePacket(packetSize))
			break;

		OnRecvPacket(_recvBuffer.Peek(), packetSize);
		_recvBuffer.Consume(packetSize);
	}
}

void IocpSession::HandleSend(int len) {

	std::lock_guard<std::mutex> lock(_sendLock);
	//std::cout << "[OnSend] " << len << " bytes" << std::endl;

	if (!_sendQueue.empty())
		_sendQueue.pop(); // ���� �Ϸ�� �� ����

	if (!_sendQueue.empty()) {
		SendInternal(_sendQueue.front()); // ���� �� ����
	}
	else {
		_sending = false; // �� �̻� ���� �� ����
	}

	// �ܺ� Ȯ��� �ݹ�
	OnSendComplete(len);
}
