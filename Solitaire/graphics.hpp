#pragma once

#include <d2d1.h>
#include <dwrite.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class Graphics
{
	ComPtr<ID2D1Factory> m_d2dFactory;
	ComPtr<ID2D1HwndRenderTarget> m_renderTarget;
	ComPtr<IDWriteFactory> m_writeFactory;
	ComPtr<IDWriteTextFormat> m_font;
	float m_fontSize;
	float m_cardWidth;

private:
	void SetCardWidth(float cardWidth);

public:
	explicit Graphics(HWND window, int width, int height, float cardWidth);
	void Resize(int width, int height, float cardWidth);

	void BeginDraw(float r, float g, float b);
	void EndDraw();

	ComPtr<ID2D1SolidColorBrush> CreateBrush(float r, float g, float b, float a = 1.0f) const;
	void RenderCard(const wchar_t* symbol, float x, float y, ID2D1SolidColorBrush* cardColor, ID2D1SolidColorBrush* bgColor) const;
	void RenderPlaceholder(float x, float y, float weight, ID2D1SolidColorBrush* outlineColor) const;
};