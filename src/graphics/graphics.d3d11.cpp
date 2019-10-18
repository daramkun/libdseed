#include <dseed.h>

#if NTDDI_VERSION >= NTDDI_WIN10
#	include <d3d11_4.h>
#	include <dxgi1_4.h>
#else
#	include <d3d11.h>
#	include <dxgi.h>
#endif
#include <atlbase.h>

class __d3d11_vga_device : public dseed::vga_device
{
public:
	__d3d11_vga_device ()
		: _refCount (1)
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
		return dseed::error_not_impl;
	}

public:
	virtual dseed::error_t adapter (dseed::vga_adapter** adapter) override
	{
		return dseed::error_not_impl;
	}

public:
	virtual bool is_support_format (dseed::pixelformat pf) override
	{
		return false;
	}
	virtual bool is_support_parallel_render () override
	{
		return false;
	}

private:
	std::atomic<int32_t> _refCount;

	CComPtr<ID3D11Device> _d3dDevice;
};

dseed::error_t dseed::create_d3d11_vga_device (vga_adapter* adapter, vga_device** device)
{
	return error_t ();
}