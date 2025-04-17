#pragma once
// DLL로 빌드시 주석 풀고 외부에서 사용할 함수에 달아주기
#ifdef SERVERCORE_EXPORTS
//#define SERVERCORE_API __declspec(dllexport)
#define SERVERCORE_API
#else
//#define SERVERCORE_API __declspec(dllimport)
#define SERVERCORE_API
#endif
