// UDP_Server.cpp : Defines the entry point for the console application.
//
/*
Author	: ��ä�� (github nhnponyo)
Date	: 2014.06.11

Windows 7, Visual Studio 13 ȯ�濡�� �����Ǿ����ϴ�.

NHN NEXT 2014�� 1�б� ��Ʈ��ũ ����
UDP �� �̿��Ͽ� pdf ������ �����ϰ� �����ϴ� ���α׷��� �ۼ��Ͻÿ�. 

����;
http://cafe.naver.com/dm202/27
http://blog.naver.com/nature128?Redirect=Log&logNo=130071446282
*/

#include "stdafx.h"
#include <stdio.h>	// printf, fprintf
#include <stdlib.h>	// atoi
#include <string.h>	// memset

#include <fcntl.h>	// fcntl
#include <signal.h>	// signal, SIGALRM
#include <errno.h>	// errno

/*
//���н� ȯ�濡�� ������ �� ��Ŭ���
#include <sys/socket.h>	// socket, bind
#include <sys/file.h>	// O_NONBLOCK, FASYNC
#include <arpa/inet.h>	// sockaddr_in, inet_ntoa
#include <unistd.h>	// close
*/

//������ ȯ�濡�� ������ �� ��Ŭ���
#include <WinSock2.h> // windows socket API

#define BUF_SIZE 64

#pragma comment ( lib, "Ws2_32.lib" )

void HandleError( char *message );

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	SOCKET servSock;
	char message[BUF_SIZE];
	int strLen;
	int clntAdrSz;

	SOCKADDR_IN servAdr, clntAdr;

	if ( argc != 2 )
	{
		printf_s( "Usage : %s <port>\n", argv[0] );
		exit( 1 );
	}

	if ( WSAStartup( MAKEWORD(2,2), &wsaData ) != 0 ) // ������ ���� ���̺귯�� �ʱ�ȭ  
	{
		HandleError( "WSAStartup Error" );
	}

	servSock = socket( PF_INET, SOCK_DGRAM, 0 ); // UDP ���� �Ҵ��� ���� SOCK_DGRAM ���� ����
	if ( servSock == INVALID_SOCKET )
	{
		HandleError( "UDP Socket Creation Error" );
	}
	
	//����ü ���� servAdr�� ���� ������ IP�� PORT ������ �ʱ�ȭ
	memset( &servAdr, 0, sizeof(servAdr) );
	servAdr.sin_family	= AF_INET;
	servAdr.sin_addr.s_addr = htonl( INADDR_ANY );
	servAdr.sin_port = htons( atoi( (const char*)(argv[1]) ) );
	
	//bind �Լ� ȣ��
	if ( bind( servSock, (SOCKADDR*) &servAdr, sizeof( servAdr ) ) == SOCKET_ERROR )
	{
		HandleError( "binding Error\n" );
	}

	while(1)
	{
		clntAdrSz = sizeof( clntAdr );
		strLen = recvfrom( servSock, message, BUF_SIZE, 0, (SOCKADDR*) &clntAdr, &clntAdrSz );
		sendto(servSock, message, strLen, 0, (SOCKADDR*)&clntAdr, sizeof(clntAdr)); //���н� ȯ��� ������ ȯ���� sendto, recvfrom�� ���, �Ű������� ������ �����ϴ�.
	}

	closesocket( servSock ); // ������ �ݴ´�
	WSACleanup();
	
	return 0;
}

void HandleError( char *message )
{
	fputs( message, stderr );
	fputc( '\n', stderr );
	exit( 1 );
}

