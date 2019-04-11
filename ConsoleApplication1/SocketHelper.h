#pragma once
#include "stdafx.h"

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdexcept>
#include <string>
#include <mutex>
#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 512
class SocketHelper
{
private:
	class socket_error : public std::exception
	{
	private:
		std::string addInfo;
	public:
		socket_error(const std::string & addInfo) : addInfo(addInfo) {};
		socket_error(const std::string & addInfo, int socketError) : addInfo(addInfo + " code: " + std::to_string(socketError)) {};
		virtual const char* what() const throw()
		{
			return addInfo.c_str();
		}
	};
public:

	void startup(WSADATA & wData);

	struct addrinfo* hatchAddrinfo(const char* name, const char* port, struct addrinfo & init);
	void printAddrinfo(struct addrinfo* addr);
	void printSockaddr(sockaddr_storage* sock_addr);

	// server side
	SOCKET bindSocket(struct addrinfo* addr);
	void listenOnSocket(SOCKET socket);
	SOCKET acceptSocket(SOCKET socket, sockaddr_storage* client_saddr = NULL);

	// client side
	SOCKET connectToSocket(struct addrinfo* addr);

	// send/recv
	//void sendToSocket(SOCKET socket, const std::string & str);
	//std::string recvFromSocket(SOCKET socket);
	void sendToSocket(SOCKET socket, char* data);
	void recvFromSocket(SOCKET socket, char* data, int len);
};