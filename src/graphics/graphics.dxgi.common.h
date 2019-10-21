#ifndef __DSEED_INTERNAL_GRAPHICS_DXGI_COMMON_H__
#define __DSEED_INTERNAL_GRAPHICS_DXGI_COMMON_H__

#include "../libs/Microsoft/DDSCommon.hxx"

constexpr DXGI_FORMAT DSEEDPF_TO_DXGIPF (dseed::pixelformat pf) noexcept
{
	switch (pf)
	{
	case dseed::pixelformat::rgba8888: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case dseed::pixelformat::rgbaf: return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case dseed::pixelformat::bgra8888: return DXGI_FORMAT_B8G8R8A8_UNORM;
	case dseed::pixelformat::bgra4444: return DXGI_FORMAT_B4G4R4A4_UNORM;
	
	case dseed::pixelformat::grayscale8: return DXGI_FORMAT_R8_UNORM;
	case dseed::pixelformat::grayscalef: return DXGI_FORMAT_R32_FLOAT;
	
	case dseed::pixelformat::yuva8888: return DXGI_FORMAT_AYUV;

	case dseed::pixelformat::yuyv8888: return DXGI_FORMAT_YUY2;
	case dseed::pixelformat::nv12: return DXGI_FORMAT_NV12;

	case dseed::pixelformat::bc1: return DXGI_FORMAT_BC1_UNORM;
	case dseed::pixelformat::bc2: return DXGI_FORMAT_BC2_UNORM;
	case dseed::pixelformat::bc3: return DXGI_FORMAT_BC3_UNORM;
	case dseed::pixelformat::bc4: return DXGI_FORMAT_BC4_UNORM;
	case dseed::pixelformat::bc5: return DXGI_FORMAT_BC5_UNORM;
	case dseed::pixelformat::bc6h: return DXGI_FORMAT_BC6H_SF16;
	case dseed::pixelformat::bc7: return DXGI_FORMAT_BC7_UNORM;

	case dseed::pixelformat::depth16: return DXGI_FORMAT_D16_UNORM;
	case dseed::pixelformat::depth24_stencil8:  return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case dseed::pixelformat::depth32: return DXGI_FORMAT_D32_FLOAT;

	case dseed::pixelformat::astc_4x4: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_4X4_UNORM;
	case dseed::pixelformat::astc_5x4: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_5X4_UNORM;
	case dseed::pixelformat::astc_5x5: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_5X5_UNORM;
	case dseed::pixelformat::astc_6x5: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_6X5_UNORM;
	case dseed::pixelformat::astc_6x6: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_6X6_UNORM;
	case dseed::pixelformat::astc_8x5: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_8X5_UNORM;
	case dseed::pixelformat::astc_8x6: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_8X6_UNORM;
	case dseed::pixelformat::astc_8x8: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_8X8_UNORM;
	case dseed::pixelformat::astc_10x5: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_10X5_UNORM;
	case dseed::pixelformat::astc_10x6: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_10X6_UNORM;
	case dseed::pixelformat::astc_10x10: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_10X10_UNORM;
	case dseed::pixelformat::astc_12x10: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_12X10_UNORM;
	case dseed::pixelformat::astc_12x12: return (DXGI_FORMAT)DXGI_FORMAT_ASTC_12X12_UNORM;

	default: return DXGI_FORMAT_UNKNOWN;
	}
}

constexpr dseed::pixelformat DXGIPF_TO_DSEEDPF (DXGI_FORMAT pf) noexcept
{
	switch (pf)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM: return dseed::pixelformat::rgba8888;
	case DXGI_FORMAT_R32G32B32A32_FLOAT: return dseed::pixelformat::rgbaf;
	case DXGI_FORMAT_B8G8R8A8_UNORM: return dseed::pixelformat::bgra8888;
	case DXGI_FORMAT_B4G4R4A4_UNORM: return dseed::pixelformat::bgra4444;

	case DXGI_FORMAT_R8_UNORM: return dseed::pixelformat::grayscale8;
	case DXGI_FORMAT_R32_FLOAT: return dseed::pixelformat::grayscalef;

	case DXGI_FORMAT_AYUV: return dseed::pixelformat::yuva8888;

	case DXGI_FORMAT_YUY2: return dseed::pixelformat::yuyv8888;
	case DXGI_FORMAT_NV12: return dseed::pixelformat::nv12;

	case DXGI_FORMAT_BC1_UNORM: return dseed::pixelformat::bc1;
	case DXGI_FORMAT_BC2_UNORM: return dseed::pixelformat::bc2;
	case DXGI_FORMAT_BC3_UNORM: return dseed::pixelformat::bc3;
	case DXGI_FORMAT_BC4_UNORM: return dseed::pixelformat::bc4;
	case DXGI_FORMAT_BC5_UNORM: return dseed::pixelformat::bc5;
	case DXGI_FORMAT_BC6H_SF16: return dseed::pixelformat::bc6h;
	case DXGI_FORMAT_BC7_UNORM: return dseed::pixelformat::bc7;

	case DXGI_FORMAT_D16_UNORM: return dseed::pixelformat::depth16;
	case DXGI_FORMAT_D24_UNORM_S8_UINT: return dseed::pixelformat::depth24_stencil8;
	case DXGI_FORMAT_D32_FLOAT: return dseed::pixelformat::depth32;

	case DXGI_FORMAT_ASTC_4X4_UNORM: return dseed::pixelformat::astc_4x4;
	case DXGI_FORMAT_ASTC_5X4_UNORM: return dseed::pixelformat::astc_5x4;
	case DXGI_FORMAT_ASTC_5X5_UNORM: return dseed::pixelformat::astc_5x5;
	case DXGI_FORMAT_ASTC_6X5_UNORM: return dseed::pixelformat::astc_6x5;
	case DXGI_FORMAT_ASTC_6X6_UNORM: return dseed::pixelformat::astc_6x6;
	case DXGI_FORMAT_ASTC_8X5_UNORM: return dseed::pixelformat::astc_8x5;
	case DXGI_FORMAT_ASTC_8X6_UNORM: return dseed::pixelformat::astc_8x6;
	case DXGI_FORMAT_ASTC_8X8_UNORM: return dseed::pixelformat::astc_8x8;
	case DXGI_FORMAT_ASTC_10X5_UNORM: return dseed::pixelformat::astc_10x5;
	case DXGI_FORMAT_ASTC_10X6_UNORM: return dseed::pixelformat::astc_10x6;
	case DXGI_FORMAT_ASTC_10X10_UNORM: return dseed::pixelformat::astc_10x10;
	case DXGI_FORMAT_ASTC_12X10_UNORM: return dseed::pixelformat::astc_12x10;
	case DXGI_FORMAT_ASTC_12X12_UNORM: return dseed::pixelformat::astc_12x12;

	default: return dseed::pixelformat::unknown;
	}
}

#endif