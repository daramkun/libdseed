#include <dseed.h>

size_t dseed::color::calc_bitmap_stride(pixelformat pf, size_t width) noexcept
{
	switch (pf)
	{
	case pixelformat::rgbaf:
		return width * 16;

	case pixelformat::rgba8:
	case pixelformat::bgra8:
	case pixelformat::yuva8:
	case pixelformat::hsva8:
	case pixelformat::rf:
		return width * 4;

	case pixelformat::rgb8:
	case pixelformat::bgr8:
	case pixelformat::yuv8:
	case pixelformat::hsv8:
		return 4 * ((width * ((24 + 7) / 8) + 3) / 4);

	case pixelformat::bgra4:
	case pixelformat::bgr565:
		return width * 2;

	case pixelformat::bgr8_indexed8:
	case pixelformat::bgra8_indexed8:
	case pixelformat::r8:
		return width;

	case pixelformat::yuyv8:
		return (((width + 2) / 4) * 4) * 2;

	case pixelformat::depth16:
		return width * 2;

	case pixelformat::depth24stencil8:
	case pixelformat::depth32:
		return width * 4;

	default: return 0;
	}
}

size_t dseed::color::calc_bitmap_plane_size(pixelformat pf, const size2i& size) noexcept
{
	size_t returnValue = 0;
	switch (pf)
	{
	case pixelformat::rgbaf:
	case pixelformat::rgba8:
	case pixelformat::bgra8:
	case pixelformat::rgb8:
	case pixelformat::bgr8:
	case pixelformat::bgra4:
	case pixelformat::bgr565:
	case pixelformat::bgr8_indexed8:
	case pixelformat::bgra8_indexed8:
	case pixelformat::r8:
	case pixelformat::rf:
	case pixelformat::yuv8:
	case pixelformat::yuva8:
	case pixelformat::hsv8:
	case pixelformat::hsva8:
	case pixelformat::depth16:
	case pixelformat::depth24stencil8:
	case pixelformat::depth32:
		returnValue = calc_bitmap_stride(pf, size.width) * size.height;
		break;

	case pixelformat::yuyv8:
		returnValue = ((size_t)ceilf(size.width / 2.0f) * size.height) * 4;
		break;
		
	case pixelformat::nv12:
		returnValue = ((size_t)size.width * size.height + (((size_t)ceilf(size.width / 2.0f) * 2) * (size_t)ceilf(size.height / 2.0f)));
		break;

	case pixelformat::bc1:
	case pixelformat::bc4:
	case pixelformat::etc1:
	case pixelformat::etc2:
	case pixelformat::pvrtc_2bpp:
	case pixelformat::pvrtc_2abpp:
	case pixelformat::pvrtc_4bpp:
	case pixelformat::pvrtc_4abpp:
		returnValue = ((size_t)ceilf(size.width / 4.0f) * (size_t)ceilf(size.height / 4.0f) * 8);
		break;

	case pixelformat::bc2:
	case pixelformat::bc3:
	case pixelformat::bc5:
	case pixelformat::bc6:
	case pixelformat::bc7:
	case pixelformat::etc2a:
	case pixelformat::astc4x4:
		returnValue = ((size_t)ceilf(size.width / 4.0f) * (size_t)ceilf(size.height / 4.0f) * 16);
		break;

	case pixelformat::astc5x4: returnValue = ((size_t)ceilf(size.width / 5.0f) * (size_t)ceilf(size.height / 4.0f) * 16); break;
	case pixelformat::astc5x5: returnValue = ((size_t)ceilf(size.width / 5.0f) * (size_t)ceilf(size.height / 5.0f) * 16); break;
	case pixelformat::astc6x5: returnValue = ((size_t)ceilf(size.width / 6.0f) * (size_t)ceilf(size.height / 4.0f) * 16); break;
	case pixelformat::astc6x6: returnValue = ((size_t)ceilf(size.width / 6.0f) * (size_t)ceilf(size.height / 6.0f) * 16); break;
	case pixelformat::astc8x5: returnValue = ((size_t)ceilf(size.width / 8.0f) * (size_t)ceilf(size.height / 5.0f) * 16); break;
	case pixelformat::astc8x6: returnValue = ((size_t)ceilf(size.width / 8.0f) * (size_t)ceilf(size.height / 6.0f) * 16); break;
	case pixelformat::astc8x8: returnValue = ((size_t)ceilf(size.width / 8.0f) * (size_t)ceilf(size.height / 8.0f) * 16); break;
	case pixelformat::astc10x5: returnValue = ((size_t)ceilf(size.width / 10.0f) * (size_t)ceilf(size.height / 5.0f) * 16); break;
	case pixelformat::astc10x6: returnValue = ((size_t)ceilf(size.width / 10.0f) * (size_t)ceilf(size.height / 6.0f) * 16); break;
	case pixelformat::astc10x8: returnValue = ((size_t)ceilf(size.width / 10.0f) * (size_t)ceilf(size.height / 8.0f) * 16); break;
	case pixelformat::astc12x10: returnValue = ((size_t)ceilf(size.width / 12.0f) * (size_t)ceilf(size.height / 10.0f) * 16); break;
	case pixelformat::astc12x12: returnValue = ((size_t)ceilf(size.width / 12.0f) * (size_t)ceilf(size.height / 12.0f) * 16); break;

	default:
		returnValue = 0;
		break;
	}

	return returnValue;
}

size_t dseed::color::calc_bitmap_total_size(pixelformat pf, const size3i& size) noexcept
{
	return calc_bitmap_plane_size(pf, size2i(size.width, size.height)) * size.depth;
}

dseed::size3i dseed::color::calc_mipmap_size(int mipLevel, const size3i& size, bool cubemap) noexcept
{
	int halfmaker = (int)powf(2, (float)mipLevel);
	dseed::size3i mipSize(
		(int32_t)ceilf((float)size.width / halfmaker),
		(int32_t)ceilf((float)size.height / halfmaker),
		cubemap ? size.depth : (int32_t)ceilf((float)size.depth / halfmaker)
	);
	if (mipSize.width == 0) mipSize.width = 1;
	if (mipSize.height == 0) mipSize.height = 1;
	if (mipSize.depth == 0) mipSize.depth = 1;
	return mipSize;
}

size_t dseed::color::calc_maximum_mipmap_levels(const dseed::size3i& size, bool cubemap) noexcept
{
	auto z = cubemap ? size.height : size.depth;
	auto y = dseed::maximum(size.height, z);
	auto x = dseed::maximum(size.width, y);
	return (size_t)(1 + (size_t)floorf((float)log2(x)));
}