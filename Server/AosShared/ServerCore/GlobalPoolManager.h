#pragma once

#include <vector>
#include <mutex>
#include <cstddef>
#include "IocpBufferPool.h"

class GlobalPoolManager {
private:
	void RegisterPool(IocpBufferPool* pool);

public:
	static GlobalPoolManager& Instance();

	IocpBufferPool* GetMyPool();

	size_t TotalCount();
	size_t AvailableCount();

private:
	GlobalPoolManager() = default;

	std::mutex _lock;
	std::vector<IocpBufferPool*> _allPools;
};


#define BUFFER_POOL_INSTANCE() GlobalPoolManager::Instance()
#define GET_BUFFER_POOL() BUFFER_POOL_INSTANCE().GetMyPool()
