#include "BaseConfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#include <direct.h> 
#else
#include <unistd.h>
#include <sys/stat.h>
#endif
#define MAX_LINE_SIZE 512
struct sBaseConfig
{
	char name[1024];//配置文件全路径
	char* keys[1024];//最多允许存储1000行设置 包括注释行
	int num;

	sBaseConfig()
	{
		name[0]=0;
		num=0;
	}
	~sBaseConfig()
	{
		for(int i=0;i<num;i++)
		{
			if(keys[i])delete keys[i];
		}
		num=0;
	}
};
sBaseConfig* ReadConfig(const char* name);//返回空表示出错
bool SaveConfig(sBaseConfig* data);
CBaseConfig::CBaseConfig(const char* name)
{
	m_pData=ReadConfig(name);
}

CBaseConfig::~CBaseConfig(void)
{
	if(m_pData)
	{
		delete ((sBaseConfig*)m_pData);
	}
}

const char* CBaseConfig::GetValue(const char* key)
{
	sBaseConfig* data=(sBaseConfig*)m_pData;
	if(!m_pData)return 0;

	for(int i=0;i<data->num;i++)
	{
		if(data->keys[i][1]&&0==strcmp(key,data->keys[i]+1))
		{
			return data->keys[i]+1+strlen(data->keys[i]+1)+1;
		}
	}
	return 0;
}
bool CBaseConfig::AddValue(const char* key)
{
	sBaseConfig* data=(sBaseConfig*)m_pData;
	if(!m_pData)return false;

	for(int i=0;i<data->num;i++)
	{
		if(data->keys[i][1]&&0==strcmp(key,data->keys[i]+1))
		{
			return false;
		}
	}

	{
		data->keys[data->num]=new char[512];
		data->keys[data->num][0]=0;//有值的行

		sprintf(data->keys[data->num]+1,"%s",key);
		data->num+=1;
	}

	SaveConfig(data);
	return true;
}
bool CBaseConfig::AddValue(const char* key,const char* value)
{
	sBaseConfig* data=(sBaseConfig*)m_pData;
	if(!m_pData)return false;

	for(int i=0;i<data->num;i++)
	{
		if(data->keys[i][1]&&0==strcmp(key,data->keys[i]+1))
		{
			return false;
		}
	}

	{
		data->keys[data->num]=new char[512];
		data->keys[data->num][0]=1;//有值的行


		sprintf(data->keys[data->num]+1,"%s",key);
		sprintf(data->keys[data->num]+1+strlen(key)+1,"%s",value);
		data->num+=1;
	}

	SaveConfig(data);
	return true;
}
bool CBaseConfig::DelValue(const char* key)
{
	sBaseConfig* data=(sBaseConfig*)m_pData;
	if(!m_pData)return false;
	
	int n=-1;
	for(int i=0;i<data->num;i++)
	{
		if(data->keys[i][1]&&0==strcmp(key,data->keys[i]+1))
		{
			n=i;
			break;
		}
	}
	if(0>n)return false;
	for(int i=n;i<data->num;i++)
	{
		memset(data->keys[i],0,MAX_LINE_SIZE);
		if(i+1<data->num)
		{
			memcpy(data->keys[i],data->keys[i+1],MAX_LINE_SIZE);
		}
	}
	data->num-=1;
	SaveConfig(data);
	return true;
}
bool CBaseConfig::SetValue(const char* key,const char* value)
{
	sBaseConfig* data=(sBaseConfig*)m_pData;
	if(!m_pData)return false;
	
	bool ret=false;
	for(int i=0;i<data->num;i++)
	{
		if(data->keys[i][1]&&0==strcmp(key,data->keys[i]+1))
		{
			sprintf(data->keys[i]+1+strlen(key)+1,"%s",value);
			ret=true;
			break;
		}
	}
	if(!ret)
	{
		return false;
	}

	SaveConfig(data);
	return true;
}
sBaseConfig* ReadConfig(const char* name)
{
	sBaseConfig* data=new sBaseConfig();
	sprintf(data->name,"%s",name);
	FILE* fp=0;
	do
	{
		fp=fopen(name,"r");
		if(!fp)
		{
			//Log("open error %s",name);
			break;
		}

		while(!feof(fp))
		{
			char buf[1024]={0};
			if(!fgets(buf,10240,fp))continue;
			else
			{
				char* p2=strstr(buf,"\r");
				if(p2)p2[0]=0;
				 p2=strstr(buf,"\n");
				if(p2)p2[0]=0;
			}
			char* p=buf;
			while(p[0]&&' '==p[0])++p;
			if('='==p[0])continue;//该行格式有问题
			else if('#'==p[0])//注释行
			{
				data->keys[data->num]=new char[MAX_LINE_SIZE];
				data->keys[data->num][0]=0;//注释行
				sprintf(data->keys[data->num]+1,"%s",p);
				data->num+=1;
			}
			else if(0==p[0])//注释行
			{
				data->keys[data->num]=new char[MAX_LINE_SIZE];
				data->keys[data->num][0]=0;//注释行
				sprintf(data->keys[data->num]+1,"%s"," ");
				data->num+=1;
			}
			else
			{
				char* p2=p;
				while(p2[0])
				{
					if(' '==p2[0])
					{
						p2[0]=0;
						++p2;
					}
					else if('='==p2[0])break;
					else ++p2;
				}
				if(!p2[0])continue;//不是标准行
				p2[0]=0;

				data->keys[data->num]=new char[512];
				data->keys[data->num][0]=1;//有值的行


				{
					char* tmp=p;
					while(' '==tmp[0])++tmp;
					sprintf(data->keys[data->num]+1,"%s",tmp);
					tmp=p2+1;
					while(' '==tmp[0])++tmp;
					sprintf(data->keys[data->num]+1+strlen(p)+1,"%s",tmp);
				}
				data->num+=1;
			}
		}
	}while(0);
	if(fp)fclose(fp);

	return data;
}
bool createDirByName(char* file)
{
	if(0==file||0==file[0])return false;
	int len=strlen(file)+2;
	char* tmp=new char[len];
	char* dir=new char[len];
	memset(dir,0,len);
	memcpy(tmp,file,len-1);
	strcat(tmp,"/");

	char* p=tmp;
	while(p=strstr(p,"\\"))
	{
		p[0]='/';
	}

	p=tmp;
#ifdef linux
	char cmd[1024]={0};
	sprintf(cmd,"mkdir -p %s",p);
	system(cmd);
	return true;
#endif
	while(p=strstr(p,"/"))
	{
		memcpy(dir,tmp,p+1-tmp);
#ifdef WIN32
		if(access(dir,0))mkdir(dir);
#else
		if(access(dir,0))mkdir(dir,S_IRWXU|S_IROTH);
#endif
		
		p++;
	}
	return true;
}
bool SaveConfig(sBaseConfig* data)
{
	FILE* fp=fopen(data->name,"w+");
	if(!fp)
	{
		createDirByName(data->name);
		fp=fopen(data->name,"w+");
		if(!fp)return false;
	}

	for(int i=0;i<data->num;i++)
	{
		if(data->keys[i][0])
		{
			fprintf(fp,"%s=%s\n",data->keys[i]+1,data->keys[i]+1+strlen(data->keys[i]+1)+1);
		}
		else
		{
			if('#'==data->keys[i][1])fprintf(fp,"%s\n",data->keys[i]+1);
			else fprintf(fp,"\n");
		}
	}

	fclose(fp);
	return true;
}