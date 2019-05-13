#pragma once
#include <string>
#include <vector>
#include "globle.h"
#include "ComThread.h"
#include "IFAL_074.h"

using std::vector;
using std::string;


//web控制端下发的闸机列表
typedef struct GATEMSG
{
	string strGateID;//闸机编号
	string strGateIP;//闸机IP
};

//闸机收到设备控制报文后，服务将处理结果返回给控制端
typedef struct GATECONTROL_BACK
{
	unsigned char MsgNumber;//流水号
	string strGateID;//闸机编号
	BASE_SOCKET clientSock;
};

typedef struct _NODE_ 
{
	BASE_SOCKET s;
	sockaddr_in Addr;
	sThreadParm par;//线程信息
	string strGateID;//闸机编号
	CIFAL_074* fal_074;//设备全状态
	unsigned char DownMsgNumber;//KS下发消息流水号(129~255)
	vector<GATECONTROL_BACK> vGateBack;
	_NODE_* pNext;

	_NODE_()
	{
		s=INVALID_BASE_SOCKET;
		fal_074=NULL;
		DownMsgNumber=0x81;
		pNext=NULL;
	}

	~_NODE_()
	{
		par.run=false;
		RELEASE_NEW(fal_074);
		RELEASE_SOCKET(s);
	}
}Node,*pNode;

class CIFSC_079;
class CGateClient
{
public:
	CGateClient();
	~CGateClient();

public:
	vector<GATEMSG> vGate;
	vector<Node*> vNode;
	CIFSC_079* m_fsc_079;//保存放映信息

	BASE_SOCKET m_SerSocket;

#ifdef WIN32
	HANDLE Device_mutex;
	HANDLE FyMsg_mutex;
#elif linux
	pthread_mutex_t Device_mutex;
	pthread_mutex_t FyMsg_mutex;
#endif

public:
	//保存闸机列表
	bool AddGateList(const char* pContent);
	//删除指定闸机
	void DeleteGate(Node* pnode);
	//添加闸机线程
	bool AddClientList(BASE_SOCKET s,sockaddr_in addr);
	//发送报文，sbuf:报文 len:报文长度
	void SendToGateClient(BASE_SOCKET sClient,char* sbuf,int len);
	//组合报文packType:消息包类型 MsgNumber：流水号 dataLen：数据长度，data:数据,len:返回数据长度
	char* CombinetPacket(char packType,unsigned char MsgNumber,short dataLen,char* data,int& len); 
	//释放所有内存
	void ReleaseMemory();
};

void* GateAcceptThread(void* lpParam);
//闸机线程
void* GateClientThread(void* lpParam);