#pragma once
#include "Draw.h"
#include "windows.h"

class CLobby : public IDrawable
{
public:
	virtual ~CLobby();

public:
	virtual void OnDraw() override;
	void OnClick(POINT InPT);

private:
	void DrawBackground();
	void DrawButton(const RECT& InRect, const wchar_t* InLabel);

private:
	RECT m_SingleButtonRect{};
	RECT m_MultiButtonRect{};
	RECT m_ReplayButtonRect{};
	RECT m_ExitButtonRect{};

	HDC m_hBackgroundDC = nullptr;

private:
	constexpr static int m_ButtonWidth = 300;
	constexpr static int m_ButtonHeight = 60;
	constexpr static int m_ButtonGap = 30;
};