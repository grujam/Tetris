#include "stdafx.h"
#include "Lobby.h"
#include "Pen.h"
#include "Brush.h"
#include "Renderer.h"
#include "TetrisManager.h"
#include "ResourceMgr.h"

CLobby::~CLobby()
{
	if (m_hBackgroundDC)
		DeleteDC(m_hBackgroundDC);
}

void CLobby::OnDraw()
{
	DrawBackground();

	RECT rect;
	GetClientRect(GET_HWND(), &rect);

	int nCenterX = (rect.right - rect.left) / 2;
	int nCenterY = (rect.bottom - rect.top) / 2;

	int nTotalHeight = m_ButtonHeight * 4 + m_ButtonGap * 3;
	int nStartY = nCenterY - nTotalHeight / 2;

	m_SingleButtonRect = RECT{ nCenterX - m_ButtonWidth / 2, nStartY, nCenterX + m_ButtonWidth / 2, nStartY + m_ButtonHeight };

	m_MultiButtonRect = RECT{ nCenterX - m_ButtonWidth / 2, m_SingleButtonRect.bottom + m_ButtonGap, nCenterX + m_ButtonWidth / 2, m_SingleButtonRect.bottom + m_ButtonGap + m_ButtonHeight };

	m_ReplayButtonRect = RECT{ nCenterX - m_ButtonWidth / 2, m_MultiButtonRect.bottom + m_ButtonGap, nCenterX + m_ButtonWidth / 2, m_MultiButtonRect.bottom + m_ButtonGap + m_ButtonHeight };

	m_ExitButtonRect = RECT{ nCenterX - m_ButtonWidth / 2, m_ReplayButtonRect.bottom + m_ButtonGap, nCenterX + m_ButtonWidth / 2, m_ReplayButtonRect.bottom + m_ButtonGap + m_ButtonHeight };

	SetTextColor(GET_HDC(), RGB_WHITE);
	SetBkMode(GET_HDC(), TRANSPARENT);


	DrawButton(m_SingleButtonRect, L"싱글 플레이");
	DrawButton(m_MultiButtonRect, L"멀티 플레이");
	DrawButton(m_ReplayButtonRect, L"리플레이 확인");
	DrawButton(m_ExitButtonRect, L"게임 종료");
}

void CLobby::DrawBackground()
{
	if (!m_hBackgroundDC)
	{
		HBITMAP hBmp = CResourceMgr::Get()->GetBitmap("Lobby");
		if (!hBmp)
			return;

		m_hBackgroundDC = CreateCompatibleDC(GET_HDC());
		SelectObject(m_hBackgroundDC, hBmp);
	}

	RECT rect;
	GetClientRect(GET_HWND(), &rect);

	BitBlt(GET_HDC(), 0, 0, rect.right - rect.left, rect.bottom - rect.top, m_hBackgroundDC, 0, 0, SRCCOPY);
}

void CLobby::DrawButton(const RECT& InRect, const wchar_t* InLabel)
{
	SCOPED_PEN_RGB(buttonPen, PS_SOLID, 2, RGB_WHITE);
	SCOPED_BRUSH_COLOR(buttonBrush, RGB(40, 40, 60));

	Rectangle(GET_HDC(), InRect.left, InRect.top, InRect.right, InRect.bottom);

	RECT textRect = InRect;
	DrawTextW(GET_HDC(), InLabel, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void CLobby::OnClick(POINT InPT)
{
	if (PtInRect(&m_SingleButtonRect, InPT))
	{
		CTetrisManager::Get()->StartSingleGame();
	}
	else if (PtInRect(&m_MultiButtonRect, InPT))
	{
		int nResult = MessageBoxW(GET_HWND(), L"호스트로 시작하시겠습니까?", L"Tetris", MB_YESNO);

		if (nResult == IDYES)
			CTetrisManager::Get()->StartMultiHost();
		else
			CTetrisManager::Get()->StartMultiJoin("127.0.0.1");
	}
	else if (PtInRect(&m_ReplayButtonRect, InPT))
	{
		CTetrisManager::Get()->OpenReplaySelect();
	}
	else if (PtInRect(&m_ExitButtonRect, InPT))
	{
		DestroyWindow(GET_HWND());
	}
}