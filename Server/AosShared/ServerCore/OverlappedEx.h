#pragma once

#include <winsock2.h>
#define INIT_OVERLAB_BUFFER_SIZE 64

class IocpSession;
class IocpListener;

// ----------------------
// enum   : OperationType - Overlapped 작업 구분
// ----------------------
enum class OperationType {
	Recv,
	Send,
	Accept,
	INVALID
};

// ----------------------
// struct : OverlappedEx - Overlapped 확장 구조
// ----------------------
struct OverlappedEx {
	OVERLAPPED overlapped = {};           // 기본 OVERLAPPED 구조체
	OperationType type;                   // Recv / Send / Accept 등 작업 타입
	WSABUF wsaBuf;                        // WSASend / WSARecv 시 사용하는 버퍼
	char buffer[INIT_OVERLAB_BUFFER_SIZE];					  // 수신/송신/AcceptEx용 임시 버퍼

	std::shared_ptr<IocpSession> session; // 세션 객체 (Accept 완료 후 처리용)
	IocpListener* listener = nullptr;     // Accept 이벤트 발생시 처리할 리스너
	OverlappedEx() : type(OperationType::INVALID) {}
	OverlappedEx(OperationType _type) : type(_type) {}
};
