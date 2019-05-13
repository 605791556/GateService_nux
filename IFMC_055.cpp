#include <iostream>
#include "IFMC_055.h"


CIFMC_055::CIFMC_055(string strDeviceID,string commonCode)
{
	memcpy(msgType,"55",sizeof(msgType));
	memset(deviceID,0,sizeof(deviceID));
	memcpy(deviceID,strDeviceID.c_str(),sizeof(deviceID));
	memcpy(controlCode,commonCode.c_str(),sizeof(controlCode));
}


CIFMC_055::~CIFMC_055(void)
{
}

bool CIFMC_055::ParseData(unsigned char* data)
{
	return true;
}

char* CIFMC_055::CombineData(int& len)
{
	char* sbuf=new char[16];

	memcpy(sbuf,msgType,2);
	memcpy(sbuf+2,deviceID,10);
	memcpy(sbuf+12,controlCode,4);
	len=16;
	return sbuf;
}
