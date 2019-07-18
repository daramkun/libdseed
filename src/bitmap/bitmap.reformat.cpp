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
using pcfn = std::function<int (PIXELCONV_ARGS)>;
using pctp = std::tuple<pixelformat_t, pixelformat_t>;
template<pixelformat_t destformat, pixelformat_t srcformat>
inline int pixelconv (PIXELCONV_ARGS) noexcept
{
	static_assert (true, "Not support those formats currently.");
	return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// RGB series Conversions
//
////////////////////////////////////////////////////////////////////////////////////////////

template<class TDest, class TSrc>
inline int pixelconv_plaincolor (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (type2format<TDest> (), size.width)
		, srcStride = get_bitmap_stride (type2format<TSrc> (), size.width);
	size_t destDepth = get_bitmap_plane_size (type2format<TDest> (), size.width, size.height)
		, srcDepth = get_bitmap_plane_size (type2format<TSrc> (), size.width, size.height);

	for (size_t z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth
			, srcDepthZ = z * srcDepth;
		for (size_t y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride
				, srcStrideY = y * srcStride;
			TDest* destPtr = (TDest*)(dest + destDepthZ + destStrideY);
			const TSrc* srcPtr = (TSrc*)(src + srcDepthZ + srcStrideY);

			for (size_t x = 0; x < size.width; ++x)
			{
				TDest* destPtrX = destPtr + x;
				const TSrc* srcPtrX = srcPtr + x;
				*destPtrX = (TDest)*srcPtrX;
			}
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Indexed BGR series conversions
//
////////////////////////////////////////////////////////////////////////////////////////////

template<> inline int pixelconv<pixelformat_bgra8888, pixelformat_bgra8888_indexed8> (PIXELCONV_ARGS)
{
	size_t destStride = get_bitmap_stride (pixelformat_bgra8888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgra8888_indexed8, size.width);
	size_t destDepth = get_bitmap_plane_size (pixelformat_bgra8888, size.width, size.height)
		, srcDepth = get_bitmap_plane_size (pixelformat_bgra8888_indexed8, size.width, size.height);

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
	size_t destDepth = get_bitmap_plane_size (pixelformat_bgra8888, size.width, size.height)
		, srcDepth = get_bitmap_plane_size (pixelformat_bgr888_indexed8, size.width, size.height);

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
	size_t destDepth = get_bitmap_plane_size(pixelformat_bgr888, size.width, size.height)
		, srcDepth = get_bitmap_plane_size (pixelformat_bgra8888_indexed8, size.width, size.height);

	for (int z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth
			, srcDepthZ = z * srcDepth;
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

////////////////////////////////////////////////////////////////////////////////////////////
//
// Packed YCbCr series <-> BGR Conversions
//
////////////////////////////////////////////////////////////////////////////////////////////
/*template<> inline int pixelconv<pixelformat_yuyv8888, pixelformat_bgr888> (PIXELCONV_ARGS) noexcept
{
	size_t destStride = get_bitmap_stride (pixelformat_yuyv8888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_bgr888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			yuyv* destPtr = (yuyv*)(dest + destDepthZ + destStrideY);
			const bgr* srcPtr = (const bgr*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; x += 2)
			{
				yuyv* destPtrX = destPtr + (x / 2);
				const bgr* srcPtrX = srcPtr + x;
				*destPtrX = yuyv (*srcPtrX, *(srcPtrX + 1));
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_yyyyuv888888, pixelformat_bgr888> (PIXELCONV_ARGS) noexcept
{
	size_t srcStride = get_bitmap_stride (pixelformat_bgr888, size.width);
	size_t destDepth = get_bitmap_plane_size (pixelformat_yyyyuv888888, size.width, size.height)
		, srcDepth = srcStride * size.height;
	size_t destYSize = size.width * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; y += 2)
		{
			uint8_t* destY1Ptr = (uint8_t*)(dest + destDepthZ + (size.width * y));
			uint8_t* destY2Ptr = (uint8_t*)(dest + destDepthZ + (size.width * (y + 1)));
			uint8_t* destUVPtr = (uint8_t*)(dest + destDepthZ + destYSize + (size.width * (y / 2)));
			const bgr* srcPtr1 = (const bgr*)(src + srcDepthZ + (y * srcStride));
			const bgr* srcPtr2 = (const bgr*)(src + srcDepthZ + ((y + 1) * srcStride));
			for (auto x = 0; x < size.width; x += 2)
			{
				const bgr* srcPtr1X = srcPtr1 + x;
				const bgr* srcPtr2X = srcPtr2 + x;

				uint8_t* destUVPtrX = destUVPtr + x;

				uint8_t* destY1PtrX = destY1Ptr + (x / 2);
				yuv yuv1 = yuv(*srcPtr1X), yuv2 = yuv(*(srcPtr1X + 1));

				destY1PtrX[0] = yuv1.y;
				destY1PtrX[1] = yuv2.y;

				uint32_t cu, cv, divider = 2;
				if (y < size.height)
					divider = 4;

				cu = yuv1.u;// + yuv2.u;
				cv = yuv1.v;// + yuv2.v;

				if (y < size.height)
				{
					uint8_t* destY2PtrX = destY2Ptr + (x / 2);
					yuv yuv3 = *srcPtr2X, yuv4 = *(srcPtr2X + 1);

					destY2PtrX[0] = yuv3.y;
					destY2PtrX[1] = yuv4.y;

					//cu += yuv3.u + yuv4.u;
					//cv += yuv3.v + yuv4.v;
				}
				destUVPtrX[0] = cu;// / divider;
				destUVPtrX[1] = cv;// / divider;

			}
		}
	}
	return 0;
}

template<> inline int pixelconv<pixelformat_bgr888, pixelformat_yuyv8888> (PIXELCONV_ARGS) noexcept
{
	size_t destStride = get_bitmap_stride (pixelformat_bgr888, size.width)
		, srcStride = get_bitmap_stride (pixelformat_yuyv8888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = srcStride * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride, srcStrideY = y * srcStride;
			bgr* destPtr = (bgr*)(dest + destDepthZ + destStrideY);
			const yuyv* srcPtr = (const yuyv*)(src + srcDepthZ + srcStrideY);
			for (auto x = 0; x < size.width; x += 2)
			{
				bgr* destPtrX = destPtr + x;
				const yuyv* srcPtrX = srcPtr + (x / 2);
				srcPtrX->to_bgr (destPtrX, destPtrX + 1);
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat_bgr888, pixelformat_yyyyuv888888> (PIXELCONV_ARGS) noexcept
{
	size_t destStride = get_bitmap_stride (pixelformat_bgr888, size.width);
	size_t destDepth = destStride * size.height, srcDepth = (size.width * size.height) + (size.width / 2 * size.height / 2);
	size_t srcYSize = size.width * size.height;

	for (auto z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth, srcDepthZ = z * srcDepth;
		for (auto y = 0; y < size.height; y += 2)
		{
			bgr* destPtr1 = (bgr*)(dest + destDepthZ + (y * destStride));
			bgr* destPtr2 = (bgr*)(dest + destDepthZ + ((y + 1) * destStride));
			const uint8_t* srcY1Ptr = (const uint8_t*)(src + srcDepthZ + (size.width * y));
			const uint8_t* srcY2Ptr = (const uint8_t*)(src + srcDepthZ + (size.width * (y + 1)));
			const uint8_t* srcUVPtr = (const uint8_t*)(src + srcDepthZ + srcYSize + (size.width * (y / 2)));
			for (auto x = 0; x < size.width; x += 2)
			{
				bgr* destPtr1X = destPtr1 + x;
				bgr* destPtr2X = destPtr2 + x;

				const uint8_t* srcUVPtrX = srcUVPtr + x;
				uint8_t u = *srcUVPtrX, v = *(srcUVPtrX + 1);

				const uint8_t* srcY1PtrX = srcY1Ptr + (x / 2);
				destPtr1X[0] = (bgr)yuv (*srcY1PtrX, u, v);
				destPtr1X[1] = (bgr)yuv (*(srcY1PtrX + 1), u, v);

				if (y < size.height)
				{
					const uint8_t* srcY2PtrX = srcY2Ptr + (x / 2);
					destPtr2X[0] = (bgr)yuv (*srcY2PtrX, u, v);
					destPtr2X[1] = (bgr)yuv (*(srcY2PtrX + 1), u, v);
				}
			}
		}
	}
	return 0;
}*/

////////////////////////////////////////////////////////////////////////////////////////////
//
// DXT series <-> RGBA Conversions
//
////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////
//
// ETC1 <-> RGB/RGBA series Conversions
//
////////////////////////////////////////////////////////////////////////////////////////////

template<> inline int pixelconv<pixelformat_rgb888, pixelformat_etc1> (PIXELCONV_ARGS)
{
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat_rgb888, size.width, size.height),
		srcArr = dseed::get_bitmap_plane_size (pixelformat_etc1, size.width, size.height);
	size_t destStride = dseed::get_bitmap_stride (dseed::pixelformat_rgb888, size.width);
	for (int z = 0; z < size.depth; ++z)
		etc1_decode_image (src + (srcArr * z), dest + (destArr * z), size.width, size.height, 3, (uint32_t)destStride);
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

////////////////////////////////////////////////////////////////////////////////////////////
//
// Conversions Dictionary
//
////////////////////////////////////////////////////////////////////////////////////////////

std::map<pctp, pcfn> g_pixelconvs = {
	{ pctp (pixelformat_rgba8888, pixelformat_rgb888), pixelconv_plaincolor<rgba, rgb> },
	{ pctp (pixelformat_rgba8888, pixelformat_rgbaf), pixelconv_plaincolor<rgba, rgbaf> },
	{ pctp (pixelformat_rgba8888, pixelformat_bgra8888), pixelconv_plaincolor<rgba, bgra> },
	{ pctp (pixelformat_rgba8888, pixelformat_bgr888), pixelconv_plaincolor<rgba, bgr> },
	{ pctp (pixelformat_rgba8888, pixelformat_bgra4444), pixelconv_plaincolor<rgba, bgra4> },
	{ pctp (pixelformat_rgba8888, pixelformat_bgr565), pixelconv_plaincolor<rgba, bgr565> },
	{ pctp (pixelformat_rgba8888, pixelformat_grayscale8), pixelconv_plaincolor<rgba, grayscale> },
	{ pctp (pixelformat_rgba8888, pixelformat_grayscalef), pixelconv_plaincolor<rgba, grayscalef> },
	{ pctp (pixelformat_rgba8888, pixelformat_yuva8888), pixelconv_plaincolor<rgba, yuva> },
	{ pctp (pixelformat_rgba8888, pixelformat_yuv888), pixelconv_plaincolor<rgba, yuv> },

	{ pctp (pixelformat_rgb888, pixelformat_rgba8888), pixelconv_plaincolor<rgb, rgba> },
	{ pctp (pixelformat_rgb888, pixelformat_rgbaf), pixelconv_plaincolor<rgb, rgbaf> },
	{ pctp (pixelformat_rgb888, pixelformat_bgra8888), pixelconv_plaincolor<rgb, bgra> },
	{ pctp (pixelformat_rgb888, pixelformat_bgr888), pixelconv_plaincolor<rgb, bgr> },
	{ pctp (pixelformat_rgb888, pixelformat_bgra4444), pixelconv_plaincolor<rgb, bgra4> },
	{ pctp (pixelformat_rgb888, pixelformat_bgr565), pixelconv_plaincolor<rgb, bgr565> },
	{ pctp (pixelformat_rgb888, pixelformat_grayscale8), pixelconv_plaincolor<rgb, grayscale> },
	{ pctp (pixelformat_rgb888, pixelformat_grayscalef), pixelconv_plaincolor<rgb, grayscalef> },
	{ pctp (pixelformat_rgb888, pixelformat_yuva8888), pixelconv_plaincolor<rgb, yuva> },
	{ pctp (pixelformat_rgb888, pixelformat_yuv888), pixelconv_plaincolor<rgb, yuv> },

	{ pctp (pixelformat_rgbaf, pixelformat_rgba8888), pixelconv_plaincolor<rgbaf, rgba> },
	{ pctp (pixelformat_rgbaf, pixelformat_rgb888), pixelconv_plaincolor<rgbaf, rgb> },
	{ pctp (pixelformat_rgbaf, pixelformat_bgra8888), pixelconv_plaincolor<rgbaf, bgra> },
	{ pctp (pixelformat_rgbaf, pixelformat_bgr888), pixelconv_plaincolor<rgbaf, bgr> },
	{ pctp (pixelformat_rgbaf, pixelformat_bgra4444), pixelconv_plaincolor<rgbaf, bgra4> },
	{ pctp (pixelformat_rgbaf, pixelformat_bgr565), pixelconv_plaincolor<rgbaf, bgr565> },
	{ pctp (pixelformat_rgbaf, pixelformat_grayscale8), pixelconv_plaincolor<rgbaf, grayscale> },
	{ pctp (pixelformat_rgbaf, pixelformat_grayscalef), pixelconv_plaincolor<rgbaf, grayscalef> },
	{ pctp (pixelformat_rgbaf, pixelformat_yuva8888), pixelconv_plaincolor<rgbaf, yuva> },
	{ pctp (pixelformat_rgbaf, pixelformat_yuv888), pixelconv_plaincolor<rgbaf, yuv> },

	{ pctp (pixelformat_bgra8888, pixelformat_rgba8888), pixelconv_plaincolor<bgra, rgba> },
	{ pctp (pixelformat_bgra8888, pixelformat_rgb888), pixelconv_plaincolor<bgra, rgb> },
	{ pctp (pixelformat_bgra8888, pixelformat_rgbaf), pixelconv_plaincolor<bgra, rgbaf> },
	{ pctp (pixelformat_bgra8888, pixelformat_bgr888), pixelconv_plaincolor<bgra, bgr> },
	{ pctp (pixelformat_bgra8888, pixelformat_bgra4444), pixelconv_plaincolor<bgra, bgra4> },
	{ pctp (pixelformat_bgra8888, pixelformat_bgr565), pixelconv_plaincolor<bgra, bgr565> },
	{ pctp (pixelformat_bgra8888, pixelformat_grayscale8), pixelconv_plaincolor<bgra, grayscale> },
	{ pctp (pixelformat_bgra8888, pixelformat_grayscalef), pixelconv_plaincolor<bgra, grayscalef> },
	{ pctp (pixelformat_bgra8888, pixelformat_yuva8888), pixelconv_plaincolor<bgra, yuva> },
	{ pctp (pixelformat_bgra8888, pixelformat_yuv888), pixelconv_plaincolor<bgra, yuv> },

	{ pctp (pixelformat_bgr888, pixelformat_rgba8888), pixelconv_plaincolor<bgr, rgba> },
	{ pctp (pixelformat_bgr888, pixelformat_rgb888), pixelconv_plaincolor<bgr, rgb> },
	{ pctp (pixelformat_bgr888, pixelformat_rgbaf), pixelconv_plaincolor<bgr, rgbaf> },
	{ pctp (pixelformat_bgr888, pixelformat_bgra8888), pixelconv_plaincolor<bgr, bgra> },
	{ pctp (pixelformat_bgr888, pixelformat_bgra4444), pixelconv_plaincolor<bgr, bgra4> },
	{ pctp (pixelformat_bgr888, pixelformat_bgr565), pixelconv_plaincolor<bgr, bgr565> },
	{ pctp (pixelformat_bgr888, pixelformat_grayscale8), pixelconv_plaincolor<bgr, grayscale> },
	{ pctp (pixelformat_bgr888, pixelformat_grayscalef), pixelconv_plaincolor<bgr, grayscalef> },
	{ pctp (pixelformat_bgr888, pixelformat_yuva8888), pixelconv_plaincolor<bgr, yuva> },
	{ pctp (pixelformat_bgr888, pixelformat_yuv888), pixelconv_plaincolor<bgr, yuv> },

	{ pctp (pixelformat_bgra4444, pixelformat_rgba8888), pixelconv_plaincolor<bgra4, rgba> },
	{ pctp (pixelformat_bgra4444, pixelformat_rgb888), pixelconv_plaincolor<bgra4, rgb> },
	{ pctp (pixelformat_bgra4444, pixelformat_rgbaf), pixelconv_plaincolor<bgra4, rgbaf> },
	{ pctp (pixelformat_bgra4444, pixelformat_bgra8888), pixelconv_plaincolor<bgra4, bgra> },
	{ pctp (pixelformat_bgra4444, pixelformat_bgr888), pixelconv_plaincolor<bgra4, bgr> },
	{ pctp (pixelformat_bgra4444, pixelformat_bgr565), pixelconv_plaincolor<bgra4, bgr565> },
	{ pctp (pixelformat_bgra4444, pixelformat_grayscale8), pixelconv_plaincolor<bgra4, grayscale> },
	{ pctp (pixelformat_bgra4444, pixelformat_grayscalef), pixelconv_plaincolor<bgra4, grayscalef> },
	{ pctp (pixelformat_bgra4444, pixelformat_yuva8888), pixelconv_plaincolor<bgra4, yuva> },
	{ pctp (pixelformat_bgra4444, pixelformat_yuv888), pixelconv_plaincolor<bgra4, yuv> },

	{ pctp (pixelformat_bgr565, pixelformat_rgba8888), pixelconv_plaincolor<bgr565, rgba> },
	{ pctp (pixelformat_bgr565, pixelformat_rgb888), pixelconv_plaincolor<bgr565, rgb> },
	{ pctp (pixelformat_bgr565, pixelformat_rgbaf), pixelconv_plaincolor<bgr565, rgbaf> },
	{ pctp (pixelformat_bgr565, pixelformat_bgra8888), pixelconv_plaincolor<bgr565, bgra> },
	{ pctp (pixelformat_bgr565, pixelformat_bgr888), pixelconv_plaincolor<bgr565, bgr> },
	{ pctp (pixelformat_bgr565, pixelformat_bgra4444), pixelconv_plaincolor<bgr565, bgra4> },
	{ pctp (pixelformat_bgr565, pixelformat_grayscale8), pixelconv_plaincolor<bgr565, grayscale> },
	{ pctp (pixelformat_bgr565, pixelformat_grayscalef), pixelconv_plaincolor<bgr565, grayscalef> },
	{ pctp (pixelformat_bgr565, pixelformat_yuva8888), pixelconv_plaincolor<bgr565, yuva> },
	{ pctp (pixelformat_bgr565, pixelformat_yuv888), pixelconv_plaincolor<bgr565, yuv> },

	{ pctp (pixelformat_grayscale8, pixelformat_rgba8888), pixelconv_plaincolor<grayscale, rgba> },
	{ pctp (pixelformat_grayscale8, pixelformat_rgb888), pixelconv_plaincolor<grayscale, rgb> },
	{ pctp (pixelformat_grayscale8, pixelformat_rgbaf), pixelconv_plaincolor<grayscale, rgbaf> },
	{ pctp (pixelformat_grayscale8, pixelformat_bgra8888), pixelconv_plaincolor<grayscale, bgra> },
	{ pctp (pixelformat_grayscale8, pixelformat_bgr888), pixelconv_plaincolor<grayscale, bgr> },
	{ pctp (pixelformat_grayscale8, pixelformat_bgra4444), pixelconv_plaincolor<grayscale, bgra4> },
	{ pctp (pixelformat_grayscale8, pixelformat_bgr565), pixelconv_plaincolor<grayscale, bgr565> },
	{ pctp (pixelformat_grayscale8, pixelformat_grayscalef), pixelconv_plaincolor<grayscale, grayscalef> },
	{ pctp (pixelformat_grayscale8, pixelformat_yuva8888), pixelconv_plaincolor<grayscale, yuva> },
	{ pctp (pixelformat_grayscale8, pixelformat_yuv888), pixelconv_plaincolor<grayscale, yuv> },

	{ pctp (pixelformat_grayscalef, pixelformat_rgba8888), pixelconv_plaincolor<grayscalef, rgba> },
	{ pctp (pixelformat_grayscalef, pixelformat_rgb888), pixelconv_plaincolor<grayscalef, rgb> },
	{ pctp (pixelformat_grayscalef, pixelformat_rgbaf), pixelconv_plaincolor<grayscalef, rgbaf> },
	{ pctp (pixelformat_grayscalef, pixelformat_bgra8888), pixelconv_plaincolor<grayscalef, bgra> },
	{ pctp (pixelformat_grayscalef, pixelformat_bgr888), pixelconv_plaincolor<grayscalef, bgr> },
	{ pctp (pixelformat_grayscalef, pixelformat_bgra4444), pixelconv_plaincolor<grayscalef, bgra4> },
	{ pctp (pixelformat_grayscalef, pixelformat_bgr565), pixelconv_plaincolor<grayscalef, bgr565> },
	{ pctp (pixelformat_grayscalef, pixelformat_grayscale8), pixelconv_plaincolor<grayscalef, grayscale> },
	{ pctp (pixelformat_grayscalef, pixelformat_yuva8888), pixelconv_plaincolor<grayscalef, yuva> },
	{ pctp (pixelformat_grayscalef, pixelformat_yuv888), pixelconv_plaincolor<grayscalef, yuv> },

	{ pctp (pixelformat_yuva8888, pixelformat_rgba8888), pixelconv_plaincolor<yuva, rgba> },
	{ pctp (pixelformat_yuva8888, pixelformat_rgb888), pixelconv_plaincolor<yuva, rgb> },
	{ pctp (pixelformat_yuva8888, pixelformat_rgbaf), pixelconv_plaincolor<yuva, rgbaf> },
	{ pctp (pixelformat_yuva8888, pixelformat_bgra8888), pixelconv_plaincolor<yuva, bgra> },
	{ pctp (pixelformat_yuva8888, pixelformat_bgr888), pixelconv_plaincolor<yuva, bgr> },
	{ pctp (pixelformat_yuva8888, pixelformat_bgra4444), pixelconv_plaincolor<yuva, bgra4> },
	{ pctp (pixelformat_yuva8888, pixelformat_bgr565), pixelconv_plaincolor<yuva, bgr565> },
	{ pctp (pixelformat_yuva8888, pixelformat_grayscale8), pixelconv_plaincolor<yuva, grayscale> },
	{ pctp (pixelformat_yuva8888, pixelformat_grayscalef), pixelconv_plaincolor<yuva, grayscalef> },
	{ pctp (pixelformat_yuva8888, pixelformat_yuv888), pixelconv_plaincolor<yuva, yuv> },

	{ pctp (pixelformat_yuv888, pixelformat_rgba8888), pixelconv_plaincolor<yuv, rgba> },
	{ pctp (pixelformat_yuv888, pixelformat_rgb888), pixelconv_plaincolor<yuv, rgb> },
	{ pctp (pixelformat_yuv888, pixelformat_rgbaf), pixelconv_plaincolor<yuv, rgbaf> },
	{ pctp (pixelformat_yuv888, pixelformat_bgra8888), pixelconv_plaincolor<yuv, bgra> },
	{ pctp (pixelformat_yuv888, pixelformat_bgr888), pixelconv_plaincolor<yuv, bgr> },
	{ pctp (pixelformat_yuv888, pixelformat_bgra4444), pixelconv_plaincolor<yuv, bgra4> },
	{ pctp (pixelformat_yuv888, pixelformat_bgr565), pixelconv_plaincolor<yuv, bgr565> },
	{ pctp (pixelformat_yuv888, pixelformat_grayscale8), pixelconv_plaincolor<yuv, grayscale> },
	{ pctp (pixelformat_yuv888, pixelformat_grayscalef), pixelconv_plaincolor<yuv, grayscalef> },
	{ pctp (pixelformat_yuv888, pixelformat_yuva8888), pixelconv_plaincolor<yuv, yuva> },

	{ pctp (pixelformat_bgra8888, pixelformat_bgra8888_indexed8), pixelconv<pixelformat_bgra8888, pixelformat_bgra8888_indexed8> },
	{ pctp (pixelformat_bgra8888, pixelformat_bgr888_indexed8), pixelconv<pixelformat_bgra8888, pixelformat_bgr888_indexed8> },
	{ pctp (pixelformat_bgr888, pixelformat_bgra8888_indexed8), pixelconv<pixelformat_bgr888, pixelformat_bgra8888_indexed8> },
	{ pctp (pixelformat_bgr888, pixelformat_bgr888_indexed8), pixelconv<pixelformat_bgr888, pixelformat_bgr888_indexed8> },

	{ pctp (pixelformat_bgra8888_indexed8, pixelformat_bgra8888), pixelconv<pixelformat_bgra8888_indexed8, pixelformat_bgra8888> },
	{ pctp (pixelformat_bgra8888_indexed8, pixelformat_bgr888), pixelconv<pixelformat_bgra8888_indexed8, pixelformat_bgr888> },
	{ pctp (pixelformat_bgr888_indexed8, pixelformat_bgra8888), pixelconv<pixelformat_bgr888_indexed8, pixelformat_bgra8888> },
	{ pctp (pixelformat_bgr888_indexed8, pixelformat_bgr888), pixelconv<pixelformat_bgr888_indexed8, pixelformat_bgr888> },

	{ pctp (pixelformat_yuv888, pixelformat_bgr888), pixelconv<pixelformat_yuv888, pixelformat_bgr888> },
	{ pctp (pixelformat_yuva8888, pixelformat_bgra8888), pixelconv<pixelformat_yuva8888, pixelformat_bgra8888> },

	{ pctp (pixelformat_bgr888, pixelformat_yuv888), pixelconv<pixelformat_bgr888, pixelformat_yuv888> },
	{ pctp (pixelformat_bgra8888, pixelformat_yuv888), pixelconv<pixelformat_bgra8888, pixelformat_yuv888> },
	
	//{ pctp (pixelformat_yuyv8888, pixelformat_bgr888), pixelconv<pixelformat_yuyv8888, pixelformat_bgr888> },
	//{ pctp (pixelformat_yyyyuv888888, pixelformat_bgr888), pixelconv<pixelformat_yyyyuv888888, pixelformat_bgr888> },

	//{ pctp (pixelformat_bgr888, pixelformat_yuyv8888), pixelconv<pixelformat_bgr888, pixelformat_yuyv8888> },
	//{ pctp (pixelformat_bgr888, pixelformat_yyyyuv888888), pixelconv<pixelformat_bgr888, pixelformat_yyyyuv888888> },

	{ pctp (pixelformat_rgba8888, pixelformat_bc1), pixelconv<pixelformat_rgba8888, pixelformat_bc1> },
	{ pctp (pixelformat_rgba8888, pixelformat_bc2), pixelconv<pixelformat_rgba8888, pixelformat_bc2> },
	{ pctp (pixelformat_rgba8888, pixelformat_bc3), pixelconv<pixelformat_rgba8888, pixelformat_bc3> },
	{ pctp (pixelformat_rgba8888, pixelformat_bc4), pixelconv<pixelformat_rgba8888, pixelformat_bc4> },
	{ pctp (pixelformat_rgba8888, pixelformat_bc5), pixelconv<pixelformat_rgba8888, pixelformat_bc5> },

	{ pctp (pixelformat_bc1, pixelformat_rgba8888), pixelconv<pixelformat_bc1, pixelformat_rgba8888> },
	{ pctp (pixelformat_bc2, pixelformat_rgba8888), pixelconv<pixelformat_bc2, pixelformat_rgba8888> },
	{ pctp (pixelformat_bc3, pixelformat_rgba8888), pixelconv<pixelformat_bc3, pixelformat_rgba8888> },
	{ pctp (pixelformat_bc4, pixelformat_rgba8888), pixelconv<pixelformat_bc4, pixelformat_rgba8888> },
	{ pctp (pixelformat_bc5, pixelformat_rgba8888), pixelconv<pixelformat_bc5, pixelformat_rgba8888> },

	{ pctp (pixelformat_rgba8888, pixelformat_etc1), pixelconv<pixelformat_rgba8888, pixelformat_etc1> },
	{ pctp (pixelformat_rgb888, pixelformat_etc1), pixelconv<pixelformat_rgb888, pixelformat_etc1> },

	{ pctp (pixelformat_etc1, pixelformat_rgba8888), pixelconv<pixelformat_etc1, pixelformat_rgba8888> },
	{ pctp (pixelformat_etc1, pixelformat_rgb888), pixelconv<pixelformat_etc1, pixelformat_rgb888> },

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

	pctp tp (reformat, originalFormat);
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