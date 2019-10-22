#include <dseed.h>

#if PLATFORM_MICROSOFT

#	if NTDDI_VERSION >= NTDDI_WIN10
#		include <d3d11_4.h>
#		include <dxgi1_4.h>
#	else
#		include <d3d11.h>
#		include <dxgi.h>
#	endif
#	include <atlbase.h>
#	include <VersionHelpers.h>

#	include "graphics.dxgi.common.h"

class __d3d11_texture2d : public dseed::texture2d
{
public:
	__d3d11_texture2d (ID3D11Texture2D* texture)
		: _refCount (1), _texture (texture)
	{

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
		reinterpret_cast<ID3D11Texture2D*>(*nativeObject = _texture)->AddRef ();
		return dseed::error_good;
	}

public:
	virtual dseed::pixelformat format () override
	{
		D3D11_TEXTURE2D_DESC desc;
		_texture->GetDesc (&desc);
		return DXGIPF_TO_DSEEDPF (desc.Format);
	}

	virtual size_t mip_level () override
	{
		D3D11_TEXTURE2D_DESC desc;
		_texture->GetDesc (&desc);
		return desc.MipLevels;
	}

public:
	virtual dseed::size2i size () override
	{
		D3D11_TEXTURE2D_DESC desc;
		_texture->GetDesc (&desc);
		return dseed::size2i (desc.Width, desc.Height);
	}

private:
	std::atomic<int32_t> _refCount;
	CComPtr<ID3D11Texture2D> _texture;
};

class __d3d11_rendertarget2d : public dseed::rendertarget2d
{
public:
	__d3d11_rendertarget2d (dseed::texture2d* texture, ID3D11RenderTargetView* rtv)
		: _refCount (1), _texture (texture), _renderTargetView (rtv)
	{

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

		reinterpret_cast<ID3D11RenderTargetView*>(*nativeObject = _renderTargetView)->AddRef ();

		return dseed::error_good;
	}

public:
	virtual dseed::error_t texture (dseed::texture2d** texture) override
	{
		if (texture == nullptr)
			return dseed::error_invalid_args;

		(*texture = _texture)->retain ();

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	dseed::auto_object<dseed::texture2d> _texture;
	CComPtr<ID3D11RenderTargetView> _renderTargetView;
};

class __d3d11_commandlist : public dseed::vga_commandlist
{
public:
	__d3d11_commandlist (ID3D11CommandList* commandList)
		: _refCount (1), _commandList (commandList)
	{

	}

public:
	virtual int32_t retain () override { ++_refCount; }
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

		reinterpret_cast<ID3D11CommandList*>(*nativeObject = _commandList)->AddRef ();

		return dseed::error_good;
	}

public:
	virtual void set_new_commandlist (ID3D11CommandList* commandList)
	{
		_commandList.Release ();
		_commandList = commandList;
	}

private:
	std::atomic<int32_t> _refCount;
	CComPtr<ID3D11CommandList> _commandList;
};

class __d3d11_commandqueue : public dseed::vga_commandqueue
{
public:
	__d3d11_commandqueue (ID3D11DeviceContext* deferredContext)
		: _refCount (1), _deferredContext (deferredContext)
	{
		_commandList = new __d3d11_commandlist (nullptr);
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

		reinterpret_cast<ID3D11DeviceContext*>(*nativeObject = _deferredContext)->AddRef ();

		return dseed::error_good;
	}

public:
	virtual dseed::error_t make_commandlist (dseed::vga_commandlist** list) override
	{
		CComPtr<ID3D11CommandList> commandList;
		if (FAILED (_deferredContext->FinishCommandList (TRUE, &commandList)))
			return dseed::error_fail;

		_commandList->set_new_commandlist (commandList);
		*list = _commandList;

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	CComPtr<ID3D11DeviceContext> _deferredContext;
	dseed::auto_object<__d3d11_commandlist> _commandList;
};

class __d3d11_vga_device : public dseed::vga_device
{
public:
	__d3d11_vga_device (dseed::vga_adapter* adapter, ID3D11Device* d3dDevice, ID3D11DeviceContext* immediateContext)
		: _refCount (1)
	{
		_vgaAdapter = adapter;
		_d3dDevice = d3dDevice;
		_immediateContext = immediateContext;
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
		if (nativeObject == nullptr) return dseed::error_invalid_args;
		if (FAILED (_d3dDevice->QueryInterface (__uuidof(ID3D11Device), nativeObject)))
			return dseed::error_fail;
		return dseed::error_good;
	}

public:
	virtual dseed::error_t adapter (dseed::vga_adapter** adapter) override
	{
		if (adapter == nullptr)
			return dseed::error_invalid_args;
		*adapter = _vgaAdapter.get ();
		(*adapter)->retain ();
		return dseed::error_good;
	}

public:
	virtual bool is_support_format (dseed::pixelformat pf) override
	{
		UINT formatSupport;
		if (FAILED (_d3dDevice->CheckFormatSupport (DSEEDPF_TO_DXGIPF (pf), &formatSupport)))
			return false;
		return formatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D;
	}
	virtual bool is_support_parallel_render () override { return true; }

public:
	virtual dseed::error_t create_commandqueue (dseed::vga_commandqueue** commandqueue) override
	{
		return dseed::error_not_impl;
	}

public:
	virtual dseed::error_t do_commandlist (dseed::vga_commandlist* commandlist) override
	{
		if (commandlist == nullptr)
			return dseed::error_invalid_args;

		CComPtr<ID3D11CommandList> d3d11CommandList;
		commandlist->native_object ((void**)&d3d11CommandList);

		_immediateContext->ExecuteCommandList (d3d11CommandList, TRUE);

		return dseed::error_good;
	}

public:
	virtual dseed::error_t create_texture2d (dseed::size2i size, dseed::pixelformat pf, int mipLevel, dseed::texture2d** texture) override
	{
		if (texture == nullptr)
			return dseed::error_invalid_args;

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = size.width;
		desc.Height = size.height;
		desc.Format = DSEEDPF_TO_DXGIPF (pf);
		desc.ArraySize = 1;
		desc.MipLevels = mipLevel;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		if (pf == dseed::pixelformat::depth16 || pf == dseed::pixelformat::depth24_stencil8 || pf == dseed::pixelformat::depth32)
			desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
		else
			desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		
		CComPtr<ID3D11Texture2D> d3dTexture;
		if (FAILED (_d3dDevice->CreateTexture2D (&desc, nullptr, &d3dTexture)))
			return dseed::error_fail;

		*texture = new __d3d11_texture2d (d3dTexture);
		if (*texture == nullptr)
			return dseed::error_out_of_memory;

		return dseed::error_good;
	}
	virtual dseed::error_t create_texture2d (dseed::bitmap* bitmap, dseed::texture2d** texture) override
	{
		if (bitmap == nullptr || texture == nullptr)
			return dseed::error_invalid_args;

		dseed::size3i size = bitmap->size ();
		if (bitmap->type () != dseed::bitmaptype_2d)
			return dseed::error_invalid_args;

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = size.width;
		desc.Height = size.height;
		desc.Format = DSEEDPF_TO_DXGIPF (bitmap->format ());
		desc.ArraySize = 1;
		desc.MipLevels = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

		D3D11_SUBRESOURCE_DATA initialData = {};
		bitmap->pixels_pointer ((void**)&initialData.pSysMem);
		initialData.SysMemSlicePitch = dseed::get_bitmap_plane_size (bitmap->format (), size.width, size.height);

		CComPtr<ID3D11Texture2D> d3dTexture;
		if (FAILED (_d3dDevice->CreateTexture2D (&desc, &initialData, &d3dTexture)))
			return dseed::error_fail;

		*texture = new __d3d11_texture2d (d3dTexture);
		if (*texture == nullptr)
			return dseed::error_out_of_memory;

		return dseed::error_good;
	}
	virtual dseed::error_t create_texture2d (dseed::bitmap_decoder* decoder, dseed::texture2d** texture)
	{
		if (decoder == nullptr || texture == nullptr)
			return dseed::error_invalid_args;

		if (decoder->frame_type () != dseed::frametype_mipmap)
		{
			if (decoder->frame_count () != 1)
				return dseed::error_fail;
		}

		if (decoder->frame_count () == 1)
		{
			dseed::auto_object<dseed::bitmap> bitmap;
			if (dseed::failed (decoder->decode_frame (0, &bitmap, nullptr)))
				return dseed::error_fail;

			return create_texture2d (bitmap, texture);
		}
		else
		{
			dseed::auto_object<dseed::bitmap> first_bitmap;
			if (dseed::failed (decoder->decode_frame (0, &first_bitmap, nullptr)))
				return dseed::error_fail;

			dseed::size3i size = first_bitmap->size ();
			if (first_bitmap->type () != dseed::bitmaptype_2d)
				return dseed::error_invalid_args;

			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = size.width;
			desc.Height = size.height;
			desc.Format = DSEEDPF_TO_DXGIPF (first_bitmap->format ());
			desc.ArraySize = 1;
			desc.MipLevels = 1;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

			std::vector<uint8_t> data;
			for (int i = 0; i < decoder->frame_count (); ++i)
			{
				dseed::auto_object<dseed::bitmap> bitmap;
				if (dseed::failed (decoder->decode_frame (i, &bitmap, nullptr)))
					return dseed::error_fail;

				dseed::size3i size = bitmap->size ();
				size_t plane = dseed::get_bitmap_plane_size (bitmap->format (), size.width, size.height);

				void* ptr;
				bitmap->pixels_pointer (&ptr);

				auto position = data.size ();
				data.resize (data.size () + plane);

				memcpy (data.data () + position, ptr, plane);
			}

			D3D11_SUBRESOURCE_DATA initialData = {};
			initialData.pSysMem = data.data ();
			initialData.SysMemSlicePitch = data.size ();

			CComPtr<ID3D11Texture2D> d3dTexture;
			if (FAILED (_d3dDevice->CreateTexture2D (&desc, &initialData, &d3dTexture)))
				return dseed::error_fail;

			*texture = new __d3d11_texture2d (d3dTexture);
			if (*texture == nullptr)
				return dseed::error_out_of_memory;

			return dseed::error_good;
		}
	}

public:
	virtual dseed::error_t create_rendertarget (dseed::texture2d* texture, dseed::rendertarget2d** rt) override
	{
		return dseed::error_not_impl;
	}
	virtual dseed::error_t create_depthstencil (dseed::texture2d* texture, dseed::depthstencil** ds) override
	{
		return dseed::error_not_impl;
	}

public:
	virtual dseed::error_t copy_resource (dseed::texture2d* texture, dseed::bitmap* copy_to, int mipLevel = 0)
	{
		if (texture == nullptr || copy_to == nullptr)
			return dseed::error_invalid_args;

		if (copy_to->type () != dseed::bitmaptype_2d)
			return dseed::error_invalid_args;

		auto texSize = texture->size ();
		auto bmpSize = copy_to->size ();
		auto texSize1 = dseed::get_mipmap_size (mipLevel, dseed::size3i (texSize.width, texSize.height, 1), false);

		if (texSize1.width != bmpSize.width || texSize1.height != bmpSize.height || texture->format () != copy_to->format ())
			return dseed::error_invalid_args;

		if (mipLevel < 0 || mipLevel >= texture->mip_level ())
			return dseed::error_invalid_args;

		CComPtr<ID3D11Texture2D> d3dTexture;
		if (dseed::failed (texture->native_object ((void**)&d3dTexture)))
			return dseed::error_fail;

		CComPtr<ID3D11Texture2D> staging;
		D3D11_TEXTURE2D_DESC desc = {};
		d3dTexture->GetDesc (&desc);
		desc.Usage = D3D11_USAGE_STAGING;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		if (FAILED (_d3dDevice->CreateTexture2D (&desc, nullptr, &staging)))
			return dseed::error_fail;

		_immediateContext->CopyResource (staging, d3dTexture);

		D3D11_MAPPED_SUBRESOURCE mapped;
		if (FAILED (_immediateContext->Map (staging, mipLevel, D3D11_MAP_READ, 0, &mapped)))
			return dseed::error_fail;

		void* bitmapData;
		copy_to->pixels_pointer (&bitmapData);

		memcpy (bitmapData, mapped.pData, dseed::get_bitmap_plane_size (texture->format (), texSize1.width, texSize1.height));

		_immediateContext->Unmap (staging, mipLevel);

		return dseed::error_good;
	}
	virtual dseed::error_t copy_resource (dseed::bitmap* bmp, dseed::texture2d* copy_to, int mipLevel = 0) override
	{
		if (bmp == nullptr || copy_to == nullptr)
			return dseed::error_invalid_args;

		if (bmp->type () != dseed::bitmaptype_2d)
			return dseed::error_invalid_args;

		auto bmpSize = bmp->size ();
		auto texSize = copy_to->size ();
		auto texSize1 = dseed::get_mipmap_size (mipLevel, dseed::size3i (texSize.width, texSize.height, 1), false);

		if (texSize1.width != bmpSize.width || texSize1.height != bmpSize.height || bmp->format () != copy_to->format ())
			return dseed::error_invalid_args;

		if (mipLevel < 0 || mipLevel >= copy_to->mip_level ())
			return dseed::error_invalid_args;

		CComPtr<ID3D11Texture2D> d3dTexture;
		if (dseed::failed (copy_to->native_object ((void**)&d3dTexture)))
			return dseed::error_fail;

		CComPtr<ID3D11Texture2D> staging;
		D3D11_TEXTURE2D_DESC desc = {};
		d3dTexture->GetDesc (&desc);
		desc.Usage = D3D11_USAGE_STAGING;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		if (FAILED (_d3dDevice->CreateTexture2D (&desc, nullptr, &staging)))
			return dseed::error_fail;

		_immediateContext->CopyResource (staging, d3dTexture);

		D3D11_MAPPED_SUBRESOURCE mapped;
		if (FAILED (_immediateContext->Map (staging, mipLevel, D3D11_MAP_WRITE, 0, &mapped)))
			return dseed::error_fail;

		void* bitmapData;
		bmp->pixels_pointer (&bitmapData);

		memcpy (mapped.pData, bitmapData, dseed::get_bitmap_plane_size (copy_to->format (), texSize1.width, texSize1.height));

		_immediateContext->Unmap (staging, mipLevel);

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;

	dseed::auto_object<dseed::vga_adapter> _vgaAdapter;
	CComPtr<ID3D11Device> _d3dDevice;
	CComPtr<ID3D11DeviceContext> _immediateContext;
};

#endif

dseed::error_t dseed::create_d3d11_vga_device (vga_adapter* adapter, vga_device** device)
{
#if PLATFORM_MICROSOFT
	CComPtr<IDXGIAdapter> dxgiAdapter = nullptr;
	if (adapter != nullptr)
	{
		if (dseed::failed (adapter->native_object ((void**)&dxgiAdapter)))
			return dseed::error_fail;
	}

	DWORD flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#	if _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#	endif

	CComPtr<ID3D11Device> d3dDevice;
	CComPtr<ID3D11DeviceContext> immediateContext;
	if (FAILED (D3D11CreateDevice (dxgiAdapter, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		flags, nullptr, 0, D3D11_SDK_VERSION, &d3dDevice, nullptr, &immediateContext)))
		return dseed::error_not_support;

	*device = new __d3d11_vga_device (adapter, d3dDevice, immediateContext);
	if (*device == nullptr)
		return dseed::error_out_of_memory;

#else
	* device = nullptr;
	return dseed::error_not_support;
#endif

	return dseed::error_good;
}

#if PLATFORM_MICROSOFT
class __d3d11_vga_swapchain : public dseed::vga_swapchain
{
public:
	__d3d11_vga_swapchain (dseed::vga_device* device, IDXGISwapChain* swapChain)
		: _refCount (1), _sync (false)
	{
		_device = device;
		_dxgiSwapChain = swapChain;
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
	virtual dseed::error_t native_object (void** nativeObject) override
	{
		if (nativeObject == nullptr) return dseed::error_invalid_args;
		reinterpret_cast<IDXGISwapChain*>(*nativeObject = _dxgiSwapChain)->AddRef ();
		return dseed::error_good;
	}

public:
	virtual dseed::error_t backbuffer (dseed::texture2d** texture) override
	{
		CComPtr<ID3D11Texture2D> backBuffer;
		if (FAILED (_dxgiSwapChain->GetBuffer (0, __uuidof(ID3D11Texture2D), (void**)&backBuffer)))
			return dseed::error_fail;
		*texture = new __d3d11_texture2d (backBuffer);
		if (*texture)
			return dseed::error_out_of_memory;
		return dseed::error_good;
	}

public:
	virtual bool vsync () override { return _sync; }
	virtual dseed::error_t set_vsync (bool vsync) override
	{
		_sync = vsync;
		return dseed::error_good;
	}

public:
	virtual dseed::error_t present () override
	{
		if (FAILED (_dxgiSwapChain->Present (_sync ? 1 : 0, 0)))
			return dseed::error_fail;
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	dseed::auto_object<dseed::vga_device> _device;
	CComPtr<IDXGISwapChain> _dxgiSwapChain;
	bool _sync;
};
#endif

dseed::error_t dseed::create_d3d11_vga_swapchain (dseed::application* app, dseed::vga_device* device, dseed::vga_swapchain** swapchain)
{
	if (app == nullptr || device == nullptr || swapchain == nullptr)
		return dseed::error_invalid_args;

#if PLATFORM_MICROSOFT
	dseed::size2i clientSize;
	app->client_size (&clientSize);

	CComPtr<ID3D11Device> d3dDevice;
	device->native_object ((void**)&d3dDevice);

	CComPtr<IDXGIFactory1> dxgiFactory;
	if (FAILED (CreateDXGIFactory1 (__uuidof(IDXGIFactory1), (void**)&dxgiFactory)))
		return dseed::error_not_support;

	CComQIPtr<IDXGISwapChain> dxgiSwapChain;
	if (IsWindows8OrGreater ())
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		if (IsWindows10OrGreater ())
		{
			swapChainDesc.BufferCount = 2;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		}
		else
		{
#	if PLATFORM_WINDOWS
			swapChainDesc.BufferCount = 1;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
#	else
			swapChainDesc.BufferCount = 2;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
#	endif
		}
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Width = clientSize.width;
		swapChainDesc.Height = clientSize.height;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		if (IsWindows8Point1OrGreater ())
		{
			swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
			if (IsWindows10OrGreater ())
			{
				swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
			}
		}

		CComQIPtr<IDXGIFactory2> dxgiFactory2 = dxgiFactory;
		if (dxgiFactory2 == nullptr)
			return dseed::error_fail;

		CComPtr<IDXGISwapChain1> dxgiSwapChain1;
#	if PLATFORM_WINDOWS
		HWND hWnd;
		app->native_object ((void**)&hWnd);

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = {};
		fullscreenDesc.RefreshRate.Numerator = 0;
		fullscreenDesc.RefreshRate.Denominator = 1;
		fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		fullscreenDesc.Windowed = true;

		if (FAILED (dxgiFactory2->CreateSwapChainForHwnd (d3dDevice, hWnd,
			&swapChainDesc, &fullscreenDesc, nullptr, &dxgiSwapChain1)))
			return dseed::error_fail;
#	elif PLATFORM_UWP
		Windows::UI::Core::CoreWindow^ window;
		app->native_object ((void**)&window);

		if (FAILED (dxgiFactory2->CreateSwapChainForCoreWindow (d3dDevice, window, &swapChainDesc, nullptr, &dxgiSwapChain1)))
			return dseed::error_fail;
#	else
#		error "Not support this platform."
#	endif

		if (IsWindows8Point1OrGreater ())
		{
			CComQIPtr<IDXGISwapChain2> dxgiSwapChain2 = dxgiSwapChain1;
			if (dxgiSwapChain2 != nullptr)
				dxgiSwapChain2->SetMaximumFrameLatency (0);
		}

		dxgiSwapChain = dxgiSwapChain1;
	}
#	if PLATFORM_WINDOWS
	else
	{
		HWND hWnd;
		app->native_object ((void**)&hWnd);

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferDesc.Width = clientSize.width;
		swapChainDesc.BufferDesc.Height = clientSize.height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		swapChainDesc.Windowed = true;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.OutputWindow = hWnd;

		if (FAILED (dxgiFactory->CreateSwapChain (d3dDevice, &swapChainDesc, &dxgiSwapChain)))
			return dseed::error_fail;
	}
#	endif

	* swapchain = new __d3d11_vga_swapchain (device, dxgiSwapChain);
	if (*swapchain == nullptr)
		return dseed::error_out_of_memory;
#else
	* swapchain = nullptr;
	return dseed::error_not_impl;
#endif

	return dseed::error_good;
}