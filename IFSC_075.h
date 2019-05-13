#pragma once
class CIFSC_075
{
public:
	char DeviceID[10];
	char EnOutFlag[1];//进出闸标志 0进1出
	short  CardType;//卡物理类型
	short  PhysicalNOLength;//会员卡物理卡号长度
	char PhysicalNO[1024];//物理卡号

public:
	CIFSC_075(void);
	~CIFSC_075(void);

	bool ParseData(unsigned char* data);
};

