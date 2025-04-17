#include "pch.h"
#include "RecvBuffer.h"

// ----------------------
// constructor
// ----------------------
RecvBuffer::RecvBuffer(int capacity) {
	_buffer.resize(capacity);
}

// ----------------------
// param   : data - ���ŵ� ������
// param   : len - ���ŵ� ������ ����
// function: ���ۿ� ������ �߰�
// return  : ���� ����
// ----------------------
bool RecvBuffer::Write(const char* data, int len) {
	if (_writePos + len > static_cast<int>(_buffer.size())) {
		Compact();
		if (_writePos + len > static_cast<int>(_buffer.size()))
			return false; // ������ �����ϸ� ����
	}
	std::memcpy(&_buffer[_writePos], data, len);
	_writePos += len;
	return true;
}

// ----------------------
// param   : packetSize - Ȯ���� ��Ŷ ũ��
// function: ����� �����Ͱ� packetSize �̻����� Ȯ��
// return  : true = �ϼ��� ��Ŷ ����
// ----------------------
bool RecvBuffer::HasCompletePacket(int packetSize) const {
	return (_writePos - _readPos) >= packetSize;
}

// ----------------------
// function: ���� ���� ������ ��ġ
// return  : �б� ���� �ּ�
// ----------------------
const char* RecvBuffer::Peek() const {
	return &_buffer[_readPos];
}

// ----------------------
// param   : len - ó���� ����Ʈ ��
// function: ���� ��ŭ �Һ�
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
// function: ���� ����� ������ ũ��
// return  : _writePos - _readPos
// ----------------------
int RecvBuffer::GetStoredSize() const {
	return _writePos - _readPos;
}

// ----------------------
// function: ���� �� ������ ��� ����
// ----------------------
void RecvBuffer::Compact() {
	if (_readPos == 0) return;
	int dataSize = _writePos - _readPos;
	std::memmove(&_buffer[0], &_buffer[_readPos], dataSize);
	_writePos = dataSize;
	_readPos = 0;
}
