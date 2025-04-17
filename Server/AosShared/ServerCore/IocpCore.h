#include "pch.h"
#include <thread>

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	bool Initialize(int threadCount);
	void Run();                        // ��Ŀ ������ ����
	void Shutdown();                   // ������ ����

	bool Register(HANDLE handle);      // ������ IOCP�� ���
	HANDLE GetHandle() const;

private:
	void WorkerThread();               // ���� ��Ŀ ������ �Լ�

private:
	HANDLE _iocpHandle;
	int _workerCount;
	std::vector<std::thread> _workerThreads;
	bool _shutdown;
};