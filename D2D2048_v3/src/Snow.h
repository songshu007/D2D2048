#pragma once
#include "../include/GameEngine/GameEngine.h"
#include "../src/Board.h"

class Snow
{
public:
	Snow(shu::Direct2dRender& rt);
	~Snow();

	void Reset();

	void Updata(float dt);
	void Render(shu::Direct2dRender& rt);

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

