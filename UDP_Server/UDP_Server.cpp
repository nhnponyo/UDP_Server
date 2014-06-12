// UDP_Server.cpp : Defines the entry point for the console application.
//
/*
Author	: 손채원 (github nhnponyo)
Date	: 2014.06.11

Windows 7, Visual Studio 13 환경에서 구현되었습니다.

NHN NEXT 2014년 1학기 네트워크 과제
UDP 를 이용하여 pdf 파일을 안전하게 전송하는 프로그램을 작성하시오. 

참고;
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
//유닉스 환경에서 컴파일 시 인클루드
#include <sys/socket.h>	// socket, bind
#include <sys/file.h>	// O_NONBLOCK, FASYNC
#include <arpa/inet.h>	// sockaddr_in, inet_ntoa
#include <unistd.h>	// close
*/

//윈도우 환경에서 컴파일 시 인클루드
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

	if ( WSAStartup( MAKEWORD(2,2), &wsaData ) != 0 ) // 윈도우 소켓 라이브러리 초기화  
	{
		HandleError( "WSAStartup Error" );
	}

	servSock = socket( PF_INET, SOCK_DGRAM, 0 ); // UDP 소켓 할당을 위해 SOCK_DGRAM 인자 전달
	if ( servSock == INVALID_SOCKET )
	{
		HandleError( "UDP Socket Creation Error" );
	}
	
	//구조체 변수 servAdr에 서버 소켓의 IP와 PORT 정보를 초기화
	memset( &servAdr, 0, sizeof(servAdr) );
	servAdr.sin_family	= AF_INET;
	servAdr.sin_addr.s_addr = htonl( INADDR_ANY );
	servAdr.sin_port = htons( atoi( (const char*)(argv[1]) ) );
	
	//bind 함수 호출
	if ( bind( servSock, (SOCKADDR*) &servAdr, sizeof( servAdr ) ) == SOCKET_ERROR )
	{
		HandleError( "binding Error\n" );
	}

	while(1)
	{
		clntAdrSz = sizeof( clntAdr );
		strLen = recvfrom( servSock, message, BUF_SIZE, 0, (SOCKADDR*) &clntAdr, &clntAdrSz );
		sendto(servSock, message, strLen, 0, (SOCKADDR*)&clntAdr, sizeof(clntAdr)); //유닉스 환경과 윈도우 환경의 sendto, recvfrom는 기능, 매개변수가 완전히 동일하다.
	}

	closesocket( servSock ); // 소켓을 닫는다
	WSACleanup();
	
	return 0;
}

void HandleError( char *message )
{
	fputs( message, stderr );
	fputc( '\n', stderr );
	exit( 1 );
}

