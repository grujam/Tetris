#pragma once
#include "Draw.h"

const POINT g_TetrominoTable[8][4][4] =
{
	// BLOCK_NONE
	{
		{ {0,0},{0,0},{0,0},{0,0} }, // UP
		{ {0,0},{0,0},{0,0},{0,0} }, // RIGHT
		{ {0,0},{0,0},{0,0},{0,0} }, // DOWN
		{ {0,0},{0,0},{0,0},{0,0} }, // LEFT
	},
	// BLOCK_I
	{
		{ {0,1},{1,1},{2,1},{3,1} }, // UP
		{ {0,0},{0,1},{0,2},{0,3} }, // RIGHT
		{ {0,0},{1,0},{2,0},{3,0} }, // DOWN
		{ {1,0},{1,1},{1,2},{1,3} }, // LEFT
	},
	// BLOCK_O
	{
		{ {0,0},{1,0},{0,1},{1,1} }, // UP
		{ {0,0},{1,0},{0,1},{1,1} }, // RIGHT
		{ {0,0},{1,0},{0,1},{1,1} }, // DOWN
		{ {0,0},{1,0},{0,1},{1,1} }, // LEFT
	},
	// BLOCK_T
	{
		{ {0,0},{1,0},{2,0},{1,1} }, // UP
		{ {1,0},{1,1},{1,2},{0,1} }, // RIGHT
		{ {0,1},{1,1},{2,1},{1,0} }, // DOWN
		{ {0,0},{0,1},{0,2},{1,1} }, // LEFT
	},
	// BLOCK_S
	{
		{ {1,0},{2,0},{0,1},{1,1} }, // UP
		{ {0,0},{0,1},{1,1},{1,2} }, // RIGHT
		{ {1,0},{2,0},{0,1},{1,1} }, // DOWN
		{ {0,0},{0,1},{1,1},{1,2} }, // LEFT
	},
	// BLOCK_Z
	{
		{ {0,0},{1,0},{1,1},{2,1} }, // UP
		{ {1,0},{1,1},{0,1},{0,2} }, // RIGHT
		{ {0,0},{1,0},{1,1},{2,1} }, // DOWN
		{ {1,0},{1,1},{0,1},{0,2} }, // LEFT
	},
	// BLOCK_J
	{
		{ {0,0},{0,1},{1,1},{2,1} }, // UP
		{ {1,0},{0,0},{0,1},{0,2} }, // RIGHT
		{ {2,1},{2,0},{1,0},{0,0} }, // DOWN
		{ {0,2},{1,2},{1,1},{1,0} }, // LEFT
	},
	// BLOCK_L
	{
		{ {2,0},{0,1},{1,1},{2,1} }, // UP
		{ {1,2},{0,0},{0,1},{0,2} }, // RIGHT
		{ {0,1},{2,0},{1,0},{0,0} }, // DOWN
		{ {0,0},{1,2},{1,1},{1,0} }, // LEFT
	},
};

enum class ETetroBlockType
{
	BLOCK_NONE,
	BLOCK_I,
	BLOCK_O,
	BLOCK_T,
	BLOCK_S,
	BLOCK_Z,
	BLOCK_J,
	BLOCK_L
};

enum class ERotationType
{
	UP = 0,
	RIGHT,
	DOWN,
	LEFT,
	TYPE_END
};

class CTetromino : public IDrawable
{
public:
	static COLORREF GetBlockColor(ETetroBlockType InType);

public:
	CTetromino(POINT InPT, bool InIsGhost);
	virtual ~CTetromino() {};
	virtual void OnDraw();

public:
	void SetRandomType();
	void SetGhost(bool InBool);
	bool IsGhost() { return m_bIsGhost; }

	void SetRotation(ERotationType InType);
	ERotationType GetRotation() { return m_eRotation; }

	void Rotate(int InDir);
	void SetPosition(POINT InPT);

	POINT GetCenterOffset(int InBoxSize) const;

	void SetBlockType(ETetroBlockType InType);
	ETetroBlockType GetBlockType() const { return m_eBlockType; }
	const POINT* GetShape() const { return g_TetrominoTable[(int)m_eBlockType][(int)m_eRotation]; }

private:
	ETetroBlockType m_eBlockType;
	ERotationType m_eRotation;
	POINT m_Origin;
	bool m_bIsGhost;
};