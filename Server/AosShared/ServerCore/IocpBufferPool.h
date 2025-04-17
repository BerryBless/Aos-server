// ----------------------
// file   : IocpBufferPool.h
// function: 스레드 단위 버퍼 풀 구현체 (stack 기반)
// ----------------------
#pragma once

#include <stack>
#include <mutex>
#include <memory>
#include "IocpBuffer.h"

class IocpBufferPool {
public:
	std::unique_ptr<IocpBuffer> Acquire() {
		if (_stack.empty())
		{
			++_totalCount;
			auto ptr = std::make_unique<IocpBuffer>() ;
			if (ptr && ptr.get() != nullptr)
				return ptr;
			std::cerr << "[POOL] Acquire: nullptr 발견, 제거됨\n";
		}
		if (!_stack.top())
		{
			std::cerr << "!_stack.top()";
		}
		std::lock_guard<std::mutex> guard(_lock);
		auto ptr = std::move(_stack.top());
		_stack.pop();
		if (!ptr) {
			std::cerr << "[POOL] _stack에서 꺼낸 포인터가 null\n";
		}
		return ptr;
	}

	void Release(std::unique_ptr<IocpBuffer> buffer) {
		if (!buffer) {
			std::cerr << "[POOL] Release(nullptr): 이중 Move 또는 잘못된 해제\n";
			return;
		}
		if (!buffer.get()) {
			std::cerr << "[POOL] Release(empty ptr): 내부 nullptr\n";
			return;
		}
		if (buffer->GetOwner() != this) {
			std::cerr << "[FATAL] 다른 풀로부터 넘어온 buffer!\n";
			std::abort();
		}
		std::lock_guard<std::mutex> guard(_lock);
		_stack.push(std::move(buffer));
	}


	size_t GetTotalCount() const
	{
		return _totalCount;
	}

	size_t GetPoolSize() const
	{
		return _stack.size();
	}

private:
	std::stack<std::unique_ptr<IocpBuffer>> _stack;
	std::mutex _lock;
	size_t _totalCount = 0;  // ← 생성된 전체 버퍼 수 (사용 + 대기)
};
