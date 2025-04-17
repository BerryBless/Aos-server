#pragma once
#include <vector>
#include <cstring>

class RecvBuffer {
public:
	RecvBuffer(int capacity = 8192);

	// 데이터 쓰기
	bool Write(const char* data, int len);

	// 다음에 읽을 수 있는 완성된 패킷 여부 확인
	bool HasCompletePacket(int packetSize) const;

	// 데이터 읽기
	const char* Peek() const;
	void Consume(int len); // 읽은 만큼 제거

	int GetStoredSize() const;

private:
	std::vector<char> _buffer;
	int _writePos = 0;
	int _readPos = 0;

	void Compact(); // 버퍼 앞 공간 재사용
};
