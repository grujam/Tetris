#pragma once
#include <vector>
#include <memory>
#include "Singleton.h"

class IDrawable;

class CRenderer : public ISingleton<CRenderer>
{
public:
    CRenderer();
    virtual ~CRenderer() {};

public:
	void Initialize(HWND InHandle);
    void OnDestroy();
    void OnUpdate(float InDeltaSeconds);

    void CreateBuffer(int InWidth, int InHeight);
    void ReleaseBuffer();

    void BeginPaint();
    void EndPaint(HDC InHDC);

    void AddDrawTargets(std::shared_ptr<IDrawable> InDrawable);
    void RemoveDrawTargets(std::shared_ptr<IDrawable> InDrawable);

    void DrawTargets();
    void OnResize(int InWidth, int InHeight);

    const HDC& GetHDC() { return m_hMemDC; }
    const HWND& GetHWND() { return m_hHwnd; }

private:
    HWND    m_hHwnd;
    HDC     m_hMemDC;
    HBITMAP m_hMemBitmap;
    HBITMAP m_hOldBitmap;
    int     m_nWidth;
    int     m_nHeight;

private:
    std::vector<std::shared_ptr<IDrawable>> m_vDrawTargets;
};