#pragma once
#include "Singleton.h"
#include "NetworkManager.h"

#include <unordered_map>
#include <vector>
#include <string>

class CReplayManager : public ISingleton<CReplayManager>
{
public:
	void StartRecording(int InSeed, int InPlayerCount);
	void RecordTick(int InPlayerIndex, int InTick, const STetrisPacket& InPacket);
	void StopRecording() { m_bIsRecording = false; }
	std::string SaveToFile();
	void SaveToFile(const std::string& InFileName);

	bool LoadFromFile(const std::string& InFileName);
	int GetLoadedSeed() const { return m_nLoadedSeed; }
	int GetLoadedPlayerCount() const { return m_nLoadedPlayerCount; }
	bool TryGetRecordedInput(int InPlayerIndex, int InTick, STetrisPacket& OutPacket) const;
	bool HasMoreTicks(int InTick) const { return InTick <= m_nLoadedMaxTick; }

	void TogglePause() { m_bPaused = !m_bPaused; }
	bool IsPaused() const { return m_bPaused; }

	void SetPlaybackSpeed(float InSpeed) { m_fPlaybackSpeed = InSpeed; }
	void SetNormalSpeed() { m_fPlaybackSpeed = 1.0f; }
	float GetPlaybackSpeed() const { return m_fPlaybackSpeed; }

	std::vector<std::string> ListReplayFiles() const;

private:
	std::string GenerateFileName() const;

private:
	// 기록용
	bool m_bIsRecording = false;
	int m_nRecordedSeed = 0;
	int m_nRecordedPlayerCount = 1;
	int m_nMaxRecordedTick = 0;
	std::unordered_map<int, STetrisPacket> m_RecordedInputs[2];

	// 재생용
	int m_nLoadedSeed = 0;
	int m_nLoadedPlayerCount = 1;
	int m_nLoadedMaxTick = 0;
	std::unordered_map<int, STetrisPacket> m_LoadedInputs[2];

	bool m_bPaused = false;
	float m_fPlaybackSpeed = 1.0f;
};
