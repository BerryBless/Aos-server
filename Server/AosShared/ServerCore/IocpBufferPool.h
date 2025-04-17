// ----------------------
// file   : IocpBufferPool.h
// function: ������ ���� ���� Ǯ ����ü (stack ���)
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
			std::cerr << "[POOL] Acquire: nullptr �߰�, ���ŵ�\n";
		}
		if (!_stack.top())
		{
			std::cerr << "!_stack.top()";
		}
		std::lock_guard<std::mutex> guard(_lock);
		auto ptr = std::move(_stack.top());
		_stack.pop();
		if (!ptr) {
			std::cerr << "[POOL] _stack���� ���� �����Ͱ� null\n";
		}
		return ptr;
	}

	void Release(std::unique_ptr<IocpBuffer> buffer) {
		if (!buffer) {
			std::cerr << "[POOL] Release(nullptr): ���� Move �Ǵ� �߸��� ����\n";
			return;
		}
		if (!buffer.get()) {
			std::cerr << "[POOL] Release(empty ptr): ���� nullptr\n";
			return;
		}
		if (buffer->GetOwner() != this) {
			std::cerr << "[FATAL] �ٸ� Ǯ�κ��� �Ѿ�� buffer!\n";
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
	size_t _totalCount = 0;  // �� ������ ��ü ���� �� (��� + ���)
};
