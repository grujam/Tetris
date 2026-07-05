#pragma once
#include "Singleton.h"

#include <memory>

class CBoard;
class CTetromino;
class CTetrisManager : public ISingleton<CTetrisManager>
{
public:
	CTetrisManager();
	virtual ~CTetrisManager() {};

public:
	void Initialize();
	void OnUpdate(float InDeltaSeconds);
	void MoveHorizontal(int InDir);
	void Rotate();
	void SoftDrop();
	void HardDrop();
	void AddScore(int InClearedLines);
	void Hold();

	int GetScore();
	int GetLevel();

private:
	void CreateObjects();
	void StartDrop();
	void SetNextBlock();
	void UpdateGhost();

	void CheckGameOver();
	void CheckIncreaseLevel();

private:
	std::shared_ptr<CBoard> m_pBoard;
	std::shared_ptr<CTetromino> m_CurrentTetromino;
	std::shared_ptr<CTetromino> m_NextTetromino;
	std::shared_ptr<CTetromino> m_GhostTetromino;

	float m_fDropSpeed;
	int m_nLevel;
	int m_nScore;
	float m_fDropTimer;
	bool m_bIsDropping;

	int m_nCurBlockRow;
	int m_nCurBlockCol;

	bool m_bIsGameOver;
	bool m_bIsHold;

private:
	constexpr static int m_nStartBlockCol = 3;
	constexpr static int m_nStartBlockRow = 1;
	constexpr static int m_nBaseScorePerLine = 100;
	constexpr static float m_nBaseDropSpeed = 2.0f;
	constexpr static float m_nLevelSpeedRatio = 0.1f;
};