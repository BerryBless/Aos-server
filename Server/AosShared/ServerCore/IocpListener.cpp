// ----------------------
// file   : IocpListener.cpp
// function: AcceptEx 구현
// ----------------------
#include "pch.h"
#include "IocpListener.h"
#include <ws2tcpip.h>
#include "IocpCore.h"

bool IocpListener::Start(std::shared_ptr<IocpCore> core, const NetAddress& bindAddr) {
	_listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSocket == INVALID_SOCKET) return false;

	sockaddr_in addr = bindAddr.GetSockAddr();
	if (::bind(_listenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
		return false;

	if (::listen(_listenSocket, SOMAXCONN) == SOCKET_ERROR)
		return false;

	// AcceptEx 함수 포인터 구하기
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	DWORD bytes = 0;
	WSAIoctl(_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidAcceptEx, sizeof(guidAcceptEx),
		&_fnAcceptEx, sizeof(_fnAcceptEx),
		&bytes, nullptr, nullptr);

	_iocpCore = core;
	return _iocpCore->Register((HANDLE)_listenSocket);
}

bool IocpListener::PostAccept() {
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
		return false;

	OverlappedEx* overlapped = new OverlappedEx(OperationType::Accept);

	overlapped->session = std::make_shared<IocpSession>();
	overlapped->session->BindSocket(clientSocket);
	overlapped->listener = this; // listener 포인터 연결

	DWORD recvBytes = 0;
	BOOL result = _fnAcceptEx(
		_listenSocket,
		clientSocket,
		overlapped->buffer,
		0,
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		&recvBytes,
		&overlapped->overlapped);

	if (result == FALSE && WSAGetLastError() != WSA_IO_PENDING) {
		delete overlapped;
		closesocket(clientSocket);
		return false;
	}
	return true;
}

void IocpListener::OnAccept(OverlappedEx* overlapped) {
	SOCKET clientSocket = overlapped->session->GetSocket();

	// AcceptEx 사용 시 필수 옵션
	setsockopt(clientSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
		(char*)&_listenSocket, sizeof(_listenSocket));

	_iocpCore->Register(overlapped->session);
	overlapped->session->OnAccept();
	PostAccept();

	delete overlapped;
}