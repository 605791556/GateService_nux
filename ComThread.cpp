#include "ComThread.h"


bool ComCreateThread(pCallBack_ThreadFun callback,sThreadParm* parm)
{
	if(0==callback||0==parm)return false;
	parm->run=true;
	parm->handle=0;


#ifdef WIN32
	parm->handle=CreateThread(0,0,(LPTHREAD_START_ROUTINE)callback,parm,0,0);
	if(!parm->handle)
	{
		parm->run=false;
		return false;
	}
	else return true;
#endif

#ifdef linux
	if(pthread_create(&(parm->handle),NULL,callback,parm))
	{
		parm->run=false;
		return false;
	}
	else return true;
#endif
}
bool ComStopThread(sThreadParm* parm)
{
	if(!parm)return false;

	parm->run=false;
#ifdef WIN32
	::WaitForSingleObject(parm->handle,INFINITE);
	::CloseHandle(parm->handle);
	parm->handle=0;
#endif
	
#ifdef linux
	void* status;
	pthread_cancel(parm->handle);
	pthread_join(parm->handle,&status);
#endif

	return true;
}