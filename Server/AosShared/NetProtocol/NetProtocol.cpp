// NetProtocol.cpp : DLL을 위해 내보낸 함수를 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "NetProtocol.h"

#include <iostream>

// 내보낸 변수의 예제입니다.
NETPROTOCOL_API int nNetProtocol=321;

// 내보낸 함수의 예제입니다.
NETPROTOCOL_API int fnNetProtocol(void)
{
    std::cout << "fnNetProtocol" << nNetProtocol;
    return 0;
}

// 내보낸 클래스의 생성자입니다.
CNetProtocol::CNetProtocol()
{
    return;
}
