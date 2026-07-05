#include "stdafx.h"
#include "Renderer.h"
#include "Draw.h"

CRenderer::CRenderer()
    : m_hHwnd(nullptr)
    , m_hMemDC(nullptr)
    , m_hMemBitmap(nullptr)
    , m_hOldBitmap(nullptr)
    , m_nWidth(0)
    , m_nHeight(0)
{
}

void CRenderer::Initialize(HWND InHandle)
{
	m_hHwnd = InHandle;
	RECT rect;
	GetClientRect(m_hHwnd, &rect);
	CreateBuffer(rect.right - rect.left, rect.bottom - rect.top);
}

void CRenderer::OnDestroy()
{
    ReleaseBuffer();
    m_hHwnd = nullptr;
}

void CRenderer::OnUpdate(float InDeltaSeconds)
{
    InvalidateRect(m_hHwnd, nullptr, FALSE);
}

void CRenderer::CreateBuffer(int InWidth, int InHeight)
{
    HDC hScreenDC = GetDC(m_hHwnd);

    m_hMemDC = CreateCompatibleDC(hScreenDC);
    m_hMemBitmap = CreateCompatibleBitmap(hScreenDC, InWidth, InHeight);
    m_hOldBitmap = (HBITMAP)SelectObject(m_hMemDC, m_hMemBitmap);

    m_nWidth = InWidth;
    m_nHeight = InHeight;

    ReleaseDC(m_hHwnd, hScreenDC);
}

void CRenderer::ReleaseBuffer()
{
    if (!m_hMemDC)
        return;

    SelectObject(m_hMemDC, m_hOldBitmap);
    DeleteObject(m_hMemBitmap);
    DeleteDC(m_hMemDC);

    m_hMemDC = nullptr;
    m_hMemBitmap = nullptr;
    m_hOldBitmap = nullptr;
}

void CRenderer::BeginPaint()
{
    RECT rc = { 0, 0, m_nWidth, m_nHeight };
    FillRect(m_hMemDC, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));

    DrawTargets();
}

void CRenderer::EndPaint(HDC InHDC)
{
    BitBlt(InHDC, 0, 0, m_nWidth, m_nHeight, m_hMemDC, 0, 0, SRCCOPY);
}

void CRenderer::AddDrawTargets(std::shared_ptr<IDrawable> InDrawable)
{
    m_vDrawTargets.push_back(InDrawable);
}

void CRenderer::RemoveDrawTargets(std::shared_ptr<IDrawable> InDrawable)
{
    auto iter = std::find(m_vDrawTargets.begin(), m_vDrawTargets.end(), InDrawable);
    m_vDrawTargets.erase(iter);
}

void CRenderer::DrawTargets()
{
    for (const auto& draw : m_vDrawTargets)
    {
        if(draw)
            draw->OnDraw();
    }
}

void CRenderer::OnResize(int InWidth, int InHeight)
{
    if (InWidth <= 0 || InHeight <= 0) return;
    if (InWidth == m_nWidth && InHeight == m_nHeight) return;

    ReleaseBuffer();
    CreateBuffer(InWidth, InHeight);
}
