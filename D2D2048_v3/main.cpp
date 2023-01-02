#include <iostream>
#include "src/Board.h"
#include "src/NetWorkUpdata.h"
#include "include/GameEngine/GameEngine.h"

#ifndef _DEBUG
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )	// 隐藏控制台
#endif // _DEBUG


// 该游戏的版本，如果检测到Network的版本号比它大，就该更新了
#define VERSION 4

class snow
{
public:
	snow(shu::Direct2dRender& rt)
		: m_rt(rt)
	{
		// 创建位图
		D2D1_BITMAP_PROPERTIES1 d2d1_bitmap_def = {};
		d2d1_bitmap_def.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		d2d1_bitmap_def.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
		d2d1_bitmap_def.dpiX = USER_DEFAULT_SCREEN_DPI;
		d2d1_bitmap_def.dpiY = USER_DEFAULT_SCREEN_DPI;
		d2d1_bitmap_def.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET;
		rt.GetDC().CreateBitmap(
			D2D1::SizeU(100, 100),
			0, 0,
			d2d1_bitmap_def,
			&m_bitmap);
		rt.GetDC().CreateBitmap(
			D2D1::SizeU(100, 100),
			0, 0,
			d2d1_bitmap_def,
			&m_bitmap2);

		// 创建高斯模糊特效，为了让雪花更有层次感，雪花越小模糊越强
		rt.GetDC().CreateEffect(CLSID_D2D1GaussianBlur, &m_GaussianBlur);

		this->Reset();
	}
	~snow() {}

	void Reset()
	{
		float r = (rand() % 10 + 5);
		m_radius = r * min(m_rt.GetSize().x, m_rt.GetSize().y) * 0.001f;
		m_pos = shu::vec2f(rand() % ((int)m_rt.GetSize().x + 1), -100.0f);
		m_speed = shu::vec2f(rand() % 50 - 25, rand() % 50 + 50 * min(m_rt.GetSize().x, m_rt.GetSize().y) * 0.001f);

		m_life = ((rand() % 100) / 100.0f) + 0.1f;
		m_white = 0.0f;
		m_clock.Reset();

		m_rt.GetDC().SetTarget(m_bitmap.Get());
		m_rt.GetDC().BeginDraw();
		m_rt.GetDC().Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
		m_rt.FillCircle(shu::vec2f(50.0f, 50.0f), m_radius, shu::color4f(shu::Color::White));
		m_rt.GetDC().EndDraw();

		m_GaussianBlur->SetInput(0, m_bitmap.Get());
		m_GaussianBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, (14 - r) * 0.8f);

		m_rt.GetDC().SetTarget(m_bitmap2.Get());
		m_rt.GetDC().BeginDraw();
		m_rt.GetDC().Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
		m_rt.GetDC().DrawImage(m_GaussianBlur.Get());
		m_rt.GetDC().EndDraw();

		m_rt.SetRenderTargetToWindow();
	}

	void Updata(float dt)
	{
		m_pos += m_speed * dt;

		if (m_clock.GetTime() >= 2.0f)
		{
			m_white -= m_life * dt;
			if (m_white <= 0.0f) this->Reset();
		}
		else
		{
			float move = m_clock.GetTime() / 2.0f;
			m_white = easeInOutBack(move);
		}
	}
	void Render(shu::Direct2dRender& rt)
	{
		rt.GetDC().DrawBitmap(
			m_bitmap2.Get(),
			D2D1::RectF(m_pos.x, m_pos.y, m_pos.x + m_bitmap2->GetSize().width, m_pos.y + m_bitmap2->GetSize().height),
			m_white,
			D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
	}

private:
	shu::Direct2dRender& m_rt;
	ComPtr<ID2D1Bitmap1> m_bitmap;
	ComPtr<ID2D1Bitmap1> m_bitmap2;
	ComPtr<ID2D1Effect> m_GaussianBlur;	// 高斯模糊特效

	shu::vec2f m_pos;
	shu::vec2f m_speed;
	float m_radius = 0.0f;
	float m_life = 0.0f;

	Clock m_clock;
	float m_white = 0.0f;
};

class MyGame : public shu::GameEngine
{
public:
	virtual bool OnUserCreate() override
	{ 
		std::system("chcp 65001");

		// 初始化棋盘
		board = new Board(m_rt);
		board->InitBoard(4, 4);

		// 初始化Network模块
		m_network = new NetWorkUpdata(is_ok, m_json_data);
		m_network_thread = std::thread([&]() { m_network->Run(); });

		// 创建渐变画笔
		this->CreateLinearBrush(shu::vec2f(512.0f, 512.0f));

		m_rt.SetFullscreenState(Full);

		return true;
	};

	virtual bool OnUserUpdata(float dt) override
	{
		static float all_dt;
		all_dt += dt;

		// 检查网络请求线程是否请求到数据
		static bool is_first = true;
		if (is_ok == true)
		{
			if (is_first == true)
			{
				// 是否有更新
				if (m_json_data["new-version"].get<int>() > VERSION)
				{
					auto& logs = m_json_data["log"];
					std::wstringstream ss;
					for (auto& log : logs.items())
					{
						std::string str = std::string(log.value());
						std::wstring wstr = to_wide_string(str);
						ss << wstr << std::endl;
					}
					if (::MessageBoxW(NULL, ss.str().c_str(), L"有可用更新 是否跳转更新？", MB_OKCANCEL) == IDOK)
					{
						std::stringstream link;
						link << "start " << std::string(m_json_data["link"]);
						std::string str = link.str();
						system(str.c_str());
					}
				}
				// 更新一下背景颜色
				this->CreateLinearBrush(m_rt.GetSize());

				is_show_snow = m_json_data["snow"];
				is_black_white = m_json_data["black-white"];

				if (is_show_snow == true)
				{
					// 初始化snow类
					for (size_t i = 0; i < 100; i++)
						m_snows.push_back(new snow(m_rt));
				}

				is_first = false;
			}
		}

		if (shu::InputKey::GetKeyStatus(shu::Key::W).isPress) board->Move(UP);
		if (shu::InputKey::GetKeyStatus(shu::Key::S).isPress) board->Move(DOWN);
		if (shu::InputKey::GetKeyStatus(shu::Key::A).isPress) board->Move(LEFT);
		if (shu::InputKey::GetKeyStatus(shu::Key::D).isPress) board->Move(RIGHT);

		if (shu::InputKey::GetKeyStatus(shu::Key::R).isPress) board->Reset();
		if (shu::InputKey::GetKeyStatus(shu::Key::T).isPress) board->Test();
		if (shu::InputKey::GetKeyStatus(shu::Key::F).isPress)
		{
			Full = !Full;
			m_rt.SetFullscreenState(Full);
		}

		last_win_size = win_size;
		win_size = shu::vec2i(this->m_rt.GetSize().x, this->m_rt.GetSize().y);
		// 当窗口大小改变时
		if (win_size != last_win_size)
		{
			board->ChangeSize(win_size);

			this->CreateLinearBrush(m_rt.GetSize());
		}

		// 更新棋盘
		board->Updata(dt);

		// 更新雪花
		if (is_show_snow == true)
		{
			for (auto& it : m_snows)
				it->Updata(dt);
		}

		m_rt.SetRenderTargetToWindow();

		// 绘制
		m_rt.BeginDraw();

		m_rt.FillRect(shu::vec2f(0.0f, 0.0f), m_rt.GetSize(), m_pLinearGradientBrush.Get());

		if (is_show_snow == true)
		{
			for (auto& it : m_snows)
				it->Render(m_rt);
		}

		board->Render(m_rt, false);

		m_rt.EndDraw();

		return true; 
	};

	virtual bool OnDisCreate() override
	{
		delete board;
		for (auto& it : m_snows) delete it;
		if (m_network_thread.joinable()) m_network_thread.join();
		delete m_network;
		return true;
	}

	void CreateLinearBrush(const shu::vec2f& screen_size)
	{
		m_pLinearGradientBrush.Reset();

		ID2D1GradientStopCollection* pGradientStops = NULL;
		
		D2D1::ColorF beginC = TO_D2D1_COLORF(187, 173, 160, 255);
		D2D1::ColorF endC = TO_D2D1_COLORF(187, 173, 160, 255);

		if (is_ok == true)
		{
			beginC.r = m_json_data["up-color"][0].get<float>() / 255.;
			beginC.g = m_json_data["up-color"][1].get<float>() / 255.;
			beginC.b = m_json_data["up-color"][2].get<float>() / 255.;
			beginC.a = m_json_data["up-color"][3].get<float>() / 255.;

			endC.r = m_json_data["down-color"][0].get<float>() / 255.;
			endC.g = m_json_data["down-color"][1].get<float>() / 255.;
			endC.b = m_json_data["down-color"][2].get<float>() / 255.;
			endC.a = m_json_data["down-color"][3].get<float>() / 255.;
		}

		D2D1_GRADIENT_STOP gradientStops[2];
		gradientStops[0].color = beginC;
		gradientStops[0].position = 0.0f;
		gradientStops[1].color = endC;
		gradientStops[1].position = 1.0f;

		m_rt.GetDC().CreateGradientStopCollection(
			gradientStops,
			2,
			D2D1_GAMMA_2_2,
			D2D1_EXTEND_MODE_CLAMP,
			&pGradientStops
		);
		m_rt.GetDC().CreateLinearGradientBrush(
			D2D1::LinearGradientBrushProperties(
				D2D1::Point2F(screen_size.x * 0.5f, 0.0f),
				D2D1::Point2F(screen_size.x * 0.5f, screen_size.y)),
			pGradientStops,
			&m_pLinearGradientBrush
		);

		pGradientStops->Release();
	}


private:

	ComPtr<ID2D1LinearGradientBrush> m_pLinearGradientBrush;	// 线性渐变画笔

	std::vector<snow*> m_snows;   // 雪花背景

	std::atomic<bool> is_ok = false;	// Http请求是否完成
	bool is_show_snow = false;	// 是否显示雪花
	bool is_black_white = false;	// 是否黑白
	bool Full = false;

	json m_json_data;
	NetWorkUpdata* m_network = nullptr;
	std::thread m_network_thread;

	Board* board = nullptr;

	shu::vec2i win_size = shu::vec2i(0, 0);
	shu::vec2i last_win_size = shu::vec2i(0, 0);
};

int main()
{
	srand(time(NULL));
	MyGame ge;
	if (ge.Init(L"Game", L"2048", { 600, 600 }))
		ge.Start();

	return 0;
}
