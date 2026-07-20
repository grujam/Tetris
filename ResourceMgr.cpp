#include "stdafx.h"
#include "ResourceMgr.h"

CResourceMgr::~CResourceMgr()
{
    for (auto& pair : m_mBitmaps)
        DeleteObject(pair.second);
}

void CResourceMgr::Initialize()
{
	LoadResources();
}

HBITMAP CResourceMgr::GetBitmap(const std::string& InKey)
{
    auto iter = m_mBitmaps.find(InKey);
    return iter != m_mBitmaps.end() ? iter->second : nullptr;
}


void CResourceMgr::LoadResources()
{
    HBITMAP hBmp = (HBITMAP)LoadImage(
        nullptr,
        L"TetrisBackground.bmp",
        IMAGE_BITMAP,
        0, 0,
        LR_LOADFROMFILE | LR_CREATEDIBSECTION
    );

    if (hBmp)
        m_mBitmaps["Lobby"] = hBmp;
}
