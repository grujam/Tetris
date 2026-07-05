#include "stdafx.h"
#include "TetrisManager.h"
#include "Tetromino.h"
#include "Board.h"

CTetrisManager::CTetrisManager()
	: m_nLevel(1)
	, m_nScore(0)
	, m_fDropTimer(0.0f)
	, m_fDropSpeed(2.0f)
	, m_bIsDropping(false)
	, m_nCurBlockCol(m_nStartBlockCol)
	, m_nCurBlockRow(m_nStartBlockRow)
{
}

void CTetrisManager::Initialize()
{
	CreateObjects();
	StartDrop();
}

void CTetrisManager::OnUpdate(float InDeltaSeconds)
{
	if (m_bIsHold)
		return;

	m_fDropTimer += InDeltaSeconds;

	if (m_bIsDropping)
	{
		if (m_fDropTimer >= m_fDropSpeed)
		{
			m_fDropTimer -= m_fDropSpeed;

			int nNextRow = m_nCurBlockRow + 1;

			if (m_pBoard->IsCollision(m_CurrentTetromino->GetShape(), m_nCurBlockCol, nNextRow))
			{
				m_pBoard->LockTetromino(m_CurrentTetromino->GetShape(), m_nCurBlockCol, m_nCurBlockRow, m_CurrentTetromino->GetBlockType());
				SetNextBlock();
			}
			else
			{
				m_nCurBlockRow = nNextRow;
				m_CurrentTetromino->SetPosition(m_pBoard->GetCellOrigin(m_nCurBlockCol, m_nCurBlockRow));
			}
		}
	}

	UpdateGhost();
}

void CTetrisManager::MoveHorizontal(int InDir)
{
	if (!m_bIsDropping || m_bIsHold)
		return;

	int nNextCol = m_nCurBlockCol + InDir;

	if (m_pBoard->IsCollision(m_CurrentTetromino->GetShape(), nNextCol, m_nCurBlockRow))
		return;

	m_nCurBlockCol = nNextCol;
	m_CurrentTetromino->SetPosition(m_pBoard->GetCellOrigin(m_nCurBlockCol, m_nCurBlockRow));
}

void CTetrisManager::Rotate()
{
	if (!m_bIsDropping || m_bIsHold)
		return;

	m_CurrentTetromino->Rotate(1);

	if (m_pBoard->IsCollision(m_CurrentTetromino->GetShape(), m_nCurBlockCol, m_nCurBlockRow))
	{
		m_CurrentTetromino->Rotate(-1);
	}
}

void CTetrisManager::SoftDrop()
{
	if (!m_bIsDropping || m_bIsHold)
		return;

	int nNextRow = m_nCurBlockRow + 1;

	if (m_pBoard->IsCollision(m_CurrentTetromino->GetShape(), m_nCurBlockCol, nNextRow))
		return; 

	m_nCurBlockRow = nNextRow;
	m_CurrentTetromino->SetPosition(m_pBoard->GetCellOrigin(m_nCurBlockCol, m_nCurBlockRow));
	m_fDropTimer = 0.0f; 
}

void CTetrisManager::HardDrop()
{
	if (!m_bIsDropping || m_bIsHold)
		return;

	while (!m_pBoard->IsCollision(m_CurrentTetromino->GetShape(), m_nCurBlockCol, m_nCurBlockRow + 1))
	{
		++m_nCurBlockRow;
	}

	m_pBoard->LockTetromino(m_CurrentTetromino->GetShape(), m_nCurBlockCol, m_nCurBlockRow, m_CurrentTetromino->GetBlockType());
	SetNextBlock();

	m_fDropTimer = 0.0f; 
}

void CTetrisManager::AddScore(int InClearedLines)
{
	m_nScore += InClearedLines * m_nBaseScorePerLine * m_nLevel;

	CheckIncreaseLevel();
}

void CTetrisManager::Hold()
{
	m_bIsHold = !m_bIsHold;
}

int CTetrisManager::GetScore()
{
	return m_nScore;
}

int CTetrisManager::GetLevel()
{
	return m_nLevel;
}

void CTetrisManager::CreateObjects()
{
	POINT cellOrigin = m_pBoard->GetCellOrigin(m_nStartBlockCol, m_nStartBlockRow);
	POINT ghostOrigin = m_pBoard->GetCellOrigin(m_nStartBlockCol, 20);

	m_pBoard = Draw::CreateDrawable<CBoard>();
	m_CurrentTetromino = Draw::CreateDrawable<CTetromino>(cellOrigin, false);
	m_GhostTetromino = Draw::CreateDrawable<CTetromino>(ghostOrigin, true);

	POINT boxOrigin = m_pBoard->GetNextBoxOrigin();
	m_NextTetromino = Draw::CreateDrawable<CTetromino>(boxOrigin, false);
	POINT offset = m_NextTetromino->GetCenterOffset(5);
	m_NextTetromino->SetPosition(POINT{ boxOrigin.x + offset.x, boxOrigin.y + offset.y });

}

void CTetrisManager::StartDrop()
{
	m_bIsDropping = true;
	m_nCurBlockCol = m_nStartBlockCol;
	m_nCurBlockRow = m_nStartBlockRow;
	m_fDropTimer = 0.0f;

	m_CurrentTetromino->SetPosition(m_pBoard->GetCellOrigin(m_nCurBlockCol, m_nCurBlockRow));
}

void CTetrisManager::SetNextBlock()
{
	std::swap(m_CurrentTetromino, m_NextTetromino);

	m_nCurBlockCol = m_nStartBlockCol;
	m_nCurBlockRow = m_nStartBlockRow;
	m_CurrentTetromino->SetPosition(m_pBoard->GetCellOrigin(m_nCurBlockCol, m_nCurBlockRow));

	m_NextTetromino->SetRandomType();
	m_NextTetromino->SetRotation(ERotationType::UP);

	POINT boxOrigin = m_pBoard->GetNextBoxOrigin();
	POINT offset = m_NextTetromino->GetCenterOffset(5);
	m_NextTetromino->SetPosition(POINT{ boxOrigin.x + offset.x, boxOrigin.y + offset.y });

	CheckGameOver();
}

void CTetrisManager::UpdateGhost()
{
	if (!m_bIsDropping)
		return;

	m_GhostTetromino->SetBlockType(m_CurrentTetromino->GetBlockType());
	m_GhostTetromino->SetRotation(m_CurrentTetromino->GetRotation());

	int nGhostRow = m_nCurBlockRow;
	while (!m_pBoard->IsCollision(m_CurrentTetromino->GetShape(), m_nCurBlockCol, nGhostRow + 1))
	{
		++nGhostRow;
	}

	m_GhostTetromino->SetPosition(m_pBoard->GetCellOrigin(m_nCurBlockCol, nGhostRow));
}

void CTetrisManager::CheckGameOver()
{
	if (!m_pBoard->IsCollision(m_CurrentTetromino->GetShape(), m_nCurBlockCol, m_nCurBlockRow))
		return;

	m_bIsDropping = false;
	m_bIsGameOver = true;

	if (m_pBoard)
		m_pBoard->SetGameOver();
}

void CTetrisManager::CheckIncreaseLevel()
{
	m_nLevel = 1 + m_nScore / 1000;

	float fDropSpeed = m_nBaseDropSpeed - m_nLevel * m_nLevelSpeedRatio;
	m_fDropSpeed = fDropSpeed <= 0.1f ? 0.1f : fDropSpeed;
}