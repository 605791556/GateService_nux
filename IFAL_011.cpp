#include <iostream>
#include "globle.h"
#include "IFAL_011.h"

CIFAL_078::CIFAL_078(void)
{
	memset(msgType,0,sizeof(msgType));
	memset(deviceID,0,sizeof(deviceID));
}


CIFAL_078::~CIFAL_078(void)
{

}

bool CIFAL_078::ParseData(unsigned char* data)
{
	try
	{
		memcpy(deviceID,data+2,10);
		RepeatCount = data[12];

		Log("设备：%s 事件状态码数:%d",deviceID,RepeatCount);
		for (int i=0;i<RepeatCount;i++)
		{
			char tmpData[MAX_BUFFER]={0};
			memcpy(tmpData,data+13+52*i,52);
			GATE_EVENT statu;
			memcpy(statu.EventID,tmpData,2);
			memcpy(statu.StatusID,tmpData+2,3);
			statu.StatusLevel = tmpData[5];
			memcpy(statu.EventArgument,tmpData+6,32);
			memcpy(statu.OccurredTOD,tmpData+38,14);
			vGateEvent.push_back(statu);
			Log("EventID:%s statusID:%s statusLevel:%c argument:%s occurredTOD:%s",
				statu.EventID,statu.StatusID,statu.StatusLevel,statu.EventArgument,statu.OccurredTOD);
		}
	}
	catch (...)
	{
		Log("CIFAL_078::ParseData error!");
		return false;
	}
	return true;
}
