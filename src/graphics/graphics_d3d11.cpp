#define USE_D3D11_NATIVE_OBJECT
#include <dseed.h>

#if PLATFORM_MICROSOFT

#	include "common_d3d11.hxx"
#	include "d3d11_sprite.hxx"

////////////////////////////////////////////////////////////////////////////////////////////
//
// VGA Logical Device
//
////////////////////////////////////////////////////////////////////////////////////////////
class __d3d11_vgadevice : public dseed::graphics::vgadevice
{
public:
	__d3d11_vgadevice (dseed::graphics::vgaadapter* adapter, ID3D11Device* d3dDevice, ID3D11DeviceContext* immediateContext, IDXGISwapChain* dxgiSwapChain)
		: _refCount (1), vgaadapter (adapter), d3dDevice (d3dDevice), immediateContext (immediateContext), dxgiSwapChain (dxgiSwapChain), vsync_enable (false)
	{

	}

	dseed::error_t initialize ()
	{
		*&spriterender = new __d3d11_sprite_render (this);
		if (dseed::failed (spriterender->initialize ()))
		{
			spriterender.release ();
			return dseed::error_fail;
		}

		return dseed::error_good;
	}

public:
	virtual int32_t retain () override { return ++_refCount; }
	virtual int32_t release () override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}
	
public:
	virtual dseed::error_t native_object (void** nativeObject) override
	{
		if (nativeObject == nullptr)
			return dseed::error_invalid_args;

		auto& no = *reinterpret_cast<dseed::graphics::d3d11_vgadevice_nativeobject*>(nativeObject);
		no.d3dDevice = d3dDevice.Get ();
		no.dxgiSwapChain = dxgiSwapChain.Get ();

		return dseed::error_good;
	}

public:
	virtual dseed::error_t adapter (dseed::graphics::vgaadapter** adapter) noexcept
	{
		if (adapter == nullptr)
			return dseed::error_invalid_args;
		(*adapter = vgaadapter)->retain ();
		return dseed::error_good;
	}

public:
	virtual bool is_support_format (dseed::color::pixelformat pf) noexcept
	{
		UINT support;
		d3dDevice->CheckFormatSupport (DSEEDPF_TO_DXGIPF (pf), &support);
		return (support & D3D11_FORMAT_SUPPORT_TEXTURE2D) ? true : false;
	}

public:
	virtual dseed::error_t sprite_render (dseed::graphics::vgarender** render) noexcept
	{
		if (render == nullptr)
			return dseed::error_invalid_args;
		(*render = spriterender)->retain ();
		return dseed::error_good;
	}

public:
	virtual bool vsync () noexcept { return vsync_enable; }
	virtual dseed::error_t set_vsync (bool vsync) noexcept { vsync_enable = vsync; return dseed::error_good; }

public:
	virtual void displaymode (dseed::graphics::displaymode* dm, bool* fullscreen) noexcept
	{
		Microsoft::WRL::ComPtr<IDXGISwapChain1> dxgiSwapChain1;
		if (SUCCEEDED (dxgiSwapChain.As<IDXGISwapChain1> (&dxgiSwapChain1)))
		{
			DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
			DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullscreenDesc;

			dxgiSwapChain1->GetDesc1 (&dxgiSwapChainDesc);
			dxgiSwapChain1->GetFullscreenDesc (&dxgiSwapChainFullscreenDesc);

			if (dm != nullptr)
			{
				dm->resolution = dseed::size2i (dxgiSwapChainDesc.Width, dxgiSwapChainDesc.Height);
				dm->format = DXGIPF_TO_DSEEDPF (dxgiSwapChainDesc.Format);
				dm->refresh_rate = dseed::fraction (
					dxgiSwapChainFullscreenDesc.RefreshRate.Numerator,
					dxgiSwapChainFullscreenDesc.RefreshRate.Denominator);
			}
			if (fullscreen != nullptr)
				*fullscreen = !dxgiSwapChainFullscreenDesc.Windowed;
		}
		else
		{
			DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
			dxgiSwapChain->GetDesc (&dxgiSwapChainDesc);

			if (dm != nullptr)
			{
				dm->resolution = dseed::size2i (dxgiSwapChainDesc.BufferDesc.Width, dxgiSwapChainDesc.BufferDesc.Height);
				dm->format = DXGIPF_TO_DSEEDPF (dxgiSwapChainDesc.BufferDesc.Format);
				dm->refresh_rate = dseed::fraction (
					dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator,
					dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator);
			}
			if (fullscreen != nullptr)
				*fullscreen = !dxgiSwapChainDesc.Windowed;
		}
	}
	virtual bool set_displaymode (const dseed::graphics::displaymode* dm, bool fullscreen) noexcept
	{
		if (dm == nullptr) return false;
		if (fullscreen)
		{
#if PLATFORM_UWP
			return false;
#endif
		}

		DXGI_MODE_DESC dxgiModeDesc = {};
		dxgiModeDesc.Width = dm->resolution.width;
		dxgiModeDesc.Height = dm->resolution.height;
		dxgiModeDesc.Format = DSEEDPF_TO_DXGIPF (dm->format);
		dxgiModeDesc.RefreshRate.Denominator = dm->refresh_rate.denominator;
		dxgiModeDesc.RefreshRate.Numerator = dm->refresh_rate.numerator;

		if (fullscreen)
		{
			dxgiModeDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
			dxgiModeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
		}

		if (FAILED (dxgiSwapChain->ResizeTarget (&dxgiModeDesc)) ||
			FAILED (dxgiSwapChain->SetFullscreenState (fullscreen, nullptr)))
			return false;

		spriterender->update_backbuffer ();

		return true;
	}

public:
	virtual dseed::error_t present () noexcept
	{
		HRESULT hr = dxgiSwapChain->Present ((int)vsync_enable, 0);

		if (hr == DXGI_ERROR_DEVICE_RESET)
			return dseed::error_device_reset;
		if (hr == DXGI_ERROR_DEVICE_REMOVED)
			return dseed::error_device_disconn;
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> dxgiSwapChain;

	dseed::autoref<dseed::graphics::vgaadapter> vgaadapter;

	dseed::autoref<__d3d11_sprite_render> spriterender;

	bool vsync_enable;
};

#endif

////////////////////////////////////////////////////////////////////////////////////////////
//
// Create VGA Logical Device
//
////////////////////////////////////////////////////////////////////////////////////////////
dseed::error_t dseed::graphics::create_d3d11_vgadevice (platform::application* app, vgaadapter* adapter, vgadevice** device) noexcept
{
#if PLATFORM_MICROSOFT
	if (app == nullptr || device == nullptr)
		return dseed::error_invalid_args;

	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
	D3D_DRIVER_TYPE d3dDriverType;
	if (adapter != nullptr)
	{
		adapter->native_object (&dxgiAdapter);
		d3dDriverType = D3D_DRIVER_TYPE_UNKNOWN;
	}
	else
	{
		dxgiAdapter = nullptr;
		d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	}

	DWORD d3dDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	d3dDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] = {
#if NTDDI_VERSION >= NTDDI_WIN10
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
#endif
#if NTDDI_VERSION >= NTDDI_WIN8
		D3D_FEATURE_LEVEL_11_1,
#endif
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
	};
	D3D_FEATURE_LEVEL currentFeatureLevel;

	Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
	if (FAILED (D3D11CreateDevice (dxgiAdapter.Get (), d3dDriverType, nullptr, d3dDeviceFlags,
		featureLevels, _countof (featureLevels), D3D11_SDK_VERSION, &d3dDevice, &currentFeatureLevel, &immediateContext)))
		return dseed::error_fail;

	UINT majorVersion = (currentFeatureLevel >> 12) & 0xf, minorVersion = (currentFeatureLevel >> 8) & 0xf;
	char versionMessage[256];
	sprintf (versionMessage, "Direct3D 11 Feature Level: %d.%d", majorVersion, minorVersion);
	dseed::diagnostics::logger::shared_logger ()->write (dseed::diagnostics::loglevel::information, "Direct3D 11 VGA Device Creation", versionMessage);

	Microsoft::WRL::ComPtr<IDXGISwapChain> dxgiSwapChain;
#if PLATFORM_WINDOWS
	HWND hWnd;
	app->native_object ((void**)&hWnd);
#elif PLATFORM_UWP
	CoreWindow^ coreWindow;
	app->native_object ((void**)&coreWindow);
#endif
	dseed::size2i clientSize;
	app->client_size (&clientSize);

#if NTDDI_VERSION >= NTDDI_WIN8
	if (IsWindows8OrGreater ())
	{
		Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;
		if (FAILED (CreateDXGIFactory2 (0, __uuidof(IDXGIFactory2), (void**)&dxgiFactory)))
			return dseed::error_fail;

		DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc = {};
#	if NTDDI_VERSION >= NTDDI_WIN10
		if (IsWindows10OrGreater ())
		{
			dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			dxgiSwapChainDesc.BufferCount = 2;
		}
		else
#	endif
		{
#	if PLATFORM_WINDOWS
			dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			dxgiSwapChainDesc.BufferCount = 1;
#	elif PLATFORM_UWP
			dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			dxgiSwapChainDesc.BufferCount = 2;
#	endif
		}
		dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		dxgiSwapChainDesc.Width = clientSize.width;
		dxgiSwapChainDesc.Height = clientSize.height;
		dxgiSwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		dxgiSwapChainDesc.SampleDesc.Count = 1;
		dxgiSwapChainDesc.Stereo = FALSE;
		dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#	if NTDDI_VERSION >= NTDDI_WIN8
		dxgiSwapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
#		if NTDDI_VERSION >= NTDDI_WIN10
		Microsoft::WRL::ComPtr<IDXGIFactory5> dxgiFactory5;
		dxgiFactory.As<IDXGIFactory5> (&dxgiFactory5);

		BOOL checkAllowTearing = FALSE;
		if (SUCCEEDED (dxgiFactory5->CheckFeatureSupport (DXGI_FEATURE_PRESENT_ALLOW_TEARING, &checkAllowTearing, sizeof (checkAllowTearing))) && checkAllowTearing)
			dxgiSwapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
#		endif
#	endif

		Microsoft::WRL::ComPtr<IDXGISwapChain1> dxgiSwapChain1;
#	if PLATFORM_WINDOWS
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullscreenDesc = {};
		dxgiSwapChainFullscreenDesc.Windowed = TRUE;

		if (FAILED (dxgiFactory->CreateSwapChainForHwnd (d3dDevice.Get (), hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullscreenDesc, nullptr, &dxgiSwapChain1)))
			return dseed::error_fail;
#	elif PLATFORM_UWP
		if (FAILED (dxgiFactory->CreateSwapChainForCoreWindow (d3dDevice.Get (), coreWindow, &dxgiSwapChainDesc, nullptr, &dxgiSwapChain1)))
			return dseed::error_fail;
#	endif

		dxgiSwapChain1.As<IDXGISwapChain> (&dxgiSwapChain);
	}
#	if PLATFORM_WINDOWS
	else
#	endif
#endif
#if PLATFORM_WINDOWS
	{
		Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
		if (FAILED (CreateDXGIFactory (__uuidof(IDXGIFactory), (void**)&dxgiFactory)))
			return dseed::error_fail;

		DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc = {};
		dxgiSwapChainDesc.BufferDesc.Width = clientSize.width;
		dxgiSwapChainDesc.BufferDesc.Height = clientSize.height;
		dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		dxgiSwapChainDesc.BufferCount = 1;
		dxgiSwapChainDesc.OutputWindow = hWnd;
		dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		dxgiSwapChainDesc.SampleDesc.Count = 1;
		dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		dxgiSwapChainDesc.Windowed = true;

		if (FAILED (dxgiFactory->CreateSwapChain (d3dDevice.Get (), &dxgiSwapChainDesc, &dxgiSwapChain)))
			return dseed::error_fail;
	}
#endif

	*device = new __d3d11_vgadevice (adapter, d3dDevice.Get (), immediateContext.Get (), dxgiSwapChain.Get ());
	if (*device == nullptr)
		return  dseed::error_out_of_memory;
	
	if (dseed::failed (reinterpret_cast<__d3d11_vgadevice*>(*device)->initialize ()))
	{
		delete* device;
		*device = nullptr;
		return dseed::error_fail;
	}

#else
	return dseed::error_not_support;
#endif
	return dseed::error_good;
}