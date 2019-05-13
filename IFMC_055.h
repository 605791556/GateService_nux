#pragma once
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <string>
using std::string;

//Éè±¸¿ØÖÆÃüÁî
class CIFMC_055
{
public:
	char msgType[2];
	char deviceID[10];
	char controlCode[4];
public:
	CIFMC_055(string deviceID,string commonCode);
	~CIFMC_055(void);

	bool ParseData(unsigned char* data);
	char* CombineData(int& len);
};

