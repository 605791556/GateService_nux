#include <iostream>
#include "IFSC_075.h"


CIFSC_075::CIFSC_075(void)
{
	memset(DeviceID,0,10);
	memset(EnOutFlag,0,1);
	CardType = 0;
	PhysicalNOLength=0;
	memset(PhysicalNO,0,1024);
}


CIFSC_075::~CIFSC_075(void)
{
	memset(DeviceID,0,10);
	memset(EnOutFlag,0,1);
	CardType = 0;
	PhysicalNOLength=0;
	memset(PhysicalNO,0,1024);
}

bool CIFSC_075::ParseData(unsigned char* data)
{
	memcpy(DeviceID,data+2,10);
	memcpy(EnOutFlag,data+12,1);
	char cCardType[1]={0};
	memcpy(cCardType,data+13,1);
	CardType=*(short*)cCardType;
	char cPhyNoLen[1]={0};
	memcpy(cPhyNoLen,data+14,1);
	PhysicalNOLength=*(short*)cPhyNoLen;
	memcpy(PhysicalNO,data+15,PhysicalNOLength);
	return true;
}
