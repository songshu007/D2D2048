#include "Board.h"

Board::Board(shu::Direct2dRender& rt)
	: m_rt(rt)
{
	m_rt.GetDC().CreateEffect(CLSID_D2D1Grayscale, &pointGrayscaleEffect);
	
}

Board::~Board()
{
	DeleteBoard(m_board, m_rows);
	DeleteBoard(m_last_board, m_rows);
}

void Board::InitBoard(int rows, int cols)
{
	// 设置棋盘大小
	m_rows = rows; this->m_cols = cols;

	// 动态分配棋盘的内存
	m_board = CreateNewBoard(rows, cols);
	m_last_board = CreateNewBoard(rows, cols);


	/*m_board[0][0] = 1;
	m_board[0][1] = 1;
	m_board[0][2] = 2;
	m_board[0][3] = 2;*/

	// 随机生成一个棋子
	RandCreateCell(m_board, m_rows, m_cols);
}

void Board::ReSize(int rows, int cols)
{
	// 删除原棋盘数组
	DeleteBoard(m_board, m_rows);
	DeleteBoard(m_last_board, m_rows);

	// 更新棋盘大小
	this->m_rows = rows;
	this->m_cols = cols;

	// 重新为棋盘分配内存
	m_board = CreateNewBoard(rows, cols);
	m_last_board = CreateNewBoard(rows, cols);

	this->Reset();
}

void Board::ChangeSize(shu::vec2i& size)
{
	static auto create_bitmap = [&](shu::vec2i& s, ID2D1Bitmap1** bitmap) {
		// 创建位图
		D2D1_BITMAP_PROPERTIES1 d2d1_bitmap_def = {};
		d2d1_bitmap_def.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		d2d1_bitmap_def.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
		d2d1_bitmap_def.dpiX = USER_DEFAULT_SCREEN_DPI;
		d2d1_bitmap_def.dpiY = USER_DEFAULT_SCREEN_DPI;
		d2d1_bitmap_def.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET;
		m_rt.GetDC().CreateBitmap(
			D2D1::SizeU(s.x, s.y),
			0, 0,
			d2d1_bitmap_def,
			&*bitmap);
	};

	if (m_layer_1 != nullptr) m_layer_1->Release();
	if (m_layer_2 != nullptr) m_layer_2->Release();
	if (m_layer_3 != nullptr) m_layer_3->Release();

	create_bitmap(size, &m_layer_1);
	create_bitmap(size, &m_layer_2);
	create_bitmap(size, &m_layer_3);
}

void Board::Updata(float dt)
{
	shu::vec2f rc = m_rt.GetSize();
	float m_spacing = min(rc.x, rc.y) * 0.03f / max(m_rows, m_cols) * 2.0f;
	int MinWidth = min(rc.x, rc.y);
	float BlockWidth = (MinWidth - (m_spacing * (max(m_rows, m_cols) + 1))) / max(m_rows, m_cols);

	float boardX = (rc.x - (max(m_rows, m_cols) * BlockWidth + (max(m_rows, m_cols) + 1) * m_spacing)) / 2.0f;
	float boardY = (rc.y - (max(m_rows, m_cols) * BlockWidth + (max(m_rows, m_cols) + 1) * m_spacing)) / 2.0f;

	if (animation_clock.GetTime() >= ANIMATION_SPEED && GameStatus == MOVE)
	{
		GameStatus = SIZEMOVE;
		size_animation_clock.Reset();
	}
	if (size_animation_clock.GetTime() >= ANIMATION_SIZE_SPEED && GameStatus == SIZEMOVE)
	{
		m_add_animation.clear();
		m_animation.clear();
		GameStatus = STATIC;
	}

	// 清理图层
	static auto clear = [&](ID2D1Bitmap1** bitmap)
	{
		m_rt.GetDC().SetTarget(*bitmap);
		m_rt.BeginDraw();
		m_rt.Clear(shu::color4f(0.0f, 0.0f, 0.0f, 0.0f));
		m_rt.GetDC().EndDraw();
	};

	clear(&m_layer_1);
	clear(&m_layer_2);
	clear(&m_layer_3);

	switch (GameStatus)
	{
	case STATIC:
	{
		{
			m_rt.GetDC().SetTarget(m_layer_1);
			m_rt.BeginDraw();

			for (size_t y = 0; y < m_rows; y++)
			{
				for (size_t x = 0; x < m_cols; x++)
				{
					float TranslationX = boardX + m_spacing + x * (BlockWidth + m_spacing);
					float TranslationY = boardY + m_spacing + y * (BlockWidth + m_spacing);

					DrawCell(m_board[y][x], shu::vec2f(TranslationX, TranslationY), BlockWidth, m_rt);
				}
			}

			// 这里不调用封装的那个Direct2dRender::EndDraw，因为封装的里面对交换链进行了Present操作
			// 这样会导致垂直同步出现问题，交换链会误以为我们渲染的太快了，因为我们在一帧的渲染中多次调用Present
			m_rt.GetDC().EndDraw();
		}

		break;
	}
	case MOVE:
	{
		// 先画没动的棋子
		{
			m_rt.GetDC().SetTarget(m_layer_1);
			m_rt.BeginDraw();

			for (size_t y = 0; y < m_rows; y++)
			{
				for (size_t x = 0; x < m_cols; x++)
				{
					int flag = 0;
					for (auto it = m_animation.begin(); it != m_animation.end(); it++)
					{
						vec2i from = it->first.first;
						vec2i to = it->first.second;
						vec2i dir = it->second;
						// 如果该棋子是要移动的棋子
						if (from == vec2i(x, y))
						{
							flag = 1;
							break;
						}
						// 如果是移动到的
						if (to == vec2i(x, y))
						{
							//if (m_last_board[y][x] == 0)
								flag = 2;
							break;
						}
					}
					// 如果该棋子是新生成的，不要画
					if (last_new_cell.first == vec2i(x, y)) flag = 1;

					float TranslationX = boardX + m_spacing + x * (BlockWidth + m_spacing);
					float TranslationY = boardY + m_spacing + y * (BlockWidth + m_spacing);
					if (flag == 0)
					{
						DrawCell(m_board[y][x], shu::vec2f(TranslationX, TranslationY), BlockWidth, m_rt);
					}
					if (flag == 1)
					{
						DrawCell(0, shu::vec2f(TranslationX, TranslationY), BlockWidth, m_rt);
					}
					if (flag == 2)
					{
						DrawCell(m_last_board[y][x], shu::vec2f(TranslationX, TranslationY), BlockWidth, m_rt);
					}
				}
			}

			// 与STATIC的情况相同
			m_rt.GetDC().EndDraw();
		}

		// 画运动的叠加层
		{
			m_rt.GetDC().SetTarget(m_layer_2);
			m_rt.BeginDraw();

			for (auto it = m_animation.begin(); it != m_animation.end(); it++)
			{
				vec2i from = it->first.first;
				vec2i to = it->first.second;
				vec2i dir = it->second;

				float cellX = boardX + m_spacing + from.x * (BlockWidth + m_spacing);
				float cellY = boardY + m_spacing + from.y * (BlockWidth + m_spacing);

				// 缓动
				float move_x = animation_clock.GetTime() / ANIMATION_SPEED;
				float movePercent = easeInOutQuart(move_x);

				vec2f pos = {
					(max(from.x, to.x) - min(from.x, to.x)) * (BlockWidth + m_spacing) * movePercent * dir.x + cellX,
					(max(from.y, to.y) - min(from.y, to.y)) * (BlockWidth + m_spacing) * movePercent * dir.y + cellY
				};

				DrawCell(m_last_board[from.y][from.x], pos, BlockWidth, m_rt);
			}

			m_rt.GetDC().EndDraw();
		}

		break;
	}
	case SIZEMOVE:
	{
		// 画棋盘
		{
			m_rt.GetDC().SetTarget(m_layer_1);
			m_rt.BeginDraw();

			for (size_t y = 0; y < m_rows; y++)
			{
				for (size_t x = 0; x < m_cols; x++)
				{
					float TranslationX = boardX + m_spacing + x * (BlockWidth + m_spacing);
					float TranslationY = boardY + m_spacing + y * (BlockWidth + m_spacing);

					bool flag = false;
					for (auto& it : m_add_animation)
					{
						if (shu::vec2i(x, y) == it)
						{
							DrawCell(0, shu::vec2f(TranslationX, TranslationY), BlockWidth, m_rt);
							flag = true;
						}
					}
					if (flag == true) continue;

					if (shu::vec2i(x, y) == last_new_cell.first)
					{
						DrawCell(0, shu::vec2f(TranslationX, TranslationY), BlockWidth, m_rt);
						continue;
					}

					DrawCell(m_board[y][x], shu::vec2f(TranslationX, TranslationY), BlockWidth, m_rt);
				}
			}

			m_rt.GetDC().EndDraw();
		}

		// 画叠加层
		{
			m_rt.GetDC().SetTarget(m_layer_2);
			m_rt.BeginDraw();

			// 棋子生成
			float move_x = size_animation_clock.GetTime() / ANIMATION_SIZE_SPEED;
			float movePercent = easeInOutCirc(move_x);

			float TranslationX = boardX + m_spacing + last_new_cell.first.x * (BlockWidth + m_spacing);
			float TranslationY = boardY + m_spacing + last_new_cell.first.y * (BlockWidth + m_spacing);

			DrawCell(m_board[last_new_cell.first.y][last_new_cell.first.x], 
				shu::vec2f(TranslationX + (BlockWidth - BlockWidth * movePercent) * 0.5f, TranslationY + (BlockWidth - BlockWidth * movePercent) * 0.5f),
				BlockWidth * movePercent, m_rt);

			// 棋子合并
			if (move_x + 0.5f >= 1.0f)
				movePercent = 1.0f;
			else
				movePercent = easeInOutBack(move_x + 0.5f);

			for (auto& it : m_add_animation)
			{
				TranslationX = boardX + m_spacing + it.x * (BlockWidth + m_spacing);
				TranslationY = boardY + m_spacing + it.y * (BlockWidth + m_spacing);

				DrawCell(m_board[it.y][it.x],
					shu::vec2f(TranslationX + (BlockWidth - BlockWidth * movePercent) * 0.5f, TranslationY + (BlockWidth - BlockWidth * movePercent) * 0.5f),
					BlockWidth* movePercent, m_rt);
			}

			m_rt.GetDC().EndDraw();
		}

		break;
	}
	case GAMEOVER:
	{
		{
			m_rt.GetDC().SetTarget(m_layer_1);
			m_rt.BeginDraw();

			for (size_t y = 0; y < m_rows; y++)
			{
				for (size_t x = 0; x < m_cols; x++)
				{
					float TranslationX = boardX + m_spacing + x * (BlockWidth + m_spacing);
					float TranslationY = boardY + m_spacing + y * (BlockWidth + m_spacing);

					DrawCell(m_board[y][x], shu::vec2f(TranslationX, TranslationY), BlockWidth, m_rt);
				}
			}

			m_rt.GetDC().EndDraw();
		}

		// 叠加层
		{
			m_rt.GetDC().SetTarget(m_layer_2);
			m_rt.BeginDraw();
			m_rt.Clear(shu::color4f(shu::Color::Orange, 0.5f));

			m_rt.DrawTextCenter(L"GameOver!!\n\"R\"键重试", shu::vec2f(0.0f, 0.0f), m_rt.GetSize(), shu::Color::White, min(m_rt.GetSize().x, m_rt.GetSize().y) * 0.1f);

			m_rt.GetDC().EndDraw();
		}

		break;
	}
	default:
		break;
	}

	// 帮助
	if (shu::InputKey::GetKeyStatus(shu::Key::H).isHold)
	{
		m_rt.GetDC().SetTarget(m_layer_3);
		m_rt.BeginDraw();
		m_rt.Clear(shu::color4f(shu::Color::Orange, 0.5f));

		m_rt.DrawTextCenter(
			L"\"R\": 重试\n\"F\": 全屏\n\"H\": 帮助", 
			shu::vec2f(0.0f, 0.0f), m_rt.GetSize(), shu::Color::White, min(m_rt.GetSize().x, m_rt.GetSize().y) * 0.1f);

		m_rt.GetDC().EndDraw();
	}

	// 胜利
	if (this->isWin() == true && is_first_win == true)
	{
		if (shu::InputKey::IsAnyKeyPress() && is_first_win_frame != true) is_first_win = false;
		is_first_win_frame = false;

		m_rt.GetDC().SetTarget(m_layer_3);
		m_rt.BeginDraw();
		m_rt.Clear(shu::color4f(shu::Color::Orange, 0.5f));

		m_rt.DrawTextCenter(
			L"You Win!!\n按任意键以继续",
			shu::vec2f(0.0f, 0.0f), m_rt.GetSize(), shu::Color::White, min(m_rt.GetSize().x, m_rt.GetSize().y) * 0.1f);

		m_rt.GetDC().EndDraw();
	}

}

void Board::Render(shu::Direct2dRender& rt, bool is_black_white)
{
	if (is_black_white == false)
	{
		m_rt.DrawBitmap(m_layer_1, shu::vec2f(0.0f, 0.0f), shu::vec2f(m_layer_1->GetSize().width, m_layer_1->GetSize().height));
		m_rt.DrawBitmap(m_layer_2, shu::vec2f(0.0f, 0.0f), shu::vec2f(m_layer_2->GetSize().width, m_layer_2->GetSize().height));
		m_rt.DrawBitmap(m_layer_3, shu::vec2f(0.0f, 0.0f), shu::vec2f(m_layer_3->GetSize().width, m_layer_3->GetSize().height));
	}
	if (is_black_white == true)
	{
		pointGrayscaleEffect->SetInput(0, m_layer_1);
		m_rt.GetDC().DrawImage(pointGrayscaleEffect.Get());
		pointGrayscaleEffect->SetInput(0, m_layer_2);
		m_rt.GetDC().DrawImage(pointGrayscaleEffect.Get());
		pointGrayscaleEffect->SetInput(0, m_layer_3);
		m_rt.GetDC().DrawImage(pointGrayscaleEffect.Get());
	}

}

void Board::Move(DIR dir)
{
	if (GameStatus == GAMEOVER) return;
	if (GameStatus == MOVE) return;
	if (GameStatus == SIZEMOVE) return;

	// 记录现在的棋盘状态
	CopyBoard(m_last_board, m_board, m_rows, m_cols);

	// 移动
	m_score += move(dir, m_board, m_rows, m_cols);

	// 如果移动无效 更新游戏状态 检查是不是GameOver了
	if (IsFailMove(this->m_board, this->m_last_board, m_rows, m_cols))
	{
		GameStatus = STATIC;
		if (IsGameOver(m_board, m_rows, m_cols))
		{
			GameStatus = GAMEOVER;
		}
	}
	else
	{
		// 移动有效就随机生成一个棋子
		this->RandCreateCell(m_board, m_rows, m_cols);
	}
}

int Board::move(DIR board, int** arr, int rows, int cols)
{
	int s = 0;
	switch (board)
	{
	case UP:
		s += this->up(arr, rows, cols);
		break;
	case DOWN:
		s += this->down(arr, rows, cols);
		break;
	case LEFT:
		s += this->left(arr, rows, cols);
		break;
	case RIGHT:
		s += this->right(arr, rows, cols);
		break;
	default: break;
	}
	return s;
}

int Board::ApplyMove(int** board, int rows, int cols, const vec2i& from, const vec2i& to, const vec2i& dir, int last_add)
{
	bool add_flag = false;
	int add = 0;
	if (from == to) return 0;

	// 如果动画开着，让游戏进入移动的状态
	if (m_animation_switch)
	{
		GameStatus = MOVE;
		animation_clock.Reset();
	}

	if (board[from.y][from.x] == board[to.y][to.x])
	{
		if (last_add == board[from.y][from.x])
		{
			int temp = board[to.y - dir.y][to.x - dir.x];
			board[to.y - dir.y][to.x - dir.x] = board[from.y][from.x];
			board[from.y][from.x] = temp;
			shu::vec2i _to = { to.x - dir.x, to.y - dir.y };
			if (m_animation_switch)
				this->m_animation.push_back(std::make_pair(std::make_pair(from, _to), dir));
			return 0;
		}
		else
		{
			add = board[to.y][to.x] += 1;
			board[from.y][from.x] = 0;
		}
	}
	else
	{
		if (board[to.y][to.x] == 0)
		{
			int temp = board[to.y][to.x];
			board[to.y][to.x] = board[from.y][from.x];
			board[from.y][from.x] = temp;
			add_flag = true;
		}
		else
		{
			return 0;
		}
	}

	if (m_animation_switch)
		this->m_animation.push_back(std::make_pair(std::make_pair(from, to), dir));
	if (m_animation_switch && !add_flag)
		this->m_add_animation.push_back(to);

	return add;
}

bool Board::IsFailMove(int** board, int** last_board, int rows, int cols)
{
	for (size_t y = 0; y < rows; y++)
	{
		for (size_t x = 0; x < cols; x++)
		{
			if (board[y][x] != last_board[y][x])
			{
				return false;
			}
		}
	}
	return true;
}

bool Board::IsGameOver(int** board, int rows, int cols)
{
	static shu::vec2i dir_[4] =
	{
		{0, -1},{0, 1},{-1, 0},{1, 0}
	};

	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < cols; x++)
		{
			int val = board[y][x];
			if (val == 0) return false;
			for (int i = 0; i < 4; i++)
			{
				shu::vec2i other = { x + dir_[i].x, y + dir_[i].y };

				// 出界检测
				if (other.x >= cols || other.x < 0) continue;
				if (other.y >= rows || other.y < 0) continue;

				if (val == board[other.y][other.x]) return false;
			}
		}
	}
	return true;	// Game Over
}

void Board::Reset()
{
	this->m_score = 0;
	GameStatus = STATIC;
	is_first_win = true;
	is_first_win_frame = true;

	for (size_t y = 0; y < m_rows; y++)
		for (size_t x = 0; x < m_cols; x++)
			this->m_board[y][x] = 0;

	this->RandCreateCell(m_board, m_rows, m_cols);
}

int Board::GetSocre()
{
	return m_score;
}

void Board::Test()
{
	int i = 1;
	for (size_t y = 0; y < m_rows; y++)
		for (size_t x = 0; x < m_cols; x++)
			m_board[y][x] = i++;
}

void Board::OpenAnimation(bool flag)
{
	m_animation_switch = flag;
}

bool Board::isWin()
{
	return this->_isWin(m_board, m_rows, m_cols);
}

bool Board::_isWin(int** board, int rows, int cols)
{
	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < cols; x++)
		{
			if (board[y][x] >= 11) return true;
		}
	}

	return false;
}

void Board::RandCreateCell(int** board, int rows, int cols)
{
	int new_cell = (rand() % 100 > 90) ? 2 : 1;
	int x, y;
	do
	{
		x = rand() % cols;
		y = rand() % rows;
	} while (board[y][x] != 0);
	board[y][x] = new_cell;

	// 如果动画开着，记录下新生成的棋子位置和数字方便动画绘制
	if (m_animation_switch)
	{
		this->last_new_cell.first = { x, y };
		this->last_new_cell.second = new_cell;
	}
}

void Board::DrawCell(int num, const shu::vec2f& pos, float size, shu::Direct2dRender& rt)
{
	rt.FillRoundedRect(pos, { size, size }, size * 0.1f, this->GetColor(num));
	static wchar_t buf[32] = { 0 };
	wsprintfW(buf, L"%d", (int)pow(2, num) == 1 ? 0 : (int)pow(2, num));
	if (num != 0)
		rt.DrawTextCenter(buf, pos, { size, size }, GetTextColor(num), size * GetTextSize(num));
}

int** Board::CreateNewBoard(int rows, int cols)
{
	int** np = nullptr;

	np = new int* [rows];

	for (size_t i = 0; i < rows; i++)
		np[i] = new int[cols];

	for (size_t y = 0; y < rows; y++)
		for (size_t x = 0; x < cols; x++)
			np[y][x] = 0;

	return np;
}

void Board::DeleteBoard(int** board, int rows)
{
	for (size_t y = 0; y < rows; y++)
		delete[] board[y];
}

void Board::CopyBoard(int** board1, int** board2, int rows, int cols)
{
	for (size_t y = 0; y < rows; y++)
		for (size_t x = 0; x < cols; x++)
			board1[y][x] = board2[y][x];
}

int Board::up(int** board, int rows, int cols)
{
	int temp_score = 0;
	static int last_add = 0;
	shu::vec2i dir = { 0, -1 };
	shu::vec2i begin_pos = { 0, 0 };
	shu::vec2i end_pos = { 0, 0 };
	for (int x = 0; x <= cols - 1; x++)
	{
		last_add = 0;
		for (int y = 1; y <= rows - 1; y++)
		{
			if (board[y][x] == 0) continue;
			begin_pos = { x, y };
			end_pos = { x , y - 1 };
			for (int my = y - 1; my >= 0; my--)
			{
				if (board[my][x] != board[y][x] && board[my][x] != 0)
					break;
				end_pos = { x, my };
			}

			last_add = ApplyMove(board, rows, cols, begin_pos, end_pos, dir, last_add);
			temp_score += last_add;
		}
	}
	return temp_score;
}

int Board::down(int** board, int rows, int cols)
{
	int temp_score = 0;
	static int last_add = 0;
	shu::vec2i dir = { 0, 1 };
	shu::vec2i begin_pos = { 0, 0 };
	shu::vec2i end_pos = { 0, 0 };
	for (int x = 0; x <= cols - 1; x++)
	{
		last_add = 0;
		for (int y = rows - 2; y >= 0; y--)
		{
			if (board[y][x] == 0) continue;
			begin_pos = { x, y };
			end_pos = { x , y + 1 };
			for (int my = y + 1; my <= rows - 1; my++)
			{
				if (board[my][x] != board[y][x] && board[my][x] != 0)
					break;
				end_pos = { x, my };
			}

			last_add = ApplyMove(board, rows, cols, begin_pos, end_pos, dir, last_add);
			temp_score += last_add;
		}
	}
	return temp_score;
}

int Board::left(int** board, int rows, int cols)
{
	int temp_score = 0;
	static int last_add = 0;
	shu::vec2i dir = { -1, 0 };
	shu::vec2i begin_pos = { 0, 0 };
	shu::vec2i end_pos = { 0, 0 };
	for (int y = 0; y < rows; y++)
	{
		last_add = 0;
		for (int x = 1; x < cols; x++)
		{
			if (board[y][x] == 0) continue;
			begin_pos = { x, y };
			end_pos = { x - 1, y };
			for (int mx = x - 1; mx >= 0; mx--)
			{
				if (board[y][mx] != board[y][x] && board[y][mx] != 0)
					break;
				end_pos = { mx, y };
			}

			last_add = ApplyMove(board, rows, cols, begin_pos, end_pos, dir, last_add);
			temp_score += last_add;

			PrintfBoard(m_board, m_rows, m_cols);
		}
	}
	return temp_score;
}

int Board::right(int** board, int rows, int cols)
{
	int temp_score = 0;
	static int last_add = 0;
	shu::vec2i dir = { 1, 0 };
	shu::vec2i begin_pos = { 0, 0 };
	shu::vec2i end_pos = { 0, 0 };
	for (int y = 0; y < rows; y++)
	{
		last_add = 0;
		for (int x = cols - 2; x >= 0; x--)
		{
			if (board[y][x] == 0) continue;
			begin_pos = { x, y };
			end_pos = { x + 1, y };
			for (int mx = x + 1; mx <= cols - 1; mx++)
			{
				if (board[y][mx] != board[y][x] && board[y][mx] != 0)
					break;
				end_pos = { mx, y };
			}

			last_add = ApplyMove(board, rows, cols, begin_pos, end_pos, dir, last_add);
			temp_score += last_add;
		}
	}
	return temp_score;
}

shu::color4f& Board::GetColor(int num)
{
	static shu::color4f color[13] =
	{
		shu::color4f(238. / 255., 228. / 255., 218 / 255., 0.35f),// 0
		shu::color4f(238. / 255., 228. / 255., 218. / 255.),	// 2
		shu::color4f(237. / 255., 224. / 255., 200. / 255.),	// 4
		shu::color4f(242. / 255., 177. / 255., 121. / 255.),	// 8
		shu::color4f(245. / 255., 149. / 255., 99. / 255.),		// 16
		shu::color4f(246. / 255., 124. / 255., 95. / 255.),		// 32
		shu::color4f(246. / 255., 94. / 255., 59. / 255.),		// 64
		shu::color4f(237. / 255., 207. / 255., 114. / 255.),	// 128
		shu::color4f(237. / 255., 240. / 255., 97. / 255.),		// 256
		shu::color4f(237. / 255., 200. / 255., 80. / 255.),		// 512
		shu::color4f(237. / 255., 197. / 255., 63. / 255.),		// 1024
		shu::color4f(237. / 255., 194. / 255., 46. / 255.),		// 2048
		shu::color4f(60. / 255., 58. / 255., 50. / 255.),		// > 2048
	};

	if (num >= 12) return color[12];

	return color[num];
}

shu::color4f Board::GetTextColor(int num)
{
	if (num >= 0 && num <= 2) return shu::color4f(0.0f, 0.0f, 0.0f, 1.0f);
	if (num >= 3 && num <= 6) return shu::color4f(1.0f, 1.0f, 1.0f, 1.0f);
	if (num >= 7 && num <= 11) return shu::color4f(0.0f, 0.0f, 0.0f, 1.0f);
	if (num >= 12) return shu::color4f(1.0f, 1.0f, 1.0f, 1.0f);
	return shu::color4f(1.0f, 1.0f, 1.0f, 1.0f);
}

float Board::GetTextSize(int num)
{
	num = pow(2, num);
	if (num >= 0 && num <= 99)
	{
		return 0.5f;
	}
	else if (num >= 100 && num <= 999)
	{
		return 0.45f;
	}
	else if (num >= 1000 && num <= 9999)
	{
		return 0.35f;
	}
	else if (num >= 10000 && num <= 99999)
	{
		return 0.25f;
	}
	else if (num >= 100000 && num <= 999999)
	{
		return 0.1f;
	}
	else if (num >= 1000000 && num <= 9999999)
	{
		return 0.1f;
	}
	return 0.1f;
}
