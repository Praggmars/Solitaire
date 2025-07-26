#include "graphics.hpp"
#include <stdexcept>

#pragma comment (lib, "d2d1")
#pragma comment (lib, "dwrite")

static void ThrowIfFailed(HRESULT result)
{
	if (FAILED(result))
	{
		char buff[32];
		snprintf(buff, sizeof(buff) - 1, "Error: 0x%08x", result);
		throw std::runtime_error(buff);
	}
}

void Graphics::SetCardWidth(float cardWidth)
{
	m_cardWidth = max(20.0f, cardWidth);
	m_fontSize = m_cardWidth / 0.6163f;
	ThrowIfFailed(m_writeFactory->CreateTextFormat(L"Arial", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, m_fontSize, L"en-us", &m_font));
}

Graphics::Graphics(HWND window, int width, int height, float cardWidth)
{
	ThrowIfFailed(CoInitialize(nullptr));
	ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&m_d2dFactory)));
	ThrowIfFailed(m_d2dFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(window, D2D1::SizeU(width, height)), &m_renderTarget));

	ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), static_cast<IUnknown**>(&m_writeFactory)));
	SetCardWidth(cardWidth);
}

void Graphics::Resize(int width, int height, float cardWidth)
{
	ThrowIfFailed(m_renderTarget->Resize(D2D1::SizeU(width, height)));
	SetCardWidth(cardWidth);
}

void Graphics::BeginDraw(float r, float g, float b)
{
	m_renderTarget->BeginDraw();
	m_renderTarget->Clear(D2D1::ColorF(r, g, b));
}

void Graphics::EndDraw()
{
	m_renderTarget->EndDraw();
}

ComPtr<ID2D1SolidColorBrush> Graphics::CreateBrush(float r, float g, float b, float a) const
{
	ComPtr<ID2D1SolidColorBrush> brush;
	ThrowIfFailed(m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(r, g, b, a), &brush));
	return brush;
}

void Graphics::RenderCard(const wchar_t* symbol, float x, float y, ID2D1SolidColorBrush* cardColor, ID2D1SolidColorBrush* bgColor) const
{
	m_renderTarget->FillRectangle(D2D1::RectF(x * m_cardWidth + 1.0f, y * m_cardWidth + 1.0f, (x + 1.0f) * m_cardWidth - 1.0f, (y + 1.5f) * m_cardWidth - 1.0f), bgColor);
	m_renderTarget->DrawTextW(symbol, wcslen(symbol), m_font.Get(), D2D1::RectF(x * m_cardWidth - 0.0195f * m_fontSize, y * m_cardWidth - 0.2671f * m_fontSize, x * m_cardWidth + 0.6358f * m_fontSize, y * m_cardWidth + 1.1909f * m_fontSize), cardColor);
}

void Graphics::RenderPlaceholder(float x, float y, float weight, ID2D1SolidColorBrush* outlineColor) const
{
	const float inset = weight * m_cardWidth * 0.6f;
	m_renderTarget->DrawRectangle(D2D1::RectF(x * m_cardWidth + inset, y * m_cardWidth + inset, (x + 1.0f) * m_cardWidth - inset, (y + 1.5f) * m_cardWidth - inset), outlineColor, weight * m_cardWidth);
}