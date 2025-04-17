#include "pch.h"
#include "GlobalPoolManager.h"

GlobalPoolManager& GlobalPoolManager::Instance() {
	static GlobalPoolManager instance;
	return instance;
}

void GlobalPoolManager::RegisterPool(IocpBufferPool* pool) {
	std::lock_guard<std::mutex> lock(_lock);
	_allPools.push_back(pool);
}

IocpBufferPool* GlobalPoolManager::GetMyPool() {
	thread_local IocpBufferPool localPool;
	thread_local bool registered = false;
	if (!registered) {
		GlobalPoolManager::Instance().RegisterPool(&localPool);
		registered = true;
	}
	return &localPool;
}


size_t GlobalPoolManager::TotalCount() {
	std::lock_guard<std::mutex> lock(_lock);

	size_t total = 0;
	for (auto* pool : _allPools)
		total += pool->GetTotalCount();

	return total;
}

size_t GlobalPoolManager::AvailableCount() {
	std::lock_guard<std::mutex> lock(_lock);

	size_t available = 0;
	for (auto* pool : _allPools)
		available += pool->GetPoolSize();

	return available;
}