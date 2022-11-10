#pragma once
#include <iostream>
#include <list>
#include "Direct2dApp/Direct2dApp.h"
#include "Clock.h"
#include "LogSys.h"
#define PI 3.14159265

#define MAX_CELL 512	// 最大的格子数，想得到最大格子数的实际值应该是 2的MAX_CELL次方

enum DIR { UP, DOWN, LEFT, RIGHT };
enum GAME_STATUS { STATIC/*静止状态*/, MOVE/*播放动画状态*/, SIZEMOVE,/*生成新棋子*/ GAMEOVER/*游戏结束*/ };
enum ANIMATION_MOVE {LINE, QUAD, MAX_QUAD, JELLY, FALL};

enum ANIMATION_SPEED_MODE { NORMAL, MEDIUM, SLOW };

typedef float (*MoveAnimationFun)(float);
typedef float (*SizeAnimationFun)(float);

float easeInOutQuad(float t);
float easeInOutQuart(float t);
float easeInOutBack(float t);
float easeInOutCirc(float t);
float easeOutElastic(float t);

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
	void setAnimationMode(ANIMATION_MOVE flag);
	void setAnimationSpeed(ANIMATION_SPEED_MODE flag);

	ANIMATION_MOVE m_animation_mode = MAX_QUAD;
	ANIMATION_SPEED_MODE m_animation_speed_mode = NORMAL;

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

	float ANIMATION_SPEED      = 0.15f;	// 动画速度
	float ANIMATION_SIZE_SPEED = 0.15f;	// 生成新棋子动画速度

	MoveAnimationFun m_move_animation_mode_fun;	// 动画函数指针
	SizeAnimationFun m_size_animation_mode_fun;	// 动画函数指针

private:
	// 在棋盘里随机生成一个棋子
	void randCell(int** arr, int rows, int cols);

	int applyMove(int** arr, int rows, int cols, const pos& from, const pos& to, const pos& dir, int last_add);

	float getTime();

	// 是否已经GameOver
	bool isGameOver(int** arr, int rows, int cols);
	
	// 是否是无效移动，无效移动返回true
	bool isFailMove(int** thisArr, int** lastArr, int rows, int cols);

	// 返回数组中0的个数
	int getArrZeroNum(int** arr, int rows, int cols);

	// 对数组的单调性进行评分，该函数是错误的
	int aiDullScore(int** arr, int rows, int cols) = delete;

	// 对数组的相同的格子数进行评分，相同的格子越多，分数越高
	int aiSameScore(int** arr, int rows, int cols);

	// 蒙特卡洛搜索
	DIR aiGetMove(int** arr, int rows, int cols, int deep);

	// 向某个方向移动，并返回分数
	int move(DIR dir, int** arr, int rows, int cols);

	D2D1_COLOR_F getColor(int num);
	D2D1_COLOR_F getTextColor(int num);
	float		 getTextSize(int num);

	int down(int** arr, int rows, int cols);
	int up(int** arr, int rows, int cols);
	int left(int** arr, int rows, int cols);
	int right(int** arr, int rows, int cols);
};

