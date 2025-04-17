#pragma once
#include "ServerCoreExports.h"

// 내부 void* 기반 DLL API
SERVERCORE_API void* CreateIocp(int threadCount);
SERVERCORE_API void RunIocp(void* iocp);
SERVERCORE_API void DestroyIocp(void* iocp);

// ----------------------
// param   : 없음
// function: IOCP 핸들을 안전하게 사용하는 RAII 클래스
// return  : void*를 숨기고 안전하게 관리
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

	// 필요 시 내부 포인터 접근 제공 (안 쓰는 게 좋음)
	void* Raw() const { return _core; }

	// 복사 금지, 이동 허용
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
