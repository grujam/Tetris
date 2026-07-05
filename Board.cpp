#include "stdafx.h"
#include "Board.h"
#include "Pen.h"
#include "Brush.h"
#include "Renderer.h"
#include "TetrisManager.h"

void CBoard::OnDraw()
{
    SCOPED_PEN_RGB(rgbPen, PS_SOLID, 5, RGB_BLACK);
    SCOPED_BRUSH_STOCK(nullBrush, BLACK_BRUSH);

    RECT rect;
    GetClientRect(GET_HWND(), &rect);
    int nHorizonCenter = (rect.right - rect.left) / 2;
    int nVerticalCenter = (rect.bottom - rect.top) / 2;

    m_arrBoardPoints[0] = POINT{ (LONG)(nHorizonCenter - m_BoardWidth / 2), rect.top };
    m_arrBoardPoints[1] = POINT{ (LONG)(nHorizonCenter + m_BoardWidth / 2), rect.top };
    m_arrBoardPoints[2] = POINT{ (LONG)(nHorizonCenter + m_BoardWidth / 2), rect.top + m_BoardHeight };
    m_arrBoardPoints[3] = POINT{ (LONG)(nHorizonCenter - m_BoardWidth / 2), rect.top + m_BoardHeight };

    Polygon(GET_HDC(), m_arrBoardPoints, 4);

    DrawBorder();
    DrawNextBox();
    DrawOccupied();
    DrawScoreandLevel();
    DrawGameOver();
}

POINT CBoard::GetCellOrigin(int InCol, int InRow) const
{
    RECT rect;
    GetClientRect(GET_HWND(), &rect);
    int nHorizonCenter = (rect.right - rect.left) / 2;

    LONG boardLeft = nHorizonCenter - m_BoardWidth / 2;
    LONG boardTop = rect.top;

    return POINT{ boardLeft + InCol * m_BlockSize, boardTop + InRow * m_BlockSize };
}

POINT CBoard::GetNextBoxOrigin() const
{
    RECT rect;
    GetClientRect(GET_HWND(), &rect);

    LONG boxWidth = m_NextBoxSize * m_BlockSize;
    LONG margin = m_BlockSize;

    return POINT{ rect.right - boxWidth - margin, rect.top + margin };
}

bool CBoard::IsCollision(const POINT* InShape, int InCol, int InRow) const
{
    for (int i = 0; i < 4; ++i)
    {
        int col = InCol + InShape[i].x;
        int row = InRow + InShape[i].y;

        if (col < 0 || col >= 10 || row >= 20)
            return true;

        if (row >= 0 && m_Occupied[col][row] != 0)
            return true;
    }
    return false;
}

void CBoard::LockTetromino(const POINT* InShape, int InCol, int InRow, ETetroBlockType InType)
{
    for (int i = 0; i < 4; ++i)
    {
        int col = InCol + InShape[i].x;
        int row = InRow + InShape[i].y;

        if (col >= 0 && col < 10 && row >= 0 && row < 20)
            m_Occupied[col][row] = (int)InType;
    }

    ClearFullLines();
}

void CBoard::DrawBorder()
{
    SCOPED_PEN_RGB(borderPen, PS_SOLID, 2, RGB_WHITE);
    SCOPED_BRUSH_STOCK(borderBrush, GRAY_BRUSH);

    const LONG left = m_arrBoardPoints[0].x;
    const LONG top = m_arrBoardPoints[0].y;
    const LONG right = m_arrBoardPoints[1].x;
    const LONG bottom = m_arrBoardPoints[2].y;

    // 위 / 아래
    for (LONG x = left; x < right; x += m_BlockSize)
    {
        Rectangle(GET_HDC(), x, top, x + m_BlockSize, top + m_BlockSize);
        Rectangle(GET_HDC(), x, bottom, x + m_BlockSize, bottom + m_BlockSize);
    }

    // 좌 / 우 (상하 모서리와 겹치지 않게 top~bottom 구간만)
    for (LONG y = top; y <= bottom; y += m_BlockSize)
    {
        Rectangle(GET_HDC(), left - m_BlockSize, y, left, y + m_BlockSize);
        Rectangle(GET_HDC(), right, y, right + m_BlockSize, y + m_BlockSize);
    }
}

void CBoard::DrawNextBox()
{
    SCOPED_PEN_RGB(nextBoxPen, PS_SOLID, 2, RGB_BLACK);
    SCOPED_BRUSH_STOCK(nextBoxBrush, BLACK_BRUSH);

    POINT origin = GetNextBoxOrigin();
    LONG boxSize = m_NextBoxSize * m_BlockSize;

    Rectangle(GET_HDC(), origin.x, origin.y, origin.x + boxSize, origin.y + boxSize);
}

void CBoard::DrawOccupied()
{
    for (int col = 0; col < 10; ++col)
    {
        for (int row = 0; row < 20; ++row)
        {
            if (m_Occupied[col][row] == 0)
                continue;

            ETetroBlockType eType = (ETetroBlockType)m_Occupied[col][row];

            SCOPED_PEN_RGB(occupiedPen, PS_SOLID, 1, RGB_BLACK);
            SCOPED_BRUSH_COLOR(occupiedBrush, CTetromino::GetBlockColor(eType));

            POINT origin = GetCellOrigin(col, row);
            Rectangle(GET_HDC(), origin.x, origin.y, origin.x + m_BlockSize, origin.y + m_BlockSize);
        }
    }
}

void CBoard::DrawScoreandLevel()
{
    POINT boxOrigin = GetNextBoxOrigin();
    LONG boxSize = m_NextBoxSize * m_BlockSize;

    POINT scorePos{ boxOrigin.x, boxOrigin.y + boxSize + m_BlockSize / 2 };
    POINT levelPos{ boxOrigin.x, boxOrigin.y + boxSize + m_BlockSize };

    int nScore = CTetrisManager::Get()->GetScore();
    int nLevel = CTetrisManager::Get()->GetLevel();

    wchar_t buf[32];
    swprintf_s(buf, L"SCORE: %d", nScore);
    wchar_t buf2[32];
    swprintf_s(buf2, L"Level: %d", nLevel);

    SetTextColor(GET_HDC(), RGB_BLACK);
    SetBkMode(GET_HDC(), TRANSPARENT);
    TextOutW(GET_HDC(), scorePos.x, scorePos.y, buf, (int)wcslen(buf));
    TextOutW(GET_HDC(), levelPos.x, levelPos.y, buf2, (int)wcslen(buf2));
}

void CBoard::DrawGameOver()
{
    if (m_bIsGameOver == false)
        return;

    const LONG left = m_arrBoardPoints[0].x;
    const LONG top = m_arrBoardPoints[0].y;
    const LONG right = m_arrBoardPoints[1].x;
    const LONG bottom = m_arrBoardPoints[2].y;

    LONG centerY = (top + bottom) / 2;

    SetTextColor(GET_HDC(), RGB_WHITE);
    SetBkMode(GET_HDC(), TRANSPARENT);

    RECT titleRect{ left, centerY - m_BlockSize, right, centerY };
    RECT scoreRect{ left, centerY, right, centerY + m_BlockSize };
    RECT levelRect{ left, centerY, right, centerY + m_BlockSize * 2 };

    DrawTextW(GET_HDC(), L"GAME OVER", -1, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    wchar_t buf[32];
    swprintf_s(buf, L"SCORE: %d", CTetrisManager::Get()->GetScore());
    DrawTextW(GET_HDC(), buf, -1, &scoreRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    wchar_t buf2[32];
    swprintf_s(buf2, L"Level: %d", CTetrisManager::Get()->GetLevel());
    DrawTextW(GET_HDC(), buf2, -1, &levelRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void CBoard::ClearFullLines()
{
    int nClearedCount = 0;

    for (int row = 19; row >= 0; --row)
    {
        bool bFull = true;
        for (int col = 0; col < 10; ++col)
        {
            if (m_Occupied[col][row] == 0)
            {
                bFull = false;
                break;
            }
        }

        if (!bFull)
            continue;

        for (int r = row; r > 0; --r)
        {
            for (int col = 0; col < 10; ++col)
            {
                m_Occupied[col][r] = m_Occupied[col][r - 1];
            }
        }

        for (int col = 0; col < 10; ++col)
        {
            m_Occupied[col][0] = 0;
        }

        ++nClearedCount;
        ++row;
    }

    CTetrisManager::Get()->AddScore(nClearedCount);
}