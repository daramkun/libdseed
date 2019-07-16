#include <dseed.h>

size_t dseed::get_bitmap_stride (dseed::pixelformat_t format, int32_t width) noexcept
{
	using namespace dseed;
	switch (format)
	{
	case pixelformat_rgbaf:
		return width * 16;

	case pixelformat_rgba8888:
	case pixelformat_bgra8888:
	case pixelformat_yuva8888:
	case pixelformat_grayscalef:
		return width * 4;

	case pixelformat_rgb888:
	case pixelformat_bgr888:
	case pixelformat_yuv888:
		return 4 * ((width * ((24 + 7) / 8) + 3) / 4);

	case pixelformat_bgra4444:
	case pixelformat_bgr565:
		return width * 2;

	case pixelformat_bgr888_indexed8:
	case pixelformat_bgra8888_indexed8:
	case pixelformat_grayscale8:
		return width;

	case pixelformat_yuyv8888:
		return width / 2;

	default: return 0;
	}
}
size_t dseed::get_bitmap_plane_size (dseed::pixelformat_t format, int32_t width, int32_t height) noexcept
{
	using namespace dseed;
	switch (format)
	{
	case pixelformat_rgbaf:
	case pixelformat_rgba8888:
	case pixelformat_bgra8888:
	case pixelformat_rgb888:
	case pixelformat_bgr888:
	case pixelformat_bgra4444:
	case pixelformat_bgr565:
	case pixelformat_bgr888_indexed8:
	case pixelformat_bgra8888_indexed8:
	case pixelformat_grayscale8:
	case pixelformat_grayscalef:
	case pixelformat_yuv888:
	case pixelformat_yuva8888:
		return get_bitmap_stride (format, width) * height;

	case pixelformat_yuyv8888:
		return (size_t)(ceil (width / 2.0) * height) * 4;
	case pixelformat_yyyyuv888888:
		return (size_t)(width * height + (ceil (width / 2.0) * ceil (height / 2.0)) * 2);

	case pixelformat_bc1:
	case pixelformat_etc1:
	case pixelformat_etc2:
	case pixelformat_pvrtc_2bpp:
	case pixelformat_pvrtc_2abpp:
	case pixelformat_pvrtc_4bpp:
	case pixelformat_pvrtc_4abpp:
		return (size_t)(ceil (width / 4.0) * ceil (height / 4.0) * 8);

	case pixelformat_bc2:
	case pixelformat_bc3:
	case pixelformat_bc6h:
	case pixelformat_bc7:
	case pixelformat_etc2a:
	case pixelformat_etc2a8:
	case pixelformat_astc_4x4:
		return (size_t)(ceil (width / 4.0) * ceil (height / 4.0) * 16);

	case pixelformat_astc_5x4: return (size_t)(ceil (width / 5.0) * ceil (height / 4.0) * 16);
	case pixelformat_astc_5x5: return (size_t)(ceil (width / 5.0) * ceil (height / 5.0) * 16);
	case pixelformat_astc_6x5: return (size_t)(ceil (width / 6.0) * ceil (height / 4.0) * 16);
	case pixelformat_astc_6x6: return (size_t)(ceil (width / 6.0) * ceil (height / 6.0) * 16);
	case pixelformat_astc_8x5: return (size_t)(ceil (width / 8.0) * ceil (height / 5.0) * 16);
	case pixelformat_astc_8x6: return (size_t)(ceil (width / 8.0) * ceil (height / 6.0) * 16);
	case pixelformat_astc_8x8: return (size_t)(ceil (width / 8.0) * ceil (height / 8.0) * 16);
	case pixelformat_astc_10x5: return (size_t)(ceil (width / 10.0) * ceil (height / 5.0) * 16);
	case pixelformat_astc_10x6: return (size_t)(ceil (width / 10.0) * ceil (height / 6.0) * 16);
	case pixelformat_astc_10x8: return (size_t)(ceil (width / 10.0) * ceil (height / 8.0) * 16);
	case pixelformat_astc_12x10: return (size_t)(ceil (width / 12.0) * ceil (height / 10.0) * 16);
	case pixelformat_astc_12x12: return (size_t)(ceil (width / 12.0) * ceil (height / 12.0) * 16);

	default: return 0;
	}
}
size_t dseed::get_bitmap_total_size (dseed::pixelformat_t format, const dseed::size3i& size) noexcept
{
	return get_bitmap_plane_size (format, size.width, size.height) * size.depth;
}
dseed::size3i dseed::get_mipmap_size (int mipLevel, const dseed::size3i& size, bool cubemap) noexcept
{
	int halfmaker = (int)pow (2, mipLevel);
	dseed::size3i mipSize (
		(int32_t)ceil (size.width / halfmaker),
		(int32_t)ceil (size.height / halfmaker),
		cubemap ? size.depth : (int32_t)ceil (size.depth / halfmaker)
	);
	if (mipSize.width == 0) mipSize.width = 1;
	if (mipSize.height == 0) mipSize.height = 1;
	if (mipSize.depth == 0) mipSize.depth = 1;
	return mipSize;
}
size_t dseed::get_maximum_mip_levels (const dseed::size3i& size, bool cubemap) noexcept
{
	return (size_t)(1 + floor (log2 (dseed::maximum (size.width, dseed::maximum (size.height, cubemap ? size.height : size.depth)))));
}