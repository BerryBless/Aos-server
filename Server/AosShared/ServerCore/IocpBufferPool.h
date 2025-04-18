// ----------------------
// file   : IocpBufferPool.h
// function: lock-free ���� Ǯ ����ü (boost::lockfree::stack ���)
// ----------------------
#pragma once

#include <boost/lockfree/stack.hpp>
#include <atomic>

class IocpBuffer;

// ----------------------
// class : IocpBufferPool
// function: ������ ���� ������ lock-free ���� Ǯ
// ----------------------
class IocpBufferPool {
public:
	IocpBufferPool();
	~IocpBufferPool();

	// ----------------------
	// function: ���� ȹ��
	// return  : ��� ������ ���� ������
	// ----------------------
	IocpBuffer* Acquire();

	// ----------------------
	// param   : buffer - ��ȯ�� ���� ������
	// function: ��� �Ϸ�� ���۸� Ǯ�� ��ȯ
	// return  : ����
	// ----------------------
	void Release(IocpBuffer* buffer);

	// ----------------------
	// function: ��ü ������ ���� ��
	// return  : ���� ��
	// ----------------------
	size_t GetTotalCount() const;

private:
	boost::lockfree::stack<IocpBuffer*> _stack;
	std::atomic<size_t> _totalCount{ 0 };
};
