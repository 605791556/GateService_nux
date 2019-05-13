#include <iostream>
#include "globle.h"
#include "IFSC_062.h"

using namespace std;

CIFSC_062::CIFSC_062()
{
	memset(DeviceID,0,10);
	memset(EnOutFlag,0,2);
	TDCodeLen = 0;
	TDCode=NULL;
}

CIFSC_062::~CIFSC_062()
{
	if (TDCode)
	{
		delete[] TDCode;
	}
}

bool CIFSC_062::ParseData(unsigned char* data)
{
	try
	{
		memcpy(DeviceID,data+2,10);
		memcpy(EnOutFlag,data+12,1);
		char CodeLen[2]={0};
		memcpy(CodeLen,data+13,2);
		if (g_globle->EndianJudge())
		{
			TDCodeLen = ZZ_SWAP_2BYTE(*(short*)CodeLen);
		}
		else
		{
			TDCodeLen=*(short*)CodeLen;
		}
		TDCode=new unsigned char[TDCodeLen+1];
		memset(TDCode,0,TDCodeLen+1);
		memcpy(TDCode,data+15,TDCodeLen);

		Log("msgType:62 deviceID:%s EnOutFlag:%c TD_Code_Length:%d TD_Code:%s",DeviceID,EnOutFlag[0],TDCodeLen,TDCode);
	}
	catch (...)
	{
		Log("CIFSC_062::ParseData error!");
		return false;
	}
	return true;
}