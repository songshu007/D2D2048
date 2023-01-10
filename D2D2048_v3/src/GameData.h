#pragma once
#include "Board.h"

class GameData
{
public:
	GameData();
	~GameData();

	GameData(const GameData&) = delete;
	void operator = (const GameData&) = delete;
	
	static GameData& Get();

	static int Board_Width;
	static int Board_Height;
	static Board* board;
};
