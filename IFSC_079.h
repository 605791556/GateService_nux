#pragma once
#include <vector>
#include <string>

using std::vector;
using std::string;

struct FY_MSG
{
	char checkTime;
	char delayTime;
	char filmName[64];//电影名称
	char CinemaTitle[64];//影厅名称
	char fileType;//影片类型
	char showTime[15];//放映时间(BCD码)，年月日时分秒
	char endTime[15];//结束时间(BCD码) 年月日时分秒

	FY_MSG()
	{
		memset(filmName,0x00,sizeof(filmName));
		memset(CinemaTitle,0x00,sizeof(CinemaTitle));
		memset(showTime,0x00,sizeof(showTime));
		memset(endTime,0x00,sizeof(endTime));
	}
};

//放映信息报文
class CIFSC_079
{
public:
	char msgType[2];
	unsigned char count;
	vector<FY_MSG> vFY;
public:
	CIFSC_079(void);
	~CIFSC_079(void);

	bool PraseData(string data);
	char* CombineData(int& datalen);
	//void Send(Node* pNode,char* sData,int nDatalen);
};

