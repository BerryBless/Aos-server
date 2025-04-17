#include "pch.h"
#include "IocpHandle.h"
#include "IocpCore.h"

SERVERCORE_API void* CreateIocp(int threadCount) {
	IocpCore* core = new IocpCore();
	core->Initialize(threadCount);
	return core;
}

SERVERCORE_API void RunIocp(void* iocp) {
	if (iocp)
		static_cast<IocpCore*>(iocp)->Run();
}

SERVERCORE_API void DestroyIocp(void* iocp) {
	if (iocp)
		delete static_cast<IocpCore*>(iocp);
}
