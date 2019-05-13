#include <iostream>
#include "globle.h"
#include "IFAL_074.h"


CIFAL_074::CIFAL_074(void)
{
	memset(DeviceID,0,sizeof(DeviceID));
}


CIFAL_074::~CIFAL_074(void)
{
}

bool CIFAL_074::ParseData(unsigned char* data)
{
	try
	{
		memcpy(DeviceID,data+2,10);
		Repeatcount = data[12];
		vGateStatus.clear();
		Log("上传设备：%s 状态码种数：%d",DeviceID,Repeatcount);
		for (int i=0;i<Repeatcount;i++)
		{
			char tmpData[MAX_BUFFER]={0};
			memcpy(tmpData,data+13+36*i,36);
			GATESTATUS statu;
			memcpy(statu.StatusID,tmpData,3);
			memcpy(statu.StatusLevel,tmpData+3,1);
			memcpy(statu.Argument,tmpData+4,32);
			vGateStatus.push_back(statu);
			Log("StatusID:%s Level:%s Argument:%s",statu.StatusID,statu.StatusLevel,statu.Argument);
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

Json::Value CIFAL_074::MakeJson()
{
	Json::Value root;

	int nCount = vGateStatus.size();
	for (int i=0;i<nCount;i++)
	{
		Json::Value js_one;
		js_one["dev_id"]=DeviceID;
		js_one["code"]=vGateStatus[i].StatusID;
		js_one["value"]=vGateStatus[i].StatusLevel;
#ifdef WIN32
		js_one["argument"]=g_globle->G2U(vGateStatus[i].Argument,strlen(vGateStatus[i].Argument));
#else
		string strTmp;
		char dstbuf[256]={0};
		g_globle->utf8togb2312(vGateStatus[i].Argument,strlen(vGateStatus[i].Argument),dstbuf,256);
		strTmp=dstbuf;
		js_one["argument"]=strTmp;
#endif
		
		root.append(js_one);
	}
	return root;
}
