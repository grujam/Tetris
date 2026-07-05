#include "stdafx.h"
#include "Brush.h"
#include "Renderer.h"

CBrush::CBrush(COLORREF InColor)
{
	m_Brush = CreateSolidBrush(InColor);
	m_OldBrush = (HBRUSH)SelectObject(GET_HDC(), m_Brush);

	m_bIsStock = false;
}

CBrush::CBrush(int InStockType)
{
	m_Brush = (HBRUSH)GetStockObject(InStockType);
	m_OldBrush = (HBRUSH)SelectObject(GET_HDC(), m_Brush);

	m_bIsStock = true;
}

CBrush::~CBrush()
{
	SelectObject(GET_HDC(), m_OldBrush);

	if (m_bIsStock == false)
		DeleteObject(m_Brush);
}
