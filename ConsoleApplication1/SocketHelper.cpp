#include "stdafx.h"

#include "SocketHelper.h"

void SocketHelper::startup(WSADATA & wData)
{
	int iResult = WSAStartup(MAKEWORD(2, 2), &wData);
	if (iResult != 0)
	{
		throw socket_error("Startup failed", iResult);
	}
}

struct addrinfo* SocketHelper::hatchAddrinfo(const char* name, const char* port, struct addrinfo & init)
{
	struct addrinfo* result = NULL;
	int iResult = getaddrinfo(name, port, &init, &result);
	if (iResult != 0)
	{
		throw socket_error("Getaddrinfo failed", iResult);
	}
	return result;
}

void SocketHelper::printAddrinfo(addrinfo * addr)
{
	int i = 0;
	struct addrinfo *p;
	for (p = addr; p != NULL; p = p->ai_next)
	{
		void *addr = NULL;

		printf("\ngetaddrinfo response %d\n", i++);
		printf("\tFlags: 0x%x\n", p->ai_flags);
		printf("\tFamily: ");
		switch (p->ai_family)
		{
		case AF_UNSPEC:
			printf("unspecified");
			break;
		case AF_INET:
			addr = &((struct sockaddr_in *)p->ai_addr)->sin_addr;
			printf("ipv4");
			break;
		case AF_INET6:
			addr = &((struct sockaddr_in6 *)p->ai_addr)->sin6_addr;
			printf("ipv6");
			break;
		}
		printf("\n\tSocket type: ");
		switch (p->ai_socktype)
		{
		case 0:
			printf("Unspecified\n");
			break;
		case SOCK_STREAM:
			printf("SOCK_STREAM (stream)\n");
			break;
		case SOCK_DGRAM:
			printf("SOCK_DGRAM (datagram) \n");
			break;
		case SOCK_RAW:
			printf("SOCK_RAW (raw) \n");
			break;
		case SOCK_RDM:
			printf("SOCK_RDM (reliable message datagram)\n");
			break;
		case SOCK_SEQPACKET:
			printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
			break;
		default:
			printf("Other %ld\n", p->ai_socktype);
			break;
		}
		char buff[46]; // 46 is enough to store ipv6
		inet_ntop(p->ai_family, addr, (PSTR)buff, 46);
		printf("\tAdress: %s", buff);
	}
}

void SocketHelper::printSockaddr(sockaddr_storage * sock_addr)
{
	char buff[46];
	if (sock_addr->ss_family == AF_INET)
	{
		sockaddr_in* clientInfo = (sockaddr_in*)((sockaddr*)&sock_addr);
		inet_ntop(clientInfo->sin_family, &clientInfo->sin_addr, (PSTR)buff, 46);
	}
	else if (sock_addr->ss_family == AF_INET6)
	{
		sockaddr_in6* clientInfo = (sockaddr_in6*)((sockaddr*)&sock_addr);
		inet_ntop(clientInfo->sin6_family, &clientInfo->sin6_addr, (PSTR)buff, 46);
	}
	printf("\nAdress: %s", buff);
}

SOCKET SocketHelper::bindSocket(struct addrinfo * addr)
{
	struct addrinfo* p;
	SOCKET listenSocket = NULL;
	for (p = addr; p != NULL; p = p->ai_next)
	{
		listenSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listenSocket == INVALID_SOCKET)
		{
			continue;
		}

		int iResult = bind(listenSocket, p->ai_addr, p->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(listenSocket);
			continue;
		}
		// creating and binding socket have been done succesfully
		break;
	}

	if (p == NULL)
	{
		throw socket_error("Couldnt find socket to bind");
	}

	return listenSocket;
}

void SocketHelper::listenOnSocket(SOCKET socket)
{
	int iResult = listen(socket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		throw socket_error("Listening on socket failed",iResult);
	}
}

SOCKET SocketHelper::acceptSocket(SOCKET socket, sockaddr_storage* client_saddr)
{
	SOCKET clientSocket;
	if (client_saddr == NULL)
	{
		clientSocket = accept(socket, NULL, NULL);
	}
	else
	{
		int client_saddrSIZE = sizeof(client_saddr);
		clientSocket = accept(socket, (sockaddr*)&client_saddr, &client_saddrSIZE);
	}
	if (clientSocket == INVALID_SOCKET)
	{
		throw socket_error("Accepting socket failed",clientSocket);
	}
	return clientSocket;
}

SOCKET SocketHelper::connectToSocket(addrinfo * addr)
{
	struct addrinfo* p = NULL;
	SOCKET connectSocket = NULL;
	for (p = addr; p != NULL; p = p->ai_next)
	{
		connectSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (connectSocket == INVALID_SOCKET)
		{
			continue;
		}

		int iResult = connect(connectSocket, p->ai_addr, p->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(connectSocket);
			continue;
		}
		// connected to socket
		break;
	}
	if (p == NULL)
	{
		throw socket_error("Couldnt connect to socket");
	}
	return connectSocket;
}

/*void SocketHelper::sendToSocket(SOCKET socket, const std::string & str)
{
	int strSize = str.length();
	int iResult = send(socket, str.c_str() , strSize + 1, 0);
	if(iResult == SOCKET_ERROR)
	{
		throw("Sending failed", WSAGetLastError());
	}
}

std::string SocketHelper::recvFromSocket(SOCKET socket)
{
	char buffer[DEFAULT_BUFLEN];
	int iResult;
	int recvLen = 0;
	do
	{
		iResult = recv(socket, buffer, DEFAULT_BUFLEN, 0);
		if (iResult > 0)
		{
			printf("Received: %d\n",iResult);
		}
		if (iResult == 0)
		{
			throw socket_error("Receiving failed",WSAGetLastError());
		}
	} while (iResult < 0);

	return std::string(buffer);
}*/

void SocketHelper::sendToSocket(SOCKET socket, char* data)
{
	int str = strlen(data);
	int iResult = send(socket, data, strlen(data)+1, 0);
	if (iResult == SOCKET_ERROR)
	{
		throw socket_error("Sending failed", WSAGetLastError());
	}
}

void SocketHelper::recvFromSocket(SOCKET socket, char* data, int len)
{
	int iResult;
	int recvLen = 0;
	do
	{
		iResult = recv(socket, data, DEFAULT_BUFLEN, 0);
		if (iResult > 0)
		{
			//printf("Received: %d\n", iResult);
		}
		if (iResult == 0)
		{
			throw socket_error("Receiving failed", WSAGetLastError());
		}
	} while (iResult < 0);
}

