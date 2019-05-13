#include <iostream>
#include <time.h>
#include "globle.h"
#include "IFSC_061.h"

using namespace std;

CIFSC_061::CIFSC_061(char* _deviceID)
{
	memcpy(msgType,"61",2);//消息类型
	memcpy(deviceID,_deviceID,sizeof(deviceID));//设备ID
	memset(result,0,sizeof(result));
	AlermGulp=0x02;
	MessageLen=0;
	ClientCount=0;
	PrintCount=0;
	Message=NULL;
}


CIFSC_061::~CIFSC_061(void)
{
	if (Message)
	{
		delete[] Message;
	}
}

bool CIFSC_061::ParseData(const char* data)
{
	try
	{
		Json::Reader r;
		Json::Value root;
		r.parse(data,root);
		string strResult = root["result"].asString();
		memcpy(result,strResult.c_str(),sizeof(result));
		string strMsg = root["msg"].asString();
#ifdef WIN32
		string strGMsg = g_globle->U2G(strMsg.c_str(),strlen(strMsg.c_str()));
		MessageLen=strlen(strGMsg.c_str());
		Message = new char[MessageLen];
		memcpy(Message,strGMsg.c_str(),MessageLen);
#else
		char dstbuf[256]={0};
		g_globle->utf8togb2312((char*)strMsg.c_str(),strlen(strMsg.c_str()),dstbuf,256);
		string strGMsg=dstbuf;
		MessageLen=strlen(strGMsg.c_str());
		Message = new char[MessageLen];
		memcpy(Message,strGMsg.c_str(),MessageLen);
#endif
		Log("过闸验证结果：result:%s message:%s",result,strGMsg.c_str());
	}
	catch (exception* e)
	{
		Log("CIFSC_061::ParseData error :%s",e);
		return false;
	}
	return true;
}

char* CIFSC_061::CombineData(int& datalen)
{
	char* sbuf=new char[35+MessageLen];
	memset(sbuf,0x00,35+MessageLen);

	memcpy(sbuf,msgType,2);
	memcpy(sbuf+2,deviceID,10);
	memcpy(sbuf+12,result,2);

	time_t rawtime;
	struct tm *ptminfo;
	time(&rawtime);
	ptminfo = localtime(&rawtime);
	char tmp[64]={0};
	sprintf(tmp,"%04d%02d%02d%02d%02d%02d",
		ptminfo->tm_year + 1900, ptminfo->tm_mon + 1, ptminfo->tm_mday,
		ptminfo->tm_hour, ptminfo->tm_min, ptminfo->tm_sec);
	memcpy(number,tmp,sizeof(number));
	memcpy(sbuf+14,number,17);

	sbuf[31]=AlermGulp;
	sbuf[32]=MessageLen;
	memcpy(sbuf+33,Message,MessageLen);
	sbuf[33+MessageLen]=ClientCount;
	sbuf[34+MessageLen]=PrintCount;
	//char prtlen[3]={0x00};
	//memcpy(sbuf+35+MessageLen,prtlen,2);
	datalen = 35+MessageLen;
	return sbuf;
}
