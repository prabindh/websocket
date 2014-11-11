#ifndef _LHS_WEBSOCKET_H
#define _LHS_WEBSOCKET_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef union __WEB_SOCKET_VARDATA1
{
	unsigned int extendedpayloadLen : 16;
	unsigned int maskingKey0 : 16;
	unsigned int payloadData0 : 16;
}WEB_SOCKET_VARDATA1;

typedef struct __WEB_SOCKET_FRAME
{
	unsigned short fin : 1;
	unsigned short rsv1 : 1;
	unsigned short rsv2 : 1;
	unsigned short rsv3 : 1;
	unsigned short opcode : 4;
	unsigned short mask : 1;
	unsigned short payloadLen : 7;
	WEB_SOCKET_VARDATA1 varData1;
	char *varData2;
}WEB_SOCKET_FRAME;

#ifdef _WIN32
#define INT64 __int64
#else
#define INT64 int64_t
#endif


void websocket_unmask(char* in, char* out, char* mask, unsigned int dataByteCnt);
void websocket_get_payload_mask(char* start, unsigned int maskoffset, char* outmask);
void websocket_get_payload_info(WEB_SOCKET_FRAME* inframe, INT64 *outlen, unsigned int* payloadOffsetbytes, unsigned int* maskOffsetbytes);
void websocket_get_frame(char* inBuff, WEB_SOCKET_FRAME* frame);
void websocket_peek_create_text_frame(char* inText, int* outLenBytes);
void websocket_create_text_frame(char* inText, char* outBuff);
void websocket_peek_create_binary_frame(unsigned int lenBytes, int* outLenBytes);
void websocket_create_binary_frame(char* inData, unsigned int inLenBytes, char* outBuff);


void websocket_process_frame(char* inBuff, int bufLen, int* outClose);
unsigned int parse_response_for_single_header( char* inBuff,  char* responseBuff,int responseBuffLen);

void websocket_form_system_info(char* inBuff, unsigned int lenBytes);

void websocket_search_register(char* addr);

#endif//#define _LHS_WEBSOCKET_H



