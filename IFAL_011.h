#pragma once
#include <vector>
#include <string>

using std::vector;
using std::string;


struct GATE_EVENT
{
	char EventID[3];
	char StatusID[4];//EventID为99时有效
	char StatusLevel;//EventID为99时有效
	char EventArgument[33];//如果故障，则表示故障原因
	char OccurredTOD[15];//发生故障时间(时间戳)

	GATE_EVENT()
	{
		memset(EventID,0,sizeof(EventID));
		memset(StatusID,0,sizeof(StatusID));
		memset(EventArgument,0,sizeof(EventArgument));
		memset(OccurredTOD,0,sizeof(OccurredTOD));
	}
};

//AGM事件信息上传
class CIFAL_078
{
public:
	char msgType[2];
	char deviceID[11];
	unsigned char RepeatCount;
	vector<GATE_EVENT> vGateEvent;

public:
	CIFAL_078(void);
	~CIFAL_078(void);

	bool ParseData(unsigned char* data);
};

