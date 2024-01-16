#pragma once
#include <iostream>
#include <list>
#include "../include/GameEngine/GameEngine.h"
#include "../include/GameEngine/Clock.h"

#define PI 3.14159265

#define MAX_CELL 512	// ���ĸ���������õ�����������ʵ��ֵӦ���� 2��MAX_CELL�η�

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

	void InitBoard(int rows, int cols);	// ��ʼ��
	void ReSize(int rows, int cols);	// �ı����̴�С
	void ChangeSize(shu::vec2i& size);	// �����ڴ�С�ı�ʱ				
	void Move(DIR dir);					// �ƶ�
	void Reset();						// ����
	int  GetSocre();					// ��ȡ����
	void Test();
	void OpenAnimation(bool flag);		// �򿪻�رն���
	bool isWin();

	void Updata(float dt);
	void Render(shu::Direct2dRender& rt, bool is_black_white);

	// =================�ع���������=====================//

	DIR GetAiMove();
	void SetAnimationMode(ANIMATION_MOVE flag);
	void SetAnimationSpeed(ANIMATION_SPEED_MODE flag);

	ANIMATION_MOVE m_animation_mode = MAX_QUAD;
	ANIMATION_SPEED_MODE m_animation_speed_mode = NORMAL;

private:
	bool _isWin(int** board, int rows, int cols);	// �Ƿ�ʤ��

	// ��һ������
	void DrawCell(int num, const shu::vec2f& pos, float size, shu::Direct2dRender& rt);

	// ����һ������
	int** CreateNewBoard(int rows, int cols);

	// ɾ��һ������
	void DeleteBoard(int** board, int rows);

	// �������̣���board2���̵����ݸ��Ƶ�board1�У�
	void CopyBoard(int** board1, int** board2, int rows, int cols);

	// �������һ������
	void RandCreateCell(int** board, int rows, int cols);

	int ApplyMove(int** board, int rows, int cols, const vec2i& from, const vec2i& to, const vec2i& dir, int last_add);

	// �Ƿ��Ѿ�GameOver
	bool IsGameOver(int** board, int rows, int cols);

	// �Ƿ�����Ч�ƶ�����Ч�ƶ�����true
	bool IsFailMove(int** board, int** last_board, int rows, int cols);

	// ��ĳ�������ƶ��������ط���
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

	// =================�ع���������=====================//

	float getTime();


	// ����������0�ĸ���
	int getArrZeroNum(int** arr, int rows, int cols);

	// ���ؿ�������
	DIR aiGetMove(int** arr, int rows, int cols, int deep);



private:
	shu::Direct2dRender& m_rt;

	ComPtr<ID2D1Effect> pointGrayscaleEffect;	// �Ҷ���Ч

	ID2D1Bitmap1* m_layer_1 = nullptr;	// ͼ��1
	ID2D1Bitmap1* m_layer_2 = nullptr;	// ͼ��2
	ID2D1Bitmap1* m_layer_3 = nullptr;	// ͼ��3
	

private:
	std::list<std::pair<std::pair<vec2i, vec2i>, vec2i>> m_animation;	// �����������ƶ�λ�õ���Ϣ
	std::list<vec2i> m_add_animation;		// �ϳɵ�����λ��
	std::pair<vec2i, int> last_new_cell;	// ��һ�������ɵ�����
	GAME_STATUS GameStatus = STATIC;	// ��Ϸ״̬
	bool m_animation_switch = true;	// ��������
	int** m_board = nullptr;
	int** m_last_board = nullptr;
	size_t m_rows = 0;	// ��
	size_t m_cols = 0;	// ��
	int m_score = 0; // ����

	bool is_win = true;
	bool is_first_win = true;
	bool is_first_win_frame = true;

	Clock animation_clock;
	Clock size_animation_clock;


	float ANIMATION_SPEED = 0.25f;	// �����ٶ�
	float ANIMATION_SIZE_SPEED = 0.25f;	// ���������Ӷ����ٶ�

	MoveAnimationFun m_move_animation_mode_fun = nullptr;	// ��������ָ��
};

