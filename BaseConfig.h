#pragma once
/*
1.构造函数的name表示配置文件全路径例如d:/demo.config
2.如果配置函数里面有数据可以通过GetValue来获取
3.SetValue表示修改 必须要当期数据有对应的KEY数据才能修改
4.AddValue表示添加 必须要当期数据没有对应的KEY数据才能添加
5.DelValue表示删除 必须要当期数据有对应的KEY数据才能删除
*/
class CBaseConfig
{
public:
	CBaseConfig(const char* name);//全路径
	~CBaseConfig(void);

	const char* GetValue(const char* key);
	bool SetValue(const char* key,const char* value);
	bool AddValue(const char* key,const char* value);
	bool AddValue(const char* key);//添加注释
	bool DelValue(const char* key);
private:
	void* m_pData;
};
