#include "pch.h"
#include "ServerStat.h"
#include <mutex>
#include "GlobalPoolManager.h"

void ServerStat::Init() {
	_running = true;
	_monitorThread = std::thread(MonitorThread);
}

void ServerStat::Shutdown() {
	_running = false;
	if (_monitorThread.joinable())
		_monitorThread.join();
}


void ServerStat::IncClient() {
	_connectedClients++;
}

void ServerStat::DecClient() {
	_connectedClients--;
}

void ServerStat::IncPacket() {
	_packetCount++;
}

void ServerStat::MonitorThread() {
	using namespace std::chrono;
	static std::mutex coutLock;
	while (_running) {
		std::this_thread::sleep_for(seconds(5));

		std::lock_guard<std::mutex> lock(coutLock);
		int packets = _packetCount.exchange(0);
		std::cout << "[���� ����] Ŭ�� ����: " << _connectedClients.load()
			<< ", TPS: " << packets / 5
			<< ", �� �������� ���� " << BUFFER_POOL_INSTANCE().TotalCount()
			<< ", ���� ���� ����: " << BUFFER_POOL_INSTANCE().AvailableCount()
			<< std::endl;
	}
}
