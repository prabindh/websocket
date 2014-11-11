// localhost_server.cpp : Defines the entry point for the console application.
// Simple server for demo purpose. Handles only one point connection to the :80 http socket in Windows. Handles multiple clients on Linux.
// Prabindh Sundareson, prabindh@yahoo.com

#ifdef WIN32
#include <winsock2.h>
#endif
#include "sha1.h"
#include "b64.h"

#include "localhost_server_websocket.h"

#define _DEBUG

//inBuff has to be null terminated
unsigned int parse_response_for_single_header(
									   char* inBuff,
									   char* responseBuff,
									   int responseBuffLen
									   )
{
	int delta = 0;
	char* keyEnd = strstr(inBuff,":");
	if(keyEnd == NULL)
		return 1;
	char* end = strstr(keyEnd,"\r\n");
	if(end == NULL)
		return 1;
	delta = 0;
	//allow for : and 1 space
	if(end-(keyEnd+2) > responseBuffLen)
		return 1;
	memcpy(responseBuff,keyEnd+2,end-(keyEnd+2));
	return 0;
}


void websocket_unmask(char* in, char* out, char* mask, unsigned int dataByteCnt)
{
	unsigned int i, j;
	for(i = 0;i < dataByteCnt;i ++)
	{
		j = i % 4;
		out[i] = in[i] ^ mask[j];
	}
}

void websocket_get_payload_mask(char* start, unsigned int maskoffset, char* outmask)
{
	outmask[0] = start[maskoffset];
	outmask[1] = start[maskoffset + 1];
	outmask[2] = start[maskoffset + 2];
	outmask[3] = start[maskoffset + 3];
}

void websocket_get_payload_info(WEB_SOCKET_FRAME* inframe, INT64 *outlen, unsigned int* payloadOffsetbytes, unsigned int* maskOffsetbytes)
{
	*outlen = 0;
	if(inframe->payloadLen <= 125)
	{
		*outlen = inframe->payloadLen;
		*payloadOffsetbytes = (inframe->mask ?  4 + 2 : 0 + 2);
		*maskOffsetbytes = 0 + 2;
	}
	else if(inframe->payloadLen == 126)
	{
		*outlen = inframe->varData1.extendedpayloadLen;
		*payloadOffsetbytes = (inframe->mask ?  6 + 2 : 2 + 2);
		*maskOffsetbytes = 2 + 2;
	}
	else if(inframe->payloadLen == 127)
	{
		*outlen = ((INT64)inframe->varData1.extendedpayloadLen << 48) | ((INT64)inframe->varData2[0] << 40) | ((INT64)inframe->varData2[1] << 32);
		*outlen |= (inframe->varData2[2] << 24) | (inframe->varData2[3] << 16) | (inframe->varData2[4] << 8) | (inframe->varData2[5]);
		*payloadOffsetbytes = (inframe->mask ?  12 + 2 : 8 + 2);
		*maskOffsetbytes = 8 + 2;
	}
}

void websocket_get_frame(char* inBuff, WEB_SOCKET_FRAME* frame)
{
	frame->fin = (inBuff[0] & 0x80) >> 7;
	frame->rsv1 = (inBuff[0] & 0x40) >> 6;
	frame->rsv2 = (inBuff[0] & 0x20) >> 5;
	frame->rsv3 = (inBuff[0] & 0x10) >> 4;
	frame->opcode = (inBuff[0] & 0xF);
	frame->mask = (inBuff[1] & 0x80) >> 7;
	frame->payloadLen = (inBuff[1] & 0x7F);
}

void websocket_peek_create_binary_frame(unsigned int inLenBytes, int* outLenBytes, int* payloadOffset)
{
        //no mask, payload only
	if(inLenBytes <= 125)
	{
	        *outLenBytes = 2 + inLenBytes;
		*payloadOffset = 2;
	}
	else if(inLenBytes < 0xFFFF)
	{
		*outLenBytes = 4 + inLenBytes;
		*payloadOffset = 4;
	}
	//else - TODO
}

void websocket_peek_create_text_frame(char* inText, int* outLenBytes)
{
	//no mask, payload
	*outLenBytes = 2 + strlen(inText);
}

void websocket_create_binary_frame(char* inBuff, unsigned int inLenBytes, char* outBuff)
{
        int i;
	int offset = 0;

        outBuff[0] = 0;
        outBuff[0] = (1 << 7);
        outBuff[0] |= 0x2;
        if(inLenBytes <= 125)
        {
                outBuff[1] = inLenBytes;
		offset = 2;
        }
        else if(inLenBytes < 0xFFFF)
        {
                outBuff[1] = 126;
                outBuff[2] = (inLenBytes & 0xFF);
                outBuff[3] = (inLenBytes >> 8)& 0xFF;
		offset = 4;
        }
	//else - TODO
        for(i = 0;i < inLenBytes;i ++)
                outBuff[offset+i] = inBuff[i];


}



void websocket_create_text_frame(char* inText, char* outBuff)
{
	int i;

	outBuff[0] = 0;
	outBuff[0] = (1 << 7);
	outBuff[0] |= 0x1;
	outBuff[1] = strlen(inText);
	for(i = 0;i < strlen(inText);i ++)
		outBuff[2+i] = inText[i];
}

void __websocket_create_text_frame(unsigned int stringLen, char* outBuff)
{
        outBuff[0] = 0;
        outBuff[0] = (1 << 7);
        outBuff[0] |= 0x1;
        outBuff[1] = stringLen;
}


void __websocket_create_binary_frame(unsigned int byteLen, char* outBuff)
{
        outBuff[0] = 0;
        outBuff[0] = (1 << 7);
        outBuff[0] |= 0x2;
	if(byteLen <= 125)
	{
        	outBuff[1] = byteLen;
	}
	else if(byteLen < 0xFFFF)
	{
		outBuff[1] = 126;
		outBuff[2] = (byteLen >> 8) & 0xFF;
		outBuff[3] = byteLen & 0xFF;
	}
}


unsigned int __localhost_send(				  
		char* outBuff, 
		 int outBufLen
		 );

void websocket_analyse_payload(char* payloadData, INT64 payloadLen)
{
	unsigned int cmd;
	if(payloadData[0]== 'C' && payloadData[1]== 'O' && payloadData[2]== 'M' && payloadData[3]== 'D')
	{
		cmd = (payloadData[4]-0x30) << 8 | (payloadData[5]-0x30);
	}
	else
		return;
	if(cmd == 1)
	{
		//Run command and return
#ifdef _DEBUG
		printf("cmd 1 processing...\n");
#endif
	}
}


void websocket_process_frame(char* inBuff, int bufLen, int* outClose)
{
	INT64 payloadLen;
	unsigned int byteCnt;
	unsigned int payloadOffset;
	unsigned int maskOffset;
	char mask[4];
	WEB_SOCKET_FRAME startFrame;
	char* payloadData;

	/* Handle WS connection */
	websocket_get_frame(inBuff, &startFrame);
	if(startFrame.mask)
	{
		websocket_get_payload_info(&startFrame, &payloadLen, &payloadOffset, &maskOffset);
		//Cant really handle large data right now
		if(payloadLen > bufLen)
		{
			printf("Error: payloadLen very large!\n");
			//close connection
			*outClose = 1;
			return;
		}
		payloadData = (char*)malloc(payloadLen+1);
		memset(payloadData, 0, payloadLen+1);
		websocket_get_payload_mask(inBuff, maskOffset, mask);
		websocket_unmask((char*)inBuff+payloadOffset, payloadData, mask, payloadLen);
		websocket_analyse_payload(payloadData, payloadLen);
		free(payloadData);

		//set close connection bit as client wants
		*outClose = ((startFrame.opcode == 8) ? 1 : 0);
		printf("closeBit = %d\n", *outClose);
	}
}
void websocket_form_system_info(char* inBuff, unsigned int lenBytes)
{
	int size = 0;
	//Perform requested operation
	system("rm ./dump.log");
	system("uname -a > ./dump.log");
	FILE *dumpfile = fopen("./dump.log", "rb");
	
	int actualLen = fread(&inBuff[2+size],1,lenBytes,dumpfile); 
	fclose(dumpfile);
	__websocket_create_binary_frame(actualLen, inBuff);
	return;
}

bool __addr_sane(char* addr)
{
	int i;
	if(addr[0] != '0') return false;
	if(addr[1] != 'x') return false;
	for(i = 2;i < 10;i ++)
	{
		if(addr[i] < '0' || addr[i] > 'z') return false;
                if(addr[i] < 'a' && addr[i] > '9') return false;
	}
	return true;
}



