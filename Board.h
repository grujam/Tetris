#pragma once
#include "Draw.h"
#include "Tetromino.h"

class CBoard : public IDrawable
{
public:
	virtual ~CBoard() {};
	
public:
	virtual void OnDraw();
	POINT GetCellOrigin(int InCol, int InRow) const;
	POINT GetNextBoxOrigin() const;

	bool IsCollision(const POINT* InShape, int InCol, int InRow) const;
	void LockTetromino(const POINT* InShape, int InCol, int InRow, ETetroBlockType InType);

	void SetGameOver() { m_bIsGameOver = true; }

private:
	void DrawBorder();
	void DrawNextBox();
	void DrawOccupied();
	void DrawScoreandLevel();
	void DrawGameOver();

	void ClearFullLines();

private:
	//보드 4개의 꼭짓점
	POINT m_arrBoardPoints[4];
	int m_Occupied[10][20];
	bool m_bIsGameOver = false;

private:
	constexpr static int m_BoardWidth = 400;
	constexpr static int m_BlockSize = 40;
	constexpr static int m_BoardHeight = 20 * m_BlockSize;
	constexpr static int m_NextBoxSize = 5;
};