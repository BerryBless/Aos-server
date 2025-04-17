#include "pch.h"
#include "RecvBuffer.h"

RecvBuffer::RecvBuffer(int capacity) {
	_buffer.resize(capacity);
}

bool RecvBuffer::Write(const char* data, int len) {
	if (_writePos + len > static_cast<int>(_buffer.size()))
		return false;

	memcpy(&_buffer[_writePos], data, len);
	_writePos += len;
	return true;
}

bool RecvBuffer::HasCompletePacket(int packetSize) const {
	return (_writePos - _readPos) >= packetSize;
}

const char* RecvBuffer::Peek() const {
	return &_buffer[_readPos];
}

void RecvBuffer::Consume(int len) {
	_readPos += len;

	if (_readPos == _writePos) {
		_readPos = 0;
		_writePos = 0;
	}
	else if (_readPos > (_buffer.size() / 2)) {
		Compact();
	}
}

int RecvBuffer::GetStoredSize() const {
	return _writePos - _readPos;
}

void RecvBuffer::Compact() {
	int dataSize = GetStoredSize();
	memmove(&_buffer[0], &_buffer[_readPos], dataSize);
	_readPos = 0;
	_writePos = dataSize;
}
