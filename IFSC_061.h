#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

using std::string;

//过闸验证结果
class CIFSC_061
{
public:
	char msgType[2];
	char deviceID[10];
	char result[3];//过闸验证结果
	char number[17];//过闸流水号
	uint8_t AlermGulp;//是否特殊票
	uint8_t MessageLen;//闸机UI提示信息长度
	char* Message;//闸机UI提示信息,长度为MessageLen
	uint8_t ClientCount;//过闸人数
	uint8_t PrintCount;//需要打印的小票数量，0为没有
	uint16_t PrintLen;//打印字节长度
public:
	CIFSC_061(char* _deviceID);
	~CIFSC_061(void);

	bool ParseData(const char* data);
	char* CombineData(int& datalen);
};

