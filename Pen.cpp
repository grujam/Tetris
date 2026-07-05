#include "stdafx.h"
#include "Pen.h"
#include "Renderer.h"

CPen::CPen(int InStyle, int InWidth, COLORREF InColor)
{
	m_Pen = CreatePen(InStyle, InWidth, InColor);
	m_OldPen = (HPEN)SelectObject(GET_HDC(), m_Pen);

	m_bIsStock = false;
}

CPen::CPen(int InStyle, int InWidth, int InStockType)
{
	m_Pen = (HPEN)GetStockObject(InStockType);
	m_OldPen = (HPEN)SelectObject(GET_HDC(), m_Pen);

	m_bIsStock = true;
}

CPen::~CPen()
{
    SelectObject(GET_HDC(), m_OldPen);

	if(m_bIsStock == false)
		DeleteObject(m_Pen);
}
