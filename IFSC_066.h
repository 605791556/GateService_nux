#pragma once

//散客过闸记录
class CIFSC_066
{
public:
	char msgType[3];
	char deviceID[11];
	char number[18];//过闸流水号
	char result;//0成功，1失败
	char ErrorDescription[17];//过闸失败原因说明
	char direction[2];//0进，1出
	short CodeLen;
	unsigned char codeLength[3];//二维码长度
	unsigned char* code;//二维码
	char OccurredTOD[15];//过闸时间(BCD码)

public:
	CIFSC_066(void);
	~CIFSC_066(void);
	bool ParseData(unsigned char* data);
};

