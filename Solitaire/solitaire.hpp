#pragma once

#include <cstdint>

struct Card
{
	enum Suit : uint8_t
	{
		Spade, Heart, Diamond, Club, SuitCount
	};

	enum Value : uint8_t
	{
		_A, _2, _3, _4, _5, _6, _7, _8, _9, _10, _J, _Q, _K, ValueCount
	};

	Suit suit;
	Value value;
	bool faceDown;
	Card* next;

	Card();
	Card(Suit suit, Value value);
	Card(uint8_t suit, uint8_t value);

	bool IsValid() const;
	bool IsBlack() const;
	bool IsRed() const;

	const wchar_t* Symbol() const;
};