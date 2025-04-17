#include "IocpHandle.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
	IocpHandle core(2); // ���������� CreateIocp ȣ��

	std::cout << "IOCP �ʱ�ȭ ����. 5�ʰ� ��� �� ���� �׽�Ʈ..." << std::endl;

	core.Run();  // ���������� RunIocp ȣ��

	std::this_thread::sleep_for(std::chrono::seconds(5));

	// core �Ҹ� �� �ڵ����� DestroyIocp ȣ��

	std::cout << "IOCP ���� �Ϸ�." << std::endl;
	return 0;
}
