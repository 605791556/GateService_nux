#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "Socket.h"
#include "ComThread.h"
using std::string;


#ifndef UNICODE
#define UNICODE
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

enum WEBMSG
{
	WEBMSG_FIND=72,//设备状态查询
	WEBMSG_CONTROL=55,//设备控制命令
	WEBMSG_MODIFY=60,//增删改设备信息
	WEBMSG_MODIFY_FY=79,//修改放映信息
	WEBMSG_MAX
};

void* WebAcceptThread(void* lpParam);
//向httpserver发送请求
string SendToHttpServer(const char* url,bool bPost=false,string strData="");
bool SendToWebClient(BASE_SOCKET sock,const char* data,int statuscode=200);
