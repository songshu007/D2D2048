#pragma once
#include <iostream>
#include <list>
#include "../include/GameEngine/GameEngine.h"
#include "../include/GameEngine/Clock.h"

#define PI 3.14159265

#define MAX_CELL 512	// 最大的格子数，想得到最大格子数的实际值应该是 2的MAX_CELL次方

enum DIR { UP, DOWN, LEFT, RIGHT };
enum GAME_STATUS { STATIC, MOVE, SIZEMOVE, GAMEOVER };
enum ANIMATION_MOVE { LINE, QUAD, MAX_QUAD, JELLY, FALL };

enum ANIMATION_SPEED_MODE { NORMAL, MEDIUM, SLOW };

typedef float (*MoveAnimationFun)(float);
typedef float (*SizeAnimationFun)(float);

static float easeInOutQuart(float t)
{
	return float(t < 0.5 ? 8 * t * t * t * t : 1 - pow(-2 * t + 2, 4) / 2);
}
static float easeInOutBack(float t)
{
	const float c1 = 1.70158;
	const float c2 = c1 * 1.525;

	return float(t < 0.5
		? (pow(2 * t, 2) * ((c2 + 1) * 2 * t - c2)) / 2
		: (pow(2 * t - 2, 2) * ((c2 + 1) * (t * 2 - 2) + c2) + 2) / 2);
}
static float easeInOutCirc(float t)
{
	return float(t < 0.5
		? (1 - sqrt(1 - pow(2 * t, 2))) / 2
		: (sqrt(1 - pow(-2 * t + 2, 2)) + 1) / 2);
}

using namespace shu;

class Board
{
public:
	Board(shu::Direct2dRender& rt);
	~Board();

	void InitBoard(int rows, int cols);	// 初始化
	void ReSize(int rows, int cols);	// 改变棋盘大小
	void ChangeSize(shu::vec2i& size);	// 当窗口大小改变时				
	void Move(DIR dir);					// 移动
	void Reset();						// 重置
	int  GetSocre();					// 获取分数
	void Test();
	void OpenAnimation(bool flag);		// 打开或关闭动画
	bool isWin();

	void Updata(float dt);
	void Render(shu::Direct2dRender& rt, bool is_black_white);

	// =================重构到了这里=====================//

	DIR GetAiMove();
	void SetAnimationMode(ANIMATION_MOVE flag);
	void SetAnimationSpeed(ANIMATION_SPEED_MODE flag);

	ANIMATION_MOVE m_animation_mode = MAX_QUAD;
	ANIMATION_SPEED_MODE m_animation_speed_mode = NORMAL;

private:
	bool _isWin(int** board, int rows, int cols);	// 是否胜利

	// 画一个棋子
	void DrawCell(int num, const shu::vec2f& pos, float size, shu::Direct2dRender& rt);

	// 创建一个棋盘
	int** CreateNewBoard(int rows, int cols);

	// 删除一个棋盘
	void DeleteBoard(int** board, int rows);

	// 复制棋盘（将board2棋盘的内容复制到board1中）
	void CopyBoard(int** board1, int** board2, int rows, int cols);

	// 随机生成一个棋子
	void RandCreateCell(int** board, int rows, int cols);

	int ApplyMove(int** board, int rows, int cols, const vec2i& from, const vec2i& to, const vec2i& dir, int last_add);

	// 是否已经GameOver
	bool IsGameOver(int** board, int rows, int cols);

	// 是否是无效移动，无效移动返回true
	bool IsFailMove(int** board, int** last_board, int rows, int cols);

	// 向某个方向移动，并返回分数
	int move(DIR board, int** arr, int rows, int cols);

	int up(int** board, int rows, int cols);
	int down(int** board, int rows, int cols);
	int left(int** board, int rows, int cols);
	int right(int** board, int rows, int cols);

	void PrintfBoard(int** board, int rows, int cols)
	{
		std::cout << "====================\n";
		for (int y = 0; y < rows; y++)
		{
			for (int x = 0; x < cols; x++)
			{
				std::cout << board[y][x] << "  ";
			}
			std::cout << "\n";
		}
	}

	shu::color4f& GetColor(int num);

	shu::color4f GetTextColor(int num);
	float		 GetTextSize(int num);

	// =================重构到了这里=====================//

	float getTime();


	// 返回数组中0的个数
	int getArrZeroNum(int** arr, int rows, int cols);

	// 蒙特卡洛搜索
	DIR aiGetMove(int** arr, int rows, int cols, int deep);



private:
	shu::Direct2dRender& m_rt;

	ComPtr<ID2D1Effect> pointGrayscaleEffect;	// 灰度特效

	ID2D1Bitmap1* m_layer_1 = nullptr;	// 图层1
	ID2D1Bitmap1* m_layer_2 = nullptr;	// 图层2
	ID2D1Bitmap1* m_layer_3 = nullptr;	// 图层3
	

private:
	std::list<std::pair<std::pair<vec2i, vec2i>, vec2i>> m_animation;	// 包含了棋子移动位置的信息
	std::list<vec2i> m_add_animation;		// 合成的棋子位置
	std::pair<vec2i, int> last_new_cell;	// 上一次新生成的棋子
	GAME_STATUS GameStatus = STATIC;	// 游戏状态
	bool m_animation_switch = true;	// 动画开关
	int** m_board = nullptr;
	int** m_last_board = nullptr;
	size_t m_rows = 0;	// 行
	size_t m_cols = 0;	// 列
	int m_score = 0; // 分数

	bool is_win = true;
	bool is_first_win = true;
	bool is_first_win_frame = true;

	Clock animation_clock;
	Clock size_animation_clock;


	float ANIMATION_SPEED = 0.25f;	// 动画速度
	float ANIMATION_SIZE_SPEED = 0.25f;	// 生成新棋子动画速度

	MoveAnimationFun m_move_animation_mode_fun = nullptr;	// 动画函数指针
};

