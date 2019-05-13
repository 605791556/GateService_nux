#ifdef WIN32
#include <Winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
#endif

#ifdef linux
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include<sys/socket.h>
#include <sys/ioctl.h>
#endif

#include "Socket.h"
#include <stdio.h>
#ifdef WIN32
volatile int nInit=0;
#endif
BASE_SOCKET GetSocket(int isTCP)
{
#ifdef WIN32
	if(0==nInit)//windows初始化
	{
		WORD wVersionRequested = MAKEWORD(2, 2);
		WSADATA wsaData;
		if(0==WSAStartup(wVersionRequested, &wsaData))
		{
			nInit++;
		}
	}
#endif

	return socket(AF_INET,isTCP?SOCK_STREAM:SOCK_DGRAM,0);
}
int CloseSocket(BASE_SOCKET socket)
{
#ifdef WIN32
	::closesocket(socket);
#endif

#ifdef linux
	close(socket);
#endif

	return 0;
}
inline int IoCtl(BASE_SOCKET socket,int nIsBlock)
{
	if(INVALID_BASE_SOCKET==socket)return 0;
	
	//设置模式 style=0时为阻塞模式
#ifdef WIN32
	u_long style=nIsBlock?0:1;
	return ioctlsocket(socket,FIONBIO,(u_long FAR*)&style);
#endif

#ifdef linux
	int style=nIsBlock?0:1;
	return ioctl(socket,FIONBIO,&style);
#endif
}
inline int CanReadSocket(BASE_SOCKET socket,int timeOut)
{
	if(socket==INVALID_BASE_SOCKET||timeOut<0)return 0;

	fd_set fSet;
	timeval time;
		
	::memset(&fSet,0,sizeof(fSet));
	::memset(&time,0,sizeof(time));	
#ifdef WIN32
	fSet.fd_array[0]=socket;
	fSet.fd_count=1;
#endif

#ifdef linux
	FD_ZERO(&fSet);
	FD_SET(socket, &fSet);
#endif
	
	time.tv_sec=timeOut/1000;
	time.tv_usec=(timeOut%1000)*1000;

	int out=::select((int)socket+1,&fSet,0,0,&time);//debug_forcetechhttp_0x1
	if(SOCKET_ERROR==out)//错误
	{
		return -1;
	}
	else if(0==out)//超时
	{
		return 0;
	}
	else//可读/写
	{
		return 1;
	}
	//if(::select((int)socket+1,&fSet,0,0,&time)<1)return 0;
	//else return 1;


}
inline int CanWriteSocket(BASE_SOCKET socket,int timeOut)
{
	if(socket==INVALID_BASE_SOCKET||timeOut<0)return 0;

	fd_set fSet;
	timeval time;
		
	::memset(&fSet,0,sizeof(fSet));
	::memset(&time,0,sizeof(time));	
#ifdef WIN32
	fSet.fd_array[0]=socket;
	fSet.fd_count=1;
#endif

#ifdef linux
	FD_ZERO(&fSet);
	FD_SET(socket, &fSet);
#endif
	
	time.tv_sec=timeOut/1000;
	time.tv_usec=(timeOut%1000)*1000;

	int out=::select((int)socket+1,0,&fSet,0,&time);//debug_forcetechhttp_0x1
	if(SOCKET_ERROR==out)//错误
	{
		return -1;
	}
	else if(0==out)//超时
	{
		return 0;
	}
	else//可读/写
	{
		return 1;
	}

	//if(::select((int)socket+1,0,&fSet,0,&time)<1)return 0;
	//else return 1;
}

int Connect(BASE_SOCKET socket,char* host,int port,int timeOut)
{
	//返回值0正常 -1连接失败/超时 -2参数错误 -3主机ip/域名无效 -4设置socket模式失败
	if(socket==INVALID_BASE_SOCKET||timeOut<0||0==host||0>port)return -2;
	///连接
	struct sockaddr_in dsttaddr;
	{
		hostent *m_phostent=gethostbyname(host);
		if(m_phostent==NULL)return -3;
		struct in_addr ip_addr;
		memcpy(&ip_addr,m_phostent->h_addr_list[0],4);///h_addr_list[0]里4个字节,每个字节8位
		memset((void *)&dsttaddr,0,sizeof(dsttaddr)); 
		dsttaddr.sin_family=AF_INET;
		dsttaddr.sin_port=htons(port);
		dsttaddr.sin_addr=ip_addr;
	}

	if(0==timeOut)//阻塞模式
	{
		if(IoCtl(socket,1))return -4;
		return connect(socket,(struct sockaddr*)&dsttaddr,sizeof(sockaddr_in));
	}
	else//非阻塞模式
	{
		if(IoCtl(socket,0))return -4;
		connect(socket,(struct sockaddr*)&dsttaddr,sizeof(sockaddr_in));
		int nResult=-1;
		if(CanWriteSocket(socket,timeOut)>0)nResult= 0;
		IoCtl(socket,1);
		return nResult;
	}
}
int Send(BASE_SOCKET socket,char* buf,int size,int timeOut)
{
	//返回-1 表示发送失败 -2参数错误 -3当前不可写
	if(socket==INVALID_BASE_SOCKET||0==buf||0>=size||timeOut<0)return -2;

	IoCtl(socket,1);
	//if(timeOut&&0==::CanWriteSocket(socket,timeOut))return -3;

	if(timeOut>0)
	{
		int n=::CanWriteSocket(socket,timeOut);
		if(n<0)return -1;
		else if(n==0)return -3;
	}
	else if(timeOut<0)return -2;
#ifdef WIN32
	return send(socket,buf,size,0);
#else
	return send(socket,buf,size,MSG_NOSIGNAL);
#endif
}
int Recv(BASE_SOCKET socket,char* buf,int size,int timeOut)
{
	//返回-1 表示接受失败 -2参数错误 -3当前不可读
	if(socket==INVALID_BASE_SOCKET||0==buf||0>=size||timeOut<0)return -2;

	IoCtl(socket,1);

	//debug_forcetechhttp_0x1
	//if(timeOut&&0==::CanReadSocket(socket,timeOut))return -3;
	if(timeOut>0)
	{
		int n=::CanReadSocket(socket,timeOut);
		if(n<0)return -1;
		else if(0==n)return -3;
	}
	else if(timeOut<0)return -2;

	return recv(socket,buf,size,0);
}





int SendTo(BASE_SOCKET socket,char* host,int port,char* buf,int size)
{
	//返回1发送失败  -2参数错误 -3主机ip/域名无效
	if(0==host||0>port||0>size)return -2;

	struct sockaddr_in dsttaddr;
	{
		hostent *m_phostent=gethostbyname(host);
		if(m_phostent==NULL)-3;
		struct in_addr ip_addr;
		memcpy(&ip_addr,m_phostent->h_addr_list[0],4);///h_addr_list[0]里4个字节,每个字节8位
		memset((void *)&dsttaddr,0,sizeof(dsttaddr)); 
		dsttaddr.sin_family=AF_INET;
		dsttaddr.sin_port=htons(port);
		dsttaddr.sin_addr=ip_addr;
	}
	return ::sendto(socket,buf,size,0,(struct sockaddr*)&dsttaddr,sizeof(sockaddr));
}
int Bind(BASE_SOCKET socket,char* host,int port)
{
	//返回值0正常 -1绑定失败  -2参数错误 -3主机ip/域名无效
	if(0==host||0>port)return -2;
	struct sockaddr_in dsttaddr;
	{
		hostent *m_phostent=gethostbyname(host);
		if(m_phostent==NULL)return -3;
		struct in_addr ip_addr;
		memcpy(&ip_addr,m_phostent->h_addr_list[0],4);///h_addr_list[0]里4个字节,每个字节8位
		memset((void *)&dsttaddr,0,sizeof(dsttaddr)); 
		dsttaddr.sin_family=AF_INET;
		dsttaddr.sin_port=htons(port);
		dsttaddr.sin_addr=ip_addr;
	}

	if(0==::bind(socket,(struct sockaddr*)&dsttaddr,sizeof(sockaddr)))
	{
		::listen(socket,1000);
		return 0;
	}
	else return -1;
}

int GetIp(BASE_SOCKET socket,char* ip,int size)
{
	//返回-1 表示发送失败 -2参数错误 -3当前不可写
	if(socket==INVALID_BASE_SOCKET||0==ip||size<16)return -2;

	::memset(ip,0,size);
	struct sockaddr_in dsttaddr;
	SOCKET_LEN_T tempSize=sizeof(sockaddr_in);
	::memset(&dsttaddr,0,tempSize);
	getsockname(socket,(struct sockaddr*)&dsttaddr,&tempSize);
	sprintf(ip,"%s",inet_ntoa(dsttaddr.sin_addr));

	return 0;
}
int GetPort(BASE_SOCKET socket,unsigned short& port)
{
	//返回-1 表示发送失败 -2参数错误 -3当前不可写
	if(socket==INVALID_BASE_SOCKET)return -2;

	struct sockaddr_in dsttaddr;
	SOCKET_LEN_T tempSize=sizeof(sockaddr_in);
	::memset(&dsttaddr,0,tempSize);
	getsockname(socket,(struct sockaddr*)&dsttaddr,&tempSize);
	port= ntohs(dsttaddr.sin_port);
	return 0;
}
BASE_SOCKET Accept(BASE_SOCKET socket,struct sockaddr* addr,SOCKET_LEN_T* addrlen,int timeOut)
{
	if(socket==INVALID_BASE_SOCKET||timeOut<0)return INVALID_BASE_SOCKET;
	IoCtl(socket,1);

	//debug_forcetechhttp_0x1
	//if(timeOut&&0==::CanReadSocket(socket,timeOut))return INVALID_BASE_SOCKET;
	if(timeOut>0)
	{
		int n=::CanReadSocket(socket,timeOut);
		if(n<=0)return INVALID_BASE_SOCKET;
	}
	else if(timeOut<0)return INVALID_BASE_SOCKET;

	return ::accept(socket,addr,addrlen);
}
