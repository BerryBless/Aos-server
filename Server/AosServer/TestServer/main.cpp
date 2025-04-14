#include <iostream>
#include "ServerCore.h"
#include "NetProtocol.h"
int main()
{
	fnServerCore();
	fnNetProtocol();
	std::cout << "TestServer";
	return 0;
}