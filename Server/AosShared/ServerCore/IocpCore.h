#include "pch.h"
#include <thread>


class IocpSession;

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	bool Initialize(int threadCount);
	void Run();                        // ��Ŀ ������ ����
	void Shutdown();                   // ������ ����

	bool Register(HANDLE handle);      // ������ IOCP�� ���
	bool Register(IocpSession* session);
	bool Register(std::shared_ptr<IocpSession> session);
	HANDLE GetHandle() const;

private:
	void WorkerThread();               // ���� ��Ŀ ������ �Լ�

private:
	HANDLE _iocpHandle;
	int _workerCount;
	std::vector<std::thread> _workerThreads;
	bool _shutdown;
};