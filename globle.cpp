#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef linux
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cstddef>
#else
#include <direct.h>
#endif
#include <iostream>
#include <string.h>
#include <time.h>
#include "globle.h"
#ifdef WIN32
#include <shlobj.h>
#include <atlconv.h>
#pragma   comment(lib,   "shell32.lib") 
#endif
#include <fstream>
#include "GateClient.h"
#include "BaseConfig.h"

using namespace std;
using std::string;

unsigned char msgType[TYPE_MAX]={0xEB,0x31,0x32,0x33,0x34,0x35,0x03};

GlobleAll* g_globle=NULL;

GlobleAll::GlobleAll()
{
	m_gateClient=NULL;;
}

GlobleAll::~GlobleAll()
{
	RELEASE_NEW(m_gateClient);
}

bool GlobleAll::InitGloble()
{
	m_configPath="/data/html/trunk/ext/Bin/gate.conf";
	m_LogPath = GetLogFileName();
	m_gateClient=new CGateClient();
	return true;
}

bool GlobleAll::ReadConf()
{
	try
	{
#ifdef WIN32
		ifstream fin(m_configPath);
		if(fin)
		{
			fin.close();
		}
		else
		{
			Log("can't find config file：%s",m_configPath.c_str());
			return false;
		}
#elif linux
		if((access(m_configPath.c_str(),F_OK))==-1)   
		{   
			Log("can't find config file：%s",m_configPath.c_str());
			return false;
		}   
#endif
		CBaseConfig config(m_configPath.c_str());
		m_WebSerIP=config.GetValue("ip");
		m_LocalIP=config.GetValue("tcpip");
		m_HeardTime=atoi(config.GetValue("time"));
		m_HttpPort=atoi(config.GetValue("httpport"));
		m_TcpPort=atoi(config.GetValue("tcpport"));
		m_GateNum=atoi(config.GetValue("gatenum"));
		m_TcpPort=atoi(config.GetValue("tcpport"));

		{
			m_GateListUrl="/index.cli.php?s=/admin/gate/clients";
			m_61Url="/index.cli.php?s=/admin/gate/checkTicket";
			m_66Url="/index.cli.php?s=/admin/gate/setTicketStatus";
			m_86Url="/index.cli.php?s=/admin/gate/checkFaceId";
			m_79Url="/index.cli.php?s=/admin/gate/sessions";
		}
	}
	catch (...)
	{
		Log("ReadConf error");
		return false;
	}
	return true;
}

string GlobleAll::GetLogFileName()
{
	static string file;
	if(file.empty())
	{
		char tmp[1024]={0};
		sprintf(tmp,"/tmp/gate.log");
		file=tmp;
#ifdef WIN32
		//mkdir(tmp);
#elif linux
		//tmp目录有，创建log文件
		FILE* fp=fopen(tmp,"a+");
		if (fp)
		{
			fclose(fp);
		}
		else
		{
			Log("fopen %s error",tmp);
		}
#endif
	}
	return file;
}

void Log(char* fmt, ...)
{
	va_list args;
	va_start (args, fmt);
	char* tmp=new char[MAX_BUFFER*10];
	memset(tmp,0,MAX_BUFFER*10);
	vsprintf(tmp,fmt, args);

	time_t t;
	tm* time2;
	t=time(NULL);
	time2=localtime(&t);
	FILE* fp=fopen(g_globle->m_LogPath.c_str(),"a+");
	if(fp)
	{
		int len = strlen(tmp);
		char *ctmp=new char[len+1024];
		memset(ctmp,0,len+1024);
		sprintf(ctmp,"%04d/%02d/%02d %02d:%02d:%02d\t%s\n",time2->tm_year+1900,time2->tm_mon+1,time2->tm_mday,time2->tm_hour,time2->tm_min,time2->tm_sec,tmp);
		
#ifdef WIN32
		cout<<ctmp;
		fprintf(fp,ctmp);
#elif linux
		fprintf(fp,ctmp);
#endif	
		fclose(fp);
		delete[] ctmp;
	}
	else
	{
		Log("fopen logpath %s error!",g_globle->m_LogPath.c_str());
	}
#ifdef DEBUG_LOG
	string str=tmp;
	str+="\r\n";
	::OutputDebugStringA(str.c_str());
#endif

	delete[] tmp;
	va_end(args);
}

bool GlobleAll::get_local_ip()
{
#ifdef WIN32
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2,2),&wsadata)!=0)
	{
		return false;
	}
#endif

	char hostname[128];
	int ret = gethostname(hostname, sizeof(hostname));
	if (ret == -1)
	{
		return false;
	}
	struct hostent *hent;
	hent = gethostbyname(hostname);
	if (NULL == hent)
	{
		return false;
	}
	while(*(hent->h_addr_list) != NULL)        //输出ipv4地址
	{
		m_LocalIP = inet_ntoa(*(struct in_addr *) *hent->h_addr_list);
		hent->h_addr_list++;
	}
	Log("Local IP = %s",m_LocalIP.c_str());
	return true;
}

bool GlobleAll::EndianJudge()
{
	short x;
	char x0;
	x=0x1122;
	x0=((char*)&x)[0];
	if(x0==0x11)
	{
		//大端
		return true;
	}
	else if(x0==0x22)
	{
		//小端
		return false;
	}
	return false;
}

#ifdef WIN32
string GlobleAll::G2U(const char* gb2312,int nLen)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len+1];
	memset(str, 0, len+1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	string strCn = str;
	if(wstr) delete[] wstr;
	if(str) delete[] str;
	return strCn;
}

//UTF-8到GB2312的转换
string GlobleAll::U2G(const char* utf8,int nLen)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len+1];
	memset(str, 0, len+1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	string strCn = str;
	if(str) delete[] str;
	if(wstr) delete[] wstr;
	return strCn;
}
#elif linux
int GlobleAll::utf8togb2312(char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen)
{   
	iconv_t cd;   

	if( (cd = iconv_open("gb2312","utf-8")) ==0 )     

		return -1;  

	memset(destbuf,0,destlen);   

	char **source = &sourcebuf;   

	char **dest = &destbuf;
	if(-1 == iconv(cd,source,&sourcelen,dest,&destlen))     
		return -1;   

	iconv_close(cd);   

	return 0;   

}
int GlobleAll::gb2312toutf8(char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen)
{   

	iconv_t cd;   if( (cd = iconv_open("utf-8","gb2312")) ==0 )     

		return -1;   memset(destbuf,0,destlen);   

	char **source = &sourcebuf;   

	char **dest = &destbuf;
	if(-1 == iconv(cd,source,&sourcelen,dest,&destlen))     

		return -1;   

	iconv_close(cd);   

	return 0;   

}  
#endif

static void Char2Hex(unsigned char ch, char *szHex)
{
	int i;
	unsigned char byte[2];
	byte[0] = ch/16;
	byte[1] = ch%16;
	for(i=0; i<2; i++)
	{
		if(byte[i] >= 0 && byte[i] <= 9)
			szHex[i] = '0' + byte[i];
		else
			szHex[i] = 'a' + byte[i] - 10;
	}
	szHex[2] = 0;
}
string GlobleAll::CharStr2HexStr( char *pucCharStr, int iSize)
{
	int i;
	char szHex[3];
	char* pszHexStr=new char[iSize*5];
	memset(pszHexStr,0,iSize*5);

	for(i=0; i<iSize; i++)
	{
		Char2Hex(pucCharStr[i], szHex);
		strcat(pszHexStr, szHex);
	}
	string str = pszHexStr;
	str="{"+str+"}";
	delete[] pszHexStr;
	return str;
}