#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "b64.h"
static char b64_array[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//should be xXjDGYUmKxnwqr5KXNPGldn5LbA=

//process multiple of 3bytes, 4hexabits - works only for 20 bytes due to isFinal
static unsigned int b64_process_core(char* inBuff, int charLen, char* outBuff, int* outLen, int isFinal)
{
	int i,j ;
	if(charLen%3) return 1;
	for(i = 0,j=0;i < charLen;i += 3, j += 4)
	{
		char temp1,temp2,temp3, temp4;
		temp1 = (inBuff[i] & 0xFC) >> 2;
		temp2 = ((inBuff[i] & 0x3) << 4) | ((inBuff[i+1] & 0xF0) >> 4);
		temp3 = ((inBuff[i+1] & 0xF) << 2) | ((inBuff[i+2] & 0xC0) >> 6);
		temp4 = (inBuff[i+2] & 0x3F);

		outBuff[j] = b64_array[temp1];
		outBuff[j+1] = b64_array[temp2];
		outBuff[j+2] = b64_array[temp3];
		if(isFinal == 1)
			outBuff[j+3] = '=';
		else
			outBuff[j+3] = b64_array[temp4];
	}
	*outLen = j;
	return 0;
}

unsigned int b64_process(char* inBuff, int charLen, char* outBuff)
{
	char tempBuff[3];
	int outLen = 0;
	int rem = charLen % 3;
	if(charLen >= 3)
		b64_process_core(inBuff, charLen-rem,outBuff, &outLen,0);
	//2 should remain for 20%3, so add 1
	tempBuff[0] = inBuff[charLen - 2];
	tempBuff[1] = inBuff[charLen - 1];
	tempBuff[2] = '\0';
	b64_process_core(tempBuff,3,&outBuff[outLen],&outLen,1);
	return 0;
}



