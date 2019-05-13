#pragma once
//人脸过闸验证结果
class CIFSC_086
{
public:
	char msgType[2];
	char deviceID[11];
	char ValidationResults[3];//"00"人脸绑定成功，"01"影票验证不通过,告警，"03"未找到消费信息,“04”未到进场时间,“05”此票已存在人脸(此时终端需要验证此接口返回人脸数据与实时人脸对比)

	string strMsg;
	string faceid;//05结果下已经绑定的人脸数据
public:
	CIFSC_086(const char* _deviceID);
	~CIFSC_086(void);

	bool ParseData(string strData);
	char* CombineData(int& datalen);
};

