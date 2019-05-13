#include <iostream>
#include "GateClient.h"
#include "WebClient.h"
#include "IFSC_061.h"
#include "IFSC_062.h"
#include "IFSC_066.h"
#include "IFSC_075.h"
#include "IFSC_071.h"
#include "IFAL_011.h"
#include "IFSC_079.h"
#include "IFSC_080.h"
#include "IFSC_085.h"
#include "IFSC_086.h"

using namespace std;

CGateClient::CGateClient()
{
	m_fsc_079 = new CIFSC_079();
	m_SerSocket=INVALID_BASE_SOCKET;
#ifdef WIN32
	Device_mutex = CreateMutex(NULL, FALSE, NULL);
	FyMsg_mutex = CreateMutex(NULL, FALSE, NULL);
#elif linux
	pthread_mutex_init(&Device_mutex, NULL);
	pthread_mutex_init(&FyMsg_mutex, NULL);
#endif
}

CGateClient::~CGateClient()
{
	RELEASE_NEW(m_fsc_079);
	RELEASE_SOCKET(m_SerSocket);

#ifdef WIN32
	CloseHandle(Device_mutex);
	CloseHandle(FyMsg_mutex);
#elif linux
	pthread_mutex_destroy(&Device_mutex);
	pthread_mutex_destroy(&FyMsg_mutex);
#endif
}

//接收客户端消息
void* GateAcceptThread(void* lpParam)
{
	Log("GateAcceptThread in");
	sThreadParm* par=(sThreadParm*)lpParam;
	g_globle->m_gateClient->m_SerSocket = GetSocket(1);
	if (g_globle->m_gateClient->m_SerSocket==INVALID_BASE_SOCKET)
	{
		Log("GateAcceptThread GetSocket Error!");
		par->run=false;
		return 0; 
	}

	//bool bReuseaddr=true;
	////允许重用本地地址和端口
	//if(setsockopt(g_globle->m_gateClient->m_SerSocket,SOL_SOCKET ,SO_REUSEADDR,(const char*)&bReuseaddr,sizeof(bool))<0)
	//{
	//	Log("setsockopt Error!");
	//	par->run=false;
	//	return 0;
	//}

	int Ret = Bind(g_globle->m_gateClient->m_SerSocket,(char*)g_globle->m_LocalIP.c_str(),g_globle->m_TcpPort);
	if (Ret==SOCKET_ERROR)
	{
		Log("GateAcceptThread Bind Error! ip:%s port:%d",g_globle->m_LocalIP.c_str(),g_globle->m_TcpPort);
		par->run=false;
		return 0;
	}

	if(listen(g_globle->m_gateClient->m_SerSocket,30)<0)
	{
		Log("GateAcceptThread listen Error!");
		par->run=false;
		return 0;
	}
	Log("服务器开始监听...");


	while (par->run)
	{
		sockaddr_in ClientAddr;
		SOCKET_LEN_T nLen = sizeof(ClientAddr);

		BASE_SOCKET sClient=Accept(g_globle->m_gateClient->m_SerSocket,(sockaddr*)&ClientAddr,&nLen);
		if (sClient==INVALID_BASE_SOCKET)
		{
			continue;
		}
		else
		{
			//如果接收成功则把用户的所有信息存入链表
			if (!g_globle->m_gateClient->AddClientList(sClient,ClientAddr))
			{
				RELEASE_SOCKET(sClient);
			}
		}
	}
	Log("GateAcceptThread out");
	return 0;
}

bool CGateClient::AddGateList(const char* pContent)
{
	try
	{
		Json::Reader r;
		Json::Value root;
		r.parse(pContent,root);
		if(root.isArray())
		{
			for(int i=0;i<root.size();i++)
			{
				GATEMSG gateMsg;
				gateMsg.strGateID=root[i]["code"].asString();
				gateMsg.strGateIP=root[i]["ip"].asString();
				vGate.push_back(gateMsg);
				Log("闸机%d：code:%s,ip:%s",i+1,gateMsg.strGateID.c_str(),gateMsg.strGateIP.c_str());
			}
			int nCount = vGate.size();
			if (nCount == 0)
			{
				Log("获取闸机数量：%d",nCount);
				return false;
			}
		}
	}
	catch (exception* e)
	{
		Log("AddGateList error :%s",e);
		return false;
	}
	return true;
}

bool CGateClient::AddClientList(BASE_SOCKET s,sockaddr_in addr)
{
	//IP判断，如果不是闸机IP，则不连接
	int ndex=-1;
	int nCount = vGate.size();
	Log("Gate List Number:%d",nCount);
	if (nCount == 0)
	{
		Log("闸机列表为空，正在尝试重新获取闸机列表...");
		string strUrl = "http://"+g_globle->m_WebSerIP+g_globle->m_GateListUrl;
		string strGateList=SendToHttpServer(strUrl.c_str());
		if(!strGateList.empty())
		{		
			g_globle->m_gateClient->AddGateList(strGateList.c_str());
			nCount = vGate.size();
		}
	}

	for (int i=0;i<nCount;i++)
	{
		string strIP=inet_ntoa(addr.sin_addr);
		if (vGate[i].strGateIP == strIP)
		{
			ndex=i;
			break;
		}	
	}
	if (ndex == -1)
	{
		if (nCount>0)
		{
			Log("%s 不是闸机IP，拒绝连接！",inet_ntoa(addr.sin_addr));
		}
		else
		{
			Log("%s 闸机列表为空，拒绝连接！",inet_ntoa(addr.sin_addr));
		}
		return false;
	}

	Node* node = new Node;
	node->s=s;
	node->Addr=addr;
	node->fal_074 = new CIFAL_074;
	if (nCount>0 && ndex>=0)
	{
		node->strGateID=vGate[ndex].strGateID;
	}

	//为每台闸机开辟新的线程
	node->par.arg=node;
	if(!ComCreateThread(GateClientThread,&node->par))
	{
		Log("闸机:%s ip：%s ComCreateThread Error！",node->strGateID.c_str(),inet_ntoa(node->Addr.sin_addr));
		RELEASE_NEW(node);
		return false;
	}

    LOCK_MUTEX(Device_mutex)
	vNode.push_back(node);
	Log("闸机:%s 连接成功！ip：%s",node->strGateID.c_str(),inet_ntoa(node->Addr.sin_addr));
	UNLOCK_MUTEX(Device_mutex)
	
	return true;
}

void* GateClientThread(void* lpParam)
{
	sThreadParm* par=(sThreadParm*)lpParam;
	Node* pnode = (Node*)(par->arg);
	BASE_SOCKET sClient = pnode->s;

	int  waitTime=0;
	unsigned char oldMsgNumber = 0x00;//闸机上发消息流水号
	char* szRequest=new char[MAX_BUFFER*10];

	char requestBuf[] = {msgType[TYPE_BEGIN],msgType[TYPE_REQUEST],0x00,0x00,0x00,msgType[TYPE_END]};
	char resPonseBuf[] = {msgType[TYPE_BEGIN],msgType[TYPE_RESPONCE],0x00,0x00,0x00,msgType[TYPE_END]};

	//下发放映信息
	{
		//厂商要求延迟一秒发送
		msleep(1000);
		int nDatalen=0,len=0;
		char* sData = g_globle->m_gateClient->m_fsc_079->CombineData(nDatalen);
		if (sData)
		{
			char* sendBuf = g_globle->m_gateClient->CombinetPacket(msgType[TYPE_DATA],pnode->DownMsgNumber,nDatalen,sData,len);						
			g_globle->m_gateClient->SendToGateClient(sClient,sendBuf,len);

			string strPack=g_globle->CharStr2HexStr(sendBuf,len);
			Log("下发放映信息：%s 流水号：%d",strPack.c_str(),pnode->DownMsgNumber);

			delete[] sendBuf;

			if (pnode->DownMsgNumber == 0xff)
			{
				pnode->DownMsgNumber = 0x81;
			}
			else
			{
				pnode->DownMsgNumber+=1;	
			}
			delete[] sData;	
		}
	}

	memset(szRequest,0,MAX_BUFFER*10);
	int NumberOfBytesRecvd;
	int dwBufferCount = 1;
	int Flags = 0;

	//数据长度
	short dataLen=0;
	int   revLen =0;

	int len = 0;
	NumberOfBytesRecvd=5;
	while (par->run)
	{
		revLen = Recv(sClient,szRequest+len,NumberOfBytesRecvd,g_globle->m_HeardTime);
		if (revLen<0)
		{
			if (revLen == -3)
			{
				waitTime+=g_globle->m_HeardTime;
				if (waitTime>3*g_globle->m_HeardTime)
				{
					//3*m_HeardTime的时间没有接收到任何消息,则关闭会话
					Log("闸机：%s 超时！",pnode->strGateID.c_str());
					break;
				}
				else
				{
					//发送Request请求
					g_globle->m_gateClient->SendToGateClient(sClient,requestBuf,sizeof(requestBuf));
					continue;
				}
			}
			else
			{
				Log("Recv return error,nRet:%d",revLen);
				break;
			}
		}
		else if(revLen == 0)
		{
			//断开连接
			Log("revLen = 0");
			break;
		}
		else if(revLen>0)
		{
			waitTime = 0;
			len+=revLen;

			//先获取前5字节，根据数据包长度接收剩余字节，防止粘包和半包
			//数据包类型
			if (len>=5)
			{
				char LenBuff[2] = {0};
				memcpy(LenBuff,szRequest+3,2);

				//大端机，进行字节序转换
				if (g_globle->EndianJudge())
				{
					dataLen = ZZ_SWAP_2BYTE(*(short*)LenBuff);
				}
				else
				{
					dataLen = *(short*)LenBuff;
				}

				if (len < 5+dataLen+1)
				{
					NumberOfBytesRecvd = 5+dataLen+1-len;//数据+0x03
					continue;
				}
				else
				{
					NumberOfBytesRecvd=5;
				}
			}
			else
			{
				NumberOfBytesRecvd=5-len;
				continue;
			}

			char ackBuf[] = {msgType[TYPE_BEGIN],msgType[TYPE_ACK],szRequest[2],0x00,0x00,msgType[TYPE_END]};
			char nakBuf[] = {msgType[TYPE_BEGIN],msgType[TYPE_NAK],szRequest[2],0x00,0x00,msgType[TYPE_END]};

			unsigned char MsgNumber = szRequest[2];//流水号
			if (MsgNumber != 0x00)
			{
				//重复流水号，不处理
				if (oldMsgNumber == MsgNumber)
				{
					len=0;
					memset(szRequest,0,MAX_BUFFER*10);
					Log("重复流水号:%d 忽略",MsgNumber);
					continue;
				}
				else
				{
					Log("---------------------------------");
					Log("流水号：%d",MsgNumber);
					oldMsgNumber = MsgNumber;
				}
			}

			//校验数据包是否完整
			unsigned char start = szRequest[0];
			if (start == msgType[TYPE_BEGIN] && szRequest[len-1] == msgType[TYPE_END])
			{
				//处理收到的数据，如果处理失败，转入异常处理，并返回NAK，成功则返回ACK或应答报文
				if (szRequest[1] == msgType[TYPE_DATA])
				{
					string strPack=g_globle->CharStr2HexStr(szRequest,len);
					int n = strPack.length();
					Log("报文信息：%s",strPack.c_str());

					//根据长度获取到数据buffer
					unsigned char* data=new unsigned char[dataLen];
					memcpy(data,szRequest+5,dataLen);

					//获取数据报文类型
					char dataType[2]={0};
					memcpy(dataType,data,2);
					int nDataType = atoi(dataType);

					int nRet=false;
					switch (nDataType)
					{
					case IFSC_062://散客单过闸验证
						{
							Log("62 in");
							Log("报文类型：散客单过闸验证");
							do 
							{
								CIFSC_062 fsc_062;
								nRet=fsc_062.ParseData(data);
								if (nRet)
								{
									Json::Value root;
									root["qr_code"]=(char*)fsc_062.TDCode;
									root["flag"]=fsc_062.EnOutFlag;
									Json::FastWriter writer;  
									string temp = writer.write(root);
									string tr = temp.substr(temp.length()-1,1);
									if (tr=="\n")
									{
										temp = temp.substr(0,temp.length()-1);
									}
									string url="http://"+g_globle->m_WebSerIP+g_globle->m_61Url;
									string strResult=SendToHttpServer(url.c_str(),true,temp);
									if (!strResult.empty() && strResult!="\"Bad Request\"")
									{
										CIFSC_061 fsc_061(fsc_062.DeviceID);
										if (fsc_061.ParseData(strResult.c_str()))
										{
											int nDatalen=0,len=0;
											char* sData = fsc_061.CombineData(nDatalen);
											char* sendBuf = g_globle->m_gateClient->CombinetPacket(msgType[TYPE_DATA],szRequest[2],nDatalen,sData,len);															
											g_globle->m_gateClient->SendToGateClient(sClient,sendBuf,len);
											Log("返回报文：%s,流水号:%d",g_globle->CharStr2HexStr(sendBuf,len).c_str(),oldMsgNumber);
											delete[] sData;
											delete[] sendBuf;
											break;
										}	
									}
								}
								g_globle->m_gateClient->SendToGateClient(sClient,nakBuf,sizeof(nakBuf));
								Log("返回nak报文：%s,流水号:%d",g_globle->CharStr2HexStr(nakBuf,sizeof(nakBuf)).c_str(),oldMsgNumber);
							} while (0);
							Log("62 out");
						}
						break;
					case IFSC_066://散客过闸记录
						{
							Log("66 in");
							Log("报文类型：散客过闸记录");
							do 
							{
								CIFSC_066 fsc_066;
								nRet=fsc_066.ParseData(data);
								if (nRet)
								{
									//如果过闸记录为成功，则发送二维码数据
									if (fsc_066.result == '0')
									{
										Json::Value root;
										root["qr_code"]=(char*)fsc_066.code;
										root["flag"]=fsc_066.direction;
										Json::FastWriter writer;  
										string temp = writer.write(root);
										string tr = temp.substr(temp.length()-1,1);
										if (tr=="\n")
										{
											temp = temp.substr(0,temp.length()-1);
										}
										string url="http://"+g_globle->m_WebSerIP+g_globle->m_66Url;
										SendToHttpServer(url.c_str(),true,temp);
										g_globle->m_gateClient->SendToGateClient(sClient,ackBuf,sizeof(ackBuf));
										Log("返回ack报文：%s,流水号:%d",g_globle->CharStr2HexStr(ackBuf,sizeof(ackBuf)).c_str(),oldMsgNumber);									
									}
									break;
								}
								g_globle->m_gateClient->SendToGateClient(sClient,nakBuf,sizeof(nakBuf));
								Log("返回nak报文：%s,流水号:%d",g_globle->CharStr2HexStr(nakBuf,sizeof(nakBuf)).c_str(),oldMsgNumber);
							} while (0);
							Log("66 out");
						}
						break;
					case IFSC_075://会员卡过闸验证
						{
							Log("报文类型：会员卡过闸验证");
							break;
						}
					case IFSC_071://获取服务器时间
						{
							Log("71 in");
							Log("报文类型：获取服务器时间");
							CIFSC_071 fsc_071;
							nRet=fsc_071.ParseData(data);
							if (nRet)
							{
								int dataLen=0,len=0;
								char* sData = fsc_071.getUnixCurrtTime(dataLen);
								if (sData)
								{
									char* sendBuf = g_globle->m_gateClient->CombinetPacket(msgType[TYPE_DATA],szRequest[2],dataLen,sData,len);						
									g_globle->m_gateClient->SendToGateClient(sClient,sendBuf,len);
									Log("返回报文：%s,流水号:%d",g_globle->CharStr2HexStr(sendBuf,len).c_str(),oldMsgNumber);
									delete[] sData;
									delete[] sendBuf;
								}
								else
								{
									g_globle->m_gateClient->SendToGateClient(sClient,nakBuf,sizeof(nakBuf));
									Log("返回nak报文：%s,流水号:%d",g_globle->CharStr2HexStr(nakBuf,sizeof(nakBuf)).c_str(),oldMsgNumber);
								}
							}
							else
							{
								g_globle->m_gateClient->SendToGateClient(sClient,nakBuf,sizeof(nakBuf));
								Log("返回nak报文：%s,流水号:%d",g_globle->CharStr2HexStr(nakBuf,sizeof(nakBuf)).c_str(),oldMsgNumber);
							}
							Log("71 out");
						}
						break;
					case IFAL_074://设备全状态报文
						{
							Log("74 in");
							Log("报文类型：设备全状态报文");
							//将设备状态保存在内存中，需要查询时，从内存读取

							nRet=pnode->fal_074->ParseData(data);
							if (nRet)
							{	
								if (pnode->strGateID == pnode->fal_074->DeviceID)
								{
									g_globle->m_gateClient->SendToGateClient(sClient,ackBuf,sizeof(ackBuf));
									Log("返回ack报文：%s,流水号:%d",g_globle->CharStr2HexStr(ackBuf,sizeof(ackBuf)).c_str(),oldMsgNumber);
								}
								else
								{
									par->run=false;
									g_globle->m_gateClient->SendToGateClient(sClient,nakBuf,sizeof(nakBuf));
									Log("设备预留ID：%s与设备状态上传ID：%s不一致，断开连接",pnode->strGateID.c_str(),pnode->fal_074->DeviceID);
									Log("返回nak报文：%s,流水号:%d",g_globle->CharStr2HexStr(nakBuf,sizeof(nakBuf)).c_str(),oldMsgNumber);
								}
							}
							else
							{
								g_globle->m_gateClient->SendToGateClient(sClient,nakBuf,sizeof(nakBuf));
								Log("返回nak报文：%s,流水号:%d",g_globle->CharStr2HexStr(nakBuf,sizeof(nakBuf)).c_str(),oldMsgNumber);
							}
							Log("74 out");
						}
						break;
					case IFAL_011://AGM事件信息上传
						{
							Log("11 in");
							Log("报文类型：AGM事件信息报文");
							CIFAL_078 fal_078;
							nRet=fal_078.ParseData(data);
							if (nRet)
							{
								g_globle->m_gateClient->SendToGateClient(sClient,ackBuf,sizeof(ackBuf));
								Log("返回ack报文：%s,流水号:%d",g_globle->CharStr2HexStr(ackBuf,sizeof(ackBuf)).c_str(),oldMsgNumber);
							}
							else
							{
								g_globle->m_gateClient->SendToGateClient(sClient,nakBuf,sizeof(nakBuf));
								Log("返回nak报文：%s,流水号:%d",g_globle->CharStr2HexStr(nakBuf,sizeof(nakBuf)).c_str(),oldMsgNumber);
							}
							Log("11 out");
						}
						break;
					case IFSC_080://放映信息查询
						{
							Log("80 in");
							Log("报文类型：放映信息查询");
							do 
							{
                                LOCK_MUTEX(g_globle->m_gateClient->FyMsg_mutex)
								if (g_globle->m_gateClient->m_fsc_079->vFY.size()==0)
								{
									Log("放映信息为空，正在尝试重新获取放映信息...");
									//获取放映信息
									string strUrl = "http://"+g_globle->m_WebSerIP+g_globle->m_79Url;
									string strFilmList=SendToHttpServer(strUrl.c_str());
									if (strFilmList!="\"Bad Request\"")
									{
										g_globle->m_gateClient->m_fsc_079->PraseData(strFilmList);
									}
								}
                                UNLOCK_MUTEX(g_globle->m_gateClient->FyMsg_mutex)
								int nDatalen=0,len=0;
								char* sData = g_globle->m_gateClient->m_fsc_079->CombineData(nDatalen);
								if (sData)
								{
									char* sendBuf = g_globle->m_gateClient->CombinetPacket(msgType[TYPE_DATA],szRequest[2],nDatalen,sData,len);						
									g_globle->m_gateClient->SendToGateClient(sClient,sendBuf,len);
									Log("返回报文：%s,流水号:%d",g_globle->CharStr2HexStr(sendBuf,len).c_str(),oldMsgNumber);
									delete[] sData;
									delete[] sendBuf;
									break;
								}
								g_globle->m_gateClient->SendToGateClient(sClient,nakBuf,sizeof(nakBuf));
								Log("返回nak报文：%s,流水号:%d",g_globle->CharStr2HexStr(nakBuf,sizeof(nakBuf)).c_str(),oldMsgNumber);
							} while (0);
							Log("80 out");
						}
						break;
					case IFSC_085://人脸过闸验证
						{
							Log("85 in");
							Log("报文类型：人脸过闸验证");
							do 
							{
								CIFSC_085 fal_085;
								nRet=fal_085.ParseData(data);
								if (nRet)
								{
									Json::Value root;
									root["qr_code"]=fal_085.TD_Code;
									root["face_id"]=(char*)fal_085.FacePicData;
									Json::FastWriter writer;  
									string temp = writer.write(root);
									string tr = temp.substr(temp.length()-1,1);
									if (tr=="\n")
									{
										temp = temp.substr(0,temp.length()-1);
									}
									string url="http://"+g_globle->m_WebSerIP+g_globle->m_86Url;
									string strResult=SendToHttpServer(url.c_str(),true,temp);
									if (!strResult.empty() && strResult!="\"Bad Request\"")
									{
										CIFSC_086 fsc_086(fal_085.deviceID);
										if (fsc_086.ParseData(strResult))
										{
											int nDatalen=0,len=0;
											char* sData = fsc_086.CombineData(nDatalen);
											if (sData)
											{
												char* sendBuf =g_globle->m_gateClient->CombinetPacket(msgType[TYPE_DATA],szRequest[2],nDatalen,sData,len);						
												g_globle->m_gateClient->SendToGateClient(sClient,sendBuf,len);
												Log("返回报文：%s,流水号:%d",g_globle->CharStr2HexStr(sendBuf,len).c_str(),oldMsgNumber);
												delete[] sData;
												delete[] sendBuf;
												break;
											}		
										}	
									}
								}
								g_globle->m_gateClient->SendToGateClient(sClient,nakBuf,sizeof(nakBuf));
								Log("返回nak报文:%s,流水号:%d",g_globle->CharStr2HexStr(nakBuf,sizeof(nakBuf)).c_str(),oldMsgNumber);
							} while (0);
							Log("85 out");
						}
						break;
					default:
						{
							Log("error! can't find dataType：%d",nDataType);
						}
						break;
					}
					if (data)
					{
						delete[] data;
					}
					Log(" ");
				}
				//Request类型，发送 Response
				else if (szRequest[1] == msgType[TYPE_REQUEST])
				{
					Log("Request");
					g_globle->m_gateClient->SendToGateClient(sClient,resPonseBuf,sizeof(resPonseBuf));
				}
				//接收Response
				else if (szRequest[1] == msgType[TYPE_RESPONCE])
				{
					Log("Response");
				}
				//接收ACK类型
				else if (szRequest[1] == msgType[TYPE_ACK])
				{
					/*vector<GATECONTROL_BACK>::iterator it = pnode->vGateBack.begin();
					for(;it != pnode->vGateBack.end();)
					{
						unsigned char number = szRequest[2];
						if((*it).MsgNumber == number && (*it).strGateID == pnode->strGateID)
						{
							SendToWebClient((*it).clientSock,"ok");
							CloseSocket((*it).clientSock);
							it=pnode->vGateBack.erase(it);
							break;
						}
						else
						{
							++it;
						}
					}*/
					Log("接收ACK报文 流水号：%d",oldMsgNumber);
				}
				//接收NAK类型
				else if (szRequest[1] == msgType[TYPE_NAK])
				{
					/*vector<GATECONTROL_BACK>::iterator it = pnode->vGateBack.begin();
					for(;it != pnode->vGateBack.end();)
					{
						unsigned char number = szRequest[2];
						if((*it).MsgNumber == number && (*it).strGateID == pnode->strGateID)
						{
							SendToWebClient((*it).clientSock,"ok");
							CloseSocket((*it).clientSock);
							it=pnode->vGateBack.erase(it);
							break;
						}
						else
						{
							++it;
						}
					}*/
					Log("接收NAK报文 流水号：%d",oldMsgNumber);
				}
			}
			else
			{
				//数据包不完整，发送Nak
				string strPack=g_globle->CharStr2HexStr(szRequest,len);
				Log("报文不完整:%s，长度：%d 流水号：%d",strPack.c_str(),len,szRequest[2]);
				g_globle->m_gateClient->SendToGateClient(sClient,nakBuf,sizeof(nakBuf));
			}
			len = 0;
			NumberOfBytesRecvd=5;
			memset(szRequest,0,MAX_BUFFER*10);
		}
	}//while

	Log("闸机:%s 断开连接！ip:%s",pnode->strGateID.c_str(),inet_ntoa(pnode->Addr.sin_addr));
	if (szRequest)
	{
		delete[] szRequest;
	}

	//删除闸机,关闭socket
	g_globle->m_gateClient->DeleteGate(pnode);
	Log("GateClientThread end!");
	return 0;
}

void CGateClient::SendToGateClient(BASE_SOCKET sClient,char* sbuf,int len)
{
	int sendlen = 0;
	do 
	{
		int per_SendLen = Send(sClient,sbuf+sendlen,len-sendlen);
		if (per_SendLen<0)
		{
			Log("SendToGateClient failed,nRet:%d",per_SendLen);
			break;
		}
		sendlen+=per_SendLen;
	} while (sendlen<len);
}

char* CGateClient::CombinetPacket(char packType,unsigned char MsgNumber,short dataLen,char* data,int& len)
{
	len= 6+dataLen;
	char* sendBuf = new char[len];
	//消息头标志
	sendBuf[0] = msgType[TYPE_BEGIN];
	//信息包类型
	sendBuf[1] = packType;
	//流水号
	sendBuf[2] = MsgNumber;
	//数据长度
	char clen[2];
	short zzDataLen=dataLen;
	if (g_globle->EndianJudge())
	{
		zzDataLen=ZZ_SWAP_2BYTE(zzDataLen);
	}
	memcpy(clen,(char*)&zzDataLen,2);
	memcpy(sendBuf+3,clen,2);
	//数据
	memcpy(sendBuf+5,data,dataLen);
	//消息尾标志
	sendBuf[dataLen+5] = msgType[TYPE_END];

	Log("CombinetPacket 流水号：%d 数据长度：%d 报文总长度：%d",MsgNumber,dataLen,len);
	return sendBuf;
}

void CGateClient::DeleteGate(Node* pnode)
{
	LOCK_MUTEX(Device_mutex);
	vector<Node*>::iterator it;
	for(it=vNode.begin();it!=vNode.end();)
	{
		if ((*it)->s == pnode->s)
		{
			RELEASE_NEW(*it);
			it=vNode.erase(it);
			break;
		}
		else
		{
			++it;
		}
	}
	UNLOCK_MUTEX(Device_mutex);
}

void CGateClient::ReleaseMemory()
{
	int nCount = vNode.size();
	for (int i=0;i<nCount;i++)
	{
		if (vNode[i]->fal_074)
		{
			delete vNode[i];
		}
	}
}