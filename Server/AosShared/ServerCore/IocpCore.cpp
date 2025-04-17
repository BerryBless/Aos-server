#include "pch.h"
#include "IocpCore.h"
#include <iostream>

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

		BOOL result = ::GetQueuedCompletionStatus(
			_iocpHandle,
			&bytesTransferred,
			&completionKey,
			&overlapped,
			INFINITE);

		if (!result) {
			if (overlapped == nullptr) continue; // ���� �̺�Ʈ �ƴ�
			std::cerr << "GQCS failed: " << GetLastError() << std::endl;
			continue;
		}

		if (overlapped) {
			// ���⿡ ���߿� OverlappedEx ó�� �߰� ����
		}
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
