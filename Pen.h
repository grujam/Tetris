#pragma once

// Pen 생성 후 자동 삭제
class CPen
{
public:
	// RGB 펜
	CPen(int InStyle, int InWidth, COLORREF InColor);

	// GetStockObject 펜
	CPen(int InStyle, int InWidth, int InStockType);
	~CPen();

private:
	HPEN m_Pen;
	HPEN m_OldPen;

	bool m_bIsStock;
};