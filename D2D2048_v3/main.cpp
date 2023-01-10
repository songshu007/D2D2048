#include "main.h"

class MyGame : public shu::GameEngine
{
public:
	virtual bool OnUserCreate() override
	{ 
		std::system("chcp 65001");
		// 设置菜单
		auto menu = ::LoadMenuW(this->m_hInstance, MAKEINTRESOURCE(IDR_MENU1));
		SetMenu(this->m_hwnd, menu);

		// 初始化棋盘
		GameData::Get().board = new Board(m_rt);
		GameData::Get().board->InitBoard(GameData::Get().Board_Width, GameData::Get().Board_Height);

		GetComputerNameA(PCName, &size);

		// 初始化线程池
		myTask = new Thread_pool(2);
		// 请求json配置数据
		myTask->AddTask([this]() {
		httplib::Client cli("http://songshu007.gitee.io");
		httplib::Result res = cli.Get("/backstage/2048/");
		if (res)
		{
			int begin = res->body.find("[####");
			int end = res->body.find("####]");
			if (begin >= 0 && end >= 0)
			{
				std::string data = res->body.substr(begin + 5, end - begin - 5);
				std::cout << data << std::endl;
				this->m_json_data = json::parse(data);
				this->is_json_ok = true;
				std::cout << "[" << std::this_thread::get_id() << "]: get json_data success!" << std::endl;
			}
		}
		else
		{
			this->is_json_ok = false;
			std::cout << "[" << std::this_thread::get_id() << "]: get json_data failed!" << std::endl;
		}
		});

		// 发送上线消息到服务器
		myTask->AddTask([this]() {
		std::stringstream ss;
		ss << "/backstage/2048/in/" << PCName;
		httplib::Client cli("http://117.50.181.42:4000");
		if (cli.Get(ss.str()))
			std::cout << "[" << std::this_thread::get_id() << "]: send in message success!" << std::endl;
		else
			std::cout << "[" << std::this_thread::get_id() << "]: send in message failed!" << std::endl;
		});

		// 创建渐变画笔
		this->CreateLinearBrush(shu::vec2f(512.0f, 512.0f));

		m_rt.SetFullscreenState(full_screen);

		return true;
	};

	virtual bool OnUserUpdata(float dt) override
	{
		static float all_dt;
		all_dt += dt;

		// 检查任务队列线程是否请求到json配置数据
		static bool is_first = true;
		if (this->is_json_ok == true)
		{
			if (is_first == true)
			{
				is_first = false;

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
						m_snows.push_back(new Snow(m_rt));
				}
			}
		}

		if (shu::InputKey::GetKeyStatus(shu::Key::W).isPress || shu::InputKey::GetKeyStatus(shu::Key::UP).isPress) GameData::Get().board->Move(UP);
		if (shu::InputKey::GetKeyStatus(shu::Key::S).isPress || shu::InputKey::GetKeyStatus(shu::Key::DOWN).isPress) GameData::Get().board->Move(DOWN);
		if (shu::InputKey::GetKeyStatus(shu::Key::A).isPress || shu::InputKey::GetKeyStatus(shu::Key::LEFT).isPress) GameData::Get().board->Move(LEFT);
		if (shu::InputKey::GetKeyStatus(shu::Key::D).isPress || shu::InputKey::GetKeyStatus(shu::Key::RIGHT).isPress) GameData::Get().board->Move(RIGHT);
#ifdef _DEBUG
		if (shu::InputKey::GetKeyStatus(shu::Key::T).isPress) GameData::Get().board->Test();
#endif // _DEBUG
		

		if (shu::InputKey::GetKeyStatus(shu::Key::R).isPress)
		{
			GameData::Get().board->Reset();
			m_is_win = false;
			m_last_is_win = false;
		}
		if (shu::InputKey::GetKeyStatus(shu::Key::F).isPress)
		{
			full_screen = !full_screen;
			m_rt.SetFullscreenState(full_screen);
		}
		if (shu::InputKey::GetKeyStatus(shu::Key::ESCAPE).isPress)
		{
			if (full_screen == true)
			{
				full_screen = false;
				m_rt.SetFullscreenState(false);
			}
		}

		m_last_is_win = m_is_win;
		m_is_win = GameData::Get().board->isWin();

		// send win message only once!!
		if (m_last_is_win == false && m_is_win == true)
		{
			// 发送胜利的消息到服务器
			myTask->AddTask([this]() {
			std::stringstream ss;
			ss << "/backstage/2048/win/" << PCName;
			httplib::Client cli("http://117.50.181.42:4000");
			if (cli.Get(ss.str()))
				std::cout << "[" << std::this_thread::get_id() << "]: send win message success!" << std::endl;
			else
				std::cout << "[" << std::this_thread::get_id() << "]: send win message failed!" << std::endl;
			});
		}

		last_win_size = win_size;
		win_size = shu::vec2i(this->m_rt.GetSize().x, this->m_rt.GetSize().y);
		// 当窗口大小改变时
		if (win_size != last_win_size)
		{
			GameData::Get().board->ChangeSize(win_size);
			this->CreateLinearBrush(m_rt.GetSize());
		}

		// 更新棋盘
		GameData::Get().board->Updata(dt);

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
		GameData::Get().board->Render(m_rt, false);
		m_rt.EndDraw();

		return true; 
	};

	virtual bool OnMenuUpdata(WORD options)
	{ 
		if (options == ID_ABOUT)
		{
			DialogBoxW(this->m_hInstance, MAKEINTRESOURCEW(IDD_ABOUT), this->m_hwnd, (DLGPROC)Dlg_About);
		}
		if (options == ID_SIZE)
		{
			DialogBoxW(this->m_hInstance, MAKEINTRESOURCEW(IDD_SIZE), this->m_hwnd, (DLGPROC)Dlg_Size);
		}
		if (options == ID_AGAIN)
		{
			GameData::Get().board->Reset();
			m_is_win = false;
			m_last_is_win = false;
		}
		if (options == ID_FULL)
		{
			full_screen = !full_screen;
			m_rt.SetFullscreenState(full_screen);
		}

		return true; 
	};

	virtual bool OnDisCreate() override
	{
		// 发送下线消息到服务器
		myTask->AddTask([this]() {
		std::stringstream ss;
		ss << "/backstage/2048/out/" << PCName;
		httplib::Client cli("http://117.50.181.42:4000");
		if (cli.Get(ss.str()))
			std::cout << "[" << std::this_thread::get_id() << "]: send out message success!" << std::endl;
		else
			std::cout << "[" << std::this_thread::get_id() << "]: send out message failed!" << std::endl;
		});

		// 关掉任务队列（阻塞等待所有任务完成）
		myTask->Close();
		delete myTask;

		for (auto& it : m_snows) delete it;
		m_snows.clear();

		return true;
	}

	void CreateLinearBrush(const shu::vec2f& screen_size)
	{
		m_pLinearGradientBrush.Reset();

		ID2D1GradientStopCollection* pGradientStops = NULL;
		
		D2D1::ColorF beginC = TO_D2D1_COLORF(187, 173, 160, 255);
		D2D1::ColorF endC = TO_D2D1_COLORF(187, 173, 160, 255);

		if (this->is_json_ok == true)
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

public:

	ComPtr<ID2D1LinearGradientBrush> m_pLinearGradientBrush;  // 线性渐变画笔
	std::vector<Snow*> m_snows;       // 雪花背景

	Thread_pool* myTask = nullptr;    // 线程池，用来进行网络请求

	std::atomic<bool> is_json_ok = false; // 请求json数据是否完成
	json m_json_data;               // json配置数据
	bool is_show_snow = false;      // 是否显示雪花
	bool is_black_white = false;    // 是否黑白
	bool full_screen = false;       // 是否全屏
	CHAR PCName[255];	            // 玩家的电脑名称
	unsigned long size = 255;

	bool m_is_win = false;
	bool m_last_is_win = false;

	shu::vec2i win_size = shu::vec2i(0, 0);
	shu::vec2i last_win_size = shu::vec2i(0, 0);
};

int main()
{
	srand(time(NULL));
	MyGame ge;

	// 初始化
	// 类名，窗口名称，窗口大小，样式（可选）
	if (ge.Init(L"Game", L"2048", { 600, 600 }, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN))
		ge.Start();

	return 0;
}
