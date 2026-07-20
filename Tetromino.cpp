#include "stdafx.h"
#include "Tetromino.h"
#include "Pen.h"
#include "Brush.h"
#include <random>

CTetromino::CTetromino(POINT InPT, bool InIsGhost)
	: m_eBlockType(ETetroBlockType::BLOCK_NONE)
	, m_eRotation(ERotationType::UP)
	, m_Origin(InPT)
	, m_bIsGhost(InIsGhost)
{
	SetRandomType();
}

void CTetromino::SetRandomType()
{
	static std::mt19937 mertwist(std::random_device{}());

	std::uniform_int_distribution<int> uit(
		static_cast<int>(ETetroBlockType::BLOCK_I),
		static_cast<int>(ETetroBlockType::BLOCK_L));

	m_eBlockType = static_cast<ETetroBlockType>(uit(mertwist));
}

void CTetromino::SetGhost(bool InBool)
{
	m_bIsGhost = InBool;
}

void CTetromino::SetRotation(ERotationType InType)
{
	m_eRotation = InType;
}

void CTetromino::Rotate(int InDir)
{
	const int nCount = (int)ERotationType::TYPE_END;
	int nNext = ((int)m_eRotation + InDir % nCount + nCount) % nCount;
	m_eRotation = (ERotationType)nNext;
}

void CTetromino::SetPosition(POINT InPT)
{
	m_Origin = InPT;
}

void CTetromino::OnDraw()
{
	if (m_eBlockType == ETetroBlockType::BLOCK_NONE)
		return;

	const POINT* shape = g_TetrominoTable[(int)m_eBlockType][(int)m_eRotation];

	if (m_bIsGhost)
	{
		SCOPED_PEN_RGB(ghostPen, PS_DASH, 1, GetBlockColor(m_eBlockType));
		SCOPED_BRUSH_STOCK(ghostBrush, NULL_BRUSH);

		for (int i = 0; i < 4; ++i)
		{
			LONG x = m_Origin.x + shape[i].x * BLOCK_SIZE;
			LONG y = m_Origin.y + shape[i].y * BLOCK_SIZE;
			Rectangle(GET_HDC(), x, y, x + BLOCK_SIZE, y + BLOCK_SIZE);
		}
		return;
	}

	SCOPED_PEN_RGB(blockPen, PS_SOLID, 1, RGB_BLACK);
	SCOPED_BRUSH_COLOR(blockBrush, GetBlockColor(m_eBlockType));


	for (int i = 0; i < 4; ++i)
	{
		LONG x = m_Origin.x + shape[i].x * BLOCK_SIZE;
		LONG y = m_Origin.y + shape[i].y * BLOCK_SIZE;

		Rectangle(GET_HDC(), x, y, x + BLOCK_SIZE, y + BLOCK_SIZE);
	}
}

COLORREF CTetromino::GetBlockColor(ETetroBlockType InType)
{
	switch (InType)
	{
	case ETetroBlockType::BLOCK_I: return RGB(0, 255, 255);
	case ETetroBlockType::BLOCK_O: return RGB(255, 255, 0);
	case ETetroBlockType::BLOCK_T: return RGB(160, 0, 160);
	case ETetroBlockType::BLOCK_S: return RGB(0, 200, 0);
	case ETetroBlockType::BLOCK_Z: return RGB(200, 0, 0);
	case ETetroBlockType::BLOCK_J: return RGB(0, 0, 200);
	case ETetroBlockType::BLOCK_L: return RGB(230, 140, 0);
	default: return RGB_BLACK;
	}
}

POINT CTetromino::GetCenterOffset(int InBoxSize) const
{
	const POINT* shape = g_TetrominoTable[(int)m_eBlockType][(int)m_eRotation];

	int nMinCol = shape[0].x, nMaxCol = shape[0].x;
	int nMinRow = shape[0].y, nMaxRow = shape[0].y;

	for (int i = 1; i < 4; ++i)
	{
		nMinCol = (shape[i].x < nMinCol) ? shape[i].x : nMinCol;
		nMaxCol = (shape[i].x > nMaxCol) ? shape[i].x : nMaxCol;
		nMinRow = (shape[i].y < nMinRow) ? shape[i].y : nMinRow;
		nMaxRow = (shape[i].y > nMaxRow) ? shape[i].y : nMaxRow;
	}

	int nWidth = nMaxCol - nMinCol + 1;
	int nHeight = nMaxRow - nMinRow + 1;

	int nOffsetX = (InBoxSize - nWidth) * BLOCK_SIZE / 2 - nMinCol * BLOCK_SIZE;
	int nOffsetY = (InBoxSize - nHeight) * BLOCK_SIZE / 2 - nMinRow * BLOCK_SIZE;

	return POINT{ nOffsetX, nOffsetY };
}

void CTetromino::SetBlockType(ETetroBlockType InType)
{
	m_eBlockType = InType;
}
