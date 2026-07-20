#include "stdafx.h"
#include "ReplayManager.h"

#include <fstream>
#include <filesystem>
#include <ctime>

namespace
{
	const std::string kReplayFolder = "Replay";
}

void CReplayManager::StartRecording(int InSeed, int InPlayerCount)
{
	m_bIsRecording = true;
	m_nRecordedSeed = InSeed;
	m_nRecordedPlayerCount = InPlayerCount;
	m_nMaxRecordedTick = 0;
	m_RecordedInputs[0].clear();
	m_RecordedInputs[1].clear();
}

void CReplayManager::RecordTick(int InPlayerIndex, int InTick, const STetrisPacket& InPacket)
{
	if (!m_bIsRecording)
		return;

	if (InPacket.nActionCount > 0)
		m_RecordedInputs[InPlayerIndex][InTick] = InPacket;

	if (InTick > m_nMaxRecordedTick)
		m_nMaxRecordedTick = InTick;
}

std::string CReplayManager::GenerateFileName() const
{
	time_t now = time(nullptr);
	tm ltm;
	localtime_s(&ltm, &now);

	char buf[64];
	sprintf_s(buf, "replay_%04d%02d%02d_%02d%02d%02d.rep",
		ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday,
		ltm.tm_hour, ltm.tm_min, ltm.tm_sec);

	return buf;
}

std::string CReplayManager::SaveToFile()
{
	std::string fileName = GenerateFileName();
	SaveToFile(fileName);
	return fileName;
}

void CReplayManager::SaveToFile(const std::string& InFileName)
{
	std::filesystem::create_directories(kReplayFolder);

	std::ofstream ofs(kReplayFolder + "/" + InFileName, std::ios::binary);
	if (!ofs.is_open())
		return;

	ofs.write((const char*)&m_nRecordedSeed, sizeof(m_nRecordedSeed));
	ofs.write((const char*)&m_nRecordedPlayerCount, sizeof(m_nRecordedPlayerCount));
	ofs.write((const char*)&m_nMaxRecordedTick, sizeof(m_nMaxRecordedTick));

	for (int p = 0; p < m_nRecordedPlayerCount; ++p)
	{
		int nCount = (int)m_RecordedInputs[p].size();
		ofs.write((const char*)&nCount, sizeof(nCount));

		for (const auto& pair : m_RecordedInputs[p])
			ofs.write((const char*)&pair.second, sizeof(STetrisPacket));
	}
}

bool CReplayManager::LoadFromFile(const std::string& InFileName)
{
	std::ifstream ifs(kReplayFolder + "/" + InFileName, std::ios::binary);
	if (!ifs.is_open())
		return false;

	m_LoadedInputs[0].clear();
	m_LoadedInputs[1].clear();

	ifs.read((char*)&m_nLoadedSeed, sizeof(m_nLoadedSeed));
	ifs.read((char*)&m_nLoadedPlayerCount, sizeof(m_nLoadedPlayerCount));
	ifs.read((char*)&m_nLoadedMaxTick, sizeof(m_nLoadedMaxTick));

	for (int p = 0; p < m_nLoadedPlayerCount; ++p)
	{
		int nCount = 0;
		ifs.read((char*)&nCount, sizeof(nCount));

		for (int i = 0; i < nCount; ++i)
		{
			STetrisPacket packet{};
			ifs.read((char*)&packet, sizeof(STetrisPacket));
			m_LoadedInputs[p][packet.nTick] = packet;
		}
	}

	m_bPaused = false;
	m_fPlaybackSpeed = 1.0f;

	return true;
}

bool CReplayManager::TryGetRecordedInput(int InPlayerIndex, int InTick, STetrisPacket& OutPacket) const
{
	auto it = m_LoadedInputs[InPlayerIndex].find(InTick);
	if (it == m_LoadedInputs[InPlayerIndex].end())
		return false;

	OutPacket = it->second;
	return true;
}

std::vector<std::string> CReplayManager::ListReplayFiles() const
{
	std::vector<std::string> files;

	if (!std::filesystem::exists(kReplayFolder))
		return files;

	for (const auto& entry : std::filesystem::directory_iterator(kReplayFolder))
	{
		if (entry.path().extension() == ".rep")
			files.push_back(entry.path().filename().string());
	}

	return files;
}