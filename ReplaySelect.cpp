#include "stdafx.h"
#include "ReplaySelect.h"
#include "Pen.h"
#include "Brush.h"
#include "Renderer.h"
#include "TetrisManager.h"
#include "ReplayManager.h"

#include <algorithm>

void CReplaySelect::RefreshFileList()
{
	m_FileList = CReplayManager::Get()->ListReplayFiles();
	std::sort(m_FileList.begin(), m_FileList.end(), std::greater<std::string>());
}

void CReplaySelect::OnDraw()
{
	RECT rect;
	GetClientRect(GET_HWND(), &rect);

	int nCenterX = (rect.right - rect.left) / 2;

	SetTextColor(GET_HDC(), RGB_WHITE);
	SetBkMode(GET_HDC(), TRANSPARENT);

	RECT titleRect{ nCenterX - m_RowWidth / 2, 40, nCenterX + m_RowWidth / 2, 90 };
	DrawTextW(GET_HDC(), L"리플레이 선택", -1, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	m_FileRowRects.clear();

	int nStartY = 110;

	if (m_FileList.empty())
	{
		RECT emptyRect{ nCenterX - m_RowWidth / 2, nStartY, nCenterX + m_RowWidth / 2, nStartY + m_RowHeight };
		DrawTextW(GET_HDC(), L"저장된 리플레이가 없습니다.", -1, &emptyRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
	else
	{
		for (size_t i = 0; i < m_FileList.size(); ++i)
		{
			RECT rowRect{ nCenterX - m_RowWidth / 2, (LONG)(nStartY + i * (m_RowHeight + m_RowGap)),
				nCenterX + m_RowWidth / 2, (LONG)(nStartY + i * (m_RowHeight + m_RowGap) + m_RowHeight) };

			std::wstring wFileName(m_FileList[i].begin(), m_FileList[i].end());
			DrawRow(rowRect, wFileName.c_str());

			m_FileRowRects.push_back(rowRect);
		}
	}

	LONG nBackY = rect.bottom - 90;
	m_BackButtonRect = RECT{ nCenterX - m_RowWidth / 2, nBackY, nCenterX + m_RowWidth / 2, nBackY + m_RowHeight };
	DrawRow(m_BackButtonRect, L"뒤로가기");
}

void CReplaySelect::DrawRow(const RECT& InRect, const wchar_t* InLabel)
{
	SCOPED_PEN_RGB(rowPen, PS_SOLID, 2, RGB_WHITE);
	SCOPED_BRUSH_COLOR(rowBrush, RGB(40, 40, 60));

	Rectangle(GET_HDC(), InRect.left, InRect.top, InRect.right, InRect.bottom);

	RECT textRect = InRect;
	DrawTextW(GET_HDC(), InLabel, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void CReplaySelect::OnClick(POINT InPT)
{
	for (int i = 0; i < m_FileRowRects.size(); ++i)
	{
		if (PtInRect(&m_FileRowRects[i], InPT))
		{
			CTetrisManager::Get()->StartReplay(m_FileList[i]);
			return;
		}
	}

	if (PtInRect(&m_BackButtonRect, InPT))
	{
		CTetrisManager::Get()->ReturnToLobby();
	}
}
