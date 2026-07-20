#pragma once
#include "Draw.h"
#include "windows.h"

#include <vector>
#include <string>

class CReplaySelect : public IDrawable
{
public:
	void RefreshFileList();

public:
	virtual void OnDraw() override;
	void OnClick(POINT InPT);

private:
	void DrawRow(const RECT& InRect, const wchar_t* InLabel);

private:
	std::vector<std::string> m_FileList;
	std::vector<RECT> m_FileRowRects;
	RECT m_BackButtonRect{};

private:
	constexpr static int m_RowWidth = 400;
	constexpr static int m_RowHeight = 50;
	constexpr static int m_RowGap = 15;
};
