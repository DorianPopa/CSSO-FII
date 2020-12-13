// CSSO-Tema6 Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

char* getConsoleCommand() {
	char*		buffer;
	size_t		bufsize = 1024;

	buffer = (char*)malloc(bufsize * sizeof(char));
	std::cin.getline(buffer, bufsize);
	return buffer;
}


int main()
{
	WSADATA					wsaData;
	SOCKET					SendingSocket;
	SOCKADDR_IN				ReceiverAddr, SrcInfo;
	int						Port = 5150;
	const char* IP =		"127.0.0.1";
	char* SendBuf =			(char*)"";
	int						BufLength = 1024;
	int len;
	int TotalByteSent = 0;

	// Init Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Client: WSAStartup failed with error %ld\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	else
		printf("Client: The Winsock DLL status is %s.\n", wsaData.szSystemStatus);

	// Create socket
	SendingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (SendingSocket == INVALID_SOCKET)
	{
		printf("Client: Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	else
		printf("Client: socket() is OK!\n");

	// Set up a SOCKADDR_IN structure
	ReceiverAddr.sin_family = AF_INET;				// The IPv4 family
	ReceiverAddr.sin_port = htons(Port);
	ReceiverAddr.sin_addr.s_addr = inet_addr(IP);	// The server IP

	printf("Client: Ready to send data to the server\n");
	while (1) {
		SendBuf = getConsoleCommand();
		if (strcmp(SendBuf, "quit") == 0)
			break;
		printf("Client: Data to be sent: \"%s\"\n", SendBuf);
		printf("Client: Sending data...\n");
		TotalByteSent = sendto(SendingSocket, SendBuf, BufLength, 0, (SOCKADDR*)&ReceiverAddr, sizeof(ReceiverAddr));	// Send data to the server.
		printf("Client: sendto() looks OK!\n");
	}

	// Some info on the receiver side...
	memset(&SrcInfo, 0, sizeof(SrcInfo));
	len = sizeof(SrcInfo);

	getsockname(SendingSocket, (SOCKADDR*)&SrcInfo, &len);
	printf("Client: Sending IP(s) used: %s\n", inet_ntoa(SrcInfo.sin_addr));
	printf("Client: Sending port used: %d\n", htons(SrcInfo.sin_port));

	// Some info on the sender side
	getpeername(SendingSocket, (SOCKADDR*)&ReceiverAddr, (int*)sizeof(ReceiverAddr));
	printf("Client: Receiving IP used: %s\n", inet_ntoa(ReceiverAddr.sin_addr));
	printf("Client: Receiving port used: %d\n", htons(ReceiverAddr.sin_port));
	printf("Client: Total byte sent: %d\n", TotalByteSent);

	// Close the socket.
	printf("Client: Finished sending. Closing the sending socket...\n");
	if (closesocket(SendingSocket) != 0)
		printf("Client: closesocket() failed! Error code: %ld\n", WSAGetLastError());
	else
		printf("Server: closesocket() is OK\n");


	printf("Client: Cleaning up...\n");
	if (WSACleanup() != 0)
		printf("Client: WSACleanup() failed! Error code: %ld\n", WSAGetLastError());
	else
		printf("Client: WSACleanup() is OK\n");

	return 0;
}
