#include "stdafx.h"
#include "TetrisManager.h"
#include "Tetromino.h"
#include "Board.h"
#include "Lobby.h"
#include "ReplayManager.h"

#include <random>

void CTetrisManager::StartMultiHost()
{
	Draw::EraseDrawable(m_pLobby);
	m_eCurrentState = ETetrisState::MultiGame;
	m_bMultiSessionsReady = false;
	m_bIsHost = true;
	m_nCurrentTick = 0;

	CNetworkManager::Get()->StartHost(5000);
}

void CTetrisManager::StartMultiJoin(const std::string& InIP)
{
	Draw::EraseDrawable(m_pLobby);
	m_eCurrentState = ETetrisState::MultiGame;
	m_bMultiSessionsReady = false;
	m_bIsHost = false;
	m_nCurrentTick = 0;

	CNetworkManager::Get()->ConnectToHost(InIP, 5000);
}

ETetroBlockType CTetrisManager::GetRandomPieceFromSeed(int InIndex) const
{
	std::mt19937 rng(m_nMatchSeed + InIndex);
	std::uniform_int_distribution<int> uit(
		static_cast<int>(ETetroBlockType::BLOCK_I),
		static_cast<int>(ETetroBlockType::BLOCK_L));
	return static_cast<ETetroBlockType>(uit(rng));
}

void CTetrisManager::MultiTick()
{
	if (!m_bMultiSessionsReady)
	{
		if (!CNetworkManager::Get()->IsConnected())
			return;

		if (m_bIsHost)
		{
			m_nMatchSeed = std::random_device{}();
			CNetworkManager::Get()->SendSeed(m_nMatchSeed);
		}
		else
		{
			if (!CNetworkManager::Get()->TryGetRemoteSeed(m_nMatchSeed))
				return;
		}

		m_nNextPieceIndex = 0;

		CReplayManager::Get()->StartRecording(m_nMatchSeed, 2);

		m_pLocalSession = std::make_unique<CPlayerSession>();
		m_pLocalSession->CreateObjects(EBoardPosition::Left,
			GetRandomPieceFromSeed(m_nNextPieceIndex++),
			GetRandomPieceFromSeed(m_nNextPieceIndex++));

		m_pRemoteSession = std::make_unique<CPlayerSession>();
		m_pRemoteSession->CreateObjects(EBoardPosition::Right,
			GetRandomPieceFromSeed(m_nNextPieceIndex++),
			GetRandomPieceFromSeed(m_nNextPieceIndex++));

		StartSessionDrop(*m_pLocalSession);
		StartSessionDrop(*m_pRemoteSession);

		m_bMultiSessionsReady = true;
	}

	if (!m_bLockStepTickSent)
	{
		m_LastLocalPacket = STetrisPacket{};
		m_LastLocalPacket.nTick = m_nCurrentTick;
		m_LastLocalPacket.nActionCount = (int)m_PendingActions.size();
		for (size_t i = 0; i < m_PendingActions.size() && i < 4; ++i)
			m_LastLocalPacket.Actions[i] = m_PendingActions[i];
		m_PendingActions.clear();

		CNetworkManager::Get()->SendInput(m_LastLocalPacket);
		m_bLockStepTickSent = true;
	}

	STetrisPacket remotePacket;

	if (!CNetworkManager::Get()->TryPopRemoteInput(remotePacket))
		return;

	CReplayManager::Get()->RecordTick(0, m_nCurrentTick, m_LastLocalPacket);
	CReplayManager::Get()->RecordTick(1, m_nCurrentTick, remotePacket);

	for (int i = 0; i < m_LastLocalPacket.nActionCount; ++i)
	{
		if (ApplySessionInput(*m_pLocalSession, m_LastLocalPacket.Actions[i], GetRandomPieceFromSeed(m_nNextPieceIndex)))
			++m_nNextPieceIndex;
	}

	for (int i = 0; i < remotePacket.nActionCount; ++i)
	{
		ApplySessionInput(*m_pRemoteSession, remotePacket.Actions[i], GetRandomPieceFromSeed(m_nNextPieceIndex));
	}

	if (SimulateSessionDropTick(*m_pLocalSession, GetRandomPieceFromSeed(m_nNextPieceIndex)))
		++m_nNextPieceIndex;

	SimulateSessionDropTick(*m_pRemoteSession, GetRandomPieceFromSeed(m_nNextPieceIndex));

	++m_nCurrentTick;
	m_bLockStepTickSent = false;

	if (m_pLocalSession->IsGameOver() || m_pRemoteSession->IsGameOver())
	{
		if (m_pLocalSession->IsGameOver() && !m_pRemoteSession->IsGameOver())
			m_pRemoteSession->SetWinner();
		else if (!m_pLocalSession->IsGameOver() && m_pRemoteSession->IsGameOver())
			m_pLocalSession->SetWinner();

		m_bWasMultiGame = true;
		m_bLocalRestartRequested = false;
		m_bRemoteRestartRequested = false;
		m_pLocalSession->GetBoard()->SetRestartVotes(0);
		m_pRemoteSession->GetBoard()->SetRestartVotes(0);

		m_eCurrentState = ETetrisState::GameEnd;

		CReplayManager::Get()->StopRecording();
		CReplayManager::Get()->SaveToFile();
	}
}

void CTetrisManager::ReplayMultiTick()
{
	if (!CReplayManager::Get()->HasMoreTicks(m_nCurrentTick))
	{
		GoToReplaySelect();
		return;
	}

	STetrisPacket localPacket, remotePacket;
	CReplayManager::Get()->TryGetRecordedInput(0, m_nCurrentTick, localPacket);
	CReplayManager::Get()->TryGetRecordedInput(1, m_nCurrentTick, remotePacket);

	for (int i = 0; i < localPacket.nActionCount; ++i)
	{
		if (ApplySessionInput(*m_pLocalSession, localPacket.Actions[i], GetRandomPieceFromSeed(m_nNextPieceIndex)))
			++m_nNextPieceIndex;
	}

	for (int i = 0; i < remotePacket.nActionCount; ++i)
	{
		ApplySessionInput(*m_pRemoteSession, remotePacket.Actions[i], GetRandomPieceFromSeed(m_nNextPieceIndex));
	}

	if (SimulateSessionDropTick(*m_pLocalSession, GetRandomPieceFromSeed(m_nNextPieceIndex)))
		++m_nNextPieceIndex;

	SimulateSessionDropTick(*m_pRemoteSession, GetRandomPieceFromSeed(m_nNextPieceIndex));

	++m_nCurrentTick;

	if (m_pLocalSession->IsGameOver() || m_pRemoteSession->IsGameOver())
		GoToReplaySelect();
}

void CTetrisManager::MultiGameEndTick()
{
	if (!m_bRemoteRestartRequested && CNetworkManager::Get()->IsRemoteRestartReceived())
		m_bRemoteRestartRequested = true;

	int nVotes = (m_bLocalRestartRequested ? 1 : 0) + (m_bRemoteRestartRequested ? 1 : 0);

	m_pLocalSession->GetBoard()->SetRestartVotes(nVotes);
	m_pRemoteSession->GetBoard()->SetRestartVotes(nVotes);

	if (m_bLocalRestartRequested && m_bRemoteRestartRequested)
		PerformMultiRestart();
}

void CTetrisManager::RequestMultiRestart()
{
	if (m_bLocalRestartRequested)
		return;

	m_bLocalRestartRequested = true;

	STetrisPacket restartPacket{};
	restartPacket.nTick = -1;
	CNetworkManager::Get()->SendInput(restartPacket);
}

void CTetrisManager::PerformMultiRestart()
{
	Draw::EraseDrawable(m_pLocalSession->GetBoard());
	Draw::EraseDrawable(m_pLocalSession->GetCurrentTetromino());
	Draw::EraseDrawable(m_pLocalSession->GetNextTetromino());
	Draw::EraseDrawable(m_pLocalSession->GetGhostTetromino());

	Draw::EraseDrawable(m_pRemoteSession->GetBoard());
	Draw::EraseDrawable(m_pRemoteSession->GetCurrentTetromino());
	Draw::EraseDrawable(m_pRemoteSession->GetNextTetromino());
	Draw::EraseDrawable(m_pRemoteSession->GetGhostTetromino());

	m_pLocalSession.reset();
	m_pRemoteSession.reset();

	CNetworkManager::Get()->ClearInputHistory();

	m_bMultiSessionsReady = false;
	m_bLocalRestartRequested = false;
	m_bRemoteRestartRequested = false;
	m_nCurrentTick = 0;
	m_bLockStepTickSent = false;
	m_nNextPieceIndex = 0;

	m_eCurrentState = ETetrisState::MultiGame;
}

void CTetrisManager::StartSessionDrop(CPlayerSession& InSession)
{
	InSession.SetDropping(true);
	InSession.SetCurCol(CPlayerSession::GetStartCol());
	InSession.SetCurRow(CPlayerSession::GetStartRow());
	InSession.SetDropTickCounter(0);
	InSession.ResetForNewGame();

	InSession.GetCurrentTetromino()->SetPosition(
		InSession.GetBoard()->GetCellOrigin(InSession.GetCurCol(), InSession.GetCurRow()));
}

bool CTetrisManager::ApplySessionInput(CPlayerSession& InSession, EInputType InInput, ETetroBlockType InForcedNextType)
{
	if (!InSession.IsDropping())
		return false;

	auto pBoard = InSession.GetBoard();
	auto& pCurrent = InSession.GetCurrentTetromino();

	switch (InInput)
	{
	case EInputType::MoveLeft:
	case EInputType::MoveRight:
	{
		int nDir = (InInput == EInputType::MoveLeft) ? -1 : 1;
		int nNextCol = InSession.GetCurCol() + nDir;
		if (!pBoard->IsCollision(pCurrent->GetShape(), nNextCol, InSession.GetCurRow()))
		{
			InSession.SetCurCol(nNextCol);
			pCurrent->SetPosition(pBoard->GetCellOrigin(nNextCol, InSession.GetCurRow()));
		}
		return false;
	}
	case EInputType::Rotate:
		pCurrent->Rotate(1);
		if (pBoard->IsCollision(pCurrent->GetShape(), InSession.GetCurCol(), InSession.GetCurRow()))
			pCurrent->Rotate(-1);
		return false;

	case EInputType::SoftDrop:
	{
		int nNextRow = InSession.GetCurRow() + 1;
		if (!pBoard->IsCollision(pCurrent->GetShape(), InSession.GetCurCol(), nNextRow))
		{
			InSession.SetCurRow(nNextRow);
			pCurrent->SetPosition(pBoard->GetCellOrigin(InSession.GetCurCol(), nNextRow));
			InSession.SetDropTickCounter(0);
		}
		return false;
	}
	case EInputType::HardDrop:
	{
		int nRow = InSession.GetCurRow();
		while (!pBoard->IsCollision(pCurrent->GetShape(), InSession.GetCurCol(), nRow + 1))
			++nRow;
		InSession.SetCurRow(nRow);

		int nCleared = pBoard->LockTetromino(pCurrent->GetShape(), InSession.GetCurCol(), nRow, pCurrent->GetBlockType());
		InSession.AddScore(nCleared);
		SessionSetNextBlock(InSession, InForcedNextType);
		InSession.SetDropTickCounter(0);
		return true;
	}
	default:
		return false;
	}
}

bool CTetrisManager::SimulateSessionDropTick(CPlayerSession& InSession, ETetroBlockType InForcedNextType)
{
	if (!InSession.IsDropping())
	{
		UpdateSessionGhost(InSession);
		return false;
	}

	InSession.SetDropTickCounter(InSession.GetDropTickCounter() + 1);
	bool bConsumedPiece = false;

	if (InSession.GetDropTickCounter() >= InSession.GetDropIntervalTicks())
	{
		InSession.SetDropTickCounter(0);

		auto pBoard = InSession.GetBoard();
		auto& pCurrent = InSession.GetCurrentTetromino();
		int nNextRow = InSession.GetCurRow() + 1;

		if (pBoard->IsCollision(pCurrent->GetShape(), InSession.GetCurCol(), nNextRow))
		{
			int nCleared = pBoard->LockTetromino(pCurrent->GetShape(), InSession.GetCurCol(), InSession.GetCurRow(), pCurrent->GetBlockType());
			InSession.AddScore(nCleared);
			SessionSetNextBlock(InSession, InForcedNextType);
			bConsumedPiece = true;
		}
		else
		{
			InSession.SetCurRow(nNextRow);
			pCurrent->SetPosition(pBoard->GetCellOrigin(InSession.GetCurCol(), nNextRow));
		}
	}

	UpdateSessionGhost(InSession);
	return bConsumedPiece;
}

void CTetrisManager::UpdateSessionGhost(CPlayerSession& InSession)
{
	if (!InSession.IsDropping())
		return;

	auto pBoard = InSession.GetBoard();
	auto& pCurrent = InSession.GetCurrentTetromino();
	auto pGhost = InSession.GetGhostTetromino();

	pGhost->SetBlockType(pCurrent->GetBlockType());
	pGhost->SetRotation(pCurrent->GetRotation());

	int nGhostRow = InSession.GetCurRow();
	while (!pBoard->IsCollision(pCurrent->GetShape(), InSession.GetCurCol(), nGhostRow + 1))
		++nGhostRow;

	pGhost->SetPosition(pBoard->GetCellOrigin(InSession.GetCurCol(), nGhostRow));
}

void CTetrisManager::SessionSetNextBlock(CPlayerSession& InSession, ETetroBlockType InForcedType)
{
	auto& pCurrent = InSession.GetCurrentTetromino();
	auto& pNext = InSession.GetNextTetromino();
	auto pBoard = InSession.GetBoard();

	std::swap(pCurrent, pNext);

	InSession.SetCurCol(CPlayerSession::GetStartCol());
	InSession.SetCurRow(CPlayerSession::GetStartRow());
	pCurrent->SetPosition(pBoard->GetCellOrigin(InSession.GetCurCol(), InSession.GetCurRow()));

	if (InForcedType == ETetroBlockType::BLOCK_NONE)
		pNext->SetRandomType();
	else
		pNext->SetBlockType(InForcedType);

	pNext->SetRotation(ERotationType::UP);

	POINT boxOrigin = pBoard->GetNextBoxOrigin();
	POINT offset = pNext->GetCenterOffset(5);
	pNext->SetPosition(POINT{ boxOrigin.x + offset.x, boxOrigin.y + offset.y });

	if (pBoard->IsCollision(pCurrent->GetShape(), InSession.GetCurCol(), InSession.GetCurRow()))
		InSession.SetGameOver();
}
