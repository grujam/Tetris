#pragma once
#include "Singleton.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <string>

#pragma comment(lib, "ws2_32.lib")

enum class EInputType : int
{
	None,
	MoveLeft,
	MoveRight,
	Rotate,
	SoftDrop,
	HardDrop
};

struct STetrisPacket
{
	int nTick = 0;
	int nActionCount = 0;
	EInputType Actions[4]{};
};

enum class ENetworkState
{
	Idle,
	WaitingForClient,
	Connecting,
	Connected,
	Failed
};

class CNetworkManager : public ISingleton<CNetworkManager>
{
public:
	void Initialize();
	void Shutdown();

	bool StartHost(unsigned short InPort);
	bool ConnectToHost(const std::string& InIP, unsigned short InPort);

	void Update();

	bool IsConnected() const { return m_eState == ENetworkState::Connected; }
	bool IsFailed() const { return m_eState == ENetworkState::Failed; }

	void SendInput(const STetrisPacket& InPacket);
	bool TryPopRemoteInput(STetrisPacket& OutPacket);

	bool IsRemoteRestartReceived() const { return m_bRemoteRestartReceived; }

	void SendSeed(int InSeed);
	bool TryGetRemoteSeed(int& OutSeed) const;

	void Disconnect();
	void ShutDown();
	void ClearInputHistory();

private:
	void SetNonBlocking(SOCKET InSocket);
	void PollAccept();
	void PollConnect();
	void PollRecv();

private:
	ENetworkState m_eState = ENetworkState::Idle;
	bool m_bIsHost = false;

	SOCKET m_ListenSocket = INVALID_SOCKET;
	SOCKET m_ConnectedSocket = INVALID_SOCKET;

	std::vector<char> m_RecvBuffer;
	bool m_bSeedReceived = false;
	int m_nRemoteSeed = 0;

	std::queue<STetrisPacket> m_RemoteInputQueue;
	bool m_bRemoteRestartReceived = false;
};