#include <iostream>
#include "globle.h"
#include "IFSC_085.h"


CIFSC_085::CIFSC_085(void)
{
	memset(msgType,0,sizeof(msgType));
	memset(deviceID,0,sizeof(deviceID));
	memset(TDCodeLength,0,sizeof(TDCodeLength));
	memset(FacePicLength,0,sizeof(FacePicLength));
	TD_Code=NULL;
	FacePicData=NULL;
}

CIFSC_085::~CIFSC_085(void)
{
	if (TD_Code)
	{
		delete[] TD_Code;
	}
	if (FacePicData)
	{
		delete[] FacePicData;
	}
}

bool CIFSC_085::ParseData(unsigned char* data)
{
	try
	{
		memcpy(msgType,data,2);
		memcpy(deviceID,data+2,10);
		EnterOutFlag=data[12];
		memcpy(TDCodeLength,data+13,2);

		if (g_globle->EndianJudge())
		{
			CodeLen = ZZ_SWAP_2BYTE(*(short*)TDCodeLength);
		}
		else
		{
			CodeLen=*(short*)TDCodeLength;
		}
		TD_Code=new char[CodeLen+1];
		memset(TD_Code,0,CodeLen+1);
		memcpy(TD_Code,data+15,CodeLen);
		memcpy(FacePicLength,data+15+CodeLen,2);

		if (g_globle->EndianJudge())
		{
			faceLen = ZZ_SWAP_2BYTE(*(short*)FacePicLength);
		}
		else
		{
			faceLen=*(short*)FacePicLength;
		}
		FacePicData=new unsigned char[faceLen+1];
		memset(FacePicData,0,faceLen+1);
		memcpy(FacePicData,data+17+CodeLen,faceLen);

		Log("msgType:%s deviceID:%s EnOutFlag:%c TD_Code_Length:%d TD_Code:%s FacePicLength:%d FacePicData:%s",
			msgType,deviceID,EnterOutFlag,CodeLen,TD_Code,faceLen,FacePicData);
		return true;
	}
	catch (...)
	{
		Log("CIFSC_085::ParseData error!");
		return false;
	}
}
