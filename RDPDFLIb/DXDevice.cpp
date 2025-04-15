#include "pch.h"
#include "DXDevice.h"
#include <windows.ui.xaml.media.dxinterop.h>

using namespace D2D1;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Platform;
using namespace Windows::System::Threading;
using namespace Concurrency;

namespace DisplayMetrics
{
	//高分辨率显示可能需要大量 GPU 和电量才能呈现。
	// 例如，出现以下情况时，高分辨率电话可能会缩短电池使用时间
	// 游戏尝试以全保真度按 60 帧/秒的速度呈现。
	// 跨所有平台和外形规格以全保真度呈现的决定
	// 应当审慎考虑。
	static const bool SupportHighResolutions = false;

	// 用于定义“高分辨率”显示的默认阈值，如果该阈值
	// 超出范围，并且 SupportHighResolutions 为 false，将缩放尺寸
	// 50%。
	static const float DpiThreshold = 192.0f;		// 200% 标准桌面显示。
	static const float WidthThreshold = 1920.0f;	// 1080p 宽。
	static const float HeightThreshold = 1080.0f;	// 1080p 高。
};

// DeviceResources 的构造函数。
RDPDFLib::view::DXDevice::DXDevice(SwapChainPanel^ panel, IDXDeviceCallback ^callback) :
	m_d3dRenderTargetSize(),
	m_outputSize(),
	m_logicalSize(),
	m_nativeOrientation(DisplayOrientations::None),
	m_currentOrientation(DisplayOrientations::None),
	m_dpi(-1.0f),
	m_effectiveDpi(-1.0f),
	m_compositionScaleX(1.0f),
	m_compositionScaleY(1.0f)
{
	m_callback = callback;
	dx_init();

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
	m_swapChainPanel = panel;
	m_logicalSize = Windows::Foundation::Size(static_cast<float>(panel->ActualWidth), static_cast<float>(panel->ActualHeight));
	m_nativeOrientation = currentDisplayInformation->NativeOrientation;
	m_currentOrientation = currentDisplayInformation->CurrentOrientation;
	m_compositionScaleX = panel->CompositionScaleX;
	m_compositionScaleY = panel->CompositionScaleY;
	m_dpi = currentDisplayInformation->LogicalDpi;
	m_d2dContext->SetDpi(m_dpi, m_dpi);

	dx_resize();

	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
}

// 配置 Direct3D 设备，并存储设备句柄和设备上下文。
void RDPDFLib::view::DXDevice::dx_init() 
{
	// 此标志为颜色通道排序与 API 默认设置不同的图面
	// 添加支持。要与 Direct2D 兼容，必须满足此要求。
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	// 此数组定义此应用程序将支持的 DirectX 硬件功能级别组。
	// 请注意，应保留顺序。
	// 请不要忘记在应用程序的说明中声明其所需的
	// 最低功能级别。除非另行说明，否则假定所有应用程序均支持 9.1。
	D3D_FEATURE_LEVEL featureLevels[] = 
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// 创建 Direct3D 11 API 设备对象和对应的上下文。
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;

	HRESULT hr = D3D11CreateDevice(
		nullptr,					// 指定 nullptr 以使用默认适配器。
		D3D_DRIVER_TYPE_HARDWARE,	// 创建使用硬件图形驱动程序的设备。
		0,							// 应为 0，除非驱动程序是 D3D_DRIVER_TYPE_SOFTWARE。
		creationFlags,				// 设置调试和 Direct2D 兼容性标志。
		featureLevels,				// 此应用程序可以支持的功能级别的列表。
		ARRAYSIZE(featureLevels),	// 上面的列表的大小。
		D3D11_SDK_VERSION,			// 对于 Windows 应用商店应用，始终将此值设置为 D3D11_SDK_VERSION。
		&device,					// 返回创建的 Direct3D 设备。
		nullptr,			// 返回所创建设备的功能级别。
		nullptr					// 返回设备的即时上下文。
		);

	if (FAILED(hr))
	{
		// 如果初始化失败，则回退到 WARP 设备。
		// 有关 WARP 的详细信息，请参阅: 
		// https://go.microsoft.com/fwlink/?LinkId=286690
		RDPDFLib::ThrowIfFailed(
			D3D11CreateDevice(
				nullptr,
				D3D_DRIVER_TYPE_WARP, // 创建 WARP 设备而不是硬件设备。
				0,
				creationFlags,
				featureLevels,
				ARRAYSIZE(featureLevels),
				D3D11_SDK_VERSION,
				&device,
				nullptr,
				nullptr
				)
			);
	}

	// 将指针存储到 Direct3D 11.3 API 设备和即时上下文中。
	RDPDFLib::ThrowIfFailed(device.As(&m_d3dDevice));

	// 创建 Direct2D 设备对象和对应的上下文。
	ComPtr<IDXGIDevice> dxgiDevice;
	RDPDFLib::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));
	D2D1_CREATION_PROPERTIES prop =
	{
		D2D1_THREADING_MODE_MULTI_THREADED,
		D2D1_DEBUG_LEVEL_NONE,
		D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS
	};
	RDPDFLib::ThrowIfFailed(D2D1CreateDevice(dxgiDevice.Get(), prop, &m_d2dDevice));
	RDPDFLib::ThrowIfFailed(m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_d2dContext));
}

// 每次更改窗口大小时需要重新创建这些资源。
void RDPDFLib::view::DXDevice::dx_resize() 
{
	// 清除特定于上一窗口大小的上下文。
	m_d2dContext->SetTarget(nullptr);
	m_d2dTargetBitmap = nullptr;

	dx_calc_size();

	// 交换链的宽度和高度必须基于窗口的
	// 面向本机的宽度和高度。如果窗口不在本机
	// 方向，则必须使尺寸反转。
	DXGI_MODE_ROTATION displayRotation = dx_get_rotate();

	bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
	m_d3dRenderTargetSize.Width = swapDimensions ? m_outputSize.Height : m_outputSize.Width;
	m_d3dRenderTargetSize.Height = swapDimensions ? m_outputSize.Width : m_outputSize.Height;

	if (m_swapChain != nullptr)
	{
		// 如果交换链已存在，请调整其大小。
		HRESULT hr = m_swapChain->ResizeBuffers(
			2, // 双缓冲交换链。
			lround(m_d3dRenderTargetSize.Width),
			lround(m_d3dRenderTargetSize.Height),
			DXGI_FORMAT_B8G8R8A8_UNORM,
			0
			);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// 如果出于任何原因移除了设备，将需要创建一个新的设备和交换链。
			dx_lost();

			// 现在一切都已设置完毕。不要继续执行此方法。HandleDeviceLost 将重新呈现此方法 
			// 并正确设置新设备。
			return;
		}
		else
		{
			RDPDFLib::ThrowIfFailed(hr);
		}
	}
	else
	{
		// 否则，使用与现有 Direct3D 设备相同的适配器新建一个。
		DXGI_SCALING scaling = DisplayMetrics::SupportHighResolutions ? DXGI_SCALING_NONE : DXGI_SCALING_STRETCH;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};

		swapChainDesc.Width = lround(m_d3dRenderTargetSize.Width);		// 匹配窗口的大小。
		swapChainDesc.Height = lround(m_d3dRenderTargetSize.Height);
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;				// 这是最常用的交换链格式。
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;								// 请不要使用多采样。
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;									// 使用双缓冲最大程度地减小延迟。
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;	// 所有 Windows 应用商店应用都必须使用 _FLIP_ SwapEffects。
		swapChainDesc.Flags = 0;
		swapChainDesc.Scaling = scaling;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		// 此序列获取用来创建上面的 Direct3D 设备的 DXGI 工厂。
		ComPtr<IDXGIDevice3> dxgiDevice;
		RDPDFLib::ThrowIfFailed(
			m_d3dDevice.As(&dxgiDevice)
			);

		ComPtr<IDXGIAdapter> dxgiAdapter;
		RDPDFLib::ThrowIfFailed(
			dxgiDevice->GetAdapter(&dxgiAdapter)
			);

		ComPtr<IDXGIFactory4> dxgiFactory;
		RDPDFLib::ThrowIfFailed(
			dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
			);

		//使用 XAML 互操作时，必须创建交换链进行复合。
		ComPtr<IDXGISwapChain1> swapChain;
		RDPDFLib::ThrowIfFailed(
			dxgiFactory->CreateSwapChainForComposition(
				m_d3dDevice.Get(),
				&swapChainDesc,
				nullptr,
				&swapChain
				)
			);

		RDPDFLib::ThrowIfFailed(
			swapChain.As(&m_swapChain)
			);

		// 将交换链与 SwapChainPanel 关联
		// UI 更改将需要调度回 UI 线程
		m_swapChainPanel->Dispatcher->RunAsync(CoreDispatcherPriority::High, ref new DispatchedHandler([=]()
		{
			//获取 SwapChainPanel 的受支持的本机接口
			ComPtr<ISwapChainPanelNative> panelNative;
			RDPDFLib::ThrowIfFailed(
				reinterpret_cast<IUnknown*>(m_swapChainPanel)->QueryInterface(IID_PPV_ARGS(&panelNative))
				);

			RDPDFLib::ThrowIfFailed(
				panelNative->SetSwapChain(m_swapChain.Get())
				);
		}, CallbackContext::Any));

		// 确保 DXGI 不会一次对多个帧排队。这样既可以减小延迟，
		// 又可以确保应用程序将只在每个 VSync 之后渲染，从而最大程度地降低功率消耗。
		RDPDFLib::ThrowIfFailed(
			dxgiDevice->SetMaximumFrameLatency(1)
			);
	}

	// 为交换链设置正确方向，并生成 2D 和 3D 矩阵
	// 转换以渲染到旋转交换链。
	// 请注意，2D 和 3D 转换的旋转角度不同。
	// 这是由坐标空间的差异引起的。此外，
	// 显式指定 3D 矩阵可以避免舍入误差。

	switch (displayRotation)
	{
	case DXGI_MODE_ROTATION_IDENTITY:
		m_orientationTransform2D = Matrix3x2F::Identity();
		break;

	case DXGI_MODE_ROTATION_ROTATE90:
		m_orientationTransform2D = 
			Matrix3x2F::Rotation(90.0f) *
			Matrix3x2F::Translation(m_logicalSize.Height, 0.0f);
		break;

	case DXGI_MODE_ROTATION_ROTATE180:
		m_orientationTransform2D = 
			Matrix3x2F::Rotation(180.0f) *
			Matrix3x2F::Translation(m_logicalSize.Width, m_logicalSize.Height);
		break;

	case DXGI_MODE_ROTATION_ROTATE270:
		m_orientationTransform2D = 
			Matrix3x2F::Rotation(270.0f) *
			Matrix3x2F::Translation(0.0f, m_logicalSize.Width);
		break;

	default:
		throw ref new FailureException();
	}

	RDPDFLib::ThrowIfFailed(
		m_swapChain->SetRotation(displayRotation)
		);

	// 在交换链上设置反向缩放
	DXGI_MATRIX_3X2_F inverseScale = { 0 };
	inverseScale._11 = 1.0f / m_effectiveCompositionScaleX;
	inverseScale._22 = 1.0f / m_effectiveCompositionScaleY;
	ComPtr<IDXGISwapChain2> spSwapChain2;
	RDPDFLib::ThrowIfFailed(
		m_swapChain.As<IDXGISwapChain2>(&spSwapChain2)
		);

	RDPDFLib::ThrowIfFailed(
		spSwapChain2->SetMatrixTransform(&inverseScale)
		);

	// 创建交换链后台缓冲区的渲染目标视图。
	ComPtr<ID3D11Texture2D1> backBuffer;
	RDPDFLib::ThrowIfFailed(
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))
		);

	// 创建与交换链后台缓冲区关联的 Direct2D 目标位图
	// 并将其设置为当前目标。
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = 
		D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
			m_dpi,
			m_dpi
			);

	ComPtr<IDXGISurface2> dxgiBackBuffer;
	RDPDFLib::ThrowIfFailed(
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer))
		);

	RDPDFLib::ThrowIfFailed(
		m_d2dContext->CreateBitmapFromDxgiSurface(
			dxgiBackBuffer.Get(),
			&bitmapProperties,
			&m_d2dTargetBitmap
			)
		);

	m_d2dContext->SetTarget(m_d2dTargetBitmap.Get());
	m_d2dContext->SetDpi(m_effectiveDpi, m_effectiveDpi);

	// 建议将灰度文本抗锯齿用于所有 Windows 应用商店应用。
	//m_d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
}

// 确定呈现器目标的尺寸及其是否将缩小。
void RDPDFLib::view::DXDevice::dx_calc_size()
{
	m_effectiveDpi = m_dpi;
	m_effectiveCompositionScaleX = m_compositionScaleX;
	m_effectiveCompositionScaleY = m_compositionScaleY;

	// 为了延长高分辨率设备上的电池使用时间，请呈现到较小的呈现器目标
	// 并允许 GPU 在显示输出时缩放输出。
	if (!DisplayMetrics::SupportHighResolutions && m_dpi > DisplayMetrics::DpiThreshold)
	{
		float width = RDPDFLib::ConvertDipsToPixels(m_logicalSize.Width, m_dpi);
		float height = RDPDFLib::ConvertDipsToPixels(m_logicalSize.Height, m_dpi);

		// 当设备为纵向时，高度大于宽度。将
		// 较大尺寸与宽度阈值进行比较，将较小尺寸
		// 与高度阈值进行比较。
		if (max(width, height) > DisplayMetrics::WidthThreshold && min(width, height) > DisplayMetrics::HeightThreshold)
		{
			// 为了缩放应用，我们更改了有效 DPI。逻辑大小不变。
			m_effectiveDpi /= 2.0f;
			m_effectiveCompositionScaleX /= 2.0f;
			m_effectiveCompositionScaleY /= 2.0f;
		}
	}

	// 计算必要的呈现器目标大小(以像素为单位)。
	m_outputSize.Width = RDPDFLib::ConvertDipsToPixels(m_logicalSize.Width, m_effectiveDpi);
	m_outputSize.Height = RDPDFLib::ConvertDipsToPixels(m_logicalSize.Height, m_effectiveDpi);

	// 防止创建大小为零的 DirectX 内容。
	m_outputSize.Width = max(m_outputSize.Width, 1);
	m_outputSize.Height = max(m_outputSize.Height, 1);
}

// 在 SizeChanged 事件的事件处理程序中调用此方法。
void RDPDFLib::view::DXDevice::SetLogicalSize(Windows::Foundation::Size logicalSize)
{
	m_locker.lock();
	if (m_logicalSize != logicalSize)
	{
		m_logicalSize = logicalSize;
		dx_resize();
	}
	m_locker.unlock();
}

// 在 DpiChanged 事件的事件处理程序中调用此方法。
void RDPDFLib::view::DXDevice::SetDpi(float dpi)
{
	m_locker.lock();
	if (dpi != m_dpi)
	{
		m_dpi = dpi;
		m_d2dContext->SetDpi(m_dpi, m_dpi);
		dx_resize();
	}
	m_locker.unlock();
}

// 在 OrientationChanged 事件的事件处理程序中调用此方法。
void RDPDFLib::view::DXDevice::SetCurrentOrientation(DisplayOrientations currentOrientation)
{
	m_locker.lock();
	if (m_currentOrientation != currentOrientation)
	{
		m_currentOrientation = currentOrientation;
		dx_resize();
	}
	m_locker.unlock();
}

//在 CompositionScaleChanged 事件的事件处理程序中调用此方法
void RDPDFLib::view::DXDevice::SetCompositionScale(float compositionScaleX, float compositionScaleY)
{
	m_locker.lock();
	if (m_compositionScaleX != compositionScaleX ||
		m_compositionScaleY != compositionScaleY)
	{
		m_compositionScaleX = compositionScaleX;
		m_compositionScaleY = compositionScaleY;
		dx_resize();
	}
	m_locker.unlock();
}

// 在 DisplayContentsInvalidated 事件的事件处理程序中调用此方法。
void RDPDFLib::view::DXDevice::ValidateDevice()
{
	m_locker.lock();
	// 如果默认适配器更改，D3D 设备将不再有效，因为该设备
	// 已创建或该设备已移除。

	// 首先，获取自设备创建以来的默认适配器信息。

	ComPtr<IDXGIDevice3> dxgiDevice;
	RDPDFLib::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

	ComPtr<IDXGIAdapter> deviceAdapter;
	RDPDFLib::ThrowIfFailed(dxgiDevice->GetAdapter(&deviceAdapter));

	ComPtr<IDXGIFactory2> deviceFactory;
	RDPDFLib::ThrowIfFailed(deviceAdapter->GetParent(IID_PPV_ARGS(&deviceFactory)));

	ComPtr<IDXGIAdapter1> previousDefaultAdapter;
	RDPDFLib::ThrowIfFailed(deviceFactory->EnumAdapters1(0, &previousDefaultAdapter));

	DXGI_ADAPTER_DESC1 previousDesc;
	RDPDFLib::ThrowIfFailed(previousDefaultAdapter->GetDesc1(&previousDesc));

	// 接下来，获取当前默认适配器的信息。

	ComPtr<IDXGIFactory4> currentFactory;
	RDPDFLib::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&currentFactory)));

	ComPtr<IDXGIAdapter1> currentDefaultAdapter;
	RDPDFLib::ThrowIfFailed(currentFactory->EnumAdapters1(0, &currentDefaultAdapter));

	DXGI_ADAPTER_DESC1 currentDesc;
	RDPDFLib::ThrowIfFailed(currentDefaultAdapter->GetDesc1(&currentDesc));

	// 如果适配器 LUID 不匹配，或者该设备报告它已被移除，
	// 则必须创建新的 D3D 设备。

	if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
		previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
		FAILED(m_d3dDevice->GetDeviceRemovedReason()))
	{
		// 发布对与旧设备相关的资源的引用。
		dxgiDevice = nullptr;
		deviceAdapter = nullptr;
		deviceFactory = nullptr;
		previousDefaultAdapter = nullptr;

		// 创建新设备和交换链。
		dx_lost();
	}
	m_locker.unlock();
}

// 重新创建所有设备资源并将其设置回当前状态。
void RDPDFLib::view::DXDevice::dx_lost()
{
	m_swapChain = nullptr;
	dx_init();
	m_d2dContext->SetDpi(m_dpi, m_dpi);
	dx_resize();
}

// 在应用程序挂起时调用此方法。它可提示驱动程序该应用程序
// 正在进入空闲状态，可以回收临时缓冲区以供其他应用程序使用。
void RDPDFLib::view::DXDevice::Trim()
{
	ComPtr<IDXGIDevice3> dxgiDevice;
	m_d3dDevice.As(&dxgiDevice);

	dxgiDevice->Trim();
}

// 此方法确定以下两个元素之间的旋转方式: 显示设备的本机方向和
// 当前显示方向。
DXGI_MODE_ROTATION RDPDFLib::view::DXDevice::dx_get_rotate()
{
	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

	// 注意: NativeOrientation 只能为 "Landscape" 或 "Portrait"，即使
	// DisplayOrientations 枚举具有其他值。
	switch (m_nativeOrientation)
	{
	case DisplayOrientations::Landscape:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;
		}
		break;

	case DisplayOrientations::Portrait:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;
		}
		break;
	}
	return rotation;
}


void RDPDFLib::view::DXDevice::StartRenderLoop()
{
	// 如果动画呈现循环已在运行，则勿启动其他线程。
	if (m_renderLoopWorker != nullptr && m_renderLoopWorker->Status == AsyncStatus::Started)
	{
		return;
	}

	// 创建一个将在后台线程上运行的任务。
	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction ^ action)
	{
		// 计算更新的帧并且在每个场消隐期呈现一次。
		while (action->Status == AsyncStatus::Started)
		{
			if (m_callback)
			{
				m_callback->dx_update();
				if (m_callback->dx_render())
				{
					// 第一个参数指示 DXGI 进行阻止直到 VSync，这使应用程序
					// 在下一个 VSync 前进入休眠。这将确保我们不会浪费任何周期渲染
					// 从不会在屏幕上显示的帧。
					DXGI_PRESENT_PARAMETERS parameters = { 0 };
					HRESULT hr = m_swapChain->Present1(1, 0, &parameters);

					// 如果通过断开连接或升级驱动程序移除了设备，则必须
					// 必须重新创建所有设备资源。
					if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
					{
						dx_lost();
					}
					else
					{
						RDPDFLib::ThrowIfFailed(hr);
					}
				}
			}
		}
	});

	// 在高优先级的专用后台线程上运行任务。
	m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void RDPDFLib::view::DXDevice::StopRenderLoop()
{
	m_renderLoopWorker->Cancel();
}
