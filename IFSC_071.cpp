#include <iostream>
#include <time.h>
#include "globle.h"
#include "IFSC_071.h"


CIFSC_071::CIFSC_071(void)
{
	memset(DeviceID,0,sizeof(DeviceID));
}


CIFSC_071::~CIFSC_071(void)
{
	memset(DeviceID,0,sizeof(DeviceID));
}

bool CIFSC_071::ParseData(unsigned char* data)
{
	memcpy(DeviceID,data+2,10);
	return true;
}

char* CIFSC_071::getUnixCurrtTime(int& dataLen)
{
	char* sData =new char[16];
	dataLen=16;
	memset(sData,0x00,16);

	try
	{
		time_t t = time(0);
		uint64_t unxTm = t;

		char msgType[]="72";//消息类型
		memcpy(sData,msgType,2);

		//Unix时间戳，13位，第一位补0
		char uTime[32]={0x00};
		sprintf(uTime,"0%ld000",unxTm);
		memcpy(sData+2,uTime,14);
	}
	catch (...)
	{
		delete[] sData;
		sData = NULL;
	}
	return sData;
}
