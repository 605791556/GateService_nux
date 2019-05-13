#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#ifdef WIN32
#include <atlconv.h>
#endif
#include "GateClient.h"
#include "WebClient.h"
#include "globle.h"
#include "IFAL_074.h"
#include "IFMC_055.h"
#include "IFSC_079.h"

using namespace std;

void* WebAcceptThread(void* lpParam)
{
	Log("WebAcceptThread in");
	sThreadParm* par=(sThreadParm*)lpParam;

	BASE_SOCKET serSock=::GetSocket(1);
	if(INVALID_BASE_SOCKET==serSock)
	{
		Log("WebAcceptThread GetSocket error");
		par->run=false;
		return 0;
	}
	if(::Bind(serSock,(char*)(g_globle->m_LocalIP.c_str()),g_globle->m_HttpPort)<0)
	{
		Log("httpserver bind error,ip:%s port:%d",g_globle->m_LocalIP.c_str(),g_globle->m_HttpPort);
		CloseSocket(serSock);
		par->run=false;
		return 0;
	}

	while(par->run)
	{
		sockaddr ClientAddr;
		SOCKET_LEN_T len =sizeof(ClientAddr);
		BASE_SOCKET ClientSock=::Accept(serSock,&ClientAddr,&len,0);
		if(INVALID_BASE_SOCKET==ClientSock)
		{
			Log("INVALID_BASE_SOCKET");
			msleep(2000);
			continue;
		}
		else
		{
			Log("Accept ClientSock");

			sockaddr_in sin;
			memcpy(&sin, &ClientAddr, sizeof(sin));
			char buf[MAX_BUFFER]={0};
			int len=0;
			while(par->run)
			{
				int recvSize=::Recv(ClientSock,buf+len,MAX_BUFFER-len,3000);
				if(recvSize<=0)
				{
					CloseSocket(ClientSock);
					break;
				}
				else
				{
					Log("len:%d",len);
				}
				if(strstr(buf,"\r\n\r\n"))
				{
					Log("buf:%s",buf);

					int nType=0;
					string strBuf = buf;
					int len1 = strBuf.find('/');
					int len2 = strBuf.find("HTTP");
					string data = strBuf.substr(len1+1,len2-len1-2);
					len1=data.find('/');
					if (len1<0)
					{
						Log("len1<0");
						// /data1
						nType = atoi(data.c_str());
					}

					int nPs =strBuf.find("POST");
					if (nPs<0)
					{
						Log("nPs<0");
						if (len1>=0)
						{
							Log("len1>0");
							// /data1/data2
							string strType=data.substr(0,len1);
							nType = atoi(strType.c_str());
							data=data.substr(len1+1,data.length()-len1);
							Log("data:%s",data.c_str());
						}
					}
					else
					{
						//获取Content-Length长度
						string s="";
						int ntmp;
						if ((ntmp=strBuf.find("Content-Length: "))>=0)
						{
							s = "Content-Length: ";
						}
						else if ((ntmp=strBuf.find("content-length: "))>=0)
						{
							s = "content-length: ";
						}
						int l1 = s.length();

						string st = strBuf.substr(ntmp+l1,strBuf.length()-ntmp);
						int len3=st.find("\r\n");
						string strContentLen = st.substr(0,len3);
						int contentLen = atoi(strContentLen.c_str());
						int len1 = strBuf.find("\r\n\r\n");
						int len2 = strBuf.length();
						data = strBuf.substr(len1+4,len2-len1-4);
						if (data.length()!=contentLen)
						{
							len+=recvSize;
							continue;
						}
					}
					Log("nType:%d",nType);
					LOCK_MUTEX(g_globle->m_gateClient->Device_mutex);
					try
					{
						switch(nType)
						{
						case WEBMSG_FIND://设备全状态查询
							{
								Log("72 in");
								Json::Value root;
								int nCount = g_globle->m_gateClient->vNode.size();
								Log("---------------------------------");
								Log("72:设备全状态查询,在线闸机数：%d",nCount);

								for (int i=0;i<nCount;i++)
								{
									root.append(g_globle->m_gateClient->vNode[i]->fal_074->MakeJson());
								}

								Json::FastWriter writer;  
								string temp = writer.write(root);
								string tr = temp.substr(temp.length()-1,1);
								if (tr=="\n")
								{
									temp = temp.substr(0,temp.length()-1);
								}
								const char* data = temp.c_str();
								SendToWebClient(ClientSock,data);
								Log("72 out");
							}
							break;
						case WEBMSG_CONTROL://设备控制命令
							{
								Log("55 in");
								len1=data.find('/');
								string deviceID=data.substr(0,len1);
								string commonCode=data.substr(len1+1,data.length()-len1);

								Log("---------------------------------");
								Log("55:设备控制命令 deviceID:%s Code:%s",deviceID.c_str(),commonCode.c_str());

								CIFMC_055 fmc_055(deviceID,commonCode);
								int nDataLen=0;
								char* sData = fmc_055.CombineData(nDataLen);

								int nCount = g_globle->m_gateClient->vNode.size();
								if (nCount==0)
								{
									Log("gate number on line is 0");
									SendToWebClient(ClientSock,"error",500);
								}
								else
								{
									for (int i=0;i<nCount;i++)
									{
										if (g_globle->m_gateClient->vNode[i]->strGateID == deviceID)
										{
											char* sendBuf = g_globle->m_gateClient->CombinetPacket(TYPE_DATA,g_globle->m_gateClient->vNode[i]->DownMsgNumber,nDataLen,sData,len);
											g_globle->m_gateClient->SendToGateClient(g_globle->m_gateClient->vNode[i]->s,sendBuf,len);
											{
												//异步处理，等待闸机处理完再返回给控制端
												/*GATECONTROL_BACK gateback;
												gateback.strGateID=g_globle->m_gateClient->vNode[i]->strGateID;
												gateback.clientSock=ClientSock;
												gateback.MsgNumber=g_globle->m_gateClient->vNode[i]->DownMsgNumber;
												g_globle->m_gateClient->vNode[i]->vGateBack.push_back(gateback);*/
											}

											string strPack=g_globle->CharStr2HexStr(sendBuf,len);
											Log("下发报文：%s 流水号：%d",strPack.c_str(),g_globle->m_gateClient->vNode[i]->DownMsgNumber);

											delete[] sendBuf;
											//流水号满255后重置为129开始
											if (g_globle->m_gateClient->vNode[i]->DownMsgNumber == 0xff)
											{
												g_globle->m_gateClient->vNode[i]->DownMsgNumber = 0x81;
											}
											else
											{
												g_globle->m_gateClient->vNode[i]->DownMsgNumber+=1;	
											}
											break;
										}
									}
									SendToWebClient(ClientSock,"ok");
								}
								if (sData)
								{
									delete[] sData;
									sData=NULL;
								}
								Log("55 out");
							}
							break;
						case WEBMSG_MODIFY://增删改设备信息
							{
								Log("60 in");
								Log("---------------------------------");
								Log("60:重新下发闸机列表：");
								g_globle->m_gateClient->vGate.clear();

								Json::Reader r;
								Json::Value root;
								r.parse(data,root);
								if(root.isArray())
								{
									for(int i=0;i<root.size();i++)
									{
										GATEMSG gateMsg;
										gateMsg.strGateID=root[i]["code"].asString();
										gateMsg.strGateIP=root[i]["ip"].asString();

										int num = g_globle->m_gateClient->vNode.size();
										for (int j=0;j<num;j++)
										{
											if (gateMsg.strGateIP == inet_ntoa(g_globle->m_gateClient->vNode[j]->Addr.sin_addr))
											{
												g_globle->m_gateClient->vNode[j]->strGateID = gateMsg.strGateID;
											}
										}
										g_globle->m_gateClient->vGate.push_back(gateMsg);
										Log("闸机%d：code:%s,ip:%s",i+1,gateMsg.strGateID.c_str(),gateMsg.strGateIP.c_str());
									}
								}
								SendToWebClient(ClientSock,"ok");
								Log("60 out");
							}
							break;
						case WEBMSG_MODIFY_FY://放映信息变化
							{
								Log("79 in");
								Log("---------------------------------");
								Log("79:放映信息变化,重新下发放映信息：");
								if(g_globle->m_gateClient->m_fsc_079->PraseData(data))
								{
									int nDatalen=0,len=0;
									char* sData = g_globle->m_gateClient->m_fsc_079->CombineData(nDatalen);
									if (sData)
									{
										for (int i=0;i<g_globle->m_gateClient->vNode.size();i++)
										{
											char* sendBuf = g_globle->m_gateClient->CombinetPacket(TYPE_DATA,g_globle->m_gateClient->vNode[i]->DownMsgNumber,nDatalen,sData,len);						
											g_globle->m_gateClient->SendToGateClient(g_globle->m_gateClient->vNode[i]->s,sendBuf,len);

											string strPack=g_globle->CharStr2HexStr(sendBuf,len);
											Log("下发放映信息：%s 流水号：%d",strPack.c_str(),g_globle->m_gateClient->vNode[i]->DownMsgNumber);

											delete[] sendBuf;

											if (g_globle->m_gateClient->vNode[i]->DownMsgNumber == 0xff)
											{
												g_globle->m_gateClient->vNode[i]->DownMsgNumber = 0x81;
											}
											else
											{
												g_globle->m_gateClient->vNode[i]->DownMsgNumber+=1;	
											}
										}
										delete[] sData;	
										SendToWebClient(ClientSock,"ok");
									}
									else
									{
										Log("m_fsc_079->CombineData Error!");
										SendToWebClient(ClientSock,"error",500);
									}
								}
								Log("79 out");
							}
							break;
						default:
							{
								Log("error! WebAcceptThread switch in default");
								break;
							}
						}//switch
					}
					catch (...)
					{
						Log("error! WebAcceptThread in catch ");
					}
					RELEASE_SOCKET(ClientSock);
					UNLOCK_MUTEX(g_globle->m_gateClient->Device_mutex);
					break;
				}
				len+=recvSize;
			}//while			
		}
	}//while
	RELEASE_SOCKET(serSock);
	Log("WebAcceptThread end!");
	par->run=false;
	return 0;
}

bool SendToWebClient(BASE_SOCKET sock,const char* data,int statuscode)
{
	Log("SendToWebClient in");
	int nDataLen=strlen(data);
	char* send_str=new char[nDataLen+512]; 
	memset(send_str,0,nDataLen+512);        
	//头
	sprintf(send_str,"HTTP/1.1 %d OK\r\n",statuscode);
	strcat(send_str, "Connection: close\r\n");                 
	char content_header[100];      
	sprintf(content_header,"Content-Length: %d\r\n", strlen(data));       
	strcat(send_str, content_header);
	strcat(send_str, "application/json: text/html\r\n");
	//内容
	strcat(send_str, "\r\n");     
	strcat(send_str, data);

	int sendLen=0;
	do 
	{
		int per_sendLen=Send(sock, send_str, strlen(send_str));
		if (per_sendLen<0)
		{
			delete[] send_str;
			Log("SendToWebClient: send return < 0");
			return false;
		}
		sendLen+=per_sendLen;
	} while (sendLen<strlen(send_str));
	delete[] send_str;
	Log("SendToWebClient out");
	return true;
}

string SendToHttpServer(const char* url,bool bPost,string strData)
{
	Log("SendToHttpServer in");
	char host[128]={0};
	int port=80;
	{
#if 1
		char tmp[16]={0};
		//memcpy(tmp,url,8);
		for(int i=0;i<7&&url[i];i++)
		{
			tmp[i]=url[i];
			if(tmp[i]>='A'&&tmp[i]<='Z')
			{
				tmp[i]+='a'-'A';
			}
		}
		const char* p2=url;
		if(!strcmp(tmp,"http://"))p2+=7;
		const char* p=strstr(p2,"/");
		if(p)
		{
			memcpy(host,p2,p-p2);
		}
		else
		{
			strcat(host,p2);
		}
		p=strstr(host,":");
		if(p)
		{
			sscanf(p+1,"%d",&port);
			((char*)p)[0]=0;
		}
#endif
	}


	int urllen=strlen(url);
	int datalen=strData.length();
	int total=1024*10+urllen+datalen;//预估数据量最大10K 如果不够自己加大点
	char* buf=new char[total];
	memset(buf,0,total);

	if (bPost)
	{
		sprintf(buf,"POST %s  HTTP/1.1\r\n",url);
		sprintf(buf,"%sHost:%s\r\n",buf,host);
		sprintf(buf,"%sContent-Type: application/json\r\n",buf);
		sprintf(buf,"%sContent-Length: %d\r\n",buf,datalen);
		sprintf(buf,"%sAccept:*/*\r\n",buf);
		sprintf(buf,"%s\r\n%s",buf,strData.c_str());
	}
	else
	{
		sprintf(buf,"GET %s  HTTP/1.1\r\n",url);
		sprintf(buf,"%sHost:%s\r\n",buf,host);
		sprintf(buf,"%sAccept:*/*\r\n\r\n",buf);
	}

	Log("host:%s,port:%d",host,port);
	Log("send buf:%s",buf);
	BASE_SOCKET s=::GetSocket(1);
	bool out=false;
	int type=0;
	do
	{
		if(Connect(s,host,port,5000)<0)
		{
			Log("Connect to %s:%d failed",host,port);
			break;
		}
		if(Send(s,buf,strlen(buf))<0)
		{
			Log("Send error");
			break;
		}
		memset(buf,0,total);

		int nRcv=0,nLen=0,headLen=0;//type=1 content-lenth type=2 trunked
		while(nRcv<total)
		{
			int n=Recv(s,buf+nRcv,total-nRcv);
			if(n>0)
				nRcv+=n;
			else 
				break;

			if(0==type)
			{
				char* p=strstr(buf,"\r\n\r\n");
				if(!p)continue;
				char ch=p[0];
				*p=0;
				if(strstr(buf,"Transfer-Encoding"))
				{
					type=2;
				}
				char* p2=strstr(buf,"Content-Length");
				if(p2)
				{
					headLen=p+4-buf;
					nLen=atoi(p2+strlen("Content-Length:"));
					type=1;
				}
				p[0]=ch;
			}

			if(2==type&&strstr(buf,"\r\n0\r\n"))
				break;
			if(1==type&&(nRcv-headLen>=nLen))
				break;
		}
	}while(0);
	::CloseSocket(s);

	string ret="";
	if(2==type)
	{
#if 1
		char* p=strstr(buf,"\r\n\r\n")+4;
		int n=0;
		while(1)
		{
			sscanf(p,"%x",&n);
			if(0==n)break;

			p=strstr(p,"\r\n")+2;
			char ch=p[n];
			p[n]=0;
			ret+=p;
			p+=n+2;
		}
#endif
	}
	else if(1==type)
	{
		ret=strstr(buf,"\r\n\r\n")+4;
	}

	Log("httpserver 返回数据：%s",ret.c_str());
	Log("SendToHttpServer out");
	if (buf)
	{
		delete[] buf;
	}
	return ret;
}
