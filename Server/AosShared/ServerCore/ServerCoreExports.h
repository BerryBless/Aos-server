#pragma once
// DLL�� ����� �ּ� Ǯ�� �ܺο��� ����� �Լ��� �޾��ֱ�
#ifdef SERVERCORE_EXPORTS
//#define SERVERCORE_API __declspec(dllexport)
#define SERVERCORE_API
#else
//#define SERVERCORE_API __declspec(dllimport)
#define SERVERCORE_API
#endif
