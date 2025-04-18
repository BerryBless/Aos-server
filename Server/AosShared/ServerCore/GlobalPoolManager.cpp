#include "pch.h"
#include "GlobalPoolManager.h"

GlobalPoolManager& GlobalPoolManager::Instance() {
	static GlobalPoolManager instance;
	return instance;
}

IocpBufferPool* GlobalPoolManager::GetMyPool() {
	thread_local IocpBufferPool localPool;
	thread_local bool registered = false;
	return &localPool;
}
