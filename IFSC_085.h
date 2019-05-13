#pragma once
class CIFSC_085
{
public:
	char msgType[2];
	char deviceID[11];
	char EnterOutFlag;//进出闸标志位，'0'进闸，'1'出闸
	short CodeLen;
	unsigned char TDCodeLength[2];//二维码长度
	char* TD_Code;//二维码
	short faceLen;
	unsigned char FacePicLength[2];//照片长度
	unsigned char* FacePicData;//人脸长度

public:
	CIFSC_085(void);
	~CIFSC_085(void);
	bool ParseData(unsigned char* data);
};

