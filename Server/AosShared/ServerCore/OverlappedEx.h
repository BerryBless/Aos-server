#pragma once

#include <winsock2.h>
#include "GlobalPoolManager.h" 
#include "IocpBuffer.h"
#define INIT_OVERLAB_BUFFER_SIZE 64

class IocpSession;
class IocpListener;

// ----------------------
// enum   : OperationType - Overlapped �۾� ����
// ----------------------
enum class OperationType {
	Recv,
	Send,
	Accept,
	INVALID
};

// ----------------------
// struct : OverlappedEx - Overlapped Ȯ�� ����
// ----------------------
struct OverlappedEx {
	OVERLAPPED overlapped = {};           // �⺻ OVERLAPPED ����ü
	OperationType type;                   // Recv / Send / Accept �� �۾� Ÿ��
	WSABUF wsaBuf;                        // WSASend / WSARecv �� ����ϴ� ����
	IocpBuffer* buffer;	  // ������ ���� Ǯ���� ȹ���� ����

	std::shared_ptr<IocpSession> session; // ���� ��ü (Accept �Ϸ� �� ó����)
	IocpListener* listener = nullptr;     // Accept �̺�Ʈ �߻��� ó���� ������

	OverlappedEx() = delete;
	OverlappedEx(OperationType _type = OperationType::INVALID) : type(_type) {
		auto pool = BUFFER_POOL_INSTANCE().GetMyPool();
		buffer = pool->Acquire();
		buffer->SetOwner(pool);
		if (!buffer) {
			std::cerr << "[FATAL] OverlappedEx ���� �� buffer�� nullptr\n";
			std::abort();
		}
		wsaBuf = *buffer->GetWSABUF();  // WSABUF ����ü ����
	}
	~OverlappedEx()
	{
		if (buffer)
			buffer->GetOwner()->Release(buffer);
		else {
			std::cerr << "[OverlappedEx] buffer=null, ������ ����\n";
		}
	}
};
