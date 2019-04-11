// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SocketHelper.h"
#include "ServerClass.h"
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

int setServer(char* port)
{
	SocketHelper sh;
	WSADATA wData;

	try
	{
		sh.startup(wData);
		struct addrinfo init;
		ZeroMemory(&init, sizeof(init));
		init.ai_family = AF_INET;
		init.ai_socktype = SOCK_STREAM;
		init.ai_protocol = IPPROTO_TCP;
		init.ai_flags = AI_PASSIVE;
		struct addrinfo * addrinfoList = sh.hatchAddrinfo(NULL, port, init);
		sh.printAddrinfo(addrinfoList);
		SOCKET serverSocket = sh.bindSocket(addrinfoList);
		freeaddrinfo(addrinfoList);
		sh.listenOnSocket(serverSocket);
		printf("\nWaiting for connection...");
		SOCKET clientSocket = sh.acceptSocket(serverSocket);
		printf("\nClient connected!\n");
		for (;;)
		{
			char recvbuff[DEFAULT_BUFLEN];
			memset(recvbuff,'\0',DEFAULT_BUFLEN);
			
			sh.recvFromSocket(clientSocket,recvbuff,DEFAULT_BUFLEN);

			std::string str(recvbuff);
			//printf("%s", recvbuff);
			printf("%s", str.c_str());
		}
	}
	catch (std::exception & ex)
	{
		printf("\n%s", ex.what());
		return -1;
	}

	return 0;
}

void receiveFrom(SOCKET s)
{
	SocketHelper sh;
	for (;;)
	{
		char recvbuff[DEFAULT_BUFLEN];
		memset(recvbuff, '\0', DEFAULT_BUFLEN);

		sh.recvFromSocket(s, recvbuff, DEFAULT_BUFLEN);

		std::string str(recvbuff);
		//printf("%s", recvbuff);
		printf("Recv: %s\n", str.c_str());
	}
}

int connectTo(char* address, char* port)
{
	SocketHelper sh;
	WSADATA wData;

	try
	{
		sh.startup(wData);
		struct addrinfo init;
		ZeroMemory(&init, sizeof(init));
		init.ai_family = AF_INET;
		init.ai_socktype = SOCK_STREAM;
		init.ai_protocol = IPPROTO_TCP;
		init.ai_flags = AI_PASSIVE;
		struct addrinfo * addrinfoList = sh.hatchAddrinfo(address, port, init);
		sh.printAddrinfo(addrinfoList);
		SOCKET serverSocket = sh.connectToSocket(addrinfoList);
		freeaddrinfo(addrinfoList);
		printf("\n");
		std::thread readingThread(&receiveFrom, serverSocket);
		readingThread.detach();
		for (;;)
		{
			char sendbuff[]= "to jest moja wiadomosc\n";
			memset(sendbuff, '\0', sizeof(char)*511);
			fgets(sendbuff,511,stdin);

			std::string pomstr(sendbuff);
			pomstr.erase(pomstr.find_last_not_of(" \n\r\t") + 1);
			strcpy(sendbuff, pomstr.c_str());

			sh.sendToSocket(serverSocket,sendbuff);
			Sleep(5);
		}
	}
	catch (std::exception & ex)
	{
		printf("\n%s", ex.what());
		return -1;
	}
	printf("\nConnection done succesfully");
	return 0;
}

int main()
{
	//connectTo("127.0.0.1","27015");
	//setServer("27015");
	ServerClass server;
	server.init();

	/*std::thread t([]()
	{
		for (;;)
		{
			char buff[100];
			fgets(buff, 100, stdin);
			Sleep(10000);
		}
	});
	t.join();
	for (;;)
	{
		printf("blabla");
		Sleep(1000);
	}*/
	//auto var = HelpFunctions::strToMap("4252124523512342",4,4);
	system("pause");
}

