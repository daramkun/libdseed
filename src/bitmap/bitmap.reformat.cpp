#include <dseed.h>

#include <vector>
#include <map>

#include "../exoquant/exoquant.h"
#include "../exoquant/exoquant.c"

#include "../etc1_utils/etc1_utils.h"
#include "../etc1_utils/etc1_utils.c"

#if defined ( USE_SQUISH )
#	include <squish.h>
#endif

using namespace dseed;

#define PIXELCONV_ARGS										uint8_t* dest, const uint8_t* src, const dseed::size3i& size, uint8_t* destPalette, uint8_t* srcPalette
using pixelconvfn = std::function<int (PIXELCONV_ARGS)>;
using pixelconvtp = std::tuple<pixelformat_t, pixelformat_t>;
template<pixelformat_t destformat, pixelformat_t srcformat>
inline int pixelconv (PIXELCONV_ARGS) noexcept
{
	static_assert (true, "Not support those formats currently.");
	return 0;
}

template<> inline int pixelconv<pixelformat_rgba8888, pixelformat_bgra8888> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_rgba8888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgra8888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	size_t simdWidth = size.width / 8 * 8;
	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgba* destPtr = (rgba*)(dest + destDepthZ + destStrideY);
			const bgra* srcPtr = (const bgra*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < simdWidth; x += 8)
			{
				dseed::vectori ((const int*)(srcPtr + x))
					.permute<2, 1, 0, 3, 6, 5, 4, 7, 10, 9, 8, 11, 14, 13, 12, 15> ()
					.store ((int*)(destPtr + x));
				dseed::vectori ((const int*)(srcPtr + x + 4))
					.permute<2, 1, 0, 3, 6, 5, 4, 7, 10, 9, 8, 11, 14, 13, 12, 15> ()
					.store ((int*)(destPtr + x + 4));
			}
			for (auto x = simdWidth; x < size.width; ++x)
			{
				rgba* destPtrX = destPtr + x;
				const bgra* srcPtrX = srcPtr + x;
				destPtrX->r = srcPtrX->r;
				destPtrX->g = srcPtrX->g;
				destPtrX->b = srcPtrX->b;
				destPtrX->a = srcPtrX->a;
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_rgba8888, pixelformat_bgra4444> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_rgba8888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgra4444, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgba* destPtr = (rgba*)(dest + destDepthZ + destStrideY);
			const bgra4* srcPtr = (const bgra4*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				rgba* destPtrX = destPtr + x;
				const bgra4* srcPtrX = srcPtr + x;
				destPtrX->r = srcPtrX->r * 17;
				destPtrX->g = srcPtrX->g * 17;
				destPtrX->b = srcPtrX->b * 17;
				destPtrX->a = srcPtrX->a * 17;
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_rgba8888, pixelformat_rgb888> (PIXELCONV_ARGS) noexcept
{
	const static auto alphaMask = dseed::vectori (0xff000000);

	size_t destStride = get_bitmap_stride (pixelformat_rgba8888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_rgb888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	size_t simdWidth = size.width / 4 * 4;
	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgba* destPtr = (rgba*)(dest + destDepthZ + destStrideY);
			const rgb* srcPtr = (const rgb*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < simdWidth; x += 8)
			{
				(dseed::vectori ((const int*)(srcPtr + x))
					.permute<0, 1, 2, 0, 3, 4, 5, 0, 6, 7, 8, 0, 9, 10, 11, 0> () | alphaMask)
					.store ((int*)(destPtr + x));
				(dseed::vectori ((const int*)(srcPtr + x + 4))
					.permute<0, 1, 2, 0, 3, 4, 5, 0, 6, 7, 8, 0, 9, 10, 11, 0> () | alphaMask)
					.store ((int*)(destPtr + x + 4));
			}
			for (auto x = simdWidth; x < size.width; ++x)
			{
				rgba* destPtrX = destPtr + x;
				const rgb* srcPtrX = srcPtr + x;
				destPtrX->r = srcPtrX->r;
				destPtrX->g = srcPtrX->g;
				destPtrX->b = srcPtrX->b;
				destPtrX->a = 255;
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_rgba8888, pixelformat_bgr888> (PIXELCONV_ARGS) noexcept
{
	const static auto alphaMask = dseed::vectori (0xff000000);

	size_t destStride = get_bitmap_stride (pixelformat_rgba8888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgr888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	size_t simdWidth = size.width / 4 * 4;
	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgba* destPtr = (rgba*)(dest + destDepthZ + destStrideY);
			const bgr* srcPtr = (const bgr*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < simdWidth; x += 8)
			{
				(dseed::vectori ((const int*)(srcPtr + x))
					.permute<2, 1, 0, 0, 5, 4, 3, 0, 8, 7, 6, 0, 11, 10, 9, 0> () | alphaMask)
					.store ((int*)(destPtr + x));
				(dseed::vectori ((const int*)(srcPtr + x + 4))
					.permute<2, 1, 0, 0, 5, 4, 3, 0, 8, 7, 6, 0, 11, 10, 9, 0> () | alphaMask)
					.store ((int*)(destPtr + x + 4));
			}
			for (auto x = simdWidth; x < size.width; ++x)
			{
				rgba* destPtrX = destPtr + x;
				const bgr* srcPtrX = srcPtr + x;
				destPtrX->r = srcPtrX->r;
				destPtrX->g = srcPtrX->g;
				destPtrX->b = srcPtrX->b;
				destPtrX->a = 255;
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_rgba8888, pixelformat_rgbaf> (PIXELCONV_ARGS) noexcept
{
	size_t destStride = get_bitmap_stride (pixelformat_rgba8888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_rgbaf, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	rgba temp[4];
	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgba* destPtr = (rgba*)(dest + destDepthZ + destStrideY);
			const rgbaf* srcPtr = (const rgbaf*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				dseed::convert_vector (dseed::multiplyvf (dseed::vectorf ((const float*)(srcPtr + x)), 255))
					.permute<0, 4, 8, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0> ()
					.store ((int*)temp);
				*(destPtr + x) = temp[0];
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_rgba8888, pixelformat_grayscale8> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_rgba8888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_grayscale8, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgba* destPtr = (rgba*)(dest + destDepthZ + destStrideY);
			const uint8_t* srcPtr = (const uint8_t*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				rgba* destPtrX = destPtr + x;
				const uint8_t* srcPtrX = srcPtr + x;
				destPtrX->r = destPtrX->g = destPtrX->b = *srcPtrX;
				destPtrX->a = 255;
			}
		}
	}
	return 0;
}

template<> inline int pixelconv<pixelformat_rgb888, pixelformat_rgba8888> (PIXELCONV_ARGS) noexcept
{
	size_t destStride = get_bitmap_stride (pixelformat_rgb888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_rgba8888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	size_t simdWidth = size.width / 4 * 4;
	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgb* destPtr = (rgb*)(dest + destDepthZ + destStrideY);
			const rgba* srcPtr = (const rgba*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < simdWidth; x += 8)
			{
				dseed::vectori ((const int*)(srcPtr + x))
					.permute<0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, 0, 0, 0, 0> ()
					.store ((int*)(destPtr + x));
				dseed::vectori ((const int*)(srcPtr + x + 4))
					.permute<0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, 0, 0, 0, 0> ()
					.store ((int*)(destPtr + x + 4));
			}
			for (auto x = simdWidth; x < size.width; ++x)
			{
				rgb* destPtrX = destPtr + x;
				const rgba* srcPtrX = srcPtr + x;
				destPtrX->r = srcPtrX->r;
				destPtrX->g = srcPtrX->g;
				destPtrX->b = srcPtrX->b;
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_rgb888, pixelformat_bgra8888> (PIXELCONV_ARGS) noexcept
{
	size_t destStride = get_bitmap_stride (pixelformat_rgb888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgra8888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	size_t simdWidth = size.width / 4 * 4;
	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgb* destPtr = (rgb*)(dest + destDepthZ + destStrideY);
			const bgra* srcPtr = (const bgra*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < simdWidth; x += 8)
			{
				dseed::vectori ((const int*)(srcPtr + x))
					.permute<2, 1, 0, 6, 5, 4, 10, 9, 8, 14, 13, 12, 0, 0, 0, 0> ()
					.store ((int*)(destPtr + x));
				dseed::vectori ((const int*)(srcPtr + x + 4))
					.permute<2, 1, 0, 6, 5, 4, 10, 9, 8, 14, 13, 12, 0, 0, 0, 0> ()
					.store ((int*)(destPtr + x + 4));
			}
			for (auto x = simdWidth; x < size.width; ++x)
			{
				rgb* destPtrX = destPtr + x;
				const bgra* srcPtrX = srcPtr + x;
				destPtrX->r = srcPtrX->r;
				destPtrX->g = srcPtrX->g;
				destPtrX->b = srcPtrX->b;
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_rgb888, pixelformat_bgr888> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_rgb888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgr888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	size_t simdWidth = size.width / 5 * 5;
	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgb* destPtr = (rgb*)(dest + destDepthZ + destStrideY);
			const bgr* srcPtr = (const bgr*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < simdWidth; x += 5)
			{
				dseed::vectori ((const int*)(srcPtr + x))
					.permute<2, 1, 0, 5, 4, 3, 8, 7, 6, 11, 10, 9, 14, 13, 12, 15> ()
					.store ((int*)(destPtr + x));
			}
			for (auto x = simdWidth; x < size.width; ++x)
			{
				rgb* destPtrX = destPtr + x;
				const bgr* srcPtrX = srcPtr + x;
				destPtrX->r = srcPtrX->r;
				destPtrX->g = srcPtrX->g;
				destPtrX->b = srcPtrX->b;
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_rgb888, pixelformat_rgbaf> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_rgb888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_rgbaf, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgb* destPtr = (rgb*)(dest + destDepthZ + destStrideY);
			const rgbaf* srcPtr = (const rgbaf*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				rgb* destPtrX = destPtr + x;
				const rgbaf* srcPtrX = srcPtr + x;
				destPtrX->r = (uint8_t)(srcPtrX->r * 255.0f);
				destPtrX->g = (uint8_t)(srcPtrX->g * 255.0f);
				destPtrX->b = (uint8_t)(srcPtrX->b * 255.0f);
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_rgb888, pixelformat_grayscale8> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_rgb888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_grayscale8, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgb* destPtr = (rgb*)(dest + destDepthZ + destStrideY);
			const uint8_t* srcPtr = (const uint8_t*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				rgb* destPtrX = destPtr + x;
				const uint8_t* srcPtrX = srcPtr + x;
				destPtrX->r = destPtrX->g = destPtrX->b = *srcPtrX;
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_rgb888, pixelformat_bgra4444> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_rgb888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgra4444, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgb* destPtr = (rgb*)(dest + destDepthZ + destStrideY);
			const bgra4* srcPtr = (const bgra4*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				rgb* destPtrX = destPtr + x;
				const bgra4* srcPtrX = srcPtr + x;
				destPtrX->r = srcPtrX->r * 17;
				destPtrX->g = srcPtrX->g * 17;
				destPtrX->b = srcPtrX->b * 17;
			}
		}
	}
	return 0;
}

template<> inline int pixelconv<pixelformat_bgra8888, pixelformat_rgbaf> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgra8888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_rgbaf, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			bgra* destPtr = (bgra*)(dest + destDepthZ + destStrideY);
			const rgbaf* srcPtr = (const rgbaf*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				bgra* destPtrX = destPtr + x;
				const rgbaf* srcPtrX = srcPtr + x;
				destPtrX->r = (uint8_t)(srcPtrX->r * 255.0f);
				destPtrX->g = (uint8_t)(srcPtrX->g * 255.0f);
				destPtrX->b = (uint8_t)(srcPtrX->b * 255.0f);
				destPtrX->a = (uint8_t)(srcPtrX->a * 255.0f);
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_bgra8888, pixelformat_bgra4444> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgra8888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgra4444, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			bgra* destPtr = (bgra*)(dest + destDepthZ + destStrideY);
			const bgra4* srcPtr = (const bgra4*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				bgra* destPtrX = destPtr + x;
				const bgra4* srcPtrX = srcPtr + x;
				destPtrX->r = srcPtrX->r * 17;
				destPtrX->g = srcPtrX->g * 17;
				destPtrX->b = srcPtrX->b * 17;
				destPtrX->a = srcPtrX->a * 17;
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_bgra8888, pixelformat_rgba8888> (PIXELCONV_ARGS)
{
	return pixelconv<pixelformat_rgba8888, pixelformat_bgra8888> (dest, src, size, destPalette, srcPalette);
}
template<> inline int pixelconv<pixelformat_bgra8888, pixelformat_rgb888> (PIXELCONV_ARGS)
{
	return pixelconv<pixelformat_rgba8888, pixelformat_bgr888> (dest, src, size, destPalette, srcPalette);
}
template<> inline int pixelconv<pixelformat_bgra8888, pixelformat_bgr888> (PIXELCONV_ARGS)
{
	return pixelconv<pixelformat_rgba8888, pixelformat_rgb888> (dest, src, size, destPalette, srcPalette);
}
template<> inline int pixelconv<pixelformat_bgra8888, pixelformat_grayscale8> (PIXELCONV_ARGS)
{
	return pixelconv<pixelformat_rgba8888, pixelformat_grayscale8> (dest, src, size, destPalette, srcPalette);
}

template<> inline int pixelconv<pixelformat_bgr888, pixelformat_rgbaf> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgr888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_rgbaf, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			bgr* destPtr = (bgr*)(dest + destDepthZ + destStrideY);
			const rgbaf* srcPtr = (const rgbaf*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				bgr* destPtrX = destPtr + x;
				const rgbaf* srcPtrX = srcPtr + x;
				destPtrX->r = (uint8_t)(srcPtrX->r * 255.0f);
				destPtrX->g = (uint8_t)(srcPtrX->g * 255.0f);
				destPtrX->b = (uint8_t)(srcPtrX->b * 255.0f);
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_bgr888, pixelformat_bgra4444> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgr888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgra4444, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			bgr* destPtr = (bgr*)(dest + destDepthZ + destStrideY);
			const bgra4* srcPtr = (const bgra4*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				bgr* destPtrX = destPtr + x;
				const bgra4* srcPtrX = srcPtr + x;
				destPtrX->r = srcPtrX->r * 17;
				destPtrX->g = srcPtrX->g * 17;
				destPtrX->b = srcPtrX->b * 17;
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_bgr888, pixelformat_rgba8888> (PIXELCONV_ARGS)
{
	return pixelconv<pixelformat_rgb888, pixelformat_bgra8888> (dest, src, size, destPalette, srcPalette);
}
template<> inline int pixelconv<pixelformat_bgr888, pixelformat_rgb888> (PIXELCONV_ARGS)
{
	return pixelconv<pixelformat_rgb888, pixelformat_bgr888> (dest, src, size, destPalette, srcPalette);
}
template<> inline int pixelconv<pixelformat_bgr888, pixelformat_bgr888> (PIXELCONV_ARGS)
{
	return pixelconv<pixelformat_rgb888, pixelformat_rgb888> (dest, src, size, destPalette, srcPalette);
}
template<> inline int pixelconv<pixelformat_bgr888, pixelformat_grayscale8> (PIXELCONV_ARGS)
{
	return pixelconv<pixelformat_rgb888, pixelformat_grayscale8> (dest, src, size, destPalette, srcPalette);
}

template<> inline int pixelconv<pixelformat_bgra4444, pixelformat_rgba8888> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgra4444, size.width)
		, srcStride = get_bitmap_stride (pixelformat_rgba8888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;

		rgba c = *(const rgba*)(src + srcDepthZ) / rgba (17);
		*((bgra4*)(dest + destDepthZ)) = bgra4 (c.r, c.g, c.b, c.a);

		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			bgra4* destPtr = (bgra4*)(dest + destDepthZ + destStrideY);
			const rgba* srcPtr = (const rgba*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				bgra4* destPtrX = destPtr + x;
				const rgba* srcPtrX = srcPtr + x;

				rgba oldPixel = *srcPtrX / rgba (17);
				rgba newPixel (destPtrX->r, destPtrX->g, destPtrX->b, destPtrX->a);
				rgba quantError = oldPixel - newPixel;
				if (x + 1 < size.width)
				{
					rgba calc = *(srcPtrX + 1) / rgba (17) + quantError * rgba (7) / rgba (16);
					*(destPtrX + 1) = bgra4 (calc.r, calc.g, calc.b, calc.a);
				}
				if (y + 1 < size.height)
				{
					rgba calc = *((const rgba*)(src + srcDepthZ + ((y + 1) * srcStride))) / rgba (17)
						+ quantError * rgba (3) / rgba (16);
					*((bgra4*)(dest + destDepthZ + ((y + 1) * destStride)))
						= bgra4 (calc.r, calc.g, calc.b, calc.a);
				}
				if (x - 1 >= 0 && y + 1 < size.height)
				{
					rgba calc = *(((const rgba*)(src + srcDepthZ + ((y + 1) * srcStride))) - 1) / rgba (17)
						+ quantError * rgba (5) / rgba (16);
					*(((bgra4*)(dest + destDepthZ + ((y + 1) * destStride))) - 1)
						= bgra4 (calc.r, calc.g, calc.b, calc.a);
				}
				if (x + 1 < size.width && y + 1 < size.height)
				{
					rgba calc = *(((const rgba*)(src + srcDepthZ + ((y + 1) * srcStride))) + 1) / rgba (17)
						+ quantError / rgba (16);
					*(((bgra4*)(dest + destDepthZ + ((y + 1) * destStride))) + 1)
						= bgra4 (calc.r, calc.g, calc.b, calc.a);
				}
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_bgra4444, pixelformat_rgbaf> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgra4444, size.width)
		, srcStride = get_bitmap_stride (pixelformat_rgbaf, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;

		rgbaf c = *(const rgbaf*)(src + srcDepthZ) * rgbaf (15);
		*((bgra4*)(dest + destDepthZ)) = bgra4 ((uint8_t)c.r, (uint8_t)c.g, (uint8_t)c.b, (uint8_t)c.a);

		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			bgra4* destPtr = (bgra4*)(dest + destDepthZ + destStrideY);
			const rgbaf* srcPtr = (const rgbaf*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				bgra4* destPtrX = destPtr + x;
				const rgbaf* srcPtrX = srcPtr + x;

				rgbaf oldPixel = *srcPtrX * rgbaf (15);
				rgbaf newPixel (destPtrX->r, destPtrX->g, destPtrX->b, destPtrX->a);
				rgbaf quantError = oldPixel - newPixel;
				if (x + 1 < size.width)
				{
					rgbaf calc = *(srcPtrX + 1) * rgbaf (15) + quantError * rgbaf (7) / rgbaf (16);
					*(destPtrX + 1) = bgra4 ((uint8_t)calc.r, (uint8_t)calc.g, (uint8_t)calc.b, (uint8_t)calc.a);
				}
				if (y + 1 < size.height)
				{
					rgbaf calc = *((const rgbaf*)(src + srcDepthZ + ((y + 1) * srcStride))) * rgbaf (15)
						+ quantError * rgbaf (3) / rgbaf (16);
					*((bgra4*)(dest + destDepthZ + ((y + 1) * destStride)))
						= bgra4 ((uint8_t)calc.r, (uint8_t)calc.g, (uint8_t)calc.b, (uint8_t)calc.a);
				}
				if (x - 1 >= 0 && y + 1 < size.height)
				{
					rgbaf calc = *(((const rgbaf*)(src + srcDepthZ + ((y + 1) * srcStride))) - 1) * rgbaf (15)
						+ quantError * rgbaf (5) / rgbaf (16);
					*(((bgra4*)(dest + destDepthZ + ((y + 1) * destStride))) - 1)
						= bgra4 ((uint8_t)calc.r, (uint8_t)calc.g, (uint8_t)calc.b, (uint8_t)calc.a);
				}
				if (x + 1 < size.width && y + 1 < size.height)
				{
					rgbaf calc = *(((const rgbaf*)(src + srcDepthZ + ((y + 1) * srcStride))) + 1) * rgbaf (15)
						+ quantError / rgbaf (16);
					*(((bgra4*)(dest + destDepthZ + ((y + 1) * destStride))) + 1)
						= bgra4 ((uint8_t)calc.r, (uint8_t)calc.g, (uint8_t)calc.b, (uint8_t)calc.a);
				}
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_bgra4444, pixelformat_bgra8888> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgra4444, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgra8888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;

		bgra c = *(const bgra*)(src + srcDepthZ) / bgra (17);
		*((bgra4*)(dest + destDepthZ)) = bgra4 (c.r, c.g, c.b, c.a);

		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			bgra4* destPtr = (bgra4*)(dest + destDepthZ + destStrideY);
			const bgra* srcPtr = (const bgra*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				bgra4* destPtrX = destPtr + x;
				const bgra* srcPtrX = srcPtr + x;

				bgra oldPixel = *srcPtrX / bgra (17);
				bgra newPixel (destPtrX->r, destPtrX->g, destPtrX->b, destPtrX->a);
				bgra quantError = oldPixel - newPixel;
				if (x + 1 < size.width)
				{
					bgra calc = *(srcPtrX + 1) / bgra (17) + quantError * bgra (7) / bgra (16);
					*(destPtrX + 1) = bgra4 (calc.r, calc.g, calc.b, calc.a);
				}
				if (y + 1 < size.height)
				{
					bgra calc = *((const bgra*)(src + srcDepthZ + ((y + 1) * srcStride))) / bgra (17)
						+ quantError * bgra (3) / bgra (16);
					*((bgra4*)(dest + destDepthZ + ((y + 1) * destStride)))
						= bgra4 (calc.r, calc.g, calc.b, calc.a);
				}
				if (x - 1 >= 0 && y + 1 < size.height)
				{
					bgra calc = *(((const bgra*)(src + srcDepthZ + ((y + 1) * srcStride))) - 1) / bgra (17)
						+ quantError * bgra (5) / bgra (16);
					*(((bgra4*)(dest + destDepthZ + ((y + 1) * destStride))) - 1)
						= bgra4 (calc.r, calc.g, calc.b, calc.a);
				}
				if (x + 1 < size.width && y + 1 < size.height)
				{
					bgra calc = *(((const bgra*)(src + srcDepthZ + ((y + 1) * srcStride))) + 1) / bgra (17)
						+ quantError / bgra (16);
					*(((bgra4*)(dest + destDepthZ + ((y + 1) * destStride))) + 1)
						= bgra4 (calc.r, calc.g, calc.b, calc.a);
				}
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_bgra4444, pixelformat_rgb888> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgra4444, size.width)
		, srcStride = get_bitmap_stride (pixelformat_rgb888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;

		rgb c = *(const rgb*)(src + srcDepthZ) / rgb (17);
		*((bgra4*)(dest + destDepthZ)) = bgra4 (c.r, c.g, c.b, 15);

		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			bgra4* destPtr = (bgra4*)(dest + destDepthZ + destStrideY);
			const rgb* srcPtr = (const rgb*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				bgra4* destPtrX = destPtr + x;
				const rgb* srcPtrX = srcPtr + x;

				rgb oldPixel = *srcPtrX / rgb (17);
				rgb newPixel (destPtrX->r, destPtrX->g, destPtrX->b);
				rgb quantError = oldPixel - newPixel;
				if (x + 1 < size.width)
				{
					rgb calc = *(srcPtrX + 1) / rgb (17) + quantError * rgb (7) / rgb (16);
					*(destPtrX + 1) = bgra4 (calc.r, calc.g, calc.b, 15);
				}
				if (y + 1 < size.height)
				{
					rgb calc = *((const rgb*)(src + srcDepthZ + ((y + 1) * srcStride))) / rgb (17)
						+ quantError * rgb (3) / rgb (16);
					*((bgra4*)(dest + destDepthZ + ((y + 1) * destStride)))
						= bgra4 (calc.r, calc.g, calc.b, 15);
				}
				if (x - 1 >= 0 && y + 1 < size.height)
				{
					rgb calc = *(((const rgb*)(src + srcDepthZ + ((y + 1) * srcStride))) - 1) / rgb (17)
						+ quantError * rgb (5) / rgb (16);
					*(((bgra4*)(dest + destDepthZ + ((y + 1) * destStride))) - 1)
						= bgra4 (calc.r, calc.g, calc.b, 15);
				}
				if (x + 1 < size.width && y + 1 < size.height)
				{
					rgb calc = *(((const rgb*)(src + srcDepthZ + ((y + 1) * srcStride))) + 1) / rgb (17)
						+ quantError / rgb (16);
					*(((bgra4*)(dest + destDepthZ + ((y + 1) * destStride))) + 1)
						= bgra4 (calc.r, calc.g, calc.b, 15);
				}
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_bgra4444, pixelformat_bgr888> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgra4444, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgr888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;

		
		bgr c = *(const bgr*)(src + srcDepthZ) / bgr (17);
		*((bgra4*)(dest + destDepthZ)) = bgra4 (c.r, c.g, c.b, 15);

		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			bgra4* destPtr = (bgra4*)(dest + destDepthZ + destStrideY);
			const bgr* srcPtr = (const bgr*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				bgra4* destPtrX = destPtr + x;
				const bgr* srcPtrX = srcPtr + x;

				bgr oldPixel = *srcPtrX / bgr (17);
				bgr newPixel (destPtrX->r, destPtrX->g, destPtrX->b);
				bgr quantError = oldPixel - newPixel;
				if (x + 1 < size.width)
				{
					bgr calc = *(srcPtrX + 1) / bgr (17) + quantError * bgr (7) / bgr (16);
					*(destPtrX + 1) = bgra4 (calc.r, calc.g, calc.b, 15);
				}
				if (y + 1 < size.height)
				{
					bgr calc = *((const bgr*)(src + srcDepthZ + ((y + 1) * srcStride))) / bgr (17)
						+ quantError * bgr (3) / bgr (16);
					*((bgra4*)(dest + destDepthZ + ((y + 1) * destStride)))
						= bgra4 (calc.r, calc.g, calc.b, 15);
				}
				if (x - 1 >= 0 && y + 1 < size.height)
				{
					bgr calc = *(((const bgr*)(src + srcDepthZ + ((y + 1) * srcStride))) - 1) / bgr (17)
						+ quantError * bgr (5) / bgr (16);
					*(((bgra4*)(dest + destDepthZ + ((y + 1) * destStride))) - 1)
						= bgra4 (calc.r, calc.g, calc.b, 15);
				}
				if (x + 1 < size.width && y + 1 < size.height)
				{
					bgr calc = *(((const bgr*)(src + srcDepthZ + ((y + 1) * srcStride))) + 1) / bgr (17)
						+ quantError / bgr (16);
					*(((bgra4*)(dest + destDepthZ + ((y + 1) * destStride))) + 1)
						= bgra4 (calc.r, calc.g, calc.b, 15);
				}
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_bgra4444, pixelformat_grayscale8> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgra4444, size.width)
		, srcStride = get_bitmap_stride (pixelformat_grayscale8, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			bgra4* destPtr = (bgra4*)(dest + destDepthZ + destStrideY);
			const uint8_t* srcPtr = (const uint8_t*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				bgra4* destPtrX = destPtr + x;
				const uint8_t* srcPtrX = srcPtr + x;
				destPtrX->r = destPtrX->g = destPtrX->b = *srcPtrX;
				destPtrX->a = 15;
			}
		}
	}
	return 0;
}

template<> inline int pixelconv<pixelformat_bgr565, pixelformat_bgr888> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgr565, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgr888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			bgr565* destPtr = (bgr565*)(dest + destDepthZ + destStrideY);
			const bgr* srcPtr = (const bgr*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				bgr565* destPtrX = destPtr + x;
				const bgr* srcPtrX = srcPtr + x;
				destPtrX->r = ceil (srcPtrX->r / 8);
				destPtrX->g = ceil (srcPtrX->g / 4);
				destPtrX->b = ceil (srcPtrX->b / 8);
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_bgr888, pixelformat_bgr565> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgr888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgr565, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			bgr* destPtr = (bgr*)(dest + destDepthZ + destStrideY);
			const bgr565* srcPtr = (const bgr565*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				bgr* destPtrX = destPtr + x;
				const bgr565* srcPtrX = srcPtr + x;
				destPtrX->r = srcPtrX->r * 8;
				destPtrX->g = srcPtrX->g * 4;
				destPtrX->b = srcPtrX->b * 8;
			}
		}
	}
	return 0;
}

template<> inline int pixelconv<pixelformat_rgbaf, pixelformat_rgba8888> (PIXELCONV_ARGS) noexcept
{
	const static auto mask = dseed::vectori (0x000000ff);

	size_t destStride = get_bitmap_stride (pixelformat_rgbaf, size.width)
		, srcStride = get_bitmap_stride (pixelformat_rgba8888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgbaf* destPtr = (rgbaf*)(dest + destDepthZ + destStrideY);
			const rgba* srcPtr = (const rgba*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				dseed::dividevf (
					dseed::convert_vector (
						dseed::andvi (
							dseed::vectori ((const int*)(srcPtr + x))
							.permute<0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0> ()
							, mask)
					), 255)
					.store ((float*)(destPtr + x));
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_rgbaf, pixelformat_bgra8888> (PIXELCONV_ARGS) noexcept
{
	const static auto mask = dseed::vectori (0x000000ff);

	size_t destStride = get_bitmap_stride (pixelformat_rgbaf, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgra8888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	rgba temp[4];
	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgbaf* destPtr = (rgbaf*)(dest + destDepthZ + destStrideY);
			const rgba* srcPtr = (const rgba*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				dseed::dividevf (
					dseed::convert_vector (
						dseed::andvi (
							dseed::vectori ((const int*)(srcPtr + x))
							.permute<2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0> ()
							, mask)
					), 255)
					.store ((float*)(destPtr + x));
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_rgbaf, pixelformat_rgb888> (PIXELCONV_ARGS)
{
	const static auto mask = dseed::vectori (0x000000ff);

	size_t destStride = get_bitmap_stride (pixelformat_rgbaf, size.width)
		, srcStride = get_bitmap_stride (pixelformat_rgba8888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgbaf* destPtr = (rgbaf*)(dest + destDepthZ + destStrideY);
			const rgba* srcPtr = (const rgba*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				dseed::dividevf (
					dseed::convert_vector (
						dseed::andvi (
							dseed::vectori ((const int*)(srcPtr + x))
							.permute<0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0> ()
							, mask)
					), 255)
					.store ((float*)(destPtr + x));
				(destPtr + x)->a = 1;
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_rgbaf, pixelformat_bgr888> (PIXELCONV_ARGS)
{
	const static auto mask = dseed::vectori (0x000000ff);

	size_t destStride = get_bitmap_stride (pixelformat_rgbaf, size.width)
		, srcStride = get_bitmap_stride (pixelformat_rgba8888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgbaf* destPtr = (rgbaf*)(dest + destDepthZ + destStrideY);
			const bgra* srcPtr = (const bgra*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				dseed::dividevf (
					dseed::convert_vector (
						dseed::andvi (
							dseed::vectori ((const int*)(srcPtr + x))
							.permute<2, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0> ()
							, mask)
					), 255)
					.store ((float*)(destPtr + x));
				(destPtr + x)->a = 1;
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_rgbaf, pixelformat_grayscale8> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_rgbaf, size.width)
		, srcStride = get_bitmap_stride (pixelformat_rgba8888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgbaf* destPtr = (rgbaf*)(dest + destDepthZ + destStrideY);
			const uint8_t* srcPtr = (const uint8_t*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				rgbaf* destPtrX = destPtr + x;
				destPtrX->r = destPtrX->g = destPtrX->b = *(srcPtr + x) / 255.0f;
				(destPtr + x)->a = 1;
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_rgbaf, pixelformat_bgra4444> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_rgbaf, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgra4444, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			rgbaf* destPtr = (rgbaf*)(dest + destDepthZ + destStrideY);
			const bgra4* srcPtr = (const bgra4*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				rgbaf* destPtrX = destPtr + x;
				const bgra4* srcPtrX = srcPtr + x;
				destPtrX->r = srcPtrX->r / 15.0f;
				destPtrX->g = srcPtrX->g / 15.0f;
				destPtrX->b = srcPtrX->b / 15.0f;
				destPtrX->a = srcPtrX->a / 15.0f;
			}
		}
	}
	return 0;
}

template<> inline int pixelconv<pixelformat_grayscale8, pixelformat_rgba8888> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_grayscale8, size.width)
		, srcStride = get_bitmap_stride (pixelformat_rgba8888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			uint8_t* destPtr = (uint8_t*)(dest + destDepthZ + destStrideY);
			const rgba* srcPtr = (const rgba*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				uint8_t* destPtrX = destPtr + x;
				const rgba* srcPtrX = srcPtr + x;
				*destPtrX = (uint8_t)(srcPtrX->r * 0.2126f + srcPtrX->g * 0.7152 + srcPtrX->b * 0.0722);
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_grayscale8, pixelformat_rgb888> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_grayscale8, size.width)
		, srcStride = get_bitmap_stride (pixelformat_rgb888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			uint8_t* destPtr = (uint8_t*)(dest + destDepthZ + destStrideY);
			const rgb* srcPtr = (const rgb*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				uint8_t* destPtrX = destPtr + x;
				const rgb* srcPtrX = srcPtr + x;
				*destPtrX = (uint8_t)(srcPtrX->r * 0.2126f + srcPtrX->g * 0.7152 + srcPtrX->b * 0.0722);
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_grayscale8, pixelformat_bgra8888> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_grayscale8, size.width)
		, srcStride = get_bitmap_stride (pixelformat_rgba8888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			uint8_t* destPtr = (uint8_t*)(dest + destDepthZ + destStrideY);
			const bgra* srcPtr = (const bgra*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				uint8_t* destPtrX = destPtr + x;
				const bgra* srcPtrX = srcPtr + x;
				*destPtrX = (uint8_t)(srcPtrX->r * 0.2126f + srcPtrX->g * 0.7152 + srcPtrX->b * 0.0722);
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_grayscale8, pixelformat_bgra4444> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_grayscale8, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgra4444, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			uint8_t* destPtr = (uint8_t*)(dest + destDepthZ + destStrideY);
			const bgra4* srcPtr = (const bgra4*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				uint8_t* destPtrX = destPtr + x;
				const bgra4* srcPtrX = srcPtr + x;
				*destPtrX = (uint8_t)(srcPtrX->r * 0.2126f + srcPtrX->g * 0.7152 + srcPtrX->b * 0.0722) * 17;
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_grayscale8, pixelformat_bgr888> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_grayscale8, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgr888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			uint8_t* destPtr = (uint8_t*)(dest + destDepthZ + destStrideY);
			const bgr* srcPtr = (const bgr*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				uint8_t* destPtrX = destPtr + x;
				const bgr* srcPtrX = srcPtr + x;
				*destPtrX = (uint8_t)(srcPtrX->r * 0.2126f + srcPtrX->g * 0.7152 + srcPtrX->b * 0.0722);
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_grayscale8, pixelformat_rgbaf> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_grayscale8, size.width)
		, srcStride = get_bitmap_stride (pixelformat_rgbaf, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			uint8_t* destPtr = (uint8_t*)(dest + destDepthZ + destStrideY);
			const rgbaf* srcPtr = (const rgbaf*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				uint8_t* destPtrX = destPtr + x;
				const rgbaf* srcPtrX = srcPtr + x;
				*destPtrX = (srcPtrX->r * 0.2126f + srcPtrX->g * 0.7152 + srcPtrX->b * 0.0722) * 255.0f;
			}
		}
	}
	return 0;
}

template<> inline int pixelconv<pixelformat_bgra8888, pixelformat_bgra8888_indexed8> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgra8888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgra8888_indexed8, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (int z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (int y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			bgra* destPtr = (bgra*)(dest + destDepthZ + destStrideY);
			const uint8_t* srcPtr = (const uint8_t*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
				memcpy (destPtr + x, srcPalette + (srcPtr[x] * 4), 4);
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_bgra8888, pixelformat_bgr888_indexed8> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgra8888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgr888_indexed8, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (int z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (int y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			bgra* destPtr = (bgra*)(dest + destDepthZ + destStrideY);
			const uint8_t* srcPtr = (const uint8_t*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
			{
				memcpy (destPtr + x, srcPalette + (srcPtr[x] * 3), 3);
				destPtr[x].a = 255;
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_bgr888, pixelformat_bgra8888_indexed8> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgr888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgra8888_indexed8, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (int z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (int y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			bgra* destPtr = (bgra*)(dest + destDepthZ + destStrideY);
			const uint8_t* srcPtr = (const uint8_t*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
				memcpy (destPtr + x, srcPalette + (srcPtr[x] * 4), 3);
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_bgr888, pixelformat_bgr888_indexed8> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgr888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgr888_indexed8, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (int z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (int y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			bgra* destPtr = (bgra*)(dest + destDepthZ + destStrideY);
			const uint8_t* srcPtr = (const uint8_t*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; ++x)
				memcpy (destPtr + x, srcPalette + (srcPtr[x] * 3), 3);
		}
	}
	return 0;
}

template<> inline int pixelconv<pixelformat_bgra8888_indexed8, pixelformat_bgra8888> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgra8888_indexed8, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgra8888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	exq_data* pExq;
	pExq = exq_init ();
	exq_feed (pExq, (uint8_t*)src, size.width * size.height * size.depth);
	exq_quantize (pExq, 256);
	exq_quantize_hq (pExq, 256);
	exq_quantize_ex (pExq, 256, 1);
	std::vector<bgra> palette (256);
	exq_get_palette (pExq, (uint8_t*)palette.data (), 256);
	memcpy (destPalette, palette.data (), sizeof (dseed::bgra) * 256);

	exq_map_image_dither (pExq, size.width, size.height * size.depth, (uint8_t*)src, dest, 0);

	exq_free (pExq);

	return 256;
}
template<> inline int pixelconv<pixelformat_bgra8888_indexed8, pixelformat_bgr888> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgra8888_indexed8, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgra8888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	std::vector<uint8_t> bgr_to_bgra (srcStride * size.height * size.depth);
	pixelconv<pixelformat_rgba8888, pixelformat_rgb888> (bgr_to_bgra.data (), src, size, nullptr, nullptr);

	exq_data* pExq;
	pExq = exq_init ();
	exq_feed (pExq, bgr_to_bgra.data (), size.width * size.height * size.depth);
	exq_quantize (pExq, 256);
	exq_quantize_hq (pExq, 256);
	exq_quantize_ex (pExq, 256, 1);
	std::vector<bgra> palette (256);
	exq_get_palette (pExq, (uint8_t*)palette.data (), 256);
	memcpy (destPalette, palette.data (), sizeof (dseed::bgra) * 256);

	exq_map_image_dither (pExq, size.width, size.height * size.depth, bgr_to_bgra.data (), dest, 0);

	exq_free (pExq);

	return 256;
}
template<> inline int pixelconv<pixelformat_bgr888_indexed8, pixelformat_bgra8888> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgra8888_indexed8, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgra8888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	exq_data* pExq;
	pExq = exq_init ();
	exq_feed (pExq, (uint8_t*)src, size.width * size.height * size.depth);
	exq_quantize (pExq, 256);
	exq_quantize_hq (pExq, 256);
	exq_quantize_ex (pExq, 256, 1);
	std::vector<bgra> palette (256);
	exq_get_palette (pExq, (uint8_t*)palette.data (), 256);

	bgr* destPaletteBGR = (bgr*)destPalette;
	for (int i = 0; i < 256; ++i)
	{
		destPaletteBGR[i].r = palette[i].r;
		destPaletteBGR[i].g = palette[i].g;
		destPaletteBGR[i].b = palette[i].b;
	}

	exq_map_image_dither (pExq, size.width, size.height * size.depth, (uint8_t*)src, dest, 0);

	exq_free (pExq);

	return 256;
}
template<> inline int pixelconv<pixelformat_bgr888_indexed8, pixelformat_bgr888> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgra8888_indexed8, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgra8888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	std::vector<uint8_t> bgr_to_bgra (srcStride * size.height * size.depth);
	pixelconv<pixelformat_rgba8888, pixelformat_rgb888> (bgr_to_bgra.data (), src, size, nullptr, nullptr);

	exq_data* pExq;
	pExq = exq_init ();
	exq_feed (pExq, bgr_to_bgra.data (), size.width * size.height * size.depth);
	exq_quantize (pExq, 256);
	exq_quantize_hq (pExq, 256);
	exq_quantize_ex (pExq, 256, 1);
	std::vector<bgra> palette (256);
	exq_get_palette (pExq, (uint8_t*)palette.data (), 256);

	bgr* destPaletteBGR = (bgr*)destPalette;
	for (int i = 0; i < 256; ++i)
	{
		destPaletteBGR[i].r = palette[i].r;
		destPaletteBGR[i].g = palette[i].g;
		destPaletteBGR[i].b = palette[i].b;
	}

	exq_map_image_dither (pExq, size.width, size.height * size.depth, bgr_to_bgra.data (), dest, 0);

	exq_free (pExq);

	return 256;
}

template<> inline int pixelconv<pixelformat_rgba8888, pixelformat_bc1> (PIXELCONV_ARGS) noexcept
{
#if defined ( USE_SQUISH )
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat_rgba8888, size.width, size.height),
		srcArr = dseed::get_bitmap_plane_size (pixelformat_bc1, size.width, size.height);
	for (int z = 0; z < size.depth; ++z)
		squish::DecompressImage (dest + (destArr * z), size.width, size.height, src + (srcArr * z), squish::kDxt1);
	return 0;
#else
	return -1;
#endif
}
template<> inline int pixelconv<pixelformat_rgba8888, pixelformat_bc2> (PIXELCONV_ARGS) noexcept
{
#if defined ( USE_SQUISH )
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat_rgba8888, size.width, size.height),
		srcArr = dseed::get_bitmap_plane_size (pixelformat_bc2, size.width, size.height);
	for (int z = 0; z < size.depth; ++z)
		squish::DecompressImage (dest + (destArr * z), size.width, size.height, src + (srcArr * z), squish::kDxt3);
	return 0;
#else
	return -1;
#endif
}
template<> inline int pixelconv<pixelformat_rgba8888, pixelformat_bc3> (PIXELCONV_ARGS) noexcept
{
#if defined ( USE_SQUISH )
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat_rgba8888, size.width, size.height),
		srcArr = dseed::get_bitmap_plane_size (pixelformat_bc3, size.width, size.height);
	for (int z = 0; z < size.depth; ++z)
		squish::DecompressImage (dest + (destArr * z), size.width, size.height, src + (srcArr * z), squish::kDxt5);
	return 0;
#else
	return -1;
#endif
}
template<> inline int pixelconv<pixelformat_rgba8888, pixelformat_bc4> (PIXELCONV_ARGS) noexcept
{
#if defined ( USE_SQUISH )
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat_rgba8888, size.width, size.height),
		srcArr = dseed::get_bitmap_plane_size (pixelformat_bc4, size.width, size.height);
	for (int z = 0; z < size.depth; ++z)
		squish::DecompressImage (dest + (destArr * z), size.width, size.height, src + (srcArr * z), squish::kBc4);
	return 0;
#else
	return -1;
#endif
}
template<> inline int pixelconv<pixelformat_rgba8888, pixelformat_bc5> (PIXELCONV_ARGS) noexcept
{
#if defined ( USE_SQUISH )
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat_rgba8888, size.width, size.height),
		srcArr = dseed::get_bitmap_plane_size (pixelformat_bc5, size.width, size.height);
	for (int z = 0; z < size.depth; ++z)
		squish::DecompressImage (dest + (destArr * z), size.width, size.height, src + (srcArr * z), squish::kBc5);
	return 0;
#else
	return -1;
#endif
}
template<> inline int pixelconv<pixelformat_bc1, pixelformat_rgba8888> (PIXELCONV_ARGS) noexcept
{
#if defined ( USE_SQUISH )
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat_bc1, size.width, size.height),
		srcArr = dseed::get_bitmap_plane_size (pixelformat_rgba8888, size.width, size.height);
	for (int z = 0; z < size.depth; ++z)
		squish::CompressImage (src + (srcArr * z), size.width, size.height, dest + (destArr * z)
			, squish::kDxt1 | squish::kColourIterativeClusterFit);
	return 0;
#else
	return -1;
#endif
}
template<> inline int pixelconv<pixelformat_bc2, pixelformat_rgba8888> (PIXELCONV_ARGS) noexcept
{
#if defined ( USE_SQUISH )
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat_bc2, size.width, size.height),
		srcArr = dseed::get_bitmap_plane_size (pixelformat_rgba8888, size.width, size.height);
	for (int z = 0; z < size.depth; ++z)
		squish::CompressImage (src + (srcArr * z), size.width, size.height, dest + (destArr * z)
			, squish::kDxt3 | squish::kColourIterativeClusterFit);
	return 0;
#else
	return -1;
#endif
}
template<> inline int pixelconv<pixelformat_bc3, pixelformat_rgba8888> (PIXELCONV_ARGS) noexcept
{
#if defined ( USE_SQUISH )
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat_bc3, size.width, size.height),
		srcArr = dseed::get_bitmap_plane_size (pixelformat_rgba8888, size.width, size.height);
	for (int z = 0; z < size.depth; ++z)
		squish::CompressImage (src + (srcArr * z), size.width, size.height, dest + (destArr * z)
			, squish::kDxt5 | squish::kColourIterativeClusterFit);
	return 0;
#else
	return -1;
#endif
}
template<> inline int pixelconv<pixelformat_bc4, pixelformat_rgba8888> (PIXELCONV_ARGS) noexcept
{
#if defined ( USE_SQUISH )
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat_bc4, size.width, size.height),
		srcArr = dseed::get_bitmap_plane_size (pixelformat_rgba8888, size.width, size.height);
	for (int z = 0; z < size.depth; ++z)
		squish::CompressImage (src + (srcArr * z), size.width, size.height, dest + (destArr * z)
			, squish::kBc4 | squish::kColourIterativeClusterFit);
	return 0;
#else
	return -1;
#endif
}
template<> inline int pixelconv<pixelformat_bc5, pixelformat_rgba8888> (PIXELCONV_ARGS) noexcept
{
#if defined ( USE_SQUISH )
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat_bc5, size.width, size.height),
		srcArr = dseed::get_bitmap_plane_size (pixelformat_rgba8888, size.width, size.height);
	for (int z = 0; z < size.depth; ++z)
		squish::CompressImage (src + (srcArr * z), size.width, size.height, dest + (destArr * z)
			, squish::kBc5 | squish::kColourIterativeClusterFit);
	return 0;
#else
	return -1;
#endif
}

template<> inline int pixelconv<pixelformat_rgb888, pixelformat_etc1> (PIXELCONV_ARGS)
{
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat_rgb888, size.width, size.height),
		srcArr = dseed::get_bitmap_plane_size (pixelformat_etc1, size.width, size.height);
	size_t destStride = dseed::get_bitmap_stride (dseed::pixelformat_rgb888, size.width);
	for (int z = 0; z < size.depth; ++z)
		etc1_decode_image (src + (srcArr * z), dest + (destArr * z), size.width, size.height, 3, destStride);
	return 0;
}
template<> inline int pixelconv<pixelformat_rgba8888, pixelformat_etc1> (PIXELCONV_ARGS)
{
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat_rgba8888, size.width, size.height),
		tempArr = dseed::get_bitmap_plane_size (pixelformat_rgb888, size.width, size.height);
	size_t tempStride = dseed::get_bitmap_stride (dseed::pixelformat_rgb888, size.width)
		, destStride = dseed::get_bitmap_stride (dseed::pixelformat_rgba8888, size.width);
	std::vector<uint8_t> temp (tempStride * size.height * size.depth);
	pixelconv<pixelformat_rgb888, pixelformat_etc1> (temp.data (), src, size, destPalette, srcPalette);
	for (int z = 0; z < size.depth; ++z)
	{
		for (int y = 0; y < size.height; ++y)
		{
			for (int x = 0; x < size.width; ++x)
			{
				rgba& destPixel = *((rgba*)(dest + (z * destArr) + (y * destStride) + (x * 4)));
				rgb& tempPixel = *((rgb*)(temp.data () + (z * tempArr) + (y * tempStride) + (x * 3)));

				destPixel.r = tempPixel.r;
				destPixel.g = tempPixel.g;
				destPixel.b = tempPixel.b;
				destPixel.a = 255;
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_etc1, pixelformat_rgb888> (PIXELCONV_ARGS)
{
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat_etc1, size.width, size.height),
		srcArr = dseed::get_bitmap_plane_size (pixelformat_rgb888, size.width, size.height);
	size_t srcStride = dseed::get_bitmap_stride (dseed::pixelformat_rgb888, size.width);
	for (int z = 0; z < size.depth; ++z)
		etc1_encode_image (src + (z * srcArr), size.width, size.height, 3, srcStride, dest + (z * destArr));
	return 0;
}
template<> inline int pixelconv<pixelformat_etc1, pixelformat_rgba8888> (PIXELCONV_ARGS)
{
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat_etc1, size.width, size.height),
		tempArr = dseed::get_bitmap_plane_size (pixelformat_rgb888, size.width, size.height);
	size_t tempStride = dseed::get_bitmap_stride (dseed::pixelformat_rgb888, size.width)
		, srcStride = dseed::get_bitmap_stride (dseed::pixelformat_rgba8888, size.width);
	std::vector<uint8_t> temp (tempStride * size.height * size.depth);
	for (int z = 0; z < size.depth; ++z)
	{
		for (int y = 0; y < size.height; ++y)
		{
			for (int x = 0; x < size.width; ++x)
			{
				const rgba& srcPixel = *((const rgba*)(dest + (y * srcStride) + (x * 4)));
				rgb& tempPixel = *((rgb*)(temp.data () + (y * tempStride) + (x * 3)));

				tempPixel.r = srcPixel.r;
				tempPixel.g = srcPixel.g;
				tempPixel.b = srcPixel.b;
			}
		}
		etc1_encode_image (temp.data () + (z * tempArr), size.width, size.height, 3, tempStride, dest + (z * destArr));
	}
	return 0;
}

std::map<pixelconvtp, pixelconvfn> g_pixelconvs = {
	{ pixelconvtp (pixelformat_rgbaf, pixelformat_rgba8888), pixelconv<pixelformat_rgbaf, pixelformat_rgba8888> },
	{ pixelconvtp (pixelformat_rgbaf, pixelformat_bgra8888), pixelconv<pixelformat_rgbaf, pixelformat_bgra8888> },
	{ pixelconvtp (pixelformat_rgbaf, pixelformat_rgb888), pixelconv<pixelformat_rgbaf, pixelformat_rgb888> },
	{ pixelconvtp (pixelformat_rgbaf, pixelformat_bgr888), pixelconv<pixelformat_rgbaf, pixelformat_bgr888> },
	{ pixelconvtp (pixelformat_rgbaf, pixelformat_bgra4444), pixelconv<pixelformat_rgbaf, pixelformat_bgra4444> },
	{ pixelconvtp (pixelformat_rgbaf, pixelformat_grayscale8), pixelconv<pixelformat_rgbaf, pixelformat_grayscale8> },

	{ pixelconvtp (pixelformat_rgba8888, pixelformat_rgbaf), pixelconv<pixelformat_rgba8888, pixelformat_rgbaf> },
	{ pixelconvtp (pixelformat_rgba8888, pixelformat_bgra8888), pixelconv<pixelformat_rgba8888, pixelformat_bgra8888> },
	{ pixelconvtp (pixelformat_rgba8888, pixelformat_bgra4444), pixelconv<pixelformat_rgba8888, pixelformat_bgra4444> },
	{ pixelconvtp (pixelformat_rgba8888, pixelformat_rgb888), pixelconv<pixelformat_rgba8888, pixelformat_rgb888> },
	{ pixelconvtp (pixelformat_rgba8888, pixelformat_bgr888), pixelconv<pixelformat_rgba8888, pixelformat_bgr888> },
	{ pixelconvtp (pixelformat_rgba8888, pixelformat_grayscale8), pixelconv<pixelformat_rgba8888, pixelformat_grayscale8> },

	{ pixelconvtp (pixelformat_bgra8888, pixelformat_rgbaf), pixelconv<pixelformat_bgra8888, pixelformat_rgbaf> },
	{ pixelconvtp (pixelformat_bgra8888, pixelformat_rgba8888), pixelconv<pixelformat_bgra8888, pixelformat_rgba8888> },
	{ pixelconvtp (pixelformat_bgra8888, pixelformat_bgra4444), pixelconv<pixelformat_bgra8888, pixelformat_bgra4444> },
	{ pixelconvtp (pixelformat_bgra8888, pixelformat_rgb888), pixelconv<pixelformat_bgra8888, pixelformat_rgb888> },
	{ pixelconvtp (pixelformat_bgra8888, pixelformat_bgr888), pixelconv<pixelformat_bgra8888, pixelformat_bgr888> },
	{ pixelconvtp (pixelformat_bgra8888, pixelformat_grayscale8), pixelconv<pixelformat_bgra8888, pixelformat_grayscale8> },

	{ pixelconvtp (pixelformat_bgra4444, pixelformat_rgbaf), pixelconv<pixelformat_bgra4444, pixelformat_rgbaf> },
	{ pixelconvtp (pixelformat_bgra4444, pixelformat_rgba8888), pixelconv<pixelformat_bgra4444, pixelformat_rgba8888> },
	{ pixelconvtp (pixelformat_bgra4444, pixelformat_bgra8888), pixelconv<pixelformat_bgra4444, pixelformat_bgra8888> },
	{ pixelconvtp (pixelformat_bgra4444, pixelformat_rgb888), pixelconv<pixelformat_bgra4444, pixelformat_rgb888> },
	{ pixelconvtp (pixelformat_bgra4444, pixelformat_bgr888), pixelconv<pixelformat_bgra4444, pixelformat_bgr888> },
	{ pixelconvtp (pixelformat_bgra4444, pixelformat_grayscale8), pixelconv<pixelformat_bgra4444, pixelformat_grayscale8> },

	{ pixelconvtp (pixelformat_rgb888, pixelformat_rgbaf), pixelconv<pixelformat_rgb888, pixelformat_rgbaf> },
	{ pixelconvtp (pixelformat_rgb888, pixelformat_rgba8888), pixelconv<pixelformat_rgb888, pixelformat_rgba8888> },
	{ pixelconvtp (pixelformat_rgb888, pixelformat_bgra8888), pixelconv<pixelformat_rgb888, pixelformat_bgra8888> },
	{ pixelconvtp (pixelformat_rgb888, pixelformat_bgra4444), pixelconv<pixelformat_rgb888, pixelformat_bgra4444> },
	{ pixelconvtp (pixelformat_rgb888, pixelformat_bgr888), pixelconv<pixelformat_rgb888, pixelformat_bgr888> },
	{ pixelconvtp (pixelformat_rgb888, pixelformat_grayscale8), pixelconv<pixelformat_rgb888, pixelformat_grayscale8> },

	{ pixelconvtp (pixelformat_bgr888, pixelformat_rgbaf), pixelconv<pixelformat_bgr888, pixelformat_rgbaf> },
	{ pixelconvtp (pixelformat_bgr888, pixelformat_rgba8888), pixelconv<pixelformat_bgr888, pixelformat_rgba8888> },
	{ pixelconvtp (pixelformat_bgr888, pixelformat_bgra8888), pixelconv<pixelformat_bgr888, pixelformat_bgra8888> },
	{ pixelconvtp (pixelformat_bgr888, pixelformat_bgra4444), pixelconv<pixelformat_bgr888, pixelformat_bgra4444> },
	{ pixelconvtp (pixelformat_bgr888, pixelformat_rgb888), pixelconv<pixelformat_bgr888, pixelformat_rgb888> },
	{ pixelconvtp (pixelformat_bgr888, pixelformat_grayscale8), pixelconv<pixelformat_bgr888, pixelformat_grayscale8> },

	{ pixelconvtp (pixelformat_grayscale8, pixelformat_rgbaf), pixelconv<pixelformat_grayscale8, pixelformat_rgbaf> },
	{ pixelconvtp (pixelformat_grayscale8, pixelformat_rgba8888), pixelconv<pixelformat_grayscale8, pixelformat_rgba8888> },
	{ pixelconvtp (pixelformat_grayscale8, pixelformat_bgra8888), pixelconv<pixelformat_grayscale8, pixelformat_bgra8888> },
	{ pixelconvtp (pixelformat_grayscale8, pixelformat_bgra4444), pixelconv<pixelformat_grayscale8, pixelformat_bgra4444> },
	{ pixelconvtp (pixelformat_grayscale8, pixelformat_rgb888), pixelconv<pixelformat_grayscale8, pixelformat_rgb888> },
	{ pixelconvtp (pixelformat_grayscale8, pixelformat_bgr888), pixelconv<pixelformat_grayscale8, pixelformat_bgr888> },

	{ pixelconvtp (pixelformat_bgr565, pixelformat_bgr888), pixelconv<pixelformat_bgr565, pixelformat_bgr888> },
	{ pixelconvtp (pixelformat_bgr888, pixelformat_bgr565), pixelconv<pixelformat_bgr888, pixelformat_bgr565> },

	{ pixelconvtp (pixelformat_bgra8888, pixelformat_bgra8888_indexed8), pixelconv<pixelformat_bgra8888, pixelformat_bgra8888_indexed8> },
	{ pixelconvtp (pixelformat_bgra8888, pixelformat_bgr888_indexed8), pixelconv<pixelformat_bgra8888, pixelformat_bgr888_indexed8> },
	{ pixelconvtp (pixelformat_bgr888, pixelformat_bgra8888_indexed8), pixelconv<pixelformat_bgr888, pixelformat_bgra8888_indexed8> },
	{ pixelconvtp (pixelformat_bgr888, pixelformat_bgr888_indexed8), pixelconv<pixelformat_bgr888, pixelformat_bgr888_indexed8> },

	{ pixelconvtp (pixelformat_bgra8888_indexed8, pixelformat_bgra8888), pixelconv<pixelformat_bgra8888_indexed8, pixelformat_bgra8888> },
	{ pixelconvtp (pixelformat_bgra8888_indexed8, pixelformat_bgr888), pixelconv<pixelformat_bgra8888_indexed8, pixelformat_bgr888> },
	{ pixelconvtp (pixelformat_bgr888_indexed8, pixelformat_bgra8888), pixelconv<pixelformat_bgr888_indexed8, pixelformat_bgra8888> },
	{ pixelconvtp (pixelformat_bgr888_indexed8, pixelformat_bgr888), pixelconv<pixelformat_bgr888_indexed8, pixelformat_bgr888> },

	{ pixelconvtp (pixelformat_rgba8888, pixelformat_bc1), pixelconv<pixelformat_rgba8888, pixelformat_bc1> },
	{ pixelconvtp (pixelformat_rgba8888, pixelformat_bc2), pixelconv<pixelformat_rgba8888, pixelformat_bc2> },
	{ pixelconvtp (pixelformat_rgba8888, pixelformat_bc3), pixelconv<pixelformat_rgba8888, pixelformat_bc3> },
	{ pixelconvtp (pixelformat_rgba8888, pixelformat_bc4), pixelconv<pixelformat_rgba8888, pixelformat_bc4> },
	{ pixelconvtp (pixelformat_rgba8888, pixelformat_bc5), pixelconv<pixelformat_rgba8888, pixelformat_bc5> },

	{ pixelconvtp (pixelformat_bc1, pixelformat_rgba8888), pixelconv<pixelformat_bc1, pixelformat_rgba8888> },
	{ pixelconvtp (pixelformat_bc2, pixelformat_rgba8888), pixelconv<pixelformat_bc2, pixelformat_rgba8888> },
	{ pixelconvtp (pixelformat_bc3, pixelformat_rgba8888), pixelconv<pixelformat_bc3, pixelformat_rgba8888> },
	{ pixelconvtp (pixelformat_bc4, pixelformat_rgba8888), pixelconv<pixelformat_bc4, pixelformat_rgba8888> },
	{ pixelconvtp (pixelformat_bc5, pixelformat_rgba8888), pixelconv<pixelformat_bc5, pixelformat_rgba8888> },

	{ pixelconvtp (pixelformat_rgba8888, pixelformat_etc1), pixelconv<pixelformat_rgba8888, pixelformat_etc1> },
	{ pixelconvtp (pixelformat_rgb888, pixelformat_etc1), pixelconv<pixelformat_rgb888, pixelformat_etc1> },

	{ pixelconvtp (pixelformat_etc1, pixelformat_rgba8888), pixelconv<pixelformat_etc1, pixelformat_rgba8888> },
	{ pixelconvtp (pixelformat_etc1, pixelformat_rgb888), pixelconv<pixelformat_etc1, pixelformat_rgb888> },

};

dseed::error_t dseed::reformat_bitmap (dseed::bitmap* original, dseed::pixelformat_t reformat, dseed::bitmap** bitmap)
{
	pixelformat_t originalFormat = original->format ();
	if (originalFormat == reformat)
	{
		*bitmap = original;
		original->retain ();
		return dseed::error_good;
	}

	bool isIndexedTarget = false;
	int bpp = 0;
	if (reformat == pixelformat_bgra8888_indexed8 || reformat == pixelformat_bgr888_indexed8)
	{
		isIndexedTarget = true;
		bpp = (reformat & 0xff) * 8;
	}

	dseed::size3i size = original->size ();

	dseed::auto_object<dseed::palette> tempPalette;
	if (isIndexedTarget)
	{
		if (dseed::failed (dseed::create_palette (nullptr, bpp, 256, &tempPalette)))
			return dseed::error_fail;
	}

	dseed::auto_object<dseed::bitmap> temp;
	if (dseed::failed (dseed::create_bitmap (dseed::bitmaptype_2d, size, reformat
		, tempPalette, &temp)))
		return dseed::error_fail;

	uint8_t* destPtr, * srcPtr, * destPalettePtr = nullptr, * srcPalettePtr = nullptr;
	original->pixels_pointer ((void**)& srcPtr);
	temp->pixels_pointer ((void**)& destPtr);

	dseed::auto_object<dseed::palette> destPalette, srcPalette;
	if (dseed::succeeded (original->palette (&srcPalette)))
		srcPalette->pixels_pointer ((void**)& srcPalettePtr);
	if (dseed::succeeded (temp->palette (&destPalette)))
		destPalette->pixels_pointer ((void**)& destPalettePtr);

	pixelconvtp tp (reformat, originalFormat);
	auto found = g_pixelconvs.find (tp);
	if (found == g_pixelconvs.end ())
		return dseed::error_not_support;

	int paletteCount = found->second (destPtr, srcPtr, size, destPalettePtr, srcPalettePtr);
	if (paletteCount == -1)
		return dseed::error_not_support;
	if (isIndexedTarget)
		tempPalette->resize (paletteCount);

	*bitmap = temp.detach ();

	return dseed::error_good;
}