#pragma once
#include "ServerCoreExports.h"

// ���� void* ��� DLL API
SERVERCORE_API void* CreateIocp(int threadCount);
SERVERCORE_API void RunIocp(void* iocp);
SERVERCORE_API void DestroyIocp(void* iocp);

// ----------------------
// param   : ����
// function: IOCP �ڵ��� �����ϰ� ����ϴ� RAII Ŭ����
// return  : void*�� ����� �����ϰ� ����
// ----------------------
class IocpHandle {
public:
	explicit IocpHandle(int threadCount)
		: _core(CreateIocp(threadCount)) {
	}

	~IocpHandle() {
		if (_core)
			DestroyIocp(_core);
	}

	void Run() {
		if (_core)
			RunIocp(_core);
	}

	// �ʿ� �� ���� ������ ���� ���� (�� ���� �� ����)
	void* Raw() const { return _core; }

	// ���� ����, �̵� ���
	IocpHandle(const IocpHandle&) = delete;
	IocpHandle& operator=(const IocpHandle&) = delete;

	IocpHandle(IocpHandle&& other) noexcept
		: _core(other._core) {
		other._core = nullptr;
	}

	IocpHandle& operator=(IocpHandle&& other) noexcept {
		if (this != &other) {
			if (_core)
				DestroyIocp(_core);
			_core = other._core;
			other._core = nullptr;
		}
		return *this;
	}

private:
	void* _core;
};
