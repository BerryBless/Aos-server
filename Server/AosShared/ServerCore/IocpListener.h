// ----------------------
// file   : IocpListener.h
// function: AcceptEx ��� �񵿱� ���� ó�� Ŭ����
// ----------------------
#pragma once

#include <winsock2.h>
#include <mswsock.h>
#include <memory>
#include "IocpSession.h"
#include "OverlappedEx.h"

class IocpCore;

// ----------------------
// param   : ����
// function: sockaddr_in �ּ� ������ ������ ��ƿ Ŭ����
// return  : bind�� sockaddr_in ����ü ��ȯ ��
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
