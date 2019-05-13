#include <iostream>
#include "globle.h"
#include "IFSC_080.h"


CIFSC_080::CIFSC_080(void)
{
	memset(msgType,0,sizeof(msgType));
	memset(deviceID,0,sizeof(deviceID));
}


CIFSC_080::~CIFSC_080(void)
{
}

bool CIFSC_080::ParseData(unsigned char* data)
{
	try
	{
		memcpy(msgType,data,2);
		memcpy(deviceID,data+2,10);
	}
	catch (...)
	{
		Log("CIFSC_080::ParseData error");
		return false;
	}
	return true;
}
