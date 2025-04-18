// ----------------------
// file   : IocpBufferPool.h
// function: lock-free 버퍼 풀 구현체 (boost::lockfree::stack 기반)
// ----------------------
#pragma once

#include <boost/lockfree/stack.hpp>
#include <atomic>

class IocpBuffer;

// ----------------------
// class : IocpBufferPool
// function: 스레드 로컬 단위의 lock-free 버퍼 풀
// ----------------------
class IocpBufferPool {
public:
	IocpBufferPool();
	~IocpBufferPool();

	// ----------------------
	// function: 버퍼 획득
	// return  : 사용 가능한 버퍼 포인터
	// ----------------------
	IocpBuffer* Acquire();

	// ----------------------
	// param   : buffer - 반환할 버퍼 포인터
	// function: 사용 완료된 버퍼를 풀에 반환
	// return  : 없음
	// ----------------------
	void Release(IocpBuffer* buffer);

	// ----------------------
	// function: 전체 생성된 버퍼 수
	// return  : 생성 수
	// ----------------------
	size_t GetTotalCount() const;

private:
	boost::lockfree::stack<IocpBuffer*> _stack;
	std::atomic<size_t> _totalCount{ 0 };
};
