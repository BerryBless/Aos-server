#pragma once

#include <winsock2.h>
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
	char buffer[INIT_OVERLAB_BUFFER_SIZE];					  // ����/�۽�/AcceptEx�� �ӽ� ����

	std::shared_ptr<IocpSession> session; // ���� ��ü (Accept �Ϸ� �� ó����)
	IocpListener* listener = nullptr;     // Accept �̺�Ʈ �߻��� ó���� ������
	OverlappedEx() : type(OperationType::INVALID) {}
	OverlappedEx(OperationType _type) : type(_type) {}
};
