#include "pch.h"
#include "IocpBufferPool.h"
#include "GlobalPoolManager.h"
#include "IocpBuffer.h"
#include <iostream>

// ----------------------
// function: ������ - ���� �ʱ�ȭ
// ----------------------
IocpBufferPool::IocpBufferPool() : _stack(1024) {}

// ----------------------
// function: �Ҹ��� - ���� ���� ����
// ----------------------
IocpBufferPool::~IocpBufferPool() {
	IocpBuffer* ptr = nullptr;
	while (_stack.pop(ptr)) {
		SAFE_DELETE (ptr);
	}
}

// ----------------------
// function: ���� ȹ��
// return  : ��� ������ ���� ������
// ----------------------
IocpBuffer* IocpBufferPool::Acquire() {
	IocpBuffer* ptr = nullptr;

	if (!_stack.pop(ptr)) {
		ptr = new IocpBuffer();
		ptr->SetOwner(this);
		++_totalCount;
		GlobalPoolManager::Instance().AddToTotal(1);
		return ptr;
	}
	else if (!ptr) {
		std::cerr << "[POOL] _stack���� ���� �����Ͱ� null\n";
		return nullptr;
	}

	GlobalPoolManager::Instance().SubFromAvailable(1);
	return ptr;
}

// ----------------------
// param   : buffer - ��ȯ�� ���� ������
// function: ��� �Ϸ�� ���۸� Ǯ�� ��ȯ
// ----------------------
void IocpBufferPool::Release(IocpBuffer* buffer) {
	if (!buffer) {
		std::cerr << "[POOL] Release(nullptr): �߸��� ����\n";
		return;
	}
	if (buffer->GetOwner() != this) {
		std::cerr << "[FATAL] �ٸ� Ǯ���� ������ buffer!\n";
		std::abort();
	}
	if (!_stack.push(buffer)) {
		std::cerr << "[POOL] push ����\n";
		SAFE_DELETE (buffer);
	}
	GlobalPoolManager::Instance().AddToAvailable(1);
}

// ----------------------
// function: ��ü ������ ���� ��
// return  : ���� ��
// ----------------------
size_t IocpBufferPool::GetTotalCount() const {
	return _totalCount;
}
