#include "pch.h"
#include "IocpSession.h"
#include <iostream>
#include "GlobalPoolManager.h"

IocpSession::IocpSession()
	: _socket(INVALID_SOCKET), _connected(false)
{
	ServerStat::IncClient(); // Ŭ���̾�Ʈ �� ����
}

IocpSession::~IocpSession()
{
	Disconnect();
	ServerStat::DecClient(); // Ŭ���̾�Ʈ �� ����
}

// ----------------------
// function: ���� ���ε� (���� ���� �� ȣ��)
// ----------------------
void IocpSession::BindSocket(SOCKET socket) {
	_socket = socket;
	_connected.store(true);
}

// ----------------------
// function: ���� ���� ó��
// ----------------------
void IocpSession::Disconnect() {
	if (_connected.exchange(false)) {
		closesocket(_socket);
		OnDisconnected();
	}
}

// ----------------------
// function: ���� �ڵ� ��ȯ
// return  : SOCKET
// ----------------------
SOCKET IocpSession::GetSocket() const {
	return _socket;
}

// ----------------------
// function: �񵿱� ���� ��û
// ----------------------
void IocpSession::PostRecv() {
	if (!_connected) return;

	auto* overlapped = new OverlappedEx(OperationType::Recv);

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
		SAFE_DELETE(overlapped);
		Disconnect();
	}
}

// ----------------------
// param   : data - ������ ������
// param   : len - ������ ����
// function: lock-free ���� ��û
// ----------------------
void IocpSession::PostSend(const char* data, int len) {
	if (!_connected) return;

	auto* packet = new std::vector<char>(data, data + len);

	if (!_sendQueue.push(packet)) {
		std::cerr << "[SendQueue] push ���� - drop\n";
		delete packet;
		return;
	}

	// ù �����ڸ� ���� ���� ����
	bool expected = false;
	if (_sending.compare_exchange_strong(expected, true)) {
		SendInternalNext();
	}
}

// ----------------------
// param   : packet - ������ ������
// function: ���� WSASend ȣ�� (���� �������� ����)
// ----------------------
void IocpSession::SendInternal(const std::vector<char>& packet) {
	auto* overlapped = new OverlappedEx(OperationType::Send);
	overlapped->type = OperationType::Send;
	memcpy(overlapped->buffer->GetData(), packet.data(), packet.size());
	overlapped->wsaBuf.buf = overlapped->buffer->GetData();
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
		SAFE_DELETE(overlapped);
		Disconnect();
	}
}

// ----------------------
// function: ���� ť���� ���� ��Ŷ ���� ����
// ----------------------
void IocpSession::SendInternalNext() {
	std::vector<char>* packet = nullptr;

	// ���� ������ �����Ͱ� ������ _sending false�� ����
	if (!_sendQueue.pop(packet)) {
		_sending.store(false);
		return;
	}

	auto* overlapped = new OverlappedEx(OperationType::Send);
	memcpy(overlapped->buffer->GetData(), packet->data(), packet->size());
	overlapped->wsaBuf.buf = overlapped->buffer->GetData();
	overlapped->wsaBuf.len = static_cast<ULONG>(packet->size());

	delete packet;

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
		SAFE_DELETE(overlapped);
		Disconnect();
	}
}

// ----------------------
// function: ���� ������ ó�� ����
// ----------------------
void IocpSession::HandleRecv(const char* data, int len) {
	if (!data) {
		std::cerr << "[ERROR] HandleRecv: data is null\n";
	}
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

		ServerStat::IncPacket();

		OnRecvPacket(_recvBuffer.Peek(), packetSize);
		_recvBuffer.Consume(packetSize);
	}
}

// ----------------------
// param   : len - ���� ���� �Ϸ�� ����Ʈ ��
// function: ���� �Ϸ� �ڵ鸵 �� ���� ����
// ----------------------
void IocpSession::HandleSend(int len) {
	SendInternalNext(); // ���� ��Ŷ ����
	OnSendComplete(len);
}
