#pragma once

#include <winsock2.h>
#include "GlobalPoolManager.h" 
#include "IocpBuffer.h"
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
	IocpBuffer* buffer;	  // 스레드 로컬 풀에서 획득한 버퍼

	std::shared_ptr<IocpSession> session; // 세션 객체 (Accept 완료 후 처리용)
	IocpListener* listener = nullptr;     // Accept 이벤트 발생시 처리할 리스너

	OverlappedEx() = delete;
	OverlappedEx(OperationType _type = OperationType::INVALID) : type(_type) {
		auto pool = BUFFER_POOL_INSTANCE().GetMyPool();
		buffer = pool->Acquire();
		buffer->SetOwner(pool);
		if (!buffer) {
			std::cerr << "[FATAL] OverlappedEx 생성 시 buffer가 nullptr\n";
			std::abort();
		}
		wsaBuf = *buffer->GetWSABUF();  // WSABUF 구조체 복사
	}
	~OverlappedEx()
	{
		if (buffer)
			buffer->GetOwner()->Release(buffer);
		else {
			std::cerr << "[OverlappedEx] buffer=null, 릴리즈 안함\n";
		}
	}
};
