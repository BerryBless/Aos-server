#pragma once
#include <array>
#include <winsock2.h>

class IocpBufferPool;

// ----------------------
// class : IocpBuffer
// function: WSA용 버퍼 래핑 클래스
// ----------------------
class IocpBuffer {
public:
	static constexpr size_t BUFFER_SIZE = 4096;

	IocpBuffer();
	~IocpBuffer();

	WSABUF* GetWSABUF();
	char* GetData();
	size_t GetSize() const;

	void SetOwner(IocpBufferPool* p) { _ownerPool = p; }
	IocpBufferPool* GetOwner() const { return _ownerPool; }

private:
	std::array<char, BUFFER_SIZE> _data;
	WSABUF _wsaBuf;

	IocpBufferPool* _ownerPool = nullptr;
};
