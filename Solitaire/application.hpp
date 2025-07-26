#include "graphics.hpp"
#include "solitaire.hpp"
#include <array>

class Application
{
	HWND m_mainWindow;
	float m_cardWidth;
	Graphics m_graphics;
	ComPtr<ID2D1SolidColorBrush> m_whiteBrush;
	ComPtr<ID2D1SolidColorBrush> m_blackBrush;
	ComPtr<ID2D1SolidColorBrush> m_redBrush;
	ComPtr<ID2D1SolidColorBrush> m_blueBrush;

	POINT m_cursor;

	std::array<Card, 52> m_cards;
	Card* m_faceDownDeck;
	Card* m_faceUpDeck;
	std::array<Card*, 4> m_collectionArea;
	std::array<Card*, 7> m_table;
	Card* m_grabbedCard;
	Card** m_ptrToGrabbed;
	bool* m_ptrToFaceDown;
	float m_grabbedCardX;
	float m_grabbedCardY;

private:
	void DrawCard(const Card& card, float x, float y);
	void Paint();
	void Paint(float width, float height);
	void Resize(int width, int height);
	void LMouseDown(int x, int y, WPARAM flags);
	void LMouseUp(int x, int y, WPARAM flags);
	void MouseMove(int x, int y, WPARAM flags);
	void RequestRedraw() const;
	HWND CreateMainWindow(const wchar_t* title, int width, int height);
	LRESULT MessageHandler(UINT msg, WPARAM wparam, LPARAM lparam);

	void InitGame();
	void FlipDeckCard();
	void TakeDeckCard(Card*& base);
	void ReleaseGrabbedCard();
	void MoveGrabbedCard(float x, float y);
	void FindClickTarget(float x, float y);

public:
	Application(const wchar_t* title, int width, int height);

	int Run();
};