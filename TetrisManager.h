#pragma once
#include "Singleton.h"
#include "NetworkManager.h"
#include "PlayerSession.h"

#include <memory>
#include <vector>
#include <string>

enum class ETetrisState
{
	Lobby,
	SingleGame,
	MultiGame,
	Pause,
	GameEnd,
	ReplaySelect,
	Replay,
	EnumEnd,
};

class CBoard;
class CTetromino;
class CLobby;
class CReplaySelect;

class CTetrisManager : public ISingleton<CTetrisManager>
{
public:
	CTetrisManager();
	virtual ~CTetrisManager() {};

public:
	void Initialize();
	void OnUpdate(float InDeltaSeconds);
	void OnClick(POINT InCoord);

	void MoveHorizontal(int InDir);
	void Rotate();
	void SoftDrop();
	void HardDrop();
	void Hold() {}

	int GetScore();
	int GetLevel();

	void StartSingleGame();
	void StartMultiHost();
	void StartMultiJoin(const std::string& InIP);
	void OpenReplaySelect();
	void StartReplay(const std::string& InFileName);
	void RestartGame();
	void ReturnToLobby();

	void TogglePauseReplay();
	void ChangeReplaySpeed(int InDir);

private:
	void LockStepTick();
	void SingleTick();
	void MultiTick();
	void ReplayTick();
	void ReplaySingleTick();
	void ReplayMultiTick();

	// 싱글플레이 전용 - 직접 멤버로 움직임 (TetrisManager.cpp)
	void CreateSingleObjects();
	void StartSingleDrop();
	void SingleSetNextBlock();
	void UpdateSingleGhost();
	void CheckSingleGameOver();
	void ApplySingleInput(EInputType InInput);
	void SimulateSingleDropTick();

	// 멀티플레이 전용 - 세션을 받아서 움직임 (TetrisManagerMulti.cpp)
	void StartSessionDrop(CPlayerSession& InSession);
	bool ApplySessionInput(CPlayerSession& InSession, EInputType InInput, ETetroBlockType InForcedNextType = ETetroBlockType::BLOCK_NONE);
	bool SimulateSessionDropTick(CPlayerSession& InSession, ETetroBlockType InForcedNextType = ETetroBlockType::BLOCK_NONE);
	void UpdateSessionGhost(CPlayerSession& InSession);
	void SessionSetNextBlock(CPlayerSession& InSession, ETetroBlockType InForcedType);

	void MultiGameEndTick();
	void RequestMultiRestart();
	void PerformMultiRestart();
	void HandleMultiDisconnect();

	void GoToLobby();
	void GoToReplaySelect();
	void EraseSingleObjects();
	void EraseMultiObjects();

	ETetroBlockType GetRandomPieceFromSeed(int InIndex) const;

private:
	std::shared_ptr<CLobby> m_pLobby;
	std::shared_ptr<CReplaySelect> m_pReplaySelect;

	// ---- 싱글플레이 데이터 ----
	std::shared_ptr<CBoard> m_pBoard;
	std::shared_ptr<CTetromino> m_CurrentTetromino;
	std::shared_ptr<CTetromino> m_NextTetromino;
	std::shared_ptr<CTetromino> m_GhostTetromino;

	int m_nCurBlockRow = 1;
	int m_nCurBlockCol = 3;
	int m_nDropTickCounter = 0;
	int m_nDropIntervalTicks = 1;
	float m_fDropSpeed = 2.0f;
	int m_nScore = 0;
	int m_nLevel = 1;
	bool m_bIsDropping = false;

	// ---- 멀티플레이 데이터 (MultiGame일 때만 생성) ----
	std::unique_ptr<CPlayerSession> m_pLocalSession;
	std::unique_ptr<CPlayerSession> m_pRemoteSession;

	std::vector<EInputType> m_PendingActions;
	bool m_bLockStepTickSent = false;
	STetrisPacket m_LastLocalPacket;
	bool m_bMultiSessionsReady = false;
	bool m_bIsHost = false;
	int m_nCurrentTick = 0;
	int m_nMatchSeed = 0;
	int m_nNextPieceIndex = 0;

	int m_nHostPieceIndex = 0;
	int m_nClientPieceIndex = 0;

	bool m_bWasMultiGame = false;
	bool m_bLocalRestartRequested = false;
	bool m_bRemoteRestartRequested = false;

	bool m_bReplayIsMulti = false;

	float m_fTickAccumulator = 0.0f;

	ETetrisState m_eCurrentState;

private:
	constexpr static int m_nStartBlockCol = 3;
	constexpr static int m_nStartBlockRow = 1;
	constexpr static int m_nBaseScorePerLine = 100;
	constexpr static float m_nBaseDropSpeed = 2.0f;
	constexpr static float m_nLevelSpeedRatio = 0.1f;
	constexpr static float m_fTickDuration = 1.0f / 60.0f;
};
