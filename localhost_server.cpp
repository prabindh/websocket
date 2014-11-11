// localhost_server.cpp : Defines the entry point for the console application.
// Simple server for demo purpose. Handles only one point connection to the :80 http socket in Windows. Handles multiple clients on Linux.
// Prabindh Sundareson, prabindh@yahoo.com

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "sys/time.h"
#else
#include "stdafx.h"
#include <winsock2.h>
#include <sys/timeb.h>
#endif

#include "index.html.h"

#include "sha1.h"
#include "b64.h"

#include "localhost_server_websocket.h"


#define __DEBUG

#ifndef WIN32
typedef unsigned int SOCKET;
#define SOCKET_ERROR (-1)
#define INVALID_SOCKET (-1)
typedef unsigned int UINT32;
typedef char _TCHAR;
#define closesocket close
#define _tmain main
#endif

extern unsigned int LoadCheckTimerSlot();
extern unsigned int get_cpu_load();

timeval startTime, endTime;
unsigned int diffTime2;

unsigned int localhost_init()
{
#ifdef WIN32
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(iResult != 0)
		return 1;
#endif
	return 0;
}

unsigned int localhost_uninit()
{
#ifdef WIN32
	WSACleanup();
#endif
	return 0;
}

//create a listener
unsigned int localhost_create_listener(SOCKET *pSocket, sockaddr_in* pAddress)
{
	int err;
	//try to bind
	*pSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(*pSocket == INVALID_SOCKET)
		return 2;
	 err = bind(*pSocket,(sockaddr*)pAddress,sizeof(sockaddr_in));
	if(err == SOCKET_ERROR)
	{
#if defined __DEBUG && defined WIN32
		int wserr = WSAGetLastError(); //10054-connection reset
#endif
		return 1;
	}
	 err = listen(*pSocket, 1);
	if(err == SOCKET_ERROR)
	{
#if defined __DEBUG && defined WIN32
		int wserr = WSAGetLastError(); //10054-connection reset
#endif
		return 1;
	}
	//setsockopt ?
	return 0;
}

UINT32 localhost_accept(SOCKET *pSocket, sockaddr_in* pAddress, SOCKET *pNewSocket)
{
	int err;
#ifdef WIN32
	int len;
#else
	socklen_t len;
#endif
	len = sizeof(sockaddr_in);
	*pNewSocket = accept(*pSocket, (sockaddr*) pAddress, &len);
	if(*pNewSocket == INVALID_SOCKET)
	{
#if defined __DEBUG && defined WIN32
		int wserr = WSAGetLastError(); //10054-connection reset
#endif
		return 2;
	}
	return 0;
}

//only receive
unsigned int localhost_recv(
				  SOCKET *pSocket,
				  char* outBuff, 
				  int outBufLen,
				  int *actualRecvd
				  )
{
	*actualRecvd = recv(*pSocket,outBuff,outBufLen,0);
	if(*actualRecvd == SOCKET_ERROR)
	{
#if defined __DEBUG && defined WIN32
		int wserr = WSAGetLastError(); //10054-connection reset, or 53 - connection aborted
#endif 
		return 1;	
	}
	return 0;
}

//only send
unsigned int localhost_send(
				  SOCKET *pSocket,
				  char* outBuff, 
				  int outBufLen
				  )
{
	int err;
	err = send(*pSocket,outBuff,outBufLen,0);
	if(err == SOCKET_ERROR)
	{
#if defined __DEBUG && defined WIN32
		int wserr = WSAGetLastError(); //10054-connection reset, or 53 - connection aborted
#endif 
		return 1;	
	}
	return 0;
}



SOCKET localSocket;
SOCKET newSocket;
sockaddr_in localAddr;
char recvBuff[1000];
char sendBuff[30000];
int portNum = 26488;//80;
int count = 0;


char tempBuff1[1000];
char keyBuff1[500];
SHA1Context shacontext;
char sha1resultBuff[500];

#ifndef WIN32
pid_t outpid;
#endif

#define WEBSOCKET_DOWN 0
#define WEBSOCKET_CONNECTED 1
unsigned int socketState = WEBSOCKET_DOWN;

unsigned int __localhost_send(				  
		char* outBuff, 
		 int outBufLen
		 )
{
	return (localhost_send(&newSocket, outBuff, outBufLen));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
* A P P L I C A T I O N     L O O P
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int _tmain(int argc, _TCHAR* argv[])
{
	int err;

	localhost_init();

	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = INADDR_ANY;
	localAddr.sin_port = htons ( portNum );

	localhost_create_listener(&localSocket, &localAddr);

	/* Respond to GET or PUT */
	while(1)
	{
		localhost_accept(&localSocket, &localAddr, &newSocket);

#ifndef WIN32
		outpid = fork();
  		//Wait in the main process
                if(outpid != 0) continue;
#endif

#ifdef __DEBUG
		printf("Received connection request @ %d\n", portNum);
#endif
		while(1)
		{
			memset(recvBuff, 0, sizeof(recvBuff));
			memset(keyBuff1, 0, sizeof(keyBuff1));
			err = localhost_recv(&newSocket, recvBuff, sizeof(recvBuff), &count);
#ifdef __DEBUG
			printf("Received HTTP request = %s\n", recvBuff);
#endif
			if((count > 0) && err >= 0)
			{
				if(strstr(recvBuff, "favicon.ico"))
				{
					strcpy(sendBuff, "HTTP/1.1 404 Not Found\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "Server: Hooya Server\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "Content-Type: text/html; charset=UTF-8\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "Content-Length: 0\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
				}
				else if(strstr(recvBuff, "getdata.dat"))
				{
#ifdef __DEBUG
					printf("got getdata.dat\n");
#endif
					static float currentTemperature = 0.0;
					currentTemperature += 0.1;
					if(currentTemperature > 0.9) currentTemperature = 0;
					char temperatureString[100];
					sprintf(temperatureString, "%5f", currentTemperature);

					strcpy(sendBuff, "HTTP/1.1 200 OK\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));		
					strcpy(sendBuff, "Server: Hooya Server\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "Content-Type: text/html; charset=UTF-8\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "Content-Length: ");
					sprintf(sendBuff+strlen(sendBuff), "%d", strlen(temperatureString));
					strcat(sendBuff, "\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, temperatureString);
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
				}
				else if(strstr(recvBuff, "putdata.dat"))
				{
					strcpy(sendBuff, "HTTP/1.1 200 OK\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));		
					strcpy(sendBuff, "Server: Hooya Server\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "Content-Type: text/html; charset=UTF-8\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "Content-Length: 7\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "PUT:200");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
				}
				else if(socketState == WEBSOCKET_CONNECTED)
				{
					int closeBit;

#ifndef WIN32
					gettimeofday(&endTime, NULL);
					diffTime2 = (endTime.tv_sec - startTime.tv_sec) * 1000 + (endTime.tv_usec - startTime.tv_usec) / 1000;
					printf("elapsed time (mSec) = %d\n", diffTime2);
#endif
					websocket_process_frame(recvBuff, sizeof(recvBuff), &closeBit);
					if(closeBit == 1)
					{
						if(newSocket) 
						{
							closesocket(newSocket);
							newSocket = 0;
						}
						if(socketState == WEBSOCKET_CONNECTED) socketState = WEBSOCKET_DOWN;
#if defined __DEBUG && !defined WIN32
						printf("closing socket, pid=%d\n", outpid);
#endif
#ifndef WIN32
						if(outpid == 0) exit(0);
						else break;
#endif
					}
					else
					{
						//send some data back to client
						//Note - the origin (in first ack) has to match for server to notice this
						int lenBytes;
						char* tempBuff;
						char tempChar[100];
						unsigned int loadData;

#if 0
						memset(sendBuff, 0, sizeof(sendBuff)); 
						//Send system info binder to client
						websocket_form_system_info(sendBuff, sizeof(sendBuff));
						localhost_send(&newSocket, sendBuff, strlen(sendBuff));
#endif

#if 0 //yes I do

#if 0
						loadData = get_cpu_load();
						websocket_peek_create_binary_frame(sizeof(loadData), &lenBytes);
						tempBuff = (char*)malloc(lenBytes);
						memset(tempBuff, 0, lenBytes);
						websocket_create_binary_frame((char*)&loadData, sizeof(loadData), tempBuff);
#else
						sprintf(tempChar, "%s", "Yes, I do!");
						websocket_peek_create_text_frame(tempChar, &lenBytes);
						tempBuff = (char*)malloc(lenBytes);
						memset(tempBuff, 0, lenBytes);
						websocket_create_text_frame(tempChar, tempBuff);
#endif
						localhost_send(&newSocket, tempBuff, lenBytes);
						free(tempBuff);
#endif //yes I do
					}
				}
				else if(strstr(recvBuff, "Sec-WebSocket-Key"))
				{
					socketState = WEBSOCKET_CONNECTED;

					strcpy(sendBuff, "HTTP/1.1 101 Switching Protocols\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "Date:Tue, 26 Jun 2013 17:38:18 GMT\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "Server:Hooya Server\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "Connection:Upgrade\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "Upgrade:websocket\r\n");
#ifndef WIN32
					gettimeofday(&startTime, NULL);
#endif					
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));

					strcpy(sendBuff, "Sec-WebSocket-Accept:");
					parse_response_for_single_header(strstr(recvBuff, "Sec-WebSocket-Key"), keyBuff1, sizeof(keyBuff1));

					strcat(keyBuff1, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
					SHA1Reset(&shacontext);
					SHA1Input(&shacontext, (const uint8_t*)keyBuff1, strlen(keyBuff1));
					SHA1Result(&shacontext, (uint8_t*)sha1resultBuff);
					memset(tempBuff1, 0,sizeof(tempBuff1));
					b64_process(sha1resultBuff,20,tempBuff1);
					strcat(sendBuff, tempBuff1);
					strcat(sendBuff, "\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));

					strcpy(sendBuff, "Access-Control-Allow-Headers:content-type, authorization, x-websocket-extensions, x-websocket-version, x-websocket-protocol\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "Access-Control-Allow-Credentials:true\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));

					//Dummy post from server
					//Check whether the client event is fired after this!! Then measure the latency! Latency is around 200 mSec
					strcpy(sendBuff, "Access-Control-Allow-Origin:http://172.24.147.179\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));

					//Send system info binder to client
					//websocket_form_system_info(sendBuff, sizeof(sendBuff));
					//localhost_send(&newSocket, sendBuff, strlen(sendBuff));

				}
				else //consider fresh
				{
					/* Send HTTP page */	
					strcpy(sendBuff, "HTTP/1.1 200 OK\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "Date: Mon, 06 May 2013 17:38:18 GMT\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "Server: Hooya Server\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "Content-Type: text/html; charset=UTF-8\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "Content-Length: ");
					sprintf(sendBuff+strlen(sendBuff), "%d",strlen(htmlpage3));
					strcat(sendBuff, "\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, "\r\n");
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
					strcpy(sendBuff, htmlpage3);
					localhost_send(&newSocket, sendBuff, strlen(sendBuff));
				}
			}//if got data
			else 
			{
				if(newSocket) 
				{
					closesocket(newSocket);
					newSocket = 0;
				}
				if(socketState == WEBSOCKET_CONNECTED) socketState = WEBSOCKET_DOWN;
				break;
			}//connection closed, wait for new connection
		}//wait for new connection
#ifndef WIN32
		if(outpid == 0) 
		{
#ifdef _DEBUG
			printf("Child exiting...");
			break;
#endif
		}
#endif
	}//while

uninit:
	if(localSocket) closesocket(localSocket);
	localhost_uninit();
	return 0;
}




