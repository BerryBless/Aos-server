#pragma once

#include <vector>
#include <cstddef>
#include "IocpBufferPool.h"

class GlobalPoolManager {
public:
	static GlobalPoolManager& Instance();

	IocpBufferPool* GetMyPool();

	size_t TotalCount() const { return _totalCount.load(); }
	size_t AvailableCount() const { return _availableCount.load(); }

	void AddToTotal(size_t n = 1) { _totalCount.fetch_add(n); }
	void AddToAvailable(size_t n = 1) { _availableCount.fetch_add(n); }
	void SubFromAvailable(size_t n = 1) { _availableCount.fetch_sub(n); }

private:
	GlobalPoolManager() = default;

	std::atomic<size_t> _totalCount{ 0 };
	std::atomic<size_t> _availableCount{ 0 };
};


#define BUFFER_POOL_INSTANCE() GlobalPoolManager::Instance()
#define GET_BUFFER_POOL() BUFFER_POOL_INSTANCE().GetMyPool()
