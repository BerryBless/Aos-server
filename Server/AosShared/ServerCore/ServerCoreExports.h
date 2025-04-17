#pragma once

#ifdef SERVERCORE_EXPORTS
#define SERVERCORE_API __declspec(dllexport)
#else
#define SERVERCORE_API __declspec(dllimport)
#endif
