#include "pch.h"
#include "IocpBufferPool.h"
#include "GlobalPoolManager.h"
#include "IocpBuffer.h"
#include <iostream>

// ----------------------
// function: 생성자 - 스택 초기화
// ----------------------
IocpBufferPool::IocpBufferPool() : _stack(1024) {}

// ----------------------
// function: 소멸자 - 남은 버퍼 해제
// ----------------------
IocpBufferPool::~IocpBufferPool() {
	IocpBuffer* ptr = nullptr;
	while (_stack.pop(ptr)) {
		SAFE_DELETE (ptr);
	}
}

// ----------------------
// function: 버퍼 획득
// return  : 사용 가능한 버퍼 포인터
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
		std::cerr << "[POOL] _stack에서 꺼낸 포인터가 null\n";
		return nullptr;
	}

	GlobalPoolManager::Instance().SubFromAvailable(1);
	return ptr;
}

// ----------------------
// param   : buffer - 반환할 버퍼 포인터
// function: 사용 완료된 버퍼를 풀에 반환
// ----------------------
void IocpBufferPool::Release(IocpBuffer* buffer) {
	if (!buffer) {
		std::cerr << "[POOL] Release(nullptr): 잘못된 해제\n";
		return;
	}
	if (buffer->GetOwner() != this) {
		std::cerr << "[FATAL] 다른 풀에서 생성된 buffer!\n";
		std::abort();
	}
	if (!_stack.push(buffer)) {
		std::cerr << "[POOL] push 실패\n";
		SAFE_DELETE (buffer);
	}
	GlobalPoolManager::Instance().AddToAvailable(1);
}

// ----------------------
// function: 전체 생성된 버퍼 수
// return  : 생성 수
// ----------------------
size_t IocpBufferPool::GetTotalCount() const {
	return _totalCount;
}
