#include "solitaire.hpp"

Card::Card()
	: suit{ SuitCount }
	, value{ ValueCount }
	, faceDown{ false }
	, next{ nullptr }
{}

Card::Card(Suit suit, Value value)
	: suit{ suit }
	, value{ value }
	, faceDown{ false }
	, next{ nullptr }
{}

Card::Card(uint8_t suit, uint8_t value) : Card(Suit{ suit }, Value{ value }) {}

bool Card::IsValid() const { return suit < SuitCount && value < ValueCount; }
bool Card::IsBlack() const { return suit == Spade || suit == Club; }
bool Card::IsRed() const { return suit == Heart || suit == Diamond; }

const wchar_t* Card::Symbol() const
{
	static const wchar_t* s_CardSymbols[] = {
		L"\xD83C\xDCA1", L"\xD83C\xDCA2", L"\xD83C\xDCA3", L"\xD83C\xDCA4", L"\xD83C\xDCA5", L"\xD83C\xDCA6", L"\xD83C\xDCA7", L"\xD83C\xDCA8", L"\xD83C\xDCA9", L"\xD83C\xDCAA", L"\xD83C\xDCAB", L"\xD83C\xDCAD", L"\xD83C\xDCAE",
		L"\xD83C\xDCB1", L"\xD83C\xDCB2", L"\xD83C\xDCB3", L"\xD83C\xDCB4", L"\xD83C\xDCB5", L"\xD83C\xDCB6", L"\xD83C\xDCB7", L"\xD83C\xDCB8", L"\xD83C\xDCB9", L"\xD83C\xDCBA", L"\xD83C\xDCBB", L"\xD83C\xDCBD", L"\xD83C\xDCBE",
		L"\xD83C\xDCC1", L"\xD83C\xDCC2", L"\xD83C\xDCC3", L"\xD83C\xDCC4", L"\xD83C\xDCC5", L"\xD83C\xDCC6", L"\xD83C\xDCC7", L"\xD83C\xDCC8", L"\xD83C\xDCC9", L"\xD83C\xDCCA", L"\xD83C\xDCCB", L"\xD83C\xDCCD", L"\xD83C\xDCCE",
		L"\xD83C\xDCD1", L"\xD83C\xDCD2", L"\xD83C\xDCD3", L"\xD83C\xDCD4", L"\xD83C\xDCD5", L"\xD83C\xDCD6", L"\xD83C\xDCD7", L"\xD83C\xDCD8", L"\xD83C\xDCD9", L"\xD83C\xDCDA", L"\xD83C\xDCDB", L"\xD83C\xDCDD", L"\xD83C\xDCDE",
		L"\xD83C\xDCA0"
	};
	if (!faceDown && suit < SuitCount && value < ValueCount)
		return s_CardSymbols[suit * ValueCount + value];
	return s_CardSymbols[SuitCount * ValueCount];
}