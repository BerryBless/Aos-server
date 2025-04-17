// ----------------------
// file   : IocpListener.h
// function: AcceptEx 기반 비동기 접속 처리 클래스
// ----------------------
#pragma once

#include <winsock2.h>
#include <mswsock.h>
#include <memory>
#include "IocpSession.h"
#include "OverlappedEx.h"

class IocpCore;

// ----------------------
// param   : 없음
// function: sockaddr_in 주소 정보를 래핑한 유틸 클래스
// return  : bind용 sockaddr_in 구조체 반환 등
// ----------------------
class NetAddress {
public:
	NetAddress(const std::string& ip, uint16_t port) {
		::memset(&_addr, 0, sizeof(_addr));
		_addr.sin_family = AF_INET;
		_addr.sin_port = htons(port);
		::inet_pton(AF_INET, ip.c_str(), &_addr.sin_addr);
	}

	sockaddr_in GetSockAddr() const { return _addr; }

private:
	sockaddr_in _addr;
};

class IocpListener {
public:
	bool Start(std::shared_ptr<IocpCore> core, const NetAddress& bindAddr);
	bool PostAccept();
	void OnAccept(OverlappedEx* overlapped);

private:
	SOCKET _listenSocket = INVALID_SOCKET;
	std::shared_ptr<IocpCore> _iocpCore;
	LPFN_ACCEPTEX _fnAcceptEx = nullptr;
};
