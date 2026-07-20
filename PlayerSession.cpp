#include "stdafx.h"
#include "PlayerSession.h"
#include "Board.h"
#include "Tetromino.h"
#include "Draw.h"

void CPlayerSession::CreateObjects(EBoardPosition InPosition, ETetroBlockType InCurrentType, ETetroBlockType InNextType)
{
	m_pBoard = Draw::CreateDrawable<CBoard>(InPosition);

	POINT cellOrigin = m_pBoard->GetCellOrigin(m_nStartBlockCol, m_nStartBlockRow);
	POINT ghostOrigin = m_pBoard->GetCellOrigin(m_nStartBlockCol, 20);

	m_CurrentTetromino = Draw::CreateDrawable<CTetromino>(cellOrigin, false);
	m_CurrentTetromino->SetBlockType(InCurrentType);

	m_GhostTetromino = Draw::CreateDrawable<CTetromino>(ghostOrigin, true);

	m_NextTetromino = Draw::CreateDrawable<CTetromino>(m_pBoard->GetNextBoxOrigin(), false);
	m_NextTetromino->SetBlockType(InNextType);

	POINT boxOrigin = m_pBoard->GetNextBoxOrigin();
	POINT offset = m_NextTetromino->GetCenterOffset(5);
	m_NextTetromino->SetPosition(POINT{ boxOrigin.x + offset.x, boxOrigin.y + offset.y });
}

void CPlayerSession::SetGameOver()
{
	m_bIsGameOver = true;
	m_bIsDropping = false;
	if (m_pBoard)
		m_pBoard->SetGameOver();
}

void CPlayerSession::SetWinner()
{
	m_bIsDropping = false;
	if (m_pBoard)
		m_pBoard->SetWinner();
}

void CPlayerSession::AddScore(int InClearedLines)
{
	m_nScore += InClearedLines * m_nBaseScorePerLine * m_nLevel;
	CheckIncreaseLevel();
	m_pBoard->SetScore(m_nScore);
	m_pBoard->SetLevel(m_nLevel);
}

void CPlayerSession::CheckIncreaseLevel()
{
	m_nLevel = 1 + m_nScore / 1000;
	float fDropSpeed = m_nBaseDropSpeed - m_nLevel * m_nLevelSpeedRatio;
	m_fDropSpeed = fDropSpeed <= 0.1f ? 0.1f : fDropSpeed;
	m_nDropIntervalTicks = (int)(m_fDropSpeed / (1.0f / 60.0f));
	if (m_nDropIntervalTicks < 1) m_nDropIntervalTicks = 1;
}
