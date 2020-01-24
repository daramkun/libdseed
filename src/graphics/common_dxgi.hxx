#ifndef __DSEED_GRAPHICS_COMMON_DXGI_HXX__
#define __DSEED_GRAPHICS_COMMON_DXGI_HXX__

#if PLATFORM_MICROSOFT

#include <wrl.h>
#include <VersionHelpers.h>
#include <dxgi.h>
#if NTDDI_VERSION >= NTDDI_WIN8
#	include <dxgi1_2.h>
#	if NTDDI_VERSION >= NTDDI_WIN10
#		include <dxgi1_5.h>
#	endif
#endif

class __dxgi_display : public dseed::graphics::display
{
public:
	__dxgi_display (IDXGIOutput* output);

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
		_dxgiOutput->QueryInterface (__uuidof(IDXGIOutput), nativeObject);
		return dseed::error_good;
	}

public:
	virtual dseed::error_t name (char* name, size_t maxNameCount) override;

public:
	virtual dseed::error_t displaymode (int index, dseed::graphics::displaymode* mode) override;
	virtual size_t displaymode_count () override;

public:
	virtual dseed::error_t area (dseed::rect2i* area) override;

public:
	virtual dseed::error_t refresh () override;

private:
	std::atomic<int32_t> _refCount;
	Microsoft::WRL::ComPtr<IDXGIOutput> _dxgiOutput;

	std::vector<dseed::graphics::displaymode> _displayModes;
};

class __dxgi_vga_adapter : public dseed::graphics::vgaadapter
{
public:
	__dxgi_vga_adapter (IDXGIAdapter* adapter);

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
		_dxgiAdapter->QueryInterface (__uuidof(IDXGIAdapter), nativeObject);
		return dseed::error_good;
	}

public:
	virtual dseed::error_t name (char* name, size_t maxNameCount) override;

public:
	virtual dseed::error_t display (int index, dseed::graphics::display** display) override;

	virtual size_t display_count () override;

public:
	virtual dseed::error_t refresh () override;

private:
	std::atomic<int32_t> _refCount;
	Microsoft::WRL::ComPtr<IDXGIAdapter> _dxgiAdapter;

	std::vector<dseed::autoref<dseed::graphics::display>> _displays;
};

class __dxgi_vga_adapter_enumerator : public dseed::graphics::vgaadapter_enumerator
{
public:
	__dxgi_vga_adapter_enumerator (IDXGIFactory* factory);

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
	virtual dseed::error_t adapter (int index, dseed::graphics::vgaadapter** adapter) override;
	virtual size_t adapter_count () override;

public:
	virtual dseed::error_t refresh () override;

private:
	std::atomic<int32_t> _refCount;
	Microsoft::WRL::ComPtr<IDXGIFactory> _dxgiFactory;

	std::vector<dseed::autoref<dseed::graphics::vgaadapter>> _adapters;
};

#include "../libs/DDSHelper.hxx"

constexpr DXGI_FORMAT DSEEDPF_TO_DXGIPF (dseed::color::pixelformat pf) noexcept
{
	switch (pf)
	{
		case dseed::color::pixelformat::rgba8: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case dseed::color::pixelformat::rgbaf: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case dseed::color::pixelformat::bgra8: return DXGI_FORMAT_B8G8R8A8_UNORM;
		case dseed::color::pixelformat::bgra4: return DXGI_FORMAT_B4G4R4A4_UNORM;

		case dseed::color::pixelformat::r8: return DXGI_FORMAT_R8_UNORM;
		case dseed::color::pixelformat::rf: return DXGI_FORMAT_R32_FLOAT;

		case dseed::color::pixelformat::yuva8: return DXGI_FORMAT_AYUV;

		case dseed::color::pixelformat::yuyv8: return DXGI_FORMAT_YUY2;
		case dseed::color::pixelformat::nv12: return DXGI_FORMAT_NV12;

		case dseed::color::pixelformat::bc1: return DXGI_FORMAT_BC1_UNORM;
		case dseed::color::pixelformat::bc2: return DXGI_FORMAT_BC2_UNORM;
		case dseed::color::pixelformat::bc3: return DXGI_FORMAT_BC3_UNORM;
		case dseed::color::pixelformat::bc4: return DXGI_FORMAT_BC4_UNORM;
		case dseed::color::pixelformat::bc5: return DXGI_FORMAT_BC5_UNORM;
		case dseed::color::pixelformat::bc6: return DXGI_FORMAT_BC6H_SF16;
		case dseed::color::pixelformat::bc7: return DXGI_FORMAT_BC7_UNORM;

		case dseed::color::pixelformat::depth16: return DXGI_FORMAT_D16_UNORM;
		case dseed::color::pixelformat::depth24stencil8:  return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case dseed::color::pixelformat::depth32: return DXGI_FORMAT_D32_FLOAT;

		case dseed::color::pixelformat::astc4x4: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_4X4_UNORM;
		case dseed::color::pixelformat::astc5x4: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_5X4_UNORM;
		case dseed::color::pixelformat::astc5x5: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_5X5_UNORM;
		case dseed::color::pixelformat::astc6x5: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_6X5_UNORM;
		case dseed::color::pixelformat::astc6x6: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_6X6_UNORM;
		case dseed::color::pixelformat::astc8x5: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_8X5_UNORM;
		case dseed::color::pixelformat::astc8x6: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_8X6_UNORM;
		case dseed::color::pixelformat::astc8x8: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_8X8_UNORM;
		case dseed::color::pixelformat::astc10x5: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_10X5_UNORM;
		case dseed::color::pixelformat::astc10x6: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_10X6_UNORM;
		case dseed::color::pixelformat::astc10x10: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_10X10_UNORM;
		case dseed::color::pixelformat::astc12x10: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_12X10_UNORM;
		case dseed::color::pixelformat::astc12x12: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_12X12_UNORM;

		default: return DXGI_FORMAT_UNKNOWN;
	}
}

constexpr dseed::color::pixelformat DXGIPF_TO_DSEEDPF (DXGI_FORMAT pf) noexcept
{
	switch (pf)
	{
		case DXGI_FORMAT_R8G8B8A8_UNORM: return dseed::color::pixelformat::rgba8;
		case DXGI_FORMAT_R32G32B32A32_FLOAT: return dseed::color::pixelformat::rgbaf;
		case DXGI_FORMAT_B8G8R8A8_UNORM: return dseed::color::pixelformat::bgra8;
		case DXGI_FORMAT_B4G4R4A4_UNORM: return dseed::color::pixelformat::bgra4;

		case DXGI_FORMAT_R8_UNORM: return dseed::color::pixelformat::r8;
		case DXGI_FORMAT_R32_FLOAT: return dseed::color::pixelformat::rf;

		case DXGI_FORMAT_AYUV: return dseed::color::pixelformat::yuva8;

		case DXGI_FORMAT_YUY2: return dseed::color::pixelformat::yuyv8;
		case DXGI_FORMAT_NV12: return dseed::color::pixelformat::nv12;

		case DXGI_FORMAT_BC1_UNORM: return dseed::color::pixelformat::bc1;
		case DXGI_FORMAT_BC2_UNORM: return dseed::color::pixelformat::bc2;
		case DXGI_FORMAT_BC3_UNORM: return dseed::color::pixelformat::bc3;
		case DXGI_FORMAT_BC4_UNORM: return dseed::color::pixelformat::bc4;
		case DXGI_FORMAT_BC5_UNORM: return dseed::color::pixelformat::bc5;
		case DXGI_FORMAT_BC6H_SF16: return dseed::color::pixelformat::bc6;
		case DXGI_FORMAT_BC7_UNORM: return dseed::color::pixelformat::bc7;

		case DXGI_FORMAT_D16_UNORM: return dseed::color::pixelformat::depth16;
		case DXGI_FORMAT_D24_UNORM_S8_UINT: return dseed::color::pixelformat::depth24stencil8;
		case DXGI_FORMAT_D32_FLOAT: return dseed::color::pixelformat::depth32;

		case DXGI_FORMAT_ASTC_4X4_UNORM: return dseed::color::pixelformat::astc4x4;
		case DXGI_FORMAT_ASTC_5X4_UNORM: return dseed::color::pixelformat::astc5x4;
		case DXGI_FORMAT_ASTC_5X5_UNORM: return dseed::color::pixelformat::astc5x5;
		case DXGI_FORMAT_ASTC_6X5_UNORM: return dseed::color::pixelformat::astc6x5;
		case DXGI_FORMAT_ASTC_6X6_UNORM: return dseed::color::pixelformat::astc6x6;
		case DXGI_FORMAT_ASTC_8X5_UNORM: return dseed::color::pixelformat::astc8x5;
		case DXGI_FORMAT_ASTC_8X6_UNORM: return dseed::color::pixelformat::astc8x6;
		case DXGI_FORMAT_ASTC_8X8_UNORM: return dseed::color::pixelformat::astc8x8;
		case DXGI_FORMAT_ASTC_10X5_UNORM: return dseed::color::pixelformat::astc10x5;
		case DXGI_FORMAT_ASTC_10X6_UNORM: return dseed::color::pixelformat::astc10x6;
		case DXGI_FORMAT_ASTC_10X10_UNORM: return dseed::color::pixelformat::astc10x10;
		case DXGI_FORMAT_ASTC_12X10_UNORM: return dseed::color::pixelformat::astc12x10;
		case DXGI_FORMAT_ASTC_12X12_UNORM: return dseed::color::pixelformat::astc12x12;

		default: return dseed::color::pixelformat::unknown;
	}
}

#endif

#endif