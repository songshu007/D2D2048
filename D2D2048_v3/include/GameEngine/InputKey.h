#pragma once
#include <iostream>

namespace shu
{
	struct KeyStatus
	{
		bool isPress = false;	// 是否按下
		bool isHold = false;	// 是否按着
		bool isRelease = false;	// 是否释放
	};

	// 按键对应的键码，详情：https://learn.microsoft.com/zh-cn/windows/win32/inputdev/virtual-key-codes
	enum class Key : uint32_t
	{
		LBUTTON = 0x01,
		RBUTTON,
		CANCEL,
		MBUTTON,
		SBUTTON1,
		SBUTTON2,
		BACK = 0x08,
		TAB,
		CLEAR = 0x0C,
		RETURN,
		SHIFT = 0x10,
		CONTROL,
		ESCAPE = 0x1B,
		// VK_MENU ...
		SPACE = 0x20,
		LEFT = 0x25,
		UP, RIGHT, DOWN,
		A = 0x41,
		B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z
	};

	class InputKey
	{
	public:
		InputKey();
		~InputKey();

		static void UpdataKeyStatus(uint32_t key, bool status);
		static void Updata();
		static KeyStatus& GetKeyStatus(Key key);
		// 是否有任意一个键按下
		static bool IsAnyKeyPress();

	private:
		static InputKey* ptr;

		// 按键的状态
		bool m_KeyNewState[256] = { 0 };
		bool m_KeyOldState[256] = { 0 };
		KeyStatus m_KeyboardState[256];

		bool m_IsAnyKeyPress = false;
	};
}

