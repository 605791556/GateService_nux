#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
class CIFSC_071
{
public:
	char DeviceID[10];

public:
	CIFSC_071(void);
	~CIFSC_071(void);

	bool ParseData(unsigned char* data);
	//获取服务器Unix时间戳
	char* getUnixCurrtTime(int& dataLen);
};

