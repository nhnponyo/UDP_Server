﻿// UDP_Server.cpp : Defines the entry point for the console application.
//
/*
Author	: 손채원 (github nhnponyo)
Date	: 2014.06.11

Windows 7, Visual Studio 13 환경에서 구현되었습니다.

NHN NEXT 2014년 1학기 네트워크 과제
UDP 를 이용하여 pdf 파일을 안전하게 전송하는 프로그램을 작성하시오. 

참고;
http://cafe.naver.com/dm202/27
http://blog.naver.com/nature128?R edirect=Log&logNo=130071446282
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

	int retVal;
	clntAdrSz = sizeof( clntAdr );
	memset( &clntAdr, 0, sizeof( clntAdr ) );
	char buf[BUF_SIZE];

	//파일 이름 받기 
	char filename[256];
	ZeroMemory( filename, 256 );

	retVal = recvfrom( servSock, filename, 256, 0, (SOCKADDR *) &clntAdr, &clntAdrSz );//유닉스 환경과 윈도우 환경의 sendto, recvfrom는 기능, 매개변수가 완전히 동일하다.
	printf_s( "%s\n", filename );
	if ( retVal == SOCKET_ERROR )
	{
		HandleError( "File Name recvfrom Error" );
		closesocket( servSock );
		exit( 0 );
	}
	filename[retVal] = NULL;
	printf_s( "받을 파일 이름: %s\n", filename );
		
	//파일 크기 받기 
	int totalbytes;
	retVal = recvfrom( servSock, (char *) &totalbytes, sizeof( totalbytes ), 0, (SOCKADDR *) &clntAdr, &clntAdrSz );
	if ( retVal == SOCKET_ERROR )
	{
		HandleError( "File Size recvfrom Error" );
		closesocket( servSock );
	}
	printf_s( "받을 파일 크기: %d\n", totalbytes );
		
	//파일 열기 
	FILE* fp;
	fopen_s( &fp, filename, "wb" );
	if ( fp == NULL )
	{
		perror( "File IO Error" );
		closesocket( servSock );
	}

	//받은 사이즈 
	int numtotal = 0;
	do
	{
		ZeroMemory( buf, BUF_SIZE );
		//파일 데이터 받기 
		retVal = recvfrom( servSock, buf, BUF_SIZE, 0, (SOCKADDR *) &clntAdr, &clntAdrSz );
		if ( retVal == SOCKET_ERROR )
		{
			HandleError( "recvfrom()" );
			break;
		}
		else
		{
			//파일에 저장 
			fwrite( buf, 1, retVal, fp );
			if ( ferror( fp ) )
			{
				perror( "File IO Error\n" );
				break;
			}

			numtotal += retVal;
			printf_s( "Send %d\n", numtotal );
			//받은 크기를 다시 보냄
			int r = sendto( servSock, (char*) &retVal, sizeof( retVal ), 0, (SOCKADDR *) &clntAdr, clntAdrSz );
			if ( r == SOCKET_ERROR )
			{
				HandleError( "send Error" );
				continue;
			}
		}
	} while ( ( retVal == BUF_SIZE ) && ( numtotal != totalbytes ) );
	fclose( fp );
	//전송 결과 출력 
	if ( numtotal == totalbytes )
	{
		printf_s( "파일 전송 완료!\n" );
	}
	else
	{
		printf_s( "파일 전송 실패!\n" );
	}

	closesocket( servSock );
	WSACleanup();
	
	return 0;
}

void HandleError( char *message )
{
	fputs( message, stderr );
	fputc( '\n', stderr );
	exit( 1 );
}

