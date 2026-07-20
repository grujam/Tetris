#pragma once
#include "Draw.h"
#include "Tetromino.h"

enum class EBoardPosition
{
	Center,
	Left,
	Right,
};

class CBoard : public IDrawable
{
public:
	CBoard(EBoardPosition InPosition);
	virtual ~CBoard() {};
	
public:
	virtual void OnDraw();
	POINT GetCellOrigin(int InCol, int InRow) const;
	POINT GetNextBoxOrigin() const;

	bool IsCollision(const POINT* InShape, int InCol, int InRow) const;
	int LockTetromino(const POINT* InShape, int InCol, int InRow, ETetroBlockType InType);

	void SetGameOver() { m_bIsGameOver = true; }
	void SetWinner() { m_bIsWinner = true; }
	void SetScore(int InScore) { m_nDisplayScore = InScore; }
	void SetLevel(int InLevel) { m_nDisplayLevel = InLevel; }
	void SetRestartVotes(int InVotes) { m_bShowRestartVotes = true; m_nRestartVotes = InVotes; }
	void SetReplayInfo(bool InPaused, float InSpeed) { m_bIsReplayBoard = true; m_bReplayPaused = InPaused; m_fReplaySpeed = InSpeed; }

private:
	void DrawBorder();
	void DrawNextBox();
	void DrawOccupied();
	void DrawScoreandLevel();
	void DrawGameOver();
	void DrawWinner();
	void DrawRestartVotes();
	void DrawReplayInfo();

	int ClearFullLines();

	int CalcCenterOffsetX() const;

private:
	//보드 4개의 꼭짓점
	POINT m_arrBoardPoints[4] = {};
	int m_Occupied[10][20] = {};
	bool m_bIsGameOver = false;
	bool m_bIsWinner = false;

	bool m_bShowRestartVotes = false;
	int m_nRestartVotes = 0;
	

	int m_nDisplayScore = 0;
	int m_nDisplayLevel = 1;

	bool m_bIsReplayBoard = false;
	bool m_bReplayPaused = false;
	float m_fReplaySpeed = 1.0f;

	EBoardPosition m_ePosition;

private:
	constexpr static int m_BoardWidth = 400;
	constexpr static int m_BlockSize = 40;
	constexpr static int m_BoardHeight = 20 * m_BlockSize;
	constexpr static int m_NextBoxSize = 5;
	constexpr static int m_MultiBoardGap = 300;
	constexpr static int m_nRestartTotal = 2;
};