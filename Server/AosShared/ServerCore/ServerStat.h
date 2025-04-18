#pragma once

#include <atomic>
#include <thread>
#include <iostream>
#include <chrono>

// ----------------------
// class  : ServerStat
// function: ���� ������ ��� ��� (TPS, ���� ��, ���� ���� �� ���)
// ----------------------
class ServerStat {
public:
	static void Init();             // ����� ������ ����
	static void Shutdown();         // ������ ���� (join)

	static void IncClient();        // Ŭ���̾�Ʈ ����
	static void DecClient();        // Ŭ���̾�Ʈ ����
	static void IncPacket();        // ��Ŷ ó�� ī��Ʈ

private:
	static void MonitorThread();    // ���� ����� ����

private:
	inline static std::atomic<int> _connectedClients = 0;
	inline static std::atomic<int> _packetCount = 0;
	inline static std::atomic<bool> _running = false;
	inline static std::thread _monitorThread;
};
