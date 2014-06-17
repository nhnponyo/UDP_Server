// UDP_Client.cpp : Defines the entry point for the console application.
//
/*
Author	: 손채원 (github nhnponyo)
Date	: 2014.06.16

Windows 7, Visual Studio 13 환경에서 구현되었습니다.

NHN NEXT 2014년 1학기 네트워크 과제
UDP 를 이용하여 pdf 파일을 안전하게 전송하는 프로그램을 작성하시오.
*/

#include "stdafx.h"
#include <stdio.h>	// printf, fprintf
#include <stdlib.h>	// atoi
#include <string.h>	// memset

/*
//유닉스 환경에서 컴파일 시 인클루드
#include <sys/socket.h>	// socket, bind
#include <sys/file.h>	// O_NONBLOCK, FASYNC
#include <arpa/inet.h>	// sockaddr_in, inet_ntoa
#include <unistd.h>	// close
*/

//윈도우 환경에서 컴파일 시 인클루드
#include <WinSock2.h> // windows socket API

#define BUF_SIZE 512

#pragma comment ( lib, "Ws2_32.lib" )

void HandleError( char *message );

int _tmain(int argc, _TCHAR* argv[])
{
	FILE *fp;
	int retval;
	WSADATA wsaData;

	if ( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) != 0 ) // 윈도우 소켓 라이브러리 초기화  
	{
		HandleError( "WSAStartup Error" );
	}

	SOCKET sock = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( sock == INVALID_SOCKET )
	{
		HandleError( "UDP Socket Creation Error" );
	}

	SOCKADDR_IN serveraddr;
	ZeroMemory( &serveraddr, sizeof( serveraddr ) );
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons( 9019 );
	serveraddr.sin_addr.s_addr = inet_addr( "127.0.0.1" ); //주소 설정

	SOCKADDR_IN peeraddr;
	int addrlen = sizeof( peeraddr );
	char buf[BUF_SIZE + 1];

	printf_s( "전송할 파일 이름을 입력하세요\n" );
	gets_s( buf );
	//scanf_s( "%s", buf ); 왜인지 아무리 입력해도 버퍼에 아무것도 들어가질 않음. 뭐가문제인지 모르겠음
	printf_s( "%s\n", buf );
	fopen_s( &fp, buf, "rb" );

	if ( fp == NULL )
	{
		HandleError( "File Not Found Error" );
	}

	retval = sendto( sock, buf, strlen( buf ), 0, (SOCKADDR *) &serveraddr, sizeof( serveraddr ) );
	if ( retval == SOCKET_ERROR )
	{
		HandleError( "File Name send Error" );
		return 0;
	}

	// 파일 크기 얻기
	fseek( fp, 0, SEEK_END ); //파일 스트림 위치를 맨뒤로 옮기고
	int totalbytes = ftell( fp ); //파일 크기 얻기 
	retval = sendto( sock, (char*) &totalbytes, sizeof( totalbytes ), 0, (SOCKADDR *) &serveraddr, sizeof( serveraddr ) );
	if ( retval == SOCKET_ERROR )
	{
		HandleError( "File Size send Error" );
	}
	rewind( fp );//파일 위치를 맨앞으로 
	int numread;
	int recv_size = 0;
	while ( 1 )
	{
		ZeroMemory( buf, BUF_SIZE );
		//파일에서 데이터 읽음 
		numread = fread( buf, 1, BUF_SIZE, fp );
		if ( numread > 0 )
		{
			//파일을 읽은 데이터를 보냄(읽은 크기 만큼) 
			retval = sendto( sock, buf, numread, 0, (SOCKADDR *) &serveraddr, sizeof( serveraddr ) );
			if ( retval == SOCKET_ERROR )
			{
				HandleError( "File send Error" );
				continue;
			}
			//서버에서 저장 성공한 곳까지의 크기 받음 
			retval = recvfrom( sock, (char*) &recv_size, sizeof( recv_size ), 0, (SOCKADDR *) &peeraddr, &addrlen );
			if ( retval == SOCKET_ERROR )
			{
				HandleError( "File Size recvfrom Error" );
				continue;
			}
		}
		else
		{
			break;
		}
	}

	closesocket( sock );
	WSACleanup();

	return 0;
}

void HandleError( char *message )
{
	fputs( message, stderr );
	fputc( '\n', stderr );
	getchar();
	exit( 1 );
}


