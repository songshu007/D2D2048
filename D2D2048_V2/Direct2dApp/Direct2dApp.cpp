#include "Direct2dApp.h"

using namespace app;

app::Direct2dApp::Direct2dApp(HWND hwnd)
	: m_hwnd(hwnd), rc({ 0, 0, 0, 0 })
{
	GetClientRect(m_hwnd, &rc);
}
app::Direct2dApp::Direct2dApp()
	: m_hwnd(NULL), rc({ 0, 0, 0, 0 })
{
}
app::Direct2dApp::~Direct2dApp()
{
	this->Release();
	CoUninitialize();
}
void app::Direct2dApp::setFullScreen(bool flag)
{
	this->FullScreen = flag;
}
void app::Direct2dApp::setVerticalSync(bool flag)
{
	this->Vertical_sync = flag;
}
bool app::Direct2dApp::initD2d(HWND hwnd)
{
	HRESULT hr = HR = S_OK;

	this->m_hwnd = hwnd;

	// 初始化 COM 对象
	hr = HR = CoInitialize(NULL);
	if (FAILED(hr)) return false;

	// 创建 D2D Factory
	hr = HR = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory1), (void**)&m_d2dFactory);
	if (FAILED(hr)) return false;

	hr = HR = this->m_d2dFactory->QueryInterface(IID_PPV_ARGS(&m_D2DMultithread));

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
	hr = HR = D3D11CreateDevice(
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
	hr = HR = m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&m_dxgiDevice);
	if (FAILED(hr)) return false;

	// 创建 D2D Device
	hr = HR = m_d2dFactory->CreateDevice(m_dxgiDevice.Get(), &m_d2dDevice);
	if (FAILED(hr)) return false;

	// 创建 D2D DeviceContext
	hr = HR = m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_d2dDeviceContext);
	if (FAILED(hr)) return false;

	// 创建 DXGI Factory
	ComPtr<IDXGIAdapter> dxgiAdapter;
	hr = HR = m_dxgiDevice->GetAdapter(&dxgiAdapter);
	if (FAILED(hr)) return false;

	hr = HR = dxgiAdapter->GetParent(IID_PPV_ARGS(&m_dxgiFactory));
	if (FAILED(hr)) return false;

	// 创建 DXGI SwapChain
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
	memset(&swapChainDesc, 0, sizeof(swapChainDesc));
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
	// 设置全屏参数（没用到，所以不能全屏）
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreen = { 0 };
	memset(&fullScreen, 0, sizeof(fullScreen));
	fullScreen.RefreshRate.Numerator = 60;
	fullScreen.RefreshRate.Denominator = 1;
	fullScreen.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	fullScreen.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	fullScreen.Windowed = true;

	hr = HR = m_dxgiFactory->CreateSwapChainForHwnd(
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

	// 创建 RT
	if (!this->createRT()) return false;

	// 创建 Dwrite
	if (!this->createDwrite()) return false;

	// 创建 WIC
	if (!this->initIWIC()) return false;

	// 设置抗锯齿
	m_d2dDeviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

	return true;
}
bool app::Direct2dApp::initIWIC()
{
	if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&this->m_wicImgFactory))))
		return false;
	return true;
}
bool app::Direct2dApp::createDwrite()
{
	HRESULT hr = HR = S_OK;
	hr = HR = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		&this->m_pDWriteFactory
	);
	if (FAILED(hr)) return false;

	hr = HR = this->m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_Brush);
	if (FAILED(hr)) return false;

	return true;
}
bool app::Direct2dApp::createRT()
{
	HRESULT hr = HR = S_OK;

	// 创建 DXGI Surface
	hr = HR = m_dxgiSwapChain->GetBuffer(0, IID_PPV_ARGS(&m_dxgiSurface));
	if (FAILED(hr)) return false;

	// 创建 渲染目标
	D2D1_BITMAP_PROPERTIES1 d2d1_bitmap_def = {};
	d2d1_bitmap_def.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	d2d1_bitmap_def.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	d2d1_bitmap_def.dpiX = USER_DEFAULT_SCREEN_DPI;
	d2d1_bitmap_def.dpiY = USER_DEFAULT_SCREEN_DPI;
	d2d1_bitmap_def.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
	d2d1_bitmap_def.colorContext = NULL;
	hr = HR = m_d2dDeviceContext->CreateBitmapFromDxgiSurface(m_dxgiSurface.Get(), d2d1_bitmap_def, &m_d2dTarget);
	if (FAILED(hr)) return false;

	return true;
}
void app::Direct2dApp::BeginDraw()
{
	this->m_d2dDeviceContext->BeginDraw();
}
void app::Direct2dApp::Clear(const D2D1_COLOR_F& color)
{
	this->m_d2dDeviceContext->Clear(color);
}
void app::Direct2dApp::drawLine(const D2D1_POINT_2F& point1, const D2D1_POINT_2F& point2, const D2D1_COLOR_F& color, const float width)
{
	this->m_Brush->SetColor(color);
	this->m_d2dDeviceContext->DrawLine(point1, point2, m_Brush.Get(), width);
}
void app::Direct2dApp::drawBitmap(ID2D1Bitmap1* bitmap, D2D1_POINT_2F pos, D2D1_SIZE_F size)
{
	this->m_d2dDeviceContext->DrawBitmap(bitmap, D2D1::RectF(pos.x, pos.y, pos.x + size.width, pos.y + size.height), 1.0f, D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
}
void app::Direct2dApp::drawCenterText(const LPCWSTR text, const D2D1_POINT_2F& pos, float width, float height, const D2D1_COLOR_F& color, const float Textsize)
{
	HRESULT hr = HR = S_OK;

	if (Textsize <= 0) return;

	this->m_Brush->SetColor(color);
	if (FAILED(hr)) return;

	hr = HR = this->m_pDWriteFactory->CreateTextFormat(
		L"微软雅黑",                // Font family name.
		NULL,                       // Font collection (NULL sets it to use the system font collection).
		DWRITE_FONT_WEIGHT_SEMI_BOLD,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		Textsize,
		L"en-us",
		&this->m_pTextFormat
	);
	if (FAILED(hr)) return;

	hr = HR = m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	if (FAILED(hr)) return;
	hr = HR = m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	if (FAILED(hr)) return;

	hr = HR = this->m_pDWriteFactory->CreateTextLayout(
		text,
		wcslen(text),
		this->m_pTextFormat.Get(),
		width, height,
		&this->m_pTextLayout
	);
	if (FAILED(hr)) return;

	this->m_d2dDeviceContext->DrawTextLayout(pos, this->m_pTextLayout.Get(), m_Brush.Get());
}
void app::Direct2dApp::drawText(const LPCWSTR text, const D2D1_POINT_2F& pos, const D2D1_COLOR_F& color, const float Textsize)
{
	HRESULT hr = HR = S_OK;
	
	if (Textsize <= 0) return;

	this->m_Brush->SetColor(color);
	if (FAILED(hr)) return;

	hr = HR = this->m_pDWriteFactory->CreateTextFormat(
		L"微软雅黑",                // Font family name.
		NULL,                       // Font collection (NULL sets it to use the system font collection).
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		Textsize,
		L"en-us",
		&this->m_pTextFormat
	);
	if (FAILED(hr)) return;

	hr = HR = this->m_pDWriteFactory->CreateTextLayout(
		text,
		wcslen(text),
		this->m_pTextFormat.Get(),
		this->rc.right - this->rc.left, this->rc.bottom - this->rc.top,
		&this->m_pTextLayout
	);
	if (FAILED(hr)) return;

	this->m_d2dDeviceContext->DrawTextLayout(pos, this->m_pTextLayout.Get(), m_Brush.Get());
}
void app::Direct2dApp::drawRect(float x, float y, float width, float height, D2D1_COLOR_F color, float strokeWidth = (1.0f))
{
	this->m_Brush->SetColor(color);
	this->m_d2dDeviceContext->DrawRectangle({x, y, x + width, y + height}, m_Brush.Get(), strokeWidth);
}
void app::Direct2dApp::fillRect(float x, float y, float width, float height, D2D1_COLOR_F color)
{
	this->m_Brush->SetColor(color);
	this->m_d2dDeviceContext->FillRectangle({ x, y, x + width, y + height }, m_Brush.Get());
}
void app::Direct2dApp::drawRoundedRect(float x, float y, float width, float height, float radiusX, float radiusY, D2D1_COLOR_F color, float strokeWidth)
{
	this->m_Brush->SetColor(color);
	this->m_d2dDeviceContext->DrawRoundedRectangle({ x, y, x + width, y + height, radiusX, radiusY }, m_Brush.Get(), strokeWidth);
}
void app::Direct2dApp::fillRoundedRect(float x, float y, float width, float height, float radiusX, float radiusY, D2D1_COLOR_F color)
{
	this->m_Brush->SetColor(color);
	this->m_d2dDeviceContext->FillRoundedRectangle({ x, y, x + width, y + height, radiusX, radiusY }, m_Brush.Get());
}
void app::Direct2dApp::EndDraw()
{
	HRESULT hr = HR = S_OK;
	hr = HR = this->m_d2dDeviceContext->EndDraw();
	hr = HR = this->m_dxgiSwapChain->Present(Vertical_sync, 0);
}
void app::Direct2dApp::ChangeSize(WORD width, WORD height)
{
	if (width == 0 || height == 0 || !m_dxgiSwapChain)
	{
		return;
	}
	HRESULT hr = HR = S_OK;

	GetClientRect(m_hwnd, &rc);
	this->m_d2dDeviceContext->SetTarget(NULL);
	// 解除所有与 SwapChain 的绑定
	if (this->m_d3dDeviceContext)
	{
		this->m_d3dDeviceContext->ClearState();
		this->m_d3dDeviceContext->Flush();
	}
	
	this->m_dxgiSurface.Reset();
	this->m_d2dTarget.Reset();

	hr = HR = this->m_dxgiSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

	this->createRT();
	this->m_d2dDeviceContext->SetTarget(this->m_d2dTarget.Get());
}
bool app::Direct2dApp::LoadBitmapFromFile(LPCWSTR file, ID2D1Bitmap1** Bitmap)
{
	HRESULT hr = HR = S_OK;
	ComPtr<IWICBitmapDecoder> WicBitmap;
	ComPtr<IWICBitmapFrameDecode> WicFrameDec;

	hr = HR = this->m_wicImgFactory->CreateDecoderFromFilename(file, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &WicBitmap);
	if (FAILED(hr)) return false;

	hr = HR = WicBitmap->GetFrame(0, &WicFrameDec);
	if (FAILED(hr)) return false;

	WICPixelFormatGUID Format;
	ComPtr<IWICBitmapSource> WicBitmapSource;
	WicFrameDec->GetPixelFormat(&Format);
	if (Format != GUID_WICPixelFormat32bppPBGRA)
	{
		// 需要转换格式
		ComPtr<IWICFormatConverter> wic_converter;
		hr = HR = this->m_wicImgFactory->CreateFormatConverter(&wic_converter);
		if (FAILED(hr)) return false;

		hr = HR = wic_converter->Initialize(
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

	hr = HR = this->m_d2dDeviceContext->CreateBitmapFromWicBitmap(WicBitmapSource.Get(), Bitmap);
	if (FAILED(hr)) return false;

	return true;
}
bool app::Direct2dApp::LoadWicBitmapFromFile(LPCWSTR file, IWICBitmap** Bitmap)
{
	HRESULT hr = HR = S_OK;
	ComPtr<IWICBitmapDecoder> WicBitmap;
	ComPtr<IWICBitmapFrameDecode> WicFrameDec;

	hr = HR = this->m_wicImgFactory->CreateDecoderFromFilename(file, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &WicBitmap);
	if (FAILED(hr)) return false;

	hr = HR = WicBitmap->GetFrame(0, &WicFrameDec);
	if (FAILED(hr)) return false;

	WICPixelFormatGUID Format;
	ComPtr<IWICBitmapSource> WicBitmapSource;
	WicFrameDec->GetPixelFormat(&Format);
	if (Format != GUID_WICPixelFormat32bppPBGRA)
	{
		// 需要转换格式
		ComPtr<IWICFormatConverter> wic_converter;
		hr = HR = this->m_wicImgFactory->CreateFormatConverter(&wic_converter);
		if (FAILED(hr)) return false;

		hr = HR = wic_converter->Initialize(
			WicFrameDec.Get(),
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.0f,
			WICBitmapPaletteTypeCustom);
		if (FAILED(hr)) return false;

		hr = HR = wic_converter.As(&WicBitmapSource);
		if (FAILED(hr)) return false;
	}
	else
	{
		hr = HR = WicFrameDec.As(&WicBitmapSource);
		if (FAILED(hr)) return false;
	}

	hr = HR = this->m_wicImgFactory->CreateBitmapFromSource(WicBitmapSource.Get(), WICBitmapCacheOnLoad, Bitmap);
	if (FAILED(hr)) return false;

	return true;
}
bool app::Direct2dApp::WicBitmapToD2dBitmap(IWICBitmap* wicBitmap, ID2D1Bitmap1** d2dBitmap)
{
	HRESULT hr = HR = S_OK;

	hr = HR = this->m_d2dDeviceContext->CreateBitmapFromWicBitmap(wicBitmap, d2dBitmap);
	if (FAILED(hr)) return false;

	return true;
}
bool app::Direct2dApp::getTarget(ID2D1Bitmap1** Bitmap)
{
	HRESULT hr = HR = S_OK;
	// 创建 一个 bitmap
	D2D1_BITMAP_PROPERTIES1 d2d1_bitmap_def = {};
	d2d1_bitmap_def.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	d2d1_bitmap_def.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	d2d1_bitmap_def.dpiX = USER_DEFAULT_SCREEN_DPI;
	d2d1_bitmap_def.dpiY = USER_DEFAULT_SCREEN_DPI;
	d2d1_bitmap_def.bitmapOptions = D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_CPU_READ;	// 设置为 后面的标志 后，只能与 前面的标志 搭配，并且该bitmap将不能用作渲染目标，但可以用CopyFromBitmap来复制其它的bitmap
	hr = HR = this->m_d2dDeviceContext->CreateBitmap
	(
		{ (UINT32)this->m_d2dTarget->GetSize().width, (UINT32)this->m_d2dTarget->GetSize().height }, 0, 0,
		d2d1_bitmap_def,
		Bitmap
	);
	if (FAILED(hr)) return false;

	// 把 RT 中的数据 copy 过来
	D2D1_POINT_2U destPoint = { 0 , 0 };
	D2D1_RECT_U srcRect = { 0, 0, this->m_d2dTarget->GetSize().width , this->m_d2dTarget->GetSize().height };
	hr = HR = (*Bitmap)->CopyFromBitmap
	(
		&destPoint,
		this->m_d2dTarget.Get(),
		&srcRect
	);
	if (FAILED(hr)) return false;
	return true;
}
RECT app::Direct2dApp::getSize()
{
	return this->rc;
}
bool app::Direct2dApp::saveBitmapToFile(ID2D1Bitmap1* Bitmap, const WCHAR* file)
{
	HRESULT hr = HR = S_OK;

	ComPtr<IWICStream> stream;
	ComPtr<IWICBitmapEncoder> pEncoder;
	ComPtr<IWICBitmapFrameEncode> pFrameEncode;
	ComPtr<IWICBitmap> pBitmap;

	D2D1_MAPPED_RECT data;
	hr = HR = Bitmap->Map(D2D1_MAP_OPTIONS_READ, &data);
	if (FAILED(hr)) return false;

	hr = HR = m_wicImgFactory->CreateBitmapFromMemory(Bitmap->GetSize().width, Bitmap->GetSize().height,
		GUID_WICPixelFormat32bppBGRA, data.pitch, Bitmap->GetSize().height * data.pitch, data.bits, &pBitmap);
	if (FAILED(hr)) return false;

	hr = HR = m_wicImgFactory->CreateStream(&stream);
	if (FAILED(hr)) return false;

	WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;

	hr = HR = stream->InitializeFromFilename(file, GENERIC_WRITE);
	if (FAILED(hr)) return false;

	hr = HR = m_wicImgFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder);
	if (FAILED(hr)) return false;

	hr = HR = pEncoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);
	if (FAILED(hr)) return false;

	hr = HR = pEncoder->CreateNewFrame(&pFrameEncode, NULL);
	if (FAILED(hr)) return false;

	hr = HR = pFrameEncode->Initialize(NULL);
	if (FAILED(hr)) return false;

	hr = HR = pFrameEncode->SetSize(Bitmap->GetSize().width, Bitmap->GetSize().height);
	if (FAILED(hr)) return false;

	hr = HR = pFrameEncode->SetPixelFormat(&format);
	if (FAILED(hr)) return false;

	hr = HR = pFrameEncode->WriteSource(pBitmap.Get(), NULL);
	if (FAILED(hr)) return false;

	hr = HR = pFrameEncode->Commit();
	if (FAILED(hr)) return false;

	hr = HR = pEncoder->Commit();
	if (FAILED(hr)) return false;
	return true;
}
void app::Direct2dApp::saveTo()
{
	wchar_t timebuf[256] = { 0 };
	SYSTEMTIME time;
	GetLocalTime(&time);
	swprintf_s(timebuf, L"%d-%d-%d-%d-%d-%d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wMilliseconds);

	ComPtr<IFileSaveDialog> pfd;
	HRESULT hr = HR = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		pfd->SetOptions(FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

		const COMDLG_FILTERSPEC rgSaveTypes[] = { {L"(*.png)", L"*.png"} };
		pfd->SetFileName(timebuf);
		pfd->SetDefaultExtension(L"png");
		pfd->SetFileTypes(ARRAYSIZE(rgSaveTypes), rgSaveTypes);

		if (pfd->Show(NULL) == S_OK)
		{
			ComPtr<IShellItem> pitem = NULL;
			if (pfd->GetResult(&pitem) == S_OK && pitem)
			{
				LPWSTR resultptr = NULL;
				if (pitem->GetDisplayName(SIGDN_FILESYSPATH, &resultptr) == S_OK && resultptr)
				{
					ID2D1Bitmap1* buf;
					this->getTarget(&buf);
					this->saveBitmapToFile(buf, resultptr);
					buf->Release();
					std::wcout.imbue(std::locale("chs"));
					std::wcout << resultptr << std::endl;
				}
				pitem->Release();
			}
		}
		pfd->Release();
	}

}
ID2D1DeviceContext* app::Direct2dApp::getDC()
{
	return this->m_d2dDeviceContext.Get();
}
ID2D1Factory1* app::Direct2dApp::getFactory()
{
	return this->m_d2dFactory.Get();
}
void app::Direct2dApp::Release()
{
	
}
void app::Direct2dApp::test()
{
	
}