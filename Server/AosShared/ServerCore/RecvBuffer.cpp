#include "pch.h"
#include "RecvBuffer.h"

// ----------------------
// constructor
// ----------------------
RecvBuffer::RecvBuffer(int capacity) {
	_buffer.resize(capacity);
}

// ----------------------
// param   : data - 수신된 데이터
// param   : len - 수신된 데이터 길이
// function: 버퍼에 데이터 추가
// return  : 성공 여부
// ----------------------
bool RecvBuffer::Write(const char* data, int len) {
	if (_writePos + len > static_cast<int>(_buffer.size())) {
		Compact();
		if (_writePos + len > static_cast<int>(_buffer.size()))
			return false; // 여전히 부족하면 실패
	}
	std::memcpy(&_buffer[_writePos], data, len);
	_writePos += len;
	return true;
}

// ----------------------
// param   : packetSize - 확인할 패킷 크기
// function: 저장된 데이터가 packetSize 이상인지 확인
// return  : true = 완성된 패킷 있음
// ----------------------
bool RecvBuffer::HasCompletePacket(int packetSize) const {
	return (_writePos - _readPos) >= packetSize;
}

// ----------------------
// function: 다음 읽을 데이터 위치
// return  : 읽기 시작 주소
// ----------------------
const char* RecvBuffer::Peek() const {
	return &_buffer[_readPos];
}

// ----------------------
// param   : len - 처리한 바이트 수
// function: 읽은 만큼 소비
// ----------------------
void RecvBuffer::Consume(int len) {
	_readPos += len;
	if (_readPos == _writePos) {
		_readPos = 0;
		_writePos = 0;
	}
	else if (_readPos > _buffer.size() / 2) {
		Compact();
	}
}

// ----------------------
// function: 현재 저장된 데이터 크기
// return  : _writePos - _readPos
// ----------------------
int RecvBuffer::GetStoredSize() const {
	return _writePos - _readPos;
}

// ----------------------
// function: 앞쪽 빈 공간을 당겨 재사용
// ----------------------
void RecvBuffer::Compact() {
	if (_readPos == 0) return;
	int dataSize = _writePos - _readPos;
	std::memmove(&_buffer[0], &_buffer[_readPos], dataSize);
	_writePos = dataSize;
	_readPos = 0;
}
