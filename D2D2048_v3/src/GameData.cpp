#include "GameData.h"

int GameData::Board_Width = 4;
int GameData::Board_Height = 4;
Board* GameData::board = nullptr;

GameData::GameData()
{
}

GameData::~GameData()
{
	if (this->board != nullptr) delete this->board;
}

GameData& GameData::Get()
{
	static GameData gd;
	return gd;
}
