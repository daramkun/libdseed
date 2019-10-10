#include <dseed.h>

size_t dseed::get_bitmap_stride (dseed::pixelformat format, int32_t width) noexcept
{
	using namespace dseed;
	switch (format)
	{
	case pixelformat::rgbaf:
		return width * 16;

	case pixelformat::rgba8888:
	case pixelformat::bgra8888:
	case pixelformat::yuva8888:
	case pixelformat::hsva8888:
	case pixelformat::grayscalef:
		return width * 4;

	case pixelformat::rgb888:
	case pixelformat::bgr888:
	case pixelformat::yuv888:
	case pixelformat::hsv888:
		return 4 * ((width * ((24 + 7) / 8) + 3) / 4);

	case pixelformat::bgra4444:
	case pixelformat::bgr565:
		return width * 2;

	case pixelformat::bgr888_indexed8:
	case pixelformat::bgra8888_indexed8:
	case pixelformat::grayscale8:
		return width;

	case pixelformat::yuyv8888:
		return (((width + 2) / 4) * 4) * 2;

	default: return 0;
	}
}
size_t dseed::get_bitmap_plane_size (dseed::pixelformat format, int32_t width, int32_t height) noexcept
{
	using namespace dseed;
	switch (format)
	{
	case pixelformat::rgbaf:
	case pixelformat::rgba8888:
	case pixelformat::bgra8888:
	case pixelformat::rgb888:
	case pixelformat::bgr888:
	case pixelformat::bgra4444:
	case pixelformat::bgr565:
	case pixelformat::bgr888_indexed8:
	case pixelformat::bgra8888_indexed8:
	case pixelformat::grayscale8:
	case pixelformat::grayscalef:
	case pixelformat::yuv888:
	case pixelformat::yuva8888:
	case pixelformat::hsv888:
	case pixelformat::hsva8888:
		return get_bitmap_stride (format, width) * height;

	case pixelformat::yuyv8888:
		return (size_t)(ceil (width / 2.0) * height) * 4;
	case pixelformat::yyyyuv888888:
		return (size_t)(width * height + ((ceil (width / 2.0) * 2) * ceil (height / 2.0)));

	case pixelformat::bc1:
	case pixelformat::bc4:
	case pixelformat::etc1:
	case pixelformat::etc2:
	case pixelformat::pvrtc_2bpp:
	case pixelformat::pvrtc_2abpp:
	case pixelformat::pvrtc_4bpp:
	case pixelformat::pvrtc_4abpp:
		return (size_t)(ceil (width / 4.0) * ceil (height / 4.0) * 8);

	case pixelformat::bc2:
	case pixelformat::bc3:
	case pixelformat::bc5:
	case pixelformat::bc6h:
	case pixelformat::bc7:
	case pixelformat::etc2a:
	case pixelformat::etc2a8:
	case pixelformat::astc_4x4:
		return (size_t)(ceil (width / 4.0) * ceil (height / 4.0) * 16);

	case pixelformat::astc_5x4: return (size_t)(ceil (width / 5.0) * ceil (height / 4.0) * 16);
	case pixelformat::astc_5x5: return (size_t)(ceil (width / 5.0) * ceil (height / 5.0) * 16);
	case pixelformat::astc_6x5: return (size_t)(ceil (width / 6.0) * ceil (height / 4.0) * 16);
	case pixelformat::astc_6x6: return (size_t)(ceil (width / 6.0) * ceil (height / 6.0) * 16);
	case pixelformat::astc_8x5: return (size_t)(ceil (width / 8.0) * ceil (height / 5.0) * 16);
	case pixelformat::astc_8x6: return (size_t)(ceil (width / 8.0) * ceil (height / 6.0) * 16);
	case pixelformat::astc_8x8: return (size_t)(ceil (width / 8.0) * ceil (height / 8.0) * 16);
	case pixelformat::astc_10x5: return (size_t)(ceil (width / 10.0) * ceil (height / 5.0) * 16);
	case pixelformat::astc_10x6: return (size_t)(ceil (width / 10.0) * ceil (height / 6.0) * 16);
	case pixelformat::astc_10x8: return (size_t)(ceil (width / 10.0) * ceil (height / 8.0) * 16);
	case pixelformat::astc_12x10: return (size_t)(ceil (width / 12.0) * ceil (height / 10.0) * 16);
	case pixelformat::astc_12x12: return (size_t)(ceil (width / 12.0) * ceil (height / 12.0) * 16);

	default: return 0;
	}
}
size_t dseed::get_bitmap_total_size (dseed::pixelformat format, const dseed::size3i& size) noexcept
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