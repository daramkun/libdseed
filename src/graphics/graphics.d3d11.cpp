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
	virtual int32_t retain () override
	{
		return int32_t ();
	}
	virtual int32_t release () override
	{
		return int32_t ();
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

public:
	virtual dseed::error_t create_texture1d (int32_t size, dseed::pixelformat pf, dseed::texture1d** texture) override
	{
		return dseed::error_not_impl;
	}
	virtual dseed::error_t create_texture2d (const dseed::size2i& size, dseed::pixelformat pf, dseed::texture2d** texture) override
	{
		return dseed::error_not_impl;
	}
	virtual dseed::error_t create_texture3d (const dseed::size3i& size, dseed::pixelformat pf, dseed::texture3d** texture) override
	{
		return dseed::error_not_impl;
	}
	virtual dseed::error_t create_texturecube (const dseed::size2i& size, dseed::pixelformat pf, dseed::texturecube** texture) override
	{
		return dseed::error_not_impl;
	}

public:
	virtual dseed::error_t create_rendertarget2d (dseed::texture2d* texture, dseed::rendertarget2d** rendertarget) override
	{
		return dseed::error_not_impl;
	}

public:
	virtual dseed::error_t create_vertexbuffer (size_t stride, size_t length, dseed::vertexbuffer** buffer) override
	{
		return dseed::error_not_impl;
	}
	virtual dseed::error_t create_indexbuffer (size_t length, int bpp, dseed::vertexbuffer** buffer) override
	{
		return dseed::error_not_impl;
	}
	virtual dseed::error_t create_constantbuffer (size_t size, dseed::vertexbuffer** buffer) override
	{
		return dseed::error_not_impl;
	}

public:
	virtual dseed::error_t create_vertexshader (dseed::blob* shaderData, dseed::vertexshader** shader) override
	{
		return dseed::error_not_impl;
	}
	virtual dseed::error_t create_pixelshader (dseed::blob* shaderData, dseed::pixelshader** shader) override
	{
		return dseed::error_not_impl;
	}
	virtual dseed::error_t create_geometryshader (dseed::blob* shaderData, dseed::geometryshader** shader) override
	{
		return dseed::error_not_impl;
	}
	virtual dseed::error_t create_hullshader (dseed::blob* shaderData, dseed::hullshader** shader) override
	{
		return dseed::error_not_impl;
	}
	virtual dseed::error_t create_domainshader (dseed::blob* shaderData, dseed::domainshader** shader) override
	{
		return dseed::error_not_impl;
	}
	virtual dseed::error_t create_computeshader (dseed::blob* shaderData, dseed::computeshader** shader) override
	{
		return dseed::error_not_impl;
	}

public:
	virtual dseed::error_t create_program (dseed::vertexshader* vs, dseed::pixelshader* ps, dseed::geometryshader* gs, dseed::hullshader* hs, dseed::domainshader* ds, dseed::program** program) override
	{
		return dseed::error_not_impl;
	}
	virtual dseed::error_t create_program (dseed::computeshader* cs, dseed::program** program) override
	{
		return dseed::error_not_impl;
	}

public:
	virtual dseed::error_t create_context (dseed::vga_commandbuffer** contex) override
	{
		return dseed::error_not_impl;
	}

private:
	std::atomic<int32_t> _refCount;

	CComPtr<ID3D11Device> _d3dDevice;
};

dseed::error_t dseed::create_d3d11_vga_device (vga_adapter* adapter, vga_device** device)
{
	return error_t ();
}