// ServerCore.cpp : DLL을 위해 내보낸 함수를 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "ServerCore.h"

#include <iostream>

// 내보낸 변수의 예제입니다.
SERVERCORE_API int nServerCore=123;

// 내보낸 함수의 예제입니다.
SERVERCORE_API int fnServerCore(void)
{
    std::cout << nServerCore << "fnServerCore";
    return 0;
}

// 내보낸 클래스의 생성자입니다.
CServerCore::CServerCore()
{
    return;
}

