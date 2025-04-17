#include <winsock2.h>
#include <ws2tcpip.h>
#include <chrono>
#include <iostream>
#include <vector>
#include <atomic>
#include <climits>
#include <thread>
#include <mutex>
#pragma comment(lib, "ws2_32.lib")

using namespace std;
using namespace std::chrono;

// ----------------------
// 전역 통계 변수 (전체 TPS 및 응답 시간 측정)
// ----------------------
std::atomic<int> g_totalCount = 0;
std::atomic<int64_t> g_totalMicros = 0;
std::atomic<int64_t> g_maxMicros = 0;
std::atomic<int64_t> g_minMicros = LLONG_MAX;
std::mutex g_coutLock;

// ----------------------
// 각 클라이언트 소켓의 상태 구조체
// ----------------------
struct ClientState {
	SOCKET sock;                                  // 연결된 소켓
	steady_clock::time_point sendTime;            // 마지막 전송 시각
	bool waitingResponse = false;                 // 응답 대기 중인지 여부
};

// ----------------------
// function: 5초마다 전체 TPS / 평균 / 최소 / 최대 출력
// ----------------------
void MonitorThread(int clientCount) {
	while (true) {
		this_thread::sleep_for(seconds(5));

		int count = g_totalCount.exchange(0);
		int64_t totalMicros = g_totalMicros.exchange(0);
		int64_t maxMicros = g_maxMicros.exchange(0);
		int64_t minMicros = g_minMicros.exchange(LLONG_MAX);

		if (count > 0) {
			int avg = static_cast<int>(totalMicros / count);
			std::lock_guard<std::mutex> lock(g_coutLock);
			{
				cout << "[총 클라수: " << clientCount
					<< "] TPS: " << count / 5
					<< ", 평균: " << avg << " µs"
					<< ", 최소: " << minMicros << " µs"
					<< ", 최대: " << maxMicros << " µs"
					<< endl;
			}
		}
		else {
			std::lock_guard<std::mutex> lock(g_coutLock);
			cout << "[총 클라수: " << clientCount << "] 5초간 전송 없음" << endl;
		}
	}
}

// ----------------------
// function: 1스레드에서 clientCount개의 클라이언트를 생성해 계속 송수신
// ----------------------
void ClientLoop(int clientCount) {
	std::vector<ClientState> clients;
	std::vector<WSAPOLLFD> pollfds;

	// ----------------------
	// 클라이언트 연결 생성
	// ----------------------
	for (int i = 0; i < clientCount; ++i) {
		SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(9001);
		inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

		if (connect(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
			int err = WSAGetLastError();
			cerr << "[Client " << i << "] 연결 실패, 에러코드: " << err << endl;
			continue;
		}
		Sleep(1);

		clients.push_back({ s });
		pollfds.push_back({ s, POLLRDNORM, 0 }); // 수신 대기 등록
	}

	const char payload[64] = { 'B' };
	char recvBuf[66];

	// ----------------------
	// 루프: 지속적으로 패킷 전송 및 수신 응답 처리
	// ----------------------
	while (true) {
		for (int i = 0; i < clients.size(); ++i) {
			auto& client = clients[i];

			if (!client.waitingResponse) {
				char packet[66];
				uint16_t len = 66;
				memcpy(packet, &len, 2);
				memcpy(packet + 2, payload, 64);
				send(client.sock, packet, 66, 0);
				client.sendTime = steady_clock::now();
				client.waitingResponse = true;
			}
		}

		int ret = WSAPoll(pollfds.data(), (ULONG)pollfds.size(), 10);
		if (ret > 0) {
			for (int i = 0; i < pollfds.size(); ++i) {
				if (pollfds[i].revents & POLLRDNORM) {
					int recvLen = recv(pollfds[i].fd, recvBuf, 66, 0);
					if (recvLen > 0 && clients[i].waitingResponse) {
						auto elapsed = duration_cast<microseconds>(
							steady_clock::now() - clients[i].sendTime).count();

						g_totalCount++;
						g_totalMicros += elapsed;

						int64_t prevMax = g_maxMicros.load();
						while (elapsed > prevMax && !g_maxMicros.compare_exchange_weak(prevMax, elapsed));

						int64_t prevMin = g_minMicros.load();
						while (elapsed < prevMin && !g_minMicros.compare_exchange_weak(prevMin, elapsed));

						clients[i].waitingResponse = false;
					}
				}
			}
		}

		this_thread::sleep_for(milliseconds(1));
	}
}

// ----------------------
// function: 1개의 클라이언트에서만 TPS / 응답 시간 추적
// ----------------------
void SingleClientMonitorThread() {
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9001);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

	if (connect(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		int err = WSAGetLastError();
		cerr << "[one client] 연결 실패, 에러코드: " << err << endl;
		return;
	}

	char sendBuf[64] = { 'S' };
	char packet[66];
	char recvBuf[66];
	uint16_t len = 66;

	int count = 0;
	int64_t totalMicros = 0;
	int64_t maxMicros = 0;
	int64_t minMicros = LLONG_MAX;
	auto lastPrint = steady_clock::now();

	while (true) {
		memcpy(packet, &len, 2);
		memcpy(packet + 2, sendBuf, 64);

		auto start = high_resolution_clock::now();
		send(s, packet, 66, 0);
		int recvLen = recv(s, recvBuf, 66, 0);
		auto end = high_resolution_clock::now();

		if (recvLen <= 0) {
			cerr << "[1클라] 수신 실패" << endl;
			break;
		}

		auto micros = duration_cast<microseconds>(end - start).count();
		totalMicros += micros;
		count++;
		if (micros > maxMicros) maxMicros = micros;
		if (micros < minMicros) minMicros = micros;

		auto now = steady_clock::now();
		if (now - lastPrint >= seconds(5)) {
			if (count > 0) {
				int avg = static_cast<int>(totalMicros / count);
				std::lock_guard<std::mutex> lock(g_coutLock);
				cout << "[1클라이언트 반응속도] TPS: " << count / 5
					<< ", 평균: " << avg << " µs"
					<< ", 최소: " << minMicros << " µs"
					<< ", 최대: " << maxMicros << " µs"
					<< endl;
			}
			count = 0;
			totalMicros = 0;
			maxMicros = 0;
			minMicros = LLONG_MAX;
			lastPrint = now;
		}

		this_thread::sleep_for(milliseconds(5));
	}

	closesocket(s);
}

// ----------------------
// param   : index - 워커 번호 (디버깅용 로그 구분에 사용 가능)
// param   : clientCount - 이 스레드가 다룰 클라이언트 수
// function: 1개 워커 스레드에서 clientCount개의 소켓을 관리
// return  : 없음
// ----------------------
void ClientWorker(int index, int clientCount) {
	{
		std::lock_guard<std::mutex> lock(g_coutLock);
		std::cout << "[Worker " << index << "] 시작, 클라 수: " << clientCount << std::endl;
	}
	ClientLoop(clientCount);
}
// ----------------------
// main 함수
// ----------------------
int main() {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	const int totalClients = 1000;
	const int clientsPerThread = 500;
	const int workerThreads = (totalClients - 1) / clientsPerThread;

	thread monitor(MonitorThread, totalClients);  // 전체 성능 측정용
	monitor.detach();

	vector<thread> threads;

	// 워커 스레드들 실행
	for (int i = 0; i < workerThreads; ++i)
		threads.emplace_back(ClientWorker, i, clientsPerThread);

	// 측정 전용 클라이언트 실행
	thread single(SingleClientMonitorThread);
	single.detach();

	for (auto& t : threads)
		t.join();

	WSACleanup();
	return 0;
}
