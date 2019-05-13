#pragma once

class CIFSC_062
{
public:
	 char   DeviceID[11];
	 char   EnOutFlag[2];//进出闸标志 0进1出
	 short  TDCodeLen;
	 unsigned char*   TDCode;
public:
	CIFSC_062();
	~CIFSC_062();

	 bool ParseData(unsigned char* data);
};