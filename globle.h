#pragma once
#include <stdint.h>
#include <string>
#include <json/json.h>
#include <json/config.h>
#include <vector>
#include "Socket.h"
#include "Base64.h"

using std::string;
using std::vector;
#ifdef WIN32
#define msleep(X) Sleep(X)
#else
#include <iconv.h>
#define msleep(X) usleep((X)*1000)
#endif
#ifdef _DEBUG
#pragma comment(lib,"json_vc71_libmtd.lib")
#else
#pragma comment(lib,"json_vc71_libmt.lib")
#endif

#define IP_UNIQUE_IDENTIFY 0 //闸机IP是否能作为唯一标识 用于连接校验

#define MAX_BUFFER   1024*10

//#define  MSG_BEGN  0xEB//消息头
//#define  MSG_END   0x03//消息尾

#define ZZ_SWAP_2BYTE(L) ((((L) & 0x00FF) << 8) | (((L) & 0xFF00) >> 8))
#define ZZ_SWAP_4BYTE(L) ((ZZ_SWAP_2BYTE ((L) & 0xFFFF) << 16) | ZZ_SWAP_2BYTE(((L) >> 16) & 0xFFFF))
#define ZZ_SWAP_8BYTE(L) ((ZZ_SWAP_4BYTE (((uint64_t)(L)) & 0xFFFFFFFF) << 32) | ZZ_SWAP_4BYTE((((uint64_t)(L)) >> 32) & 0xFFFFFFFF))

#define RELEASE_NEW(x)  {if(x != NULL ){delete x;x=NULL;}}
#define RELEASE_HANDLE(x)  {if(x != NULL && x!=INVALID_HANDLE_VALUE){ CloseHandle(x);x = NULL;}}
#define RELEASE_SOCKET(x)  {if(x !=INVALID_BASE_SOCKET) { CloseSocket(x);x=INVALID_BASE_SOCKET;}}

#ifdef WIN32 
#define LOCK_MUTEX(X)	WaitForSingleObject(X, INFINITE); 
#elif  linux
#define LOCK_MUTEX(X)   pthread_mutex_lock(&X);
#endif

#ifdef WIN32 
#define UNLOCK_MUTEX(X)	ReleaseMutex(X); 
#elif  linux 
#define UNLOCK_MUTEX(X) pthread_mutex_unlock(&X);
#endif

enum TYPE_MSG
{
	TYPE_BEGIN,//消息头
	TYPE_DATA ,
	TYPE_ACK ,
	TYPE_NAK ,
	TYPE_REQUEST ,
	TYPE_RESPONCE,
	TYPE_END,//消息尾
	TYPE_MAX
};
extern unsigned char msgType[TYPE_MAX];

//数据报文类型
enum DATA_TYPE
{
	IFSC_061=61,//过闸验证结果
	IFSC_062=62,//散客过闸验证
	IFSC_066=66,//散客过闸记录
	IFSC_067=67,//会员卡过闸记录
	IFSC_071=71,//获取服务器时间
	IFAL_072=72,//设备状态查询
	IFAL_074=9,//设备全状态/状态变化
	IFSC_075=75,//会员卡过闸验证
	IFAL_011=11,//设备事件信息上传
	IFMC_055=55,//设备控制命令
	IFSC_080=80,//放映信息查询
	IFSC_085=85,//人脸过闸验证
	IFSC_086=86//人脸过闸验证结果
};

void Log(char* fmt, ...);

class CGateClient;
class GlobleAll
{
public:
	GlobleAll();
	~GlobleAll();

public:
	string m_LogPath;//日志路径全称
	string m_WebSerIP;//前端http服务器IP
	string m_LocalIP;//本地IP
	string m_configPath;
	int    m_HeardTime;//接收心跳时间
	int    m_HttpPort;//本机http服务端口
	int    m_TcpPort;//本机TCP/IP服务端口
	int    m_GateNum;//服务器允许连上的最大闸机数

	string m_GateListUrl;//获取闸机列表地址
	string m_61Url;//散客过闸验证地址
	string m_66Url;//验证结果地址
	string m_86Url;//人脸验证地址
	string m_79Url;//获取放映信息地址

	CGateClient* m_gateClient;
public:
	//初始化公用变量
	bool InitGloble();
	bool ReadConf();
	string GetLogFileName();
	bool get_local_ip();

#ifdef WIN32
	//gb2312转utf8
	string G2U(const char* gb2312,int nLen=0);
	//UTF-8转GB2312
	string U2G(const char* utf8,int nLen=0);
#elif linux
	int utf8togb2312( char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen);
	int gb2312toutf8( char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen);
#endif

	//判断机器是大端还是小端，return true:大端，false:小端
	bool EndianJudge();
	//输出16进制报文信息
	string CharStr2HexStr( char *pucCharStr, int iSize);
};

extern GlobleAll* g_globle;