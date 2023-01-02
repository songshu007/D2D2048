#include "Render.h"

using namespace Microsoft::WRL;

bool shu::Direct2dRender::Init(HWND hwnd)
{
	HRESULT hr = S_OK;

	this->m_hwnd = hwnd;

	// 初始化 COM 对象
	hr = CoInitialize(NULL);
	if (FAILED(hr)) return false;

	// 创建 D2D Factory（多线程工厂）
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory1), (void**)&m_d2dFactory);
	if (FAILED(hr)) return false;

	// 创建 D3D Device 和 D3D DeviceContext
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};
	hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&m_d3dDevice,
		NULL,
		&m_d3dDeviceContext
	);
	if (FAILED(hr)) return false;

	// 创建 DXGI Device
	hr = m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&m_dxgiDevice);
	if (FAILED(hr)) return false;

	// 创建 D2D Device
	hr = m_d2dFactory->CreateDevice(m_dxgiDevice.Get(), &m_d2dDevice);
	if (FAILED(hr)) return false;

	// 创建 D2D DeviceContext
	hr = m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_d2dDeviceContext);
	if (FAILED(hr)) return false;

	// 创建 DXGI Factory
	ComPtr<IDXGIAdapter> dxgiAdapter;
	hr = m_dxgiDevice->GetAdapter(&dxgiAdapter);
	if (FAILED(hr)) return false;

	hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&m_dxgiFactory));
	if (FAILED(hr)) return false;

	// 创建 DXGI SwapChain
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
	memset(&swapChainDesc, 0, sizeof(DXGI_SWAP_CHAIN_DESC1));
	swapChainDesc.Width = 0;                           // use automatic sizing
	swapChainDesc.Height = 0;
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // this is the most common swapchain format
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;                // don't use multi-sampling
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;                     // use double buffering to enable flip
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // all apps must use this SwapEffect
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = 0;

	hr = m_dxgiFactory->CreateSwapChainForHwnd(
		m_d3dDevice.Get(),
		this->m_hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&m_dxgiSwapChain
	);
	if (FAILED(hr))
	{
		return false;
	}

	hr = this->m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_Brush);
	if (FAILED(hr)) return false;

	// 创建 RT
	if (!this->CreateRenderTarget()) return false;

	// 创建 Dwrite
	if (!this->createDwrite()) return false;

	// 创建 WIC
	if (!this->createWic()) return false;

	// 设置抗锯齿
	m_d2dDeviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

	m_d2dFactory.As(&m_D2DMultithread);

	return true;
}

void shu::Direct2dRender::BeginDraw()
{
	this->m_d2dDeviceContext->BeginDraw();
}

void shu::Direct2dRender::Clear(const color4f& color)
{
	this->m_d2dDeviceContext->Clear(D2D1::ColorF(color.r, color.g, color.b, color.a));
}

void shu::Direct2dRender::FillRoundedRect(const vec2f& pos, const vec2f& size, float radius, const color4f& color)
{
	this->m_Brush->SetColor(D2D1::ColorF(color.r, color.g, color.b, color.a));
	this->m_d2dDeviceContext->FillRoundedRectangle(
		D2D1::RoundedRect( D2D1::RectF( pos.x, pos.y, pos.x + size.x, pos.y + size.y ), radius, radius ),
		m_Brush.Get());
}

void shu::Direct2dRender::DrawRect(const vec2f& pos, const vec2f& size, const color4f& color, float strokeWidth)
{
	this->m_Brush->SetColor(D2D1::ColorF(color.r, color.g, color.b, color.a));
	this->m_d2dDeviceContext->DrawRectangle(
		D2D1::RectF( pos.x, pos.y, pos.x + size.x, pos.y + size.y ),
		m_Brush.Get(), strokeWidth);
}

void shu::Direct2dRender::FillRect(const vec2f& pos, const vec2f& size, ID2D1Brush* brush)
{
	this->m_d2dDeviceContext->FillRectangle(
		D2D1::RectF( pos.x, pos.y, pos.x + size.x, pos.y + size.y ),
		brush);
}

void shu::Direct2dRender::FillRect(const vec2f& pos, const vec2f& size, const color4f& color)
{
	this->m_Brush->SetColor(D2D1::ColorF(color.r, color.g, color.b, color.a));
	this->m_d2dDeviceContext->FillRectangle(
		D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y),
		m_Brush.Get());
}

void shu::Direct2dRender::FillCircle(const vec2f& pos, float radius, const color4f& color)
{
	this->m_Brush->SetColor(D2D1::ColorF(color.r, color.g, color.b, color.a));
	this->m_d2dDeviceContext->FillEllipse(
		D2D1::Ellipse(D2D1::Point2F(pos.x, pos.y), radius, radius), 
		m_Brush.Get());
}

void shu::Direct2dRender::FillCircle(const vec2f& pos, float radius, ID2D1Brush* brush)
{
	this->m_d2dDeviceContext->FillEllipse(
		D2D1::Ellipse(D2D1::Point2F(pos.x, pos.y), radius, radius),
		brush);
}

void shu::Direct2dRender::DrawLine(const vec2f& p1, const vec2f& p2, const color4f& color, float strokeWidth)
{
	this->m_Brush->SetColor(D2D1::ColorF(color.r, color.g, color.b, color.a));
	this->m_d2dDeviceContext->DrawLine(
		D2D1::Point2F( p1.x, p1.y ),
		D2D1::Point2F( p2.x, p2.y ),
		m_Brush.Get(),
		strokeWidth);
}

void shu::Direct2dRender::DrawTextCenter(const std::wstring& text, const vec2f& pos, const vec2f& rect, const color4f& color, float Textsize)
{
	HRESULT hr = S_OK;

	if (Textsize <= 0) return;

	this->m_Brush->SetColor(D2D1::ColorF(color.r, color.g, color.b, color.a));
	if (FAILED(hr)) return;

	hr = this->m_pDWriteFactory->CreateTextFormat(
		L"微软雅黑",                
		NULL,                     
		DWRITE_FONT_WEIGHT_SEMI_BOLD,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		Textsize,
		L"en-us",
		&this->m_pTextFormat
	);
	if (FAILED(hr)) return;

	hr = m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	if (FAILED(hr)) return;
	hr = m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	if (FAILED(hr)) return;

	hr = this->m_pDWriteFactory->CreateTextLayout(
		text.c_str(),
		(UINT32)text.length(),
		this->m_pTextFormat.Get(),
		rect.x, rect.y,
		&this->m_pTextLayout
	);
	if (FAILED(hr)) return;

	this->m_d2dDeviceContext->DrawTextLayout({ pos.x, pos.y }, this->m_pTextLayout.Get(), m_Brush.Get());
}

void shu::Direct2dRender::DrawText(const std::wstring& text, const vec2f& pos, const vec2f& rect, const color4f& color, float Textsize)
{
	HRESULT hr = S_OK;

	if (Textsize <= 0) return;

	this->m_Brush->SetColor(D2D1::ColorF(color.r, color.g, color.b, color.a));
	if (FAILED(hr)) return;

	hr = this->m_pDWriteFactory->CreateTextFormat(
		L"微软雅黑",
		NULL,
		DWRITE_FONT_WEIGHT_SEMI_BOLD,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		Textsize,
		L"en-us",
		&this->m_pTextFormat
	);
	if (FAILED(hr)) return;

	hr = this->m_pDWriteFactory->CreateTextLayout(
		text.c_str(),
		(UINT32)text.length(),
		this->m_pTextFormat.Get(),
		rect.x, rect.y,
		&this->m_pTextLayout
	);
	if (FAILED(hr)) return;

	this->m_d2dDeviceContext->DrawTextLayout({ pos.x, pos.y }, this->m_pTextLayout.Get(), m_Brush.Get());
}

void shu::Direct2dRender::DrawBitmap(ID2D1Bitmap1* bitmap, const vec2f& pos, const vec2f& size)
{
	this->m_d2dDeviceContext->DrawBitmap(bitmap, D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), 1.0f, D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
}

void shu::Direct2dRender::EndDraw()
{
	HRESULT hr = S_OK;

	m_D2DMultithread->Enter();
	hr = this->m_d2dDeviceContext->EndDraw();
	hr = this->m_dxgiSwapChain->Present(1, 0);
	m_D2DMultithread->Leave();
}

bool shu::Direct2dRender::LoadBitmapFromFile(std::wstring file, ID2D1Bitmap1** Bitmap)
{
	HRESULT hr = S_OK;
	ComPtr<IWICBitmapDecoder> WicBitmap;
	ComPtr<IWICBitmapFrameDecode> WicFrameDec;

	hr = this->m_wicImgFactory->CreateDecoderFromFilename(file.c_str(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &WicBitmap);
	if (FAILED(hr)) return false;

	hr = WicBitmap->GetFrame(0, &WicFrameDec);
	if (FAILED(hr)) return false;

	WICPixelFormatGUID Format;
	ComPtr<IWICBitmapSource> WicBitmapSource;
	WicFrameDec->GetPixelFormat(&Format);
	if (Format != GUID_WICPixelFormat32bppPBGRA)
	{
		// 需要转换格式
		ComPtr<IWICFormatConverter> wic_converter;
		hr = this->m_wicImgFactory->CreateFormatConverter(&wic_converter);
		if (FAILED(hr)) return false;

		hr = wic_converter->Initialize(
			WicFrameDec.Get(),
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.0f,
			WICBitmapPaletteTypeCustom);
		if (FAILED(hr)) return false;

		wic_converter.As(&WicBitmapSource);
	}
	else
	{
		WicFrameDec.As(&WicBitmapSource);
	}
	
	hr = this->m_d2dDeviceContext->CreateBitmapFromWicBitmap(WicBitmapSource.Get(), Bitmap);
	if (FAILED(hr)) return false;

	return true;
}

void shu::Direct2dRender::ChangeSize(uint32_t width, uint32_t height)
{
	if (width == 0 || height == 0 || !m_dxgiSwapChain)
	{
		return;
	}
	HRESULT hr = S_OK;

	m_size.x = (float)width; m_size.y = (float)height;

	m_D2DMultithread->Enter();
	this->m_d2dDeviceContext->SetTarget(NULL);
	// 解除所有与 SwapChain 的绑定
	if (this->m_d3dDeviceContext)
	{
		this->m_d3dDeviceContext->ClearState();
		this->m_d3dDeviceContext->Flush();
	}

	this->m_dxgiSurface.Reset();
	this->m_d2dTarget.Reset();

	hr = this->m_dxgiSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

	this->CreateRenderTarget();
	this->m_d2dDeviceContext->SetTarget(this->m_d2dTarget.Get());
	m_D2DMultithread->Leave();
}

void shu::Direct2dRender::SetFullscreenState(bool flag)
{
	m_dxgiSwapChain->SetFullscreenState(flag, NULL);
}

void shu::Direct2dRender::SetRenderTargetToWindow()
{
	m_D2DMultithread->Enter();
	this->m_d2dDeviceContext->SetTarget(this->m_d2dTarget.Get());
	m_D2DMultithread->Leave();
}

ID2D1DeviceContext& shu::Direct2dRender::GetDC()
{
	return *m_d2dDeviceContext.Get();
}

shu::vec2f& shu::Direct2dRender::GetSize()
{
	return m_size;
}

ID2D1Bitmap1* shu::Direct2dRender::GetRenderTarget()
{
	// 此函数不是线程安全的（待改进）
	return m_d2dTarget.Get();
}

ID2D1Factory1* shu::Direct2dRender::GetD2dFactory1()
{
	return m_d2dFactory.Get();
}

ID2D1SolidColorBrush* shu::Direct2dRender::GetBrush()
{
	return m_Brush.Get();
}

bool shu::Direct2dRender::CreateRenderTarget()
{
	HRESULT hr = S_OK;

	// 创建 DXGI Surface
	hr = m_dxgiSwapChain->GetBuffer(0, IID_PPV_ARGS(&m_dxgiSurface));
	if (FAILED(hr)) return false;

	// 创建 渲染目标
	D2D1_BITMAP_PROPERTIES1 d2d1_bitmap_def = {};
	d2d1_bitmap_def.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	d2d1_bitmap_def.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	d2d1_bitmap_def.dpiX = USER_DEFAULT_SCREEN_DPI;
	d2d1_bitmap_def.dpiY = USER_DEFAULT_SCREEN_DPI;
	d2d1_bitmap_def.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
	d2d1_bitmap_def.colorContext = NULL;
	hr = m_d2dDeviceContext->CreateBitmapFromDxgiSurface(m_dxgiSurface.Get(), d2d1_bitmap_def, &m_d2dTarget);
	if (FAILED(hr)) return false;

	return true;
}

bool shu::Direct2dRender::createDwrite()
{
	HRESULT hr = S_OK;
	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		&this->m_pDWriteFactory
	);
	if (FAILED(hr)) return false;

	return true;
}

bool shu::Direct2dRender::createWic()
{
	if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&this->m_wicImgFactory))))
		return false;
	return true;
}
