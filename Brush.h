#pragma once

// Brush 생성 후 자동 삭제
class CBrush
{
public:
	CBrush(COLORREF InColor);
	CBrush(int InStockType);

	~CBrush();

private:
	HBRUSH m_Brush;
	HBRUSH m_OldBrush;

	bool m_bIsStock;
};