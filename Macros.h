#pragma once

// Singletons
#define GET_RENDER()	CRenderer::Get()
#define GET_HDC()		GET_RENDER()->GetHDC()
#define GET_HWND()		GET_RENDER()->GetHWND()
#define ADD_DRAWABLE(drawable) GET_RENDER()->AddDrawTargets(drawable)

// Draw Helpers
#define SCOPED_PEN_RGB(name, style, width, color)	CPen name(style, width, color)
#define SCOPED_PEN_STOCK(name, stocktype)			CPen name(stocktype)

#define SCOPED_BRUSH_COLOR(name, color)				CBrush name(color)
#define SCOPED_BRUSH_STOCK(name, stocktype)			CBrush name(stocktype)

// Colors
#define RGB_RED		RGB(255,0,0)
#define RGB_BLUE	RGB(0,255,0)
#define RGB_GREEN	RGB(0,0,255)
#define RGB_BLACK	RGB(0,0,0)
#define RGB_WHITE	RGB(255,255,255)
