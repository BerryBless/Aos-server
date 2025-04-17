#include "pch.h"
#include "IocpBuffer.h"

IocpBuffer::IocpBuffer() {
	_wsaBuf.buf = reinterpret_cast<CHAR*>(_data.data());
	_wsaBuf.len = static_cast<ULONG>(_data.size());
}

IocpBuffer::~IocpBuffer()
{
	std::cerr << "[IocpBuffer] ¼Ò¸ê @" << this << std::endl;
}

WSABUF* IocpBuffer::GetWSABUF() {
	return &_wsaBuf;
}

char* IocpBuffer::GetData() {
	return _data.data();
}

size_t IocpBuffer::GetSize() const {
	return _data.size();
}
