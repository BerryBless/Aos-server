#pragma once
#include <vector>
#include <cstring>

class RecvBuffer {
public:
	RecvBuffer(int capacity = 8192);

	// ������ ����
	bool Write(const char* data, int len);

	// ������ ���� �� �ִ� �ϼ��� ��Ŷ ���� Ȯ��
	bool HasCompletePacket(int packetSize) const;

	// ������ �б�
	const char* Peek() const;
	void Consume(int len); // ���� ��ŭ ����

	int GetStoredSize() const;

private:
	std::vector<char> _buffer;
	int _writePos = 0;
	int _readPos = 0;

	void Compact(); // ���� �� ���� ����
};
