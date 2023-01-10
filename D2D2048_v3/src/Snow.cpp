#include "Snow.h"

Snow::Snow(shu::Direct2dRender& rt)
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

Snow::~Snow()
{
}

void Snow::Reset()
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

void Snow::Updata(float dt)
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

void Snow::Render(shu::Direct2dRender& rt)
{
	rt.GetDC().DrawBitmap(
		m_bitmap2.Get(),
		D2D1::RectF(m_pos.x, m_pos.y, m_pos.x + m_bitmap2->GetSize().width, m_pos.y + m_bitmap2->GetSize().height),
		m_white,
		D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
}
