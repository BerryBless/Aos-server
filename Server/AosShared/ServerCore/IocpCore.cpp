#include "pch.h"
#include "IocpCore.h"
#include <iostream>
#include "IocpSession.h"
#include "IocpListener.h"

// ----------------------
// param   : 없음
// function: 생성자 - IOCP 핸들 초기화
// return  : 없음
// ----------------------
IocpCore::IocpCore()
	: _iocpHandle(nullptr), _shutdown(false), _workerCount(0) {
}

// ----------------------
// param   : 없음
// function: 소멸자 - 스레드 종료
// return  : 없음
// ----------------------
IocpCore::~IocpCore() {
	Shutdown();
}

// ----------------------
// param   : threadCount - 생성할 워커 스레드 수
// function: IOCP 핸들 생성 및 스레드 초기화
// return  : 성공 여부
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
// param   : 없음
// function: IOCP 워커 스레드 실행
// return  : 없음
// ----------------------
void IocpCore::Run()
{
	for (int i = 0; i < _workerCount; ++i) {
		_workerThreads.emplace_back([this]() { WorkerThread(); });
	}
}

// ----------------------
// param   : 없음
// function: Worker 스레드 내부 GQCS 루프
// return  : 없음
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
// param   : handle - 등록할 소켓/파일 핸들
// function: IOCP 핸들에 대상 핸들을 등록
// return  : 성공 여부
// ----------------------
bool IocpCore::Register(HANDLE handle) {
	HANDLE result = ::CreateIoCompletionPort(handle, _iocpHandle, 0, 0);
	return (result == _iocpHandle);
}

bool IocpCore::Register(IocpSession* session)
{
	HANDLE result = ::CreateIoCompletionPort(
		reinterpret_cast<HANDLE>(session->GetSocket()), // 소켓 핸들
		_iocpHandle,
		reinterpret_cast<ULONG_PTR>(session),           // 세션 포인터를 completionKey로
		0
	);

	return (result == _iocpHandle);

}
bool IocpCore::Register(std::shared_ptr<IocpSession> session)
{
	return Register(session.get()); // 기존 raw 포인터 버전 재사용
}
// ----------------------
// param   : 없음
// function: IOCP 핸들 반환
// return  : HANDLE 값
// ----------------------
HANDLE IocpCore::GetHandle() const {
	return _iocpHandle;
}

// ----------------------
// param   : 없음
// function: 모든 워커 스레드 종료
// return  : 없음
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
