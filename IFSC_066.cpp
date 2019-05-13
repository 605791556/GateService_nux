#include <iostream>
#include "globle.h"
#include "IFSC_066.h"

CIFSC_066::CIFSC_066(void)
{
	memset(msgType,0,sizeof(msgType));
	memset(deviceID,0,sizeof(deviceID));
	memset(number,0,sizeof(number));
	memset(direction,0,sizeof(direction));
	memset(ErrorDescription,0,sizeof(ErrorDescription));
	memset(codeLength,0,sizeof(codeLength));
	memset(OccurredTOD,0,sizeof(OccurredTOD));
	code = NULL;
}


CIFSC_066::~CIFSC_066(void)
{
	if (code)
	{
		delete[] code;
	}
}

bool CIFSC_066::ParseData(unsigned char* data)
{
	try
	{
		memcpy(msgType,data,2);
		memcpy(deviceID,data+2,10);
		memcpy(number,data+12,17);
		result = data[29];
		memcpy(ErrorDescription,data+30,16);
		memcpy(direction,data+46,1);
		memcpy(codeLength,data+47,2);

		if (g_globle->EndianJudge())
		{
			CodeLen = ZZ_SWAP_2BYTE(*(short*)codeLength);
		}
		else
		{
			CodeLen = *(short*)codeLength;
		}
		code = new unsigned char[CodeLen+1];
		memset(code,0,CodeLen+1);
		memcpy(code,data+49,CodeLen);
		memcpy(OccurredTOD,data+49+CodeLen,14);

		Log("msgType:%s deviceID:%s number:%s result:%c direction:%c TD_Code_Length:%d TD_Code:%s OccurredTOD:%s",msgType,deviceID,number,result,direction,CodeLen,code,OccurredTOD);
		return true;
	}
	catch (...)
	{
		Log("CIFSC_066::ParseData error!");
		return false;
	}
}
