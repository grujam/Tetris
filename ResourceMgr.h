#pragma once
#include "Singleton.h"
#include <string>
#include <unordered_map>

class CResourceMgr : public ISingleton<CResourceMgr>
{
public:
	CResourceMgr() {};
	virtual ~CResourceMgr();

public:
	void Initialize();
	HBITMAP GetBitmap(const std::string& InKey);

private:
	void LoadResources();

private:
	std::unordered_map<std::string, HBITMAP> m_mBitmaps;
};