#include "pch.h"
#include "IocpCore.h"
#include <iostream>
#include "IocpSession.h"
#include "IocpListener.h"

// ----------------------
// param   : ����
// function: ������ - IOCP �ڵ� �ʱ�ȭ
// return  : ����
// ----------------------
IocpCore::IocpCore()
	: _iocpHandle(nullptr), _shutdown(false), _workerCount(0) {
}

// ----------------------
// param   : ����
// function: �Ҹ��� - ������ ����
// return  : ����
// ----------------------
IocpCore::~IocpCore() {
	Shutdown();
}

// ----------------------
// param   : threadCount - ������ ��Ŀ ������ ��
// function: IOCP �ڵ� ���� �� ������ �ʱ�ȭ
// return  : ���� ����
// ----------------------
bool IocpCore::Initialize(int threadCount) {
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (_iocpHandle == NULL) {
		std::cerr << "CreateIoCompletionPort failed: " << GetLastError() << std::endl;
		return false;
	}

	_workerCount = threadCount;
	for (int i = 0; i < threadCount; ++i) {
		_workerThreads.emplace_back([this]() { WorkerThread(); });
	}

	return true;
}

// ----------------------
// param   : ����
// function: IOCP ��Ŀ ������ ����
// return  : ����
// ----------------------
void IocpCore::Run()
{
	for (int i = 0; i < _workerCount; ++i) {
		_workerThreads.emplace_back([this]() { WorkerThread(); });
	}
}

// ----------------------
// param   : ����
// function: Worker ������ ���� GQCS ����
// return  : ����
// ----------------------
void IocpCore::WorkerThread() {
	while (!_shutdown) {
		DWORD bytesTransferred = 0;
		ULONG_PTR completionKey = 0;
		LPOVERLAPPED overlapped = nullptr;

		BOOL result = GetQueuedCompletionStatus(
			_iocpHandle,
			&bytesTransferred,
			&completionKey,
			&overlapped,
			INFINITE
		);

		IocpSession* session = reinterpret_cast<IocpSession*>(completionKey);
		OverlappedEx* ex = reinterpret_cast<OverlappedEx*>(overlapped);

		if (!result || bytesTransferred == 0) {
			session->Disconnect();
			SAFE_DELETE(ex);
			continue;
		}
		switch (ex->type) {
		case OperationType::Recv:
			session->HandleRecv(ex->buffer->GetData(), bytesTransferred);
			session->PostRecv();
			break;
		case OperationType::Send:
			session->HandleSend(bytesTransferred);
			if (!ex->buffer)
			break;
		case OperationType::Accept:
			if (ex->listener)
			{
				ex->listener->OnAccept(ex);
			}
			break;
		}

		SAFE_DELETE(ex);
		//SAFE_RELEASE_AND_DELETE(ex);
	}
}


// ----------------------
// param   : handle - ����� ����/���� �ڵ�
// function: IOCP �ڵ鿡 ��� �ڵ��� ���
// return  : ���� ����
// ----------------------
bool IocpCore::Register(HANDLE handle) {
	HANDLE result = ::CreateIoCompletionPort(handle, _iocpHandle, 0, 0);
	return (result == _iocpHandle);
}

bool IocpCore::Register(IocpSession* session)
{
	HANDLE result = ::CreateIoCompletionPort(
		reinterpret_cast<HANDLE>(session->GetSocket()), // ���� �ڵ�
		_iocpHandle,
		reinterpret_cast<ULONG_PTR>(session),           // ���� �����͸� completionKey��
		0
	);

	return (result == _iocpHandle);

}
bool IocpCore::Register(std::shared_ptr<IocpSession> session)
{
	return Register(session.get()); // ���� raw ������ ���� ����
}
// ----------------------
// param   : ����
// function: IOCP �ڵ� ��ȯ
// return  : HANDLE ��
// ----------------------
HANDLE IocpCore::GetHandle() const {
	return _iocpHandle;
}

// ----------------------
// param   : ����
// function: ��� ��Ŀ ������ ����
// return  : ����
// ----------------------
void IocpCore::Shutdown() {
	_shutdown = true;
	for (int i = 0; i < _workerCount; ++i) {
		::PostQueuedCompletionStatus(_iocpHandle, 0, 0, NULL);
	}
	for (auto& t : _workerThreads) {
		if (t.joinable())
			t.join();
	}
	_workerThreads.clear();

	if (_iocpHandle)
		::CloseHandle(_iocpHandle);
}
