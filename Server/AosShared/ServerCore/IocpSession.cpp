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
		//std::cerr << "WSARecv 실패: " << err << std::endl;
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

		// 현재 송신 중이 아니면 바로 보낸다
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
		// 최소 패킷 헤더 길이 (예: 2바이트 고정 길이)
		if (_recvBuffer.GetStoredSize() < 2)
			break;

		// 헤더에서 전체 길이 추출 (예: uint16_t)
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
		_sendQueue.pop(); // 이전 완료된 것 제거

	if (!_sendQueue.empty()) {
		SendInternal(_sendQueue.front()); // 다음 꺼 보냄
	}
	else {
		_sending = false; // 더 이상 보낼 게 없음
	}

	// 외부 확장용 콜백
	OnSendComplete(len);
}
