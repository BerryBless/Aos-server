#include "pch.h"
#include <thread>

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	bool Initialize(int threadCount);
	void Run();                        // 워커 스레드 시작
	void Shutdown();                   // 스레드 정지

	bool Register(HANDLE handle);      // 소켓을 IOCP에 등록
	HANDLE GetHandle() const;

private:
	void WorkerThread();               // 내부 워커 스레드 함수

private:
	HANDLE _iocpHandle;
	int _workerCount;
	std::vector<std::thread> _workerThreads;
	bool _shutdown;
};