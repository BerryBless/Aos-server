#include "IocpHandle.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
	IocpHandle core(2); // 내부적으로 CreateIocp 호출

	std::cout << "IOCP 초기화 성공. 5초간 대기 후 종료 테스트..." << std::endl;

	core.Run();  // 내부적으로 RunIocp 호출

	std::this_thread::sleep_for(std::chrono::seconds(5));

	// core 소멸 시 자동으로 DestroyIocp 호출

	std::cout << "IOCP 종료 완료." << std::endl;
	return 0;
}
