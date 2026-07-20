#pragma once
#include <memory>
#include "NetworkManager.h"
#include "Board.h"

class CBoard;
class CTetromino;

// 멀티용 TetrisManager 접근 세션 같은 느낌..
class CPlayerSession
{
public:
	void CreateObjects(EBoardPosition InPosition, ETetroBlockType InCurrentType, ETetroBlockType InNextType);

	std::shared_ptr<CBoard> GetBoard() const { return m_pBoard; }
	std::shared_ptr<CTetromino>& GetCurrentTetromino() { return m_CurrentTetromino; }
	std::shared_ptr<CTetromino>& GetNextTetromino() { return m_NextTetromino; }
	std::shared_ptr<CTetromino> GetGhostTetromino() const { return m_GhostTetromino; }

	int GetCurCol() const { return m_nCurBlockCol; }
	int GetCurRow() const { return m_nCurBlockRow; }
	void SetCurCol(int InCol) { m_nCurBlockCol = InCol; }
	void SetCurRow(int InRow) { m_nCurBlockRow = InRow; }

	int GetDropTickCounter() const { return m_nDropTickCounter; }
	void SetDropTickCounter(int InCount) { m_nDropTickCounter = InCount; }
	int GetDropIntervalTicks() const { return m_nDropIntervalTicks; }

	bool IsDropping() const { return m_bIsDropping; }
	void SetDropping(bool InBool) { m_bIsDropping = InBool; }

	bool IsGameOver() const { return m_bIsGameOver; }
	void SetGameOver();
	void SetWinner();

	int GetScore() const { return m_nScore; }
	int GetLevel() const { return m_nLevel; }
	void AddScore(int InClearedLines);

	void ResetForNewGame()
	{
		m_nScore = 0;
		m_nLevel = 1;
		m_fDropSpeed = m_nBaseDropSpeed;
		m_nDropIntervalTicks = (int)(m_fDropSpeed / (1.0f / 60.0f));
		if (m_nDropIntervalTicks < 1) m_nDropIntervalTicks = 1;
	}

	constexpr static int GetStartCol() { return m_nStartBlockCol; }
	constexpr static int GetStartRow() { return m_nStartBlockRow; }

private:
	void CheckIncreaseLevel();

private:
	std::shared_ptr<CBoard> m_pBoard;
	std::shared_ptr<CTetromino> m_CurrentTetromino;
	std::shared_ptr<CTetromino> m_NextTetromino;
	std::shared_ptr<CTetromino> m_GhostTetromino;

	int m_nCurBlockRow = 1;
	int m_nCurBlockCol = 3;

	int m_nDropTickCounter = 0;
	int m_nDropIntervalTicks = 1;

	float m_fDropSpeed = 2.0f;
	int m_nLevel = 1;
	int m_nScore = 0;
	bool m_bIsDropping = false;
	bool m_bIsGameOver = false;

private:
	constexpr static int m_nStartBlockCol = 3;
	constexpr static int m_nStartBlockRow = 1;
	constexpr static int m_nBaseScorePerLine = 100;
	constexpr static float m_nBaseDropSpeed = 2.0f;
	constexpr static float m_nLevelSpeedRatio = 0.1f;
};
