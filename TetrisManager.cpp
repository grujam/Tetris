#include "stdafx.h"
#include "TetrisManager.h"
#include "Tetromino.h"
#include "Board.h"
#include "Lobby.h"
#include "ReplaySelect.h"
#include "ReplayManager.h"

#include <random>
#include <cmath>

CTetrisManager::CTetrisManager()
	: m_nCurBlockCol(m_nStartBlockCol)
	, m_nCurBlockRow(m_nStartBlockRow)
	, m_eCurrentState(ETetrisState::Lobby)
{
}

void CTetrisManager::Initialize()
{
	CNetworkManager::Get()->Initialize();

	m_eCurrentState = ETetrisState::Lobby;
	m_pLobby = Draw::CreateDrawable<CLobby>();
}

void CTetrisManager::OnUpdate(float InDeltaSeconds)
{
	CNetworkManager::Get()->Update();

	float fDeltaSeconds = InDeltaSeconds;

	if (m_eCurrentState == ETetrisState::Replay)
	{
		bool bPaused = CReplayManager::Get()->IsPaused();
		float fSpeed = CReplayManager::Get()->GetPlaybackSpeed();

		if (m_pBoard)
			m_pBoard->SetReplayInfo(bPaused, fSpeed);
		if (m_pLocalSession)
			m_pLocalSession->GetBoard()->SetReplayInfo(bPaused, fSpeed);
		if (m_pRemoteSession)
			m_pRemoteSession->GetBoard()->SetReplayInfo(bPaused, fSpeed);

		if (bPaused)
			return;

		fDeltaSeconds *= fSpeed;
	}

	m_fTickAccumulator += fDeltaSeconds;

	while (m_fTickAccumulator >= m_fTickDuration)
	{
		LockStepTick();
		m_fTickAccumulator -= m_fTickDuration;
	}
}

void CTetrisManager::LockStepTick()
{
	bool bMultiActive = (m_eCurrentState == ETetrisState::MultiGame) || (m_eCurrentState == ETetrisState::GameEnd && m_bWasMultiGame);

	if (bMultiActive && CNetworkManager::Get()->IsFailed())
	{
		HandleMultiDisconnect();
		return;
	}

	if (m_eCurrentState == ETetrisState::MultiGame)
		MultiTick();
	else if (m_eCurrentState == ETetrisState::SingleGame)
		SingleTick();
	else if (m_eCurrentState == ETetrisState::GameEnd && m_bWasMultiGame)
		MultiGameEndTick();
	else if (m_eCurrentState == ETetrisState::Replay)
		ReplayTick();
}

void CTetrisManager::OnClick(POINT InCoord)
{
	if (m_eCurrentState == ETetrisState::Lobby)
		m_pLobby->OnClick(InCoord);
	else if (m_eCurrentState == ETetrisState::ReplaySelect)
		m_pReplaySelect->OnClick(InCoord);
}

void CTetrisManager::SingleTick()
{
	STetrisPacket packet{};
	packet.nTick = m_nCurrentTick;
	packet.nActionCount = (int)m_PendingActions.size();
	for (size_t i = 0; i < m_PendingActions.size() && i < 4; ++i)
		packet.Actions[i] = m_PendingActions[i];
	m_PendingActions.clear();

	CReplayManager::Get()->RecordTick(0, m_nCurrentTick, packet);

	for (int i = 0; i < packet.nActionCount; ++i)
		ApplySingleInput(packet.Actions[i]);

	SimulateSingleDropTick();

	++m_nCurrentTick;
}

void CTetrisManager::ReplayTick()
{
	if (m_bReplayIsMulti)
		ReplayMultiTick();
	else
		ReplaySingleTick();
}

void CTetrisManager::ReplaySingleTick()
{
	if (!CReplayManager::Get()->HasMoreTicks(m_nCurrentTick))
	{
		GoToReplaySelect();
		return;
	}

	STetrisPacket packet;
	if (CReplayManager::Get()->TryGetRecordedInput(0, m_nCurrentTick, packet))
	{
		for (int i = 0; i < packet.nActionCount; ++i)
			ApplySingleInput(packet.Actions[i]);
	}

	SimulateSingleDropTick();

	++m_nCurrentTick;
}

void CTetrisManager::SimulateSingleDropTick()
{
	if (!m_bIsDropping)
	{
		UpdateSingleGhost();
		return;
	}

	++m_nDropTickCounter;

	if (m_nDropTickCounter >= m_nDropIntervalTicks)
	{
		m_nDropTickCounter = 0;
		int nNextRow = m_nCurBlockRow + 1;

		if (m_pBoard->IsCollision(m_CurrentTetromino->GetShape(), m_nCurBlockCol, nNextRow))
		{
			int nCleared = m_pBoard->LockTetromino(m_CurrentTetromino->GetShape(), m_nCurBlockCol, m_nCurBlockRow, m_CurrentTetromino->GetBlockType());

			m_nScore += nCleared * m_nBaseScorePerLine * m_nLevel;
			m_nLevel = 1 + m_nScore / 1000;
			float fDropSpeed = m_nBaseDropSpeed - m_nLevel * m_nLevelSpeedRatio;
			m_fDropSpeed = fDropSpeed <= 0.1f ? 0.1f : fDropSpeed;
			m_nDropIntervalTicks = (int)(m_fDropSpeed / m_fTickDuration);
			if (m_nDropIntervalTicks < 1) m_nDropIntervalTicks = 1;
			m_pBoard->SetScore(m_nScore);
			m_pBoard->SetLevel(m_nLevel);

			SingleSetNextBlock();
		}
		else
		{
			m_nCurBlockRow = nNextRow;
			m_CurrentTetromino->SetPosition(m_pBoard->GetCellOrigin(m_nCurBlockCol, m_nCurBlockRow));
		}
	}

	UpdateSingleGhost();
}

void CTetrisManager::MoveHorizontal(int InDir)
{
	if (m_eCurrentState != ETetrisState::MultiGame && m_eCurrentState != ETetrisState::SingleGame)
		return;

	m_PendingActions.push_back(InDir < 0 ? EInputType::MoveLeft : EInputType::MoveRight);
}

void CTetrisManager::Rotate()
{
	if (m_eCurrentState != ETetrisState::MultiGame && m_eCurrentState != ETetrisState::SingleGame)
		return;

	m_PendingActions.push_back(EInputType::Rotate);
}

void CTetrisManager::SoftDrop()
{
	if (m_eCurrentState != ETetrisState::MultiGame && m_eCurrentState != ETetrisState::SingleGame)
		return;

	m_PendingActions.push_back(EInputType::SoftDrop);
}

void CTetrisManager::HardDrop()
{
	if (m_eCurrentState != ETetrisState::MultiGame && m_eCurrentState != ETetrisState::SingleGame)
		return;

	m_PendingActions.push_back(EInputType::HardDrop);
}

void CTetrisManager::ApplySingleInput(EInputType InInput)
{
	if (!m_bIsDropping)
		return;

	switch (InInput)
	{
	case EInputType::MoveLeft:
	case EInputType::MoveRight:
	{
		int nDir = (InInput == EInputType::MoveLeft) ? -1 : 1;
		int nNextCol = m_nCurBlockCol + nDir;
		if (!m_pBoard->IsCollision(m_CurrentTetromino->GetShape(), nNextCol, m_nCurBlockRow))
		{
			m_nCurBlockCol = nNextCol;
			m_CurrentTetromino->SetPosition(m_pBoard->GetCellOrigin(m_nCurBlockCol, m_nCurBlockRow));
		}
		break;
	}
	case EInputType::Rotate:
		m_CurrentTetromino->Rotate(1);
		if (m_pBoard->IsCollision(m_CurrentTetromino->GetShape(), m_nCurBlockCol, m_nCurBlockRow))
			m_CurrentTetromino->Rotate(-1);
		break;
	case EInputType::SoftDrop:
	{
		int nNextRow = m_nCurBlockRow + 1;
		if (!m_pBoard->IsCollision(m_CurrentTetromino->GetShape(), m_nCurBlockCol, nNextRow))
		{
			m_nCurBlockRow = nNextRow;
			m_CurrentTetromino->SetPosition(m_pBoard->GetCellOrigin(m_nCurBlockCol, m_nCurBlockRow));
			m_nDropTickCounter = 0;
		}
		break;
	}
	case EInputType::HardDrop:
	{
		while (!m_pBoard->IsCollision(m_CurrentTetromino->GetShape(), m_nCurBlockCol, m_nCurBlockRow + 1))
			++m_nCurBlockRow;

		int nCleared = m_pBoard->LockTetromino(m_CurrentTetromino->GetShape(), m_nCurBlockCol, m_nCurBlockRow, m_CurrentTetromino->GetBlockType());

		m_nScore += nCleared * m_nBaseScorePerLine * m_nLevel;
		m_nLevel = 1 + m_nScore / 1000;
		float fDropSpeed = m_nBaseDropSpeed - m_nLevel * m_nLevelSpeedRatio;
		m_fDropSpeed = fDropSpeed <= 0.1f ? 0.1f : fDropSpeed;
		m_nDropIntervalTicks = (int)(m_fDropSpeed / m_fTickDuration);
		if (m_nDropIntervalTicks < 1) m_nDropIntervalTicks = 1;
		m_pBoard->SetScore(m_nScore);
		m_pBoard->SetLevel(m_nLevel);

		SingleSetNextBlock();
		m_nDropTickCounter = 0;
		break;
	}
	default:
		break;
	}
}

void CTetrisManager::UpdateSingleGhost()
{
	if (!m_bIsDropping)
		return;

	m_GhostTetromino->SetBlockType(m_CurrentTetromino->GetBlockType());
	m_GhostTetromino->SetRotation(m_CurrentTetromino->GetRotation());

	int nGhostRow = m_nCurBlockRow;
	while (!m_pBoard->IsCollision(m_CurrentTetromino->GetShape(), m_nCurBlockCol, nGhostRow + 1))
		++nGhostRow;

	m_GhostTetromino->SetPosition(m_pBoard->GetCellOrigin(m_nCurBlockCol, nGhostRow));
}

void CTetrisManager::SingleSetNextBlock()
{
	std::swap(m_CurrentTetromino, m_NextTetromino);

	m_nCurBlockCol = m_nStartBlockCol;
	m_nCurBlockRow = m_nStartBlockRow;
	m_CurrentTetromino->SetPosition(m_pBoard->GetCellOrigin(m_nCurBlockCol, m_nCurBlockRow));

	m_NextTetromino->SetBlockType(GetRandomPieceFromSeed(m_nNextPieceIndex++));
	m_NextTetromino->SetRotation(ERotationType::UP);

	POINT boxOrigin = m_pBoard->GetNextBoxOrigin();
	POINT offset = m_NextTetromino->GetCenterOffset(5);
	m_NextTetromino->SetPosition(POINT{ boxOrigin.x + offset.x, boxOrigin.y + offset.y });

	CheckSingleGameOver();
}

void CTetrisManager::CheckSingleGameOver()
{
	if (!m_pBoard->IsCollision(m_CurrentTetromino->GetShape(), m_nCurBlockCol, m_nCurBlockRow))
		return;

	if (m_eCurrentState == ETetrisState::Replay)
	{
		m_bIsDropping = false;
		GoToReplaySelect();
		return;
	}

	m_bIsDropping = false;
	m_eCurrentState = ETetrisState::GameEnd;
	m_bWasMultiGame = false;
	m_pBoard->SetGameOver();

	CReplayManager::Get()->StopRecording();
	CReplayManager::Get()->SaveToFile();
}

void CTetrisManager::CreateSingleObjects()
{
	m_pBoard = Draw::CreateDrawable<CBoard>(EBoardPosition::Center);

	POINT cellOrigin = m_pBoard->GetCellOrigin(m_nStartBlockCol, m_nStartBlockRow);
	POINT ghostOrigin = m_pBoard->GetCellOrigin(m_nStartBlockCol, 20);

	m_CurrentTetromino = Draw::CreateDrawable<CTetromino>(cellOrigin, false);
	m_CurrentTetromino->SetBlockType(GetRandomPieceFromSeed(m_nNextPieceIndex++));

	m_GhostTetromino = Draw::CreateDrawable<CTetromino>(ghostOrigin, true);

	POINT boxOrigin = m_pBoard->GetNextBoxOrigin();
	m_NextTetromino = Draw::CreateDrawable<CTetromino>(boxOrigin, false);
	m_NextTetromino->SetBlockType(GetRandomPieceFromSeed(m_nNextPieceIndex++));
	POINT offset = m_NextTetromino->GetCenterOffset(5);
	m_NextTetromino->SetPosition(POINT{ boxOrigin.x + offset.x, boxOrigin.y + offset.y });
}

void CTetrisManager::StartSingleDrop()
{
	m_bIsDropping = true;
	m_nCurBlockCol = m_nStartBlockCol;
	m_nCurBlockRow = m_nStartBlockRow;
	m_nDropTickCounter = 0;
	m_nScore = 0;
	m_nLevel = 1;
	m_fDropSpeed = m_nBaseDropSpeed;
	m_nDropIntervalTicks = (int)(m_fDropSpeed / m_fTickDuration);

	m_CurrentTetromino->SetPosition(m_pBoard->GetCellOrigin(m_nCurBlockCol, m_nCurBlockRow));
}

int CTetrisManager::GetScore() { return m_nScore; }
int CTetrisManager::GetLevel() { return m_nLevel; }

void CTetrisManager::StartSingleGame()
{
	Draw::EraseDrawable(m_pLobby);
	m_eCurrentState = ETetrisState::SingleGame;

	m_nMatchSeed = std::random_device{}();
	m_nNextPieceIndex = 0;
	m_nCurrentTick = 0;
	m_PendingActions.clear();

	CReplayManager::Get()->StartRecording(m_nMatchSeed, 1);

	CreateSingleObjects();
	StartSingleDrop();
}

void CTetrisManager::RestartGame()
{
	if (m_eCurrentState != ETetrisState::GameEnd)
		return;

	if (m_bWasMultiGame)
	{
		RequestMultiRestart();
		return;
	}

	Draw::EraseDrawable(m_pBoard);
	Draw::EraseDrawable(m_CurrentTetromino);
	Draw::EraseDrawable(m_NextTetromino);
	Draw::EraseDrawable(m_GhostTetromino);

	m_eCurrentState = ETetrisState::SingleGame;

	m_nMatchSeed = std::random_device{}();
	m_nNextPieceIndex = 0;
	m_nCurrentTick = 0;
	m_PendingActions.clear();

	CReplayManager::Get()->StartRecording(m_nMatchSeed, 1);

	CreateSingleObjects();
	StartSingleDrop();
}

void CTetrisManager::OpenReplaySelect()
{
	Draw::EraseDrawable(m_pLobby);
	m_eCurrentState = ETetrisState::ReplaySelect;

	m_pReplaySelect = Draw::CreateDrawable<CReplaySelect>();
	m_pReplaySelect->RefreshFileList();
}

void CTetrisManager::StartReplay(const std::string& InFileName)
{
	if (!CReplayManager::Get()->LoadFromFile(InFileName))
		return;

	if (m_pReplaySelect)
	{
		Draw::EraseDrawable(m_pReplaySelect);
		m_pReplaySelect.reset();
	}

	m_eCurrentState = ETetrisState::Replay;

	m_nMatchSeed = CReplayManager::Get()->GetLoadedSeed();
	m_nNextPieceIndex = m_nMatchSeed;
	m_nCurrentTick = 0;
	m_bReplayIsMulti = (CReplayManager::Get()->GetLoadedPlayerCount() >= 2);

	if (m_bReplayIsMulti)
	{
		m_pLocalSession = std::make_unique<CPlayerSession>();
		m_pLocalSession->CreateObjects(EBoardPosition::Left,
			GetRandomPieceFromSeed(m_nNextPieceIndex),
			GetRandomPieceFromSeed(m_nNextPieceIndex));

		m_pRemoteSession = std::make_unique<CPlayerSession>();
		m_pRemoteSession->CreateObjects(EBoardPosition::Right,
			GetRandomPieceFromSeed(m_nNextPieceIndex),
			GetRandomPieceFromSeed(m_nNextPieceIndex));

		StartSessionDrop(*m_pLocalSession);
		StartSessionDrop(*m_pRemoteSession);
	}
	else
	{
		CreateSingleObjects();
		StartSingleDrop();
	}
}

void CTetrisManager::TogglePauseReplay()
{
	if (m_eCurrentState != ETetrisState::Replay)
		return;

	CReplayManager::Get()->TogglePause();
}

void CTetrisManager::ChangeReplaySpeed(int InDir)
{
	if (m_eCurrentState != ETetrisState::Replay)
		return;

	static const float speeds[] = { 0.5f, 1.0f, 2.0f, 4.0f, 8.0f };
	constexpr int nCount = sizeof(speeds) / sizeof(speeds[0]);

	float fCurrent = CReplayManager::Get()->GetPlaybackSpeed();
	int nIndex = 0;
	for (int i = 0; i < nCount; ++i)
	{
		if (fabsf(speeds[i] - fCurrent) < 0.001f)
		{
			nIndex = i;
			break;
		}
	}

	nIndex += InDir;
	if (nIndex < 0) nIndex = 0;
	if (nIndex >= nCount) nIndex = nCount - 1;

	CReplayManager::Get()->SetPlaybackSpeed(speeds[nIndex]);
}

void CTetrisManager::ReturnToLobby()
{
	if (m_eCurrentState != ETetrisState::GameEnd && m_eCurrentState != ETetrisState::Replay
		&& m_eCurrentState != ETetrisState::ReplaySelect)
		return;

	GoToLobby();
}

void CTetrisManager::EraseSingleObjects()
{
	if (!m_pBoard)
		return;

	Draw::EraseDrawable(m_pBoard);
	Draw::EraseDrawable(m_CurrentTetromino);
	Draw::EraseDrawable(m_NextTetromino);
	Draw::EraseDrawable(m_GhostTetromino);
	m_pBoard.reset();
	m_CurrentTetromino.reset();
	m_NextTetromino.reset();
	m_GhostTetromino.reset();
}

void CTetrisManager::EraseMultiObjects()
{
	if (m_pLocalSession)
	{
		Draw::EraseDrawable(m_pLocalSession->GetBoard());
		Draw::EraseDrawable(m_pLocalSession->GetCurrentTetromino());
		Draw::EraseDrawable(m_pLocalSession->GetNextTetromino());
		Draw::EraseDrawable(m_pLocalSession->GetGhostTetromino());
	}
	if (m_pRemoteSession)
	{
		Draw::EraseDrawable(m_pRemoteSession->GetBoard());
		Draw::EraseDrawable(m_pRemoteSession->GetCurrentTetromino());
		Draw::EraseDrawable(m_pRemoteSession->GetNextTetromino());
		Draw::EraseDrawable(m_pRemoteSession->GetGhostTetromino());
	}
	m_pLocalSession.reset();
	m_pRemoteSession.reset();
}

void CTetrisManager::GoToLobby()
{
	if (m_pReplaySelect)
	{
		Draw::EraseDrawable(m_pReplaySelect);
		m_pReplaySelect.reset();
	}

	EraseSingleObjects();
	EraseMultiObjects();

	CNetworkManager::Get()->Disconnect();

	m_bWasMultiGame = false;
	m_bMultiSessionsReady = false;
	m_bLocalRestartRequested = false;
	m_bRemoteRestartRequested = false;

	m_eCurrentState = ETetrisState::Lobby;
	m_pLobby = Draw::CreateDrawable<CLobby>();
}

void CTetrisManager::GoToReplaySelect()
{
	EraseSingleObjects();
	EraseMultiObjects();

	m_eCurrentState = ETetrisState::ReplaySelect;
	m_pReplaySelect = Draw::CreateDrawable<CReplaySelect>();
	m_pReplaySelect->RefreshFileList();
}

void CTetrisManager::HandleMultiDisconnect()
{
	MessageBoxW(GET_HWND(), L"상대방과의 연결이 끊어졌습니다.", L"Tetris", MB_OK);

	GoToLobby();
}
