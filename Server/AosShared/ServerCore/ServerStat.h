#pragma once

#include <atomic>
#include <thread>
#include <iostream>
#include <chrono>

// ----------------------
// class  : ServerStat
// function: 서버 디버깅용 통계 모듈 (TPS, 연결 수, 버퍼 생성 수 출력)
// ----------------------
class ServerStat {
public:
	static void Init();             // 모니터 스레드 시작
	static void Shutdown();         // 스레드 종료 (join)

	static void IncClient();        // 클라이언트 연결
	static void DecClient();        // 클라이언트 해제
	static void IncPacket();        // 패킷 처리 카운트

private:
	static void MonitorThread();    // 내부 모니터 루프

private:
	inline static std::atomic<int> _connectedClients = 0;
	inline static std::atomic<int> _packetCount = 0;
	inline static std::atomic<bool> _running = false;
	inline static std::thread _monitorThread;
};
