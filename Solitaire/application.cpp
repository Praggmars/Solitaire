#include "application.hpp"
#include <windowsx.h>
#include <algorithm>
#include <random>

static constexpr float g_Spacing= 0.1f;
static constexpr float g_TableOffset = g_Spacing * 2.0f + 1.5f + 0.1f;
static constexpr float g_faceDownOffset = 0.125f;
static constexpr float g_faceUpOffset = 0.25f;

static float CardWidthFromScreenSize(int width, int height)
{
	return std::min<float>(width, height * 1.5f) / (7.0f + 8.0f * g_Spacing);
}

static Card* FindTop(Card* card)
{
	for (; card; card = card->next)
		if (!card->next)
			break;
	return card;
}

void Application::DrawCard(const Card& card, float x, float y)
{
	ID2D1SolidColorBrush* color = m_blueBrush.Get();
	if (!card.faceDown)
	{
		if (card.IsBlack())
			color = m_blackBrush.Get();
		if (card.IsRed())
			color = m_redBrush.Get();
	}
	m_graphics.RenderCard(card.Symbol(), x, y, color, m_whiteBrush.Get());
}

void Application::Paint()
{
	m_graphics.BeginDraw(0.125f, 0.375f, 0.25f);
	RECT rect;
	GetClientRect(m_mainWindow, &rect);
	Paint(static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top));
	m_graphics.EndDraw();
	ValidateRect(m_mainWindow, nullptr);
}

void Application::Paint(float width, float height)
{
	constexpr float strokeWidth = g_Spacing * 0.375f;
	m_graphics.RenderPlaceholder(g_Spacing, g_Spacing, strokeWidth, m_whiteBrush.Get());
	m_graphics.RenderPlaceholder(g_Spacing * 2.0f + 1.0f, g_Spacing, strokeWidth, m_whiteBrush.Get());
	for (size_t i = 0; i < m_collectionArea.size(); ++i)
		m_graphics.RenderPlaceholder(g_Spacing + (g_Spacing + 1.0f) * (i + 3), g_Spacing, strokeWidth, m_whiteBrush.Get());
	for (size_t i = 0; i < m_table.size(); ++i)
		m_graphics.RenderPlaceholder(g_Spacing + (g_Spacing + 1.0f) * i, g_TableOffset, strokeWidth, m_whiteBrush.Get());

	if (m_faceDownDeck)
		DrawCard(Card(), g_Spacing, g_Spacing);
	if (const Card* c = FindTop(m_faceUpDeck))
		DrawCard(*c, g_Spacing * 2.0f + 1.0f, g_Spacing);
	for (size_t i = 0; i < m_collectionArea.size(); ++i)
		if (const Card* c = FindTop(m_collectionArea[i]))
			DrawCard(*c, g_Spacing + (g_Spacing + 1.0f) * (i + 3), g_Spacing);
	for (size_t i = 0; i < m_table.size(); ++i)
	{
		float yOffs = g_TableOffset;
		for (const Card* c = m_table[i]; c; c = c->next)
		{
			DrawCard(*c, g_Spacing + (g_Spacing + 1.0f) * i, yOffs);
			yOffs += c->faceDown ? g_faceDownOffset : g_faceUpOffset;
		}
	}
	if (m_grabbedCard)
	{
		float yOffs = m_grabbedCardY;
		for (const Card* c = m_grabbedCard; c; c = c->next)
		{
			DrawCard(*c, m_grabbedCardX, yOffs);
			yOffs += c->faceDown ? g_faceDownOffset : g_faceUpOffset;
		}
	}
}

void Application::Resize(int width, int height)
{
	m_cardWidth = CardWidthFromScreenSize(width, height);
	m_graphics.Resize(width, height, m_cardWidth);
}

void Application::LMouseDown(int x, int y, WPARAM flags)
{
	SetCapture(m_mainWindow);

	FindClickTarget(static_cast<float>(x) / m_cardWidth, static_cast<float>(y) / m_cardWidth);

	m_cursor = { x, y };
}

void Application::LMouseUp(int x, int y, WPARAM flags)
{
	ReleaseGrabbedCard();
	m_cursor = { x, y };
	SetCapture(nullptr);
}

void Application::MouseMove(int x, int y, WPARAM flags)
{
	const POINT delta = { x - m_cursor.x, y - m_cursor.y };
	if (MK_LBUTTON)
	{
		MoveGrabbedCard(static_cast<float>(delta.x) / m_cardWidth, static_cast<float>(delta.y) / m_cardWidth);
	}
	m_cursor = { x, y };
}

void Application::RequestRedraw() const
{
	InvalidateRect(m_mainWindow, nullptr, false);
}

HWND Application::CreateMainWindow(const wchar_t* title, int width, int height)
{
	WNDCLASSEXW wc{};
	wc.cbSize = sizeof(wc);
	wc.hInstance = GetModuleHandleW(nullptr);
	wc.lpszClassName = title;
	wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = DefWindowProcW;
	RegisterClassExW(&wc);

	DWORD style = WS_OVERLAPPEDWINDOW;
	DWORD exStyle = 0;
	RECT rect{ 0, 0, width, height };
	AdjustWindowRectEx(&rect, style, false, exStyle);
	HWND wnd = CreateWindowExW(exStyle, title, title, style, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, wc.hInstance, nullptr);
	SetWindowLongPtrW(wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	SetWindowLongPtrW(wnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(static_cast<WNDPROC>([](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) -> LRESULT {
		return reinterpret_cast<Application*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))->MessageHandler(msg, wparam, lparam);
	})));
	return wnd;
}

LRESULT Application::MessageHandler(UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_PAINT:
		Paint();
		return 0;
	case WM_MOUSEMOVE:
		MouseMove(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), wparam);
		return 0;
	case WM_LBUTTONDOWN:
		LMouseDown(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), wparam);
		return 0;
	case WM_LBUTTONUP:
		LMouseUp(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), wparam);
		return 0;
	case WM_CONTEXTMENU:
	{
		HMENU menu = CreatePopupMenu();
		AppendMenuW(menu, MF_STRING, 1, L"Restart");
		SetForegroundWindow(m_mainWindow);
		TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_TOPALIGN, LOWORD(lparam), HIWORD(lparam), 0, m_mainWindow, nullptr);
		return 0;
	}
	case WM_COMMAND:
		if (LOWORD(wparam) == 1)
		{
			InitGame();
			RequestRedraw();
		}
		return 0;
	case WM_SIZE:
		Resize(LOWORD(lparam), HIWORD(lparam));
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(m_mainWindow, msg, wparam, lparam);
}

void Application::InitGame()
{
	for (uint8_t suit = 0; suit < Card::SuitCount; ++suit)
		for (uint8_t value = 0; value < Card::ValueCount; ++value)
			m_cards[suit * Card::ValueCount + value] = Card(suit, value);
	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle(m_cards.begin(), m_cards.end(), gen);

	for (Card*& c : m_collectionArea)
		c = nullptr;
	size_t cardCounter = 0;
	for (size_t i = 0; i < m_table.size(); ++i)
	{
		m_table[i] = &m_cards[cardCounter++];
		for (size_t j = 0; j < i; ++j)
		{
			m_cards[cardCounter - 1].faceDown = true;
			m_cards[cardCounter - 1].next = &m_cards[cardCounter];
			++cardCounter;
		}
	}

	m_faceUpDeck = nullptr;
	m_faceDownDeck = &m_cards[cardCounter];
	while (++cardCounter < m_cards.size())
		m_cards[cardCounter - 1].next = &m_cards[cardCounter];
	m_grabbedCard = nullptr;
	m_ptrToGrabbed = nullptr;
	m_ptrToFaceDown = nullptr;
}

void Application::FlipDeckCard()
{
	if (m_faceDownDeck)
	{
		Card* moved = m_faceDownDeck;
		m_faceDownDeck = m_faceDownDeck->next;
		moved->next = nullptr;
		if (Card* c = FindTop(m_faceUpDeck))
			c->next = moved;
		else
			m_faceUpDeck = moved;
		RequestRedraw();
	}
	else
	{
		if (m_faceUpDeck)
		{
			m_faceDownDeck = m_faceUpDeck;
			m_faceUpDeck = nullptr;
			RequestRedraw();
		}
	}
}

void Application::TakeDeckCard(Card*& base)
{
	if (base)
	{
		m_ptrToGrabbed = &base;
		for (Card* c = base; c; c = c->next)
		{
			if (!c->next)
			{
				m_grabbedCard = c;
				*m_ptrToGrabbed = nullptr;
				break;
			}
			m_ptrToGrabbed = &c->next;
		}
		RequestRedraw();
	}
}

void Application::ReleaseGrabbedCard()
{
	if (m_grabbedCard && m_ptrToGrabbed)
	{
		const float x = m_grabbedCardX + 0.5f;
		const float y = m_grabbedCardY + 0.75f;

		if (y >= g_Spacing && y <= g_Spacing + 1.5f)
		{
			for (size_t i = 0; i < m_collectionArea.size(); ++i)
			{
				if (x >= g_Spacing + (g_Spacing + 1.0f) * (i + 3) && x <= g_Spacing + (g_Spacing + 1.0f) * (i + 3) + 1.0f)
				{
					if (Card* c = FindTop(m_collectionArea[i]))
					{
						if (c->suit == m_grabbedCard->suit && c->value + 1 == m_grabbedCard->value)
						{
							c->next = m_grabbedCard;
							m_grabbedCard = nullptr;
							m_ptrToGrabbed = nullptr;
							if (m_ptrToFaceDown)
								*m_ptrToFaceDown = false;
							m_ptrToFaceDown = nullptr;
						}
					}
					else
					{
						if (m_grabbedCard->value == Card::_A)
						{
							m_collectionArea[i] = m_grabbedCard;
							m_grabbedCard = nullptr;
							m_ptrToGrabbed = nullptr;
							if (m_ptrToFaceDown)
								*m_ptrToFaceDown = false;
							m_ptrToFaceDown = nullptr;
						}
					}
					break;
				}
			}
		}
		else
		{
			for (size_t i = 0; i < m_table.size(); ++i)
			{
				if (x >= g_Spacing + (g_Spacing + 1.0f) * i && x <= g_Spacing + (g_Spacing + 1.0f) * i + 1.0f)
				{
					float lowerY= g_TableOffset + 1.5f;
					Card* topCard = m_table[i];
					for (; topCard; topCard = topCard->next)
					{
						if (!topCard->next)
							break;
						lowerY += topCard->faceDown ? g_faceDownOffset : g_faceUpOffset;
					}
					if (y >= g_TableOffset && y <= lowerY)
					{
						if (topCard)
						{
							if ((topCard->IsBlack() ^ m_grabbedCard->IsBlack()) && (topCard->value == m_grabbedCard->value + 1) && !topCard->faceDown)
							{
								topCard->next = m_grabbedCard;
								m_grabbedCard = nullptr;
								m_ptrToGrabbed = nullptr;
								if (m_ptrToFaceDown)
									*m_ptrToFaceDown = false;
								m_ptrToFaceDown = nullptr;
							}
						}
						else
						{
							if (m_grabbedCard->value == Card::_K)
							{
								m_table[i] = m_grabbedCard;
								m_grabbedCard = nullptr;
								m_ptrToGrabbed = nullptr;
								if (m_ptrToFaceDown)
									*m_ptrToFaceDown = false;
								m_ptrToFaceDown = nullptr;
							}
						}
					}
				}
			}
		}

		if (m_grabbedCard && m_ptrToGrabbed)
		{
			*m_ptrToGrabbed = m_grabbedCard;
			m_grabbedCard = nullptr;
			m_ptrToGrabbed = nullptr;
			m_ptrToFaceDown = nullptr;
		}
		RequestRedraw();
	}
}

void Application::MoveGrabbedCard(float x, float y)
{
	if (m_grabbedCard)
	{
		m_grabbedCardX += x;
		m_grabbedCardY += y;
		RequestRedraw();
	}
}

void Application::FindClickTarget(float x, float y)
{
	if (y >= g_Spacing && y <= g_Spacing + 1.5f)
	{
		if (x >= g_Spacing && x <= g_Spacing + 1.0f)
		{
			FlipDeckCard();
		}
		else if (x >= g_Spacing * 2.0f + 1.0f && x <= g_Spacing * 2.0f + 2.0f)
		{
			TakeDeckCard(m_faceUpDeck);
			m_grabbedCardX = g_Spacing * 2.0f + 1.0f;
			m_grabbedCardY = 0.1f;
		}
		else
		{
			for (size_t i = 0; i < m_collectionArea.size(); ++i)
			{
				if (x >= g_Spacing + (g_Spacing + 1.0f) * (i + 3) && x <= g_Spacing + (g_Spacing + 1.0f) * (i + 3) + 1.0f)
				{
					TakeDeckCard(m_collectionArea[i]);
					m_grabbedCardX = g_Spacing + (g_Spacing + 1.0f) * (i + 3);
					m_grabbedCardY = 0.1f;
					break;
				}
			}
		}
	}
	else
	{
		for (size_t i = 0; i < m_table.size(); ++i)
		{
			if (x >= g_Spacing + (g_Spacing + 1.0f) * i && x <= g_Spacing + (g_Spacing + 1.0f) * i + 1.0f)
			{
				float yOffs = g_TableOffset;
				m_ptrToGrabbed = &m_table[i];
				m_grabbedCardX = g_Spacing + (g_Spacing + 1.0f) * i;
				for (Card* c = m_table[i]; c; c = c->next)
				{
					if (!c->faceDown)
					{
						if (y >= yOffs && y <= yOffs + 1.5f)
						{
							m_grabbedCardY = yOffs;
							m_grabbedCard = c;
						}
					}
					yOffs += c->faceDown ? g_faceDownOffset : g_faceUpOffset;
					if (y < yOffs || !c->next)
						break;
					m_ptrToGrabbed = &c->next;
					m_ptrToFaceDown = &c->faceDown;
				}
				if (m_grabbedCard)
				{
					*m_ptrToGrabbed = nullptr;
					RequestRedraw();
				}
				else
					m_ptrToGrabbed = nullptr;
				break;
			}
		}
	}
}

Application::Application(const wchar_t* title, int width, int height)
	: m_mainWindow{CreateMainWindow(title, width, height)}
	, m_cardWidth{CardWidthFromScreenSize(width, height)}
	, m_graphics(m_mainWindow, width, height, m_cardWidth)
	, m_whiteBrush{m_graphics.CreateBrush(0.9f, 0.9f, 0.9f)}
	, m_blackBrush{m_graphics.CreateBrush(0.1f, 0.05f, 0.15f)}
	, m_redBrush{m_graphics.CreateBrush(0.8f, 0.2f, 0.1f)}
	, m_blueBrush{m_graphics.CreateBrush(0.14f, 0.24f, 0.39f)}
{
	InitGame();
}

int Application::Run()
{
	UpdateWindow(m_mainWindow);
	ShowWindow(m_mainWindow, SW_SHOWDEFAULT);
	MSG msg{};
	while (GetMessageW(&msg, nullptr, 0, 0))
{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
}
	return static_cast<int>(msg.wParam);
}
