#include "stdafx.h"
#include "Renderer.h"
#include "TetrisManager.h"

#include <windows.h>

const int g_nWindowWidth = 1000;
const int g_nWindowHeight = 880;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        GET_RENDER()->Initialize(hwnd);
        CTetrisManager::Get()->Initialize();
        return 0;
    }

    case WM_SIZE:
    {
        GET_RENDER()->OnResize(LOWORD(lParam), HIWORD(lParam));
        return 0;
    }
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_LEFT:
            CTetrisManager::Get()->MoveHorizontal(-1);
            break;
        case VK_RIGHT:
            CTetrisManager::Get()->MoveHorizontal(1);
            break;
        case VK_UP:
            CTetrisManager::Get()->Rotate();
            break;
        case VK_DOWN:
            CTetrisManager::Get()->SoftDrop();
            break;
        case VK_SPACE:
            CTetrisManager::Get()->HardDrop();
            break;
        case 'H':
            CTetrisManager::Get()->Hold();
            break;
        }
        return 0;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        GET_RENDER()->BeginPaint();
        GET_RENDER()->EndPaint(hdc);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_ERASEBKGND:
        // 백그라운드 지우기 방지 = 깜박임 방지
        return 1;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"TetrisWindow";

    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"Tetris",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, g_nWindowWidth, g_nWindowHeight,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    LARGE_INTEGER frequency, lastTime, currentTime;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&lastTime);

    MSG msg{};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            QueryPerformanceCounter(&currentTime);
            float deltaSeconds = (float)(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
            lastTime = currentTime;

            CTetrisManager::Get()->OnUpdate(deltaSeconds);

            InvalidateRect(hwnd, nullptr, FALSE);
        }
    }
    return 0;
}