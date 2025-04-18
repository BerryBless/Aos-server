#include "pch.h"
#include "IocpSession.h"
#include <iostream>
#include "GlobalPoolManager.h"

IocpSession::IocpSession()
	: _socket(INVALID_SOCKET), _connected(false)
{
	ServerStat::IncClient(); // 클라이언트 수 증가
}

IocpSession::~IocpSession()
{
	Disconnect();
	ServerStat::DecClient(); // 클라이언트 수 감소
}

// ----------------------
// function: 소켓 바인딩 (접속 수락 후 호출)
// ----------------------
void IocpSession::BindSocket(SOCKET socket) {
	_socket = socket;
	_connected.store(true);
}

// ----------------------
// function: 연결 종료 처리
// ----------------------
void IocpSession::Disconnect() {
	if (_connected.exchange(false)) {
		closesocket(_socket);
		OnDisconnected();
	}
}

// ----------------------
// function: 소켓 핸들 반환
// return  : SOCKET
// ----------------------
SOCKET IocpSession::GetSocket() const {
	return _socket;
}

// ----------------------
// function: 비동기 수신 요청
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
// param   : data - 전송할 데이터
// param   : len - 데이터 길이
// function: lock-free 전송 요청
// ----------------------
void IocpSession::PostSend(const char* data, int len) {
	if (!_connected) return;

	auto* packet = new std::vector<char>(data, data + len);

	if (!_sendQueue.push(packet)) {
		std::cerr << "[SendQueue] push 실패 - drop\n";
		delete packet;
		return;
	}

	// 첫 전송자만 실제 전송 시작
	bool expected = false;
	if (_sending.compare_exchange_strong(expected, true)) {
		SendInternalNext();
	}
}

// ----------------------
// param   : packet - 전송할 데이터
// function: 실제 WSASend 호출 (이전 버전에서 사용됨)
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
// function: 전송 큐에서 다음 패킷 꺼내 전송
// ----------------------
void IocpSession::SendInternalNext() {
	std::vector<char>* packet = nullptr;

	// 다음 전송할 데이터가 없으면 _sending false로 리셋
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
// function: 수신 데이터 처리 루프
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
		// 최소 패킷 헤더 길이 (예: 2바이트 고정 길이)
		if (_recvBuffer.GetStoredSize() < 2)
			break;

		// 헤더에서 전체 길이 추출 (예: uint16_t)
		uint16_t packetSize = *(uint16_t*)_recvBuffer.Peek();
		if (!_recvBuffer.HasCompletePacket(packetSize))
			break;

		ServerStat::IncPacket();

		OnRecvPacket(_recvBuffer.Peek(), packetSize);
		_recvBuffer.Consume(packetSize);
	}
}

// ----------------------
// param   : len - 실제 전송 완료된 바이트 수
// function: 전송 완료 핸들링 및 다음 전송
// ----------------------
void IocpSession::HandleSend(int len) {
	SendInternalNext(); // 다음 패킷 전송
	OnSendComplete(len);
}
