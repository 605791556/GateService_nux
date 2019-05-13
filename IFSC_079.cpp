#include <iostream>
#include "globle.h"
#include "IFSC_079.h"


CIFSC_079::CIFSC_079(void)
{
	memcpy(msgType,"79",2);
}


CIFSC_079::~CIFSC_079(void)
{
}

bool CIFSC_079::PraseData(string data)
{
	try
	{
		vFY.clear();
		Json::Reader r;
		Json::Value root;
		r.parse(data,root);
		if(root.isArray())
		{
			int nCount = root.size();
			Log("放映信息条数：%d",nCount);
			for(int i=0;i<nCount;i++)
			{
				string strTmp;
				FY_MSG msg;

				msg.checkTime=root[i]["check_time"].asInt();
				msg.delayTime=root[i]["delay_time"].asInt();

#ifdef WIN32
				strTmp=root[i]["film_name"].asString();
				strTmp=g_globle->U2G(strTmp.c_str(),strlen(strTmp.c_str()));
				const char* ctmp = strTmp.c_str();
				memcpy(msg.filmName,ctmp,strlen(ctmp));

				strTmp=root[i]["cinema_title"].asString();
				strTmp=g_globle->U2G(strTmp.c_str(),strlen(strTmp.c_str()));
				const char* ctmp2 = strTmp.c_str();
				memcpy(msg.CinemaTitle,ctmp2,strlen(ctmp2));
#elif linux
				strTmp=root[i]["film_name"].asString();
				char dstbuf[256]={0};
				g_globle->utf8togb2312((char*)strTmp.c_str(),strlen(strTmp.c_str()),dstbuf,256);
				memcpy(msg.filmName,dstbuf,strlen(dstbuf));

				strTmp=root[i]["cinema_title"].asString();
				char dstbuf2[256]={0};
				g_globle->utf8togb2312((char*)strTmp.c_str(),strlen(strTmp.c_str()),dstbuf2,256);
				memcpy(msg.CinemaTitle,dstbuf2,strlen(dstbuf2));
#endif
				

				msg.fileType=root[i]["film_type"].asInt();
				strTmp = root[i]["show_time"].asString();
				const char* ctmp3 = strTmp.c_str();
				memcpy(msg.showTime,ctmp3,strlen(ctmp3));

				strTmp = root[i]["end_time"].asString();
				memcpy(msg.endTime,strTmp.c_str(),strlen(strTmp.c_str()));

				Log("check_time：%d delay_time：%d film_name：%s cinema_title：%s film_type：%d show_time：%s end_time：%s",
					msg.checkTime,msg.delayTime,msg.filmName,msg.CinemaTitle,msg.fileType,msg.showTime,msg.endTime);

				vFY.push_back(msg);
			}
		}
	}
	catch (...)
	{
		Log("CIFSC_079::PraseData error");
		return false;
	}
	return true;
}

char* CIFSC_079::CombineData(int& datalen)
{
	count = vFY.size();

	datalen = 3+count*159;
	char* sbuf=new char[datalen];
	memset(sbuf,0x00,datalen);
	try
	{
		memcpy(sbuf,msgType,2);
		sbuf[2] = count;
		for (int i=0;i<count;i++)
		{
			FY_MSG msg = vFY[i];
			sbuf[3+159*i]=msg.checkTime;
			sbuf[4+159*i]=msg.delayTime;
			memcpy(sbuf+5+159*i,msg.filmName,64);
			memcpy(sbuf+69+159*i,msg.CinemaTitle,64);
			sbuf[133+159*i]=msg.fileType;
			memcpy(sbuf+134+159*i,msg.showTime,14);
			memcpy(sbuf+148+159*i,msg.endTime,14);
		}
		return sbuf;
	}
	catch (...)
	{
		if (sbuf)
		{
		   delete[] sbuf;
		}
		Log("CIFSC_079::CombineData error!");
		return NULL;
	}
}

// void CIFSC_079::Send(Node* pNode,char* sData,int nDatalen)
// {
// 	int len=0;
// 	char* sendBuf = CombinetPacket(TYPE_DATA,pNode->DownMsgNumber,nDatalen,sData,len);						
// 	SendToGateClient(pNode->s,sendBuf,len);
// 
// 	string strPack=CharStr2HexStr(sendBuf,len);
// 	Log("下发放映信息：%s 流水号：%d",strPack.c_str(),pNode->DownMsgNumber);
// 
// 	delete[] sendBuf;
// 
// 	if (pNode->DownMsgNumber == 0xff)
// 	{
// 		pNode->DownMsgNumber = 0x81;
// 	}
// 	else
// 	{
// 		pNode->DownMsgNumber+=1;	
// 	}
// }
