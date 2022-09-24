#pragma once
#include <iostream>
#include <list>
#include "Direct2dApp/Direct2dApp.h"
#include "Clock.h"
#include "LogSys.h"

#define ANIMATION_SPEED 0.15f	// 动画速度
#define ANIMATION_SIZE_SPEED 0.15f	// 生成新棋子动画速度

enum DIR { UP, DOWN, LEFT, RIGHT };
enum GAME_STATUS { STATIC/*静止状态*/, MOVE/*播放动画状态*/, SIZEMOVE,/*生成新棋子*/ GAMEOVER/*游戏结束*/ };

class pos
{
public:
	int x;
	int y;

	friend std::ostream& operator << (std::ostream& out, pos& p)
	{
		std::cout << "x: " << p.x << " y: " << p.y;
		return out;
	}
	friend bool operator == (const pos& a, const pos& b)
	{
		return (a.x == b.x && a.y == b.y);
	}
};

class fpos
{
public:
	float x;
	float y;
};

class Board
{
public:
	Board();
	Board(int x, int y);
	~Board();

	void initBoard(int rows, int cols);
	void Move(DIR dir);
	void Render(app::Direct2dApp& App, RECT& _rc);
	int getSocre();
	void Test();
	void Reset();
	void ReSize(int width, int height);
	void setAnimation(bool flag);
	DIR GetAiMove();

private:
	std::list<std::pair<std::pair<pos, pos>, pos>> m_animation;
	GAME_STATUS game_Status;
	Clock animation_clock;
	Clock size_animation_clock;
	std::pair<pos, int> last_new_cell;
	int** m_board;
	int** m_last_board;
	int m_rows;	// 行
	int m_cols;	// 列
	int m_score;
	bool m_animation_switch = true;

private:
	/// <summary>
	/// 在棋盘里随机生成一个棋子
	/// </summary>
	void randCell(int** arr, int rows, int cols);
	int applyMove(int** arr, int rows, int cols, const pos& from, const pos& to, const pos& dir, int last_add);
	float getTime();
	/// <summary>
	/// 是否已经GameOver
	/// </summary>
	/// <returns></returns>
	bool isGameOver(int** arr, int rows, int cols);
	
	/// <summary>
	/// 是否是无效移动，无效移动返回true
	/// </summary>
	/// <param name="thisArr"></param>
	/// <param name="lastArr"></param>
	/// <param name="rows"></param>
	/// <param name="cols"></param>
	/// <returns></returns>
	bool isFailMove(int** thisArr, int** lastArr, int rows, int cols);

	/// <summary>
	/// 蒙特卡洛搜索
	/// </summary>
	/// <param name="arr"></param>
	/// <param name="rows"></param>
	/// <param name="cols"></param>
	/// <returns></returns>
	DIR aiGetMove(int** arr, int rows, int cols);

	/// <summary>
	/// 向某个方向移动，并返回分数
	/// </summary>
	/// <param name="arr"></param>
	/// <param name="rows"></param>
	/// <param name="cols"></param>
	/// <returns></returns>
	int move(DIR dir, int** arr, int rows, int cols);

	D2D1_COLOR_F getColor(int num);
	D2D1_COLOR_F getTextColor(int num);
	float		 getTextSize(int num);

	int down(int** arr, int rows, int cols);
	int up(int** arr, int rows, int cols);
	int left(int** arr, int rows, int cols);
	int right(int** arr, int rows, int cols);
};

