#pragma once
#ifdef WIN32
#include "windows.h"
#endif

#ifdef WIN32
#define THREAD HANDLE
#endif

#ifdef linux
#define THREAD pthread_t
#include <pthread.h>
#endif

struct sThreadParm
{
	void* arg;//线程参数
	volatile bool run;//标志 如果false的话 线程需要退出
	THREAD handle;//不要操作该值
};
typedef void* (*pCallBack_ThreadFun)(void* arg);
bool ComCreateThread(pCallBack_ThreadFun callback,sThreadParm* parm);
bool ComStopThread(sThreadParm* parm);
