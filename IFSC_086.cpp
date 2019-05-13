#include <iostream>
#include "globle.h"
#include "IFSC_086.h"


CIFSC_086::CIFSC_086(const char* _deviceID)
{
	faceid="";
	memcpy(msgType,"86",2);//消息类型
	memcpy(deviceID,_deviceID,11);//设备ID
	memset(ValidationResults,0,sizeof(ValidationResults));
}


CIFSC_086::~CIFSC_086(void)
{
}

bool CIFSC_086::ParseData(string strData)
{
	try
	{
		Json::Reader r;
		Json::Value root;
		r.parse(strData,root);
		
		string result=root["result"].asString();
		memcpy(ValidationResults,result.c_str(),2);
#ifdef WIN32
		strMsg=g_globle->U2G(root["msg"].asString().c_str(),strlen(root["msg"].asString().c_str()));
#else
		string strTmp;
		strTmp=root["msg"].asString();
		char dstbuf[256]={0};
		g_globle->utf8togb2312((char*)strTmp.c_str(),strlen(strTmp.c_str()),dstbuf,256);
		strMsg=dstbuf;
#endif
		if (strcmp(ValidationResults,"05")==0)
		{
			faceid = root["face_id"].asString();
		}
		Log("人脸验证结果 result:%s msg:%s",result.c_str(),strMsg.c_str());
	}
	catch (exception* e)
	{
		Log("AddGateList error :%s",e);
		return false;
	}
	return true;
}

char* CIFSC_086::CombineData(int& datalen)
{
	//“05”此票已存在人脸(此时终端需要验证此接口返回人脸数据与实时人脸对比)
	uint16_t faceLen = faceid.length();
	if (strcmp(ValidationResults,"05")==0)
	{
		datalen = faceLen+16;
	}
	else
	{
		datalen=16;
	}

	char* sbuf=new char[datalen];
	memset(sbuf,0x00,datalen);
	try
	{
		memcpy(sbuf,msgType,2);
		memcpy(sbuf+2,deviceID,10);
		memcpy(sbuf+12,ValidationResults,2);
		if (strcmp(ValidationResults,"05")==0)
		{
			uint16_t zzFaceLen=faceLen;
			if (g_globle->EndianJudge())
			{
				zzFaceLen = ZZ_SWAP_2BYTE(zzFaceLen);
			}
			memcpy(sbuf+14, (char*)&zzFaceLen,2);
			memcpy(sbuf+16,faceid.c_str(),faceLen);

			Log("msgType:%s deviceID:%s ValidationResults:%s FacePicLength:%d FacePicData:%s",
				msgType,deviceID,ValidationResults,faceLen,faceid.c_str());
		}
		else
		{
			uint16_t faceLen = 0;
			memcpy(sbuf+14, (char*)&faceLen,2);
			Log("msgType:%s deviceID:%s ValidationResults:%s",msgType,deviceID,ValidationResults);
		}
	}
	catch (...)
	{
		Log("CIFSC_086::CombineData error");
		delete[] sbuf;
		sbuf = NULL;
	}
	return sbuf;
}
