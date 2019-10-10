#include <dseed.h>

#include <vector>
#include <map>

#include "../libs/exoquant/exoquant.h"
#include "../libs/exoquant/exoquant.c"

#include "../libs/etc1_utils/etc1_utils.h"
#include "../libs/etc1_utils/etc1_utils.c"

#if defined ( USE_SQUISH )
#	include <squish.h>
#endif

using namespace dseed;

#define PIXELCONV_ARGS										uint8_t* dest, const uint8_t* src, const dseed::size3i& size, uint8_t* destPalette, uint8_t* srcPalette
using pcfn = std::function<int (PIXELCONV_ARGS)>;
using pctp = std::tuple<pixelformat, pixelformat>;
template<pixelformat destformat, pixelformat srcformat>
inline int pixelconv (PIXELCONV_ARGS) noexcept
{
	static_assert (true, "Not support those formats currently.");
	return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// RGB/BGR/Grayscale/YUV series Conversions
//
////////////////////////////////////////////////////////////////////////////////////////////

template<class TDest, class TSrc>
inline int pixelconv_plaincolor (PIXELCONV_ARGS) noexcept
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

template<class TDest, class TSrc>
inline int pixelconv_from_indexedcolor (PIXELCONV_ARGS) noexcept
{
	size_t destStride = get_bitmap_stride (type2format<TDest> (), size.width)
		, srcStride = get_bitmap_stride (type2indexedformat<TSrc> (), size.width);
	size_t destDepth = get_bitmap_plane_size (type2format<TDest> (), size.width, size.height)
		, srcDepth = get_bitmap_plane_size (type2indexedformat<TSrc> (), size.width, size.height);

	for (size_t z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth
			, srcDepthZ = z * srcDepth;
		for (size_t y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride
				, srcStrideY = y * srcStride;
			TDest* destPtr = (TDest*)(dest + destDepthZ + destStrideY);
			const uint8_t* srcPtr = (uint8_t*)(src + srcDepthZ + srcStrideY);

			for (size_t x = 0; x < size.width; ++x)
			{
				TDest* destPtrX = destPtr + x;
				const uint8_t* srcPtrX = srcPtr + x;
				TSrc indexedColor = ((const TSrc*)srcPalette)[*srcPtrX];
				*destPtrX = (TDest)indexedColor;
			}
		}
	}

	return 0;
}

template<class TDest, class TSrc>
inline int pixelconv_to_indexedcolor (PIXELCONV_ARGS) noexcept
{
	std::vector<uint8_t> conved;
	conved.resize (get_bitmap_plane_size (dseed::pixelformat::bgra8888, size.width, size.height) * size.depth);
	pixelconv_plaincolor<bgra, TSrc> (conved.data (), src, size, nullptr, nullptr);

	if (typeid(TDest) == typeid(bgra))
		return pixelconv_to_indexedcolor<bgra, bgra> (dest, conved.data (), size, destPalette, nullptr);
	else if (typeid(TDest) == typeid(bgr))
		return pixelconv_to_indexedcolor<bgr, bgra> (dest, conved.data (), size, destPalette, nullptr);
	return 0;
}

template<> 
inline int pixelconv_to_indexedcolor<bgra, bgra> (PIXELCONV_ARGS) noexcept
{
	exq_data* pExq;
	pExq = exq_init ();
	exq_feed (pExq, (uint8_t*)src, size.width * size.height * size.depth);
	exq_quantize_hq (pExq, 256);
	std::vector<bgra> palette (256);
	exq_get_palette (pExq, (uint8_t*)palette.data (), 256);
	memcpy (destPalette, palette.data (), sizeof (dseed::bgra) * 256);

	exq_map_image_dither (pExq, size.width, size.height * size.depth, (uint8_t*)src, dest, 0);

	exq_free (pExq);

	return 256;
}
template<>
inline int pixelconv_to_indexedcolor<bgr, bgra> (PIXELCONV_ARGS) noexcept
{
	std::vector<bgra> palette (256);
	size_t count = pixelconv_to_indexedcolor<bgra, bgra> (dest, src, size, (uint8_t*)palette.data (), nullptr);

	bgr* destPaletteBGR = (bgr*)destPalette;
	for (int i = 0; i < count; ++i)
		destPaletteBGR[i] = palette[i];

	return (int)count;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Packed YCbCr series <-> BGR Conversions
//
////////////////////////////////////////////////////////////////////////////////////////////

#if COMPILER_MSVC
#	pragma pack (push, 1)
#else
#	pragma pack (1)
#endif
struct yuyv { uint8_t y1, u, y2, v; yuyv (uint8_t y1, uint8_t y2, uint8_t u, uint8_t v) : y1 (y1), y2 (y2), u (u), v (v) { } };
struct uv { uint8_t u, v; uv (uint8_t u, uint8_t v) : u (u), v (v) { } };
#if COMPILER_MSVC
#	pragma pack (pop)
#else
#	pragma pack ()
#endif

template<class TSrc>
inline int pixelconv_to_chromasubsample_yuv422 (PIXELCONV_ARGS) noexcept
{
	size_t destStride = get_bitmap_stride (dseed::pixelformat::yuyv8888, size.width)
		, srcStride = get_bitmap_stride (type2format<TSrc> (), size.width);
	size_t destDepth = get_bitmap_plane_size (dseed::pixelformat::yuyv8888, size.width, size.height)
		, srcDepth = get_bitmap_plane_size (type2format<TSrc> (), size.width, size.height);

	for (size_t z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth
			, srcDepthZ = z * srcDepth;
		for (size_t y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride
				, srcStrideY = y * srcStride;
			yuyv* destPtr = (yuyv*)(dest + destDepthZ + destStrideY);
			const TSrc* srcPtr = (TSrc*)(src + srcDepthZ + srcStrideY);

			for (size_t x = 0; x < size.width; x += 2)
			{
				yuyv* destPtrX = destPtr + (x / 2);
				const TSrc* srcPtrX = srcPtr + x;
				yuv srcColor1 = *srcPtrX;
				grayscale srcColor2 = {0};
				if (x + 1 < size.width)
					srcColor2 = *(srcPtrX + 1);
				*destPtrX = yuyv (srcColor1.y, srcColor2.color, srcColor1.u, srcColor1.v);
			}
		}
	}

	return 0;
}
template<class TDest>
inline int pixelconv_from_chromasubsample_yuv422 (PIXELCONV_ARGS) noexcept
{
	size_t destStride = get_bitmap_stride (type2format<TDest> (), size.width)
		, srcStride = get_bitmap_stride (dseed::pixelformat::yuyv8888, size.width);
	size_t destDepth = get_bitmap_plane_size (type2format<TDest> (), size.width, size.height)
		, srcDepth = get_bitmap_plane_size (dseed::pixelformat::yuyv8888, size.width, size.height);

	for (size_t z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth
			, srcDepthZ = z * srcDepth;
		for (size_t y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride
				, srcStrideY = y * srcStride;
			TDest* destPtr = (TDest*)(dest + destDepthZ + destStrideY);
			const yuyv* srcPtr = (yuyv*)(src + srcDepthZ + srcStrideY);

			for (size_t x = 0; x < size.width; x += 2)
			{
				TDest* destPtrX = destPtr + x;
				const yuyv* srcPtrX = srcPtr + (x / 2);
				*destPtrX = yuv (srcPtrX->y1, srcPtrX->u, srcPtrX->v);
				if (x + 1 < size.width)
					* (destPtrX + 1) = yuv (srcPtrX->y2, srcPtrX->u, srcPtrX->v);
			}
		}
	}

	return 0;
}

template<class TSrc>
inline int pixelconv_to_chromasubsample_nv12 (PIXELCONV_ARGS) noexcept
{
	size_t destYStride = size.width
		, destUVStride = (size_t)ceil(size.width / 2.0) * 2
		, srcStride = get_bitmap_stride (type2format<TSrc> (), size.width);
	size_t destDepth = get_bitmap_plane_size (dseed::pixelformat::nv12, size.width, size.height)
		, srcDepth = get_bitmap_plane_size (type2format<TSrc> (), size.width, size.height);
	size_t ySize = size.width * size.height;

	for (size_t z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth
			, srcDepthZ = z * srcDepth;
		for (size_t y = 0; y < size.height; ++y)
		{
			size_t destYStrideY = y * destYStride
				, destUVStrideY = (y / 2) * destUVStride
				, srcStrideY = y * srcStride;
			grayscale* destYPtr = (grayscale*)(dest + destDepthZ + destYStrideY);
			uv* destUVPtr = (uv*)(dest + destDepthZ + ySize + destUVStrideY);
			const TSrc* srcPtr = (TSrc*)(src + srcDepthZ + srcStrideY);

			for (size_t x = 0; x < size.width; ++x)
			{
				grayscale* destYPtrX = destYPtr + x;
				const TSrc* srcPtrX = srcPtr + x;
				*destYPtrX = *srcPtrX;

				if ((x + 1) % 2 == 1 && (y + 1) % 2 == 1)
				{
					uv* destUVPtrX = destUVPtr + (x / 2);
					yuv yuv = *srcPtrX;
					*destUVPtrX = uv (yuv.u, yuv.v);
				}
			}
		}
	}

	return 0;
}
template<class TDest>
inline int pixelconv_from_chromasubsample_nv12 (PIXELCONV_ARGS) noexcept
{
	size_t destStride = get_bitmap_stride (type2format<TDest> (), size.width)
		, srcYStride = size.width
		, srcUVStride = (size_t)ceil (size.width / 2.0) * 2;
	size_t destDepth = get_bitmap_plane_size (type2format<TDest> (), size.width, size.height)
		, srcDepth = get_bitmap_plane_size (dseed::pixelformat::nv12, size.width, size.height);
	size_t ySize = size.width * size.height;

	for (size_t z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth
			, srcDepthZ = z * srcDepth;
		for (size_t y = 0; y < size.height; ++y)
		{
			size_t destStrideY = y * destStride
				, srcYStrideY = y * srcYStride
				, srcUVStrideY = (y / 2) * srcUVStride;
			TDest* destPtr = (TDest*)(dest + destDepthZ + destStrideY);
			const grayscale* srcYPtr = (grayscale*)(src + srcDepthZ + srcYStrideY);
			const uv* srcUVPtr = (uv*)(src + srcDepthZ + ySize + srcUVStrideY);

			for (size_t x = 0; x < size.width; ++x)
			{
				TDest* destPtrX = destPtr + x;
				const grayscale* srcYPtrX = srcYPtr + x;
				const uv* srcUVPtrX = srcUVPtr + (x / 2);
				*destPtrX = yuv (srcYPtrX->color, srcUVPtrX->u, srcUVPtrX->v);
			}
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// DXT series <-> RGBA Conversions
//
////////////////////////////////////////////////////////////////////////////////////////////

enum BCENUM { BC1 = (1 << 0), BC2 = (1 << 1), BC3 = (1 << 2), BC4 = (1 << 3), BC5 = (1 << 4) };
constexpr pixelformat BC_TO_PF (BCENUM compression) noexcept { switch (compression) { case BC1: return pixelformat::bc1; case BC2: return pixelformat::bc2;
case BC3: return pixelformat::bc3; case BC4: return pixelformat::bc4; case BC5: return pixelformat::bc5; default: return pixelformat::unknown; } }

template<BCENUM compression>
inline int pixelconv_to_dxt (PIXELCONV_ARGS) noexcept
{
#if defined ( USE_SQUISH )
	size_t destDepth = dseed::get_bitmap_plane_size (pixelformat::rgba8888, size.width, size.height),
		srcDepth = dseed::get_bitmap_plane_size (BC_TO_PF(compression), size.width, size.height);
	for (int z = 0; z < size.depth; ++z)
		squish::DecompressImage (dest + (destDepth * z), size.width, size.height, src + (srcDepth * z), compression);
	return 0;
#else
	return -1;
#endif
}
template<BCENUM compression>
inline int pixelconv_from_dxt (PIXELCONV_ARGS) noexcept
{
#if defined ( USE_SQUISH )
	size_t destDepth = dseed::get_bitmap_plane_size (BC_TO_PF (compression), size.width, size.height),
		srcDepth = dseed::get_bitmap_plane_size (pixelformat::rgba8888, size.width, size.height);
	for (int z = 0; z < size.depth; ++z)
		squish::CompressImage (src + (srcDepth * z), size.width, size.height, dest + (destDepth * z)
			, compression | squish::kColourIterativeClusterFit);
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

template<> inline int pixelconv<pixelformat::rgb888, pixelformat::etc1> (PIXELCONV_ARGS)
{
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat::rgb888, size.width, size.height),
		srcArr = dseed::get_bitmap_plane_size (pixelformat::etc1, size.width, size.height);
	size_t destStride = dseed::get_bitmap_stride (dseed::pixelformat::rgb888, size.width);
	for (int z = 0; z < size.depth; ++z)
		etc1_decode_image (src + (srcArr * z), dest + (destArr * z), size.width, size.height, 3, (uint32_t)destStride);
	return 0;
}
template<> inline int pixelconv<pixelformat::rgba8888, pixelformat::etc1> (PIXELCONV_ARGS)
{
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat::rgba8888, size.width, size.height),
		tempArr = dseed::get_bitmap_plane_size (pixelformat::rgb888, size.width, size.height);
	size_t tempStride = dseed::get_bitmap_stride (dseed::pixelformat::rgb888, size.width)
		, destStride = dseed::get_bitmap_stride (dseed::pixelformat::rgba8888, size.width);
	std::vector<uint8_t> temp (tempStride * size.height * size.depth);
	pixelconv<pixelformat::rgb888, pixelformat::etc1> (temp.data (), src, size, destPalette, srcPalette);
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
template<> inline int pixelconv<pixelformat::etc1, pixelformat::rgb888> (PIXELCONV_ARGS)
{
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat::etc1, size.width, size.height),
		srcArr = dseed::get_bitmap_plane_size (pixelformat::rgb888, size.width, size.height);
	size_t srcStride = dseed::get_bitmap_stride (dseed::pixelformat::rgb888, size.width);
	for (int z = 0; z < size.depth; ++z)
		etc1_encode_image (src + (z * srcArr), size.width, size.height, 3, (etc1_uint32)srcStride, dest + (z * destArr));
	return 0;
}
template<> inline int pixelconv<pixelformat::etc1, pixelformat::rgba8888> (PIXELCONV_ARGS)
{
	size_t destArr = dseed::get_bitmap_plane_size (pixelformat::etc1, size.width, size.height),
		tempArr = dseed::get_bitmap_plane_size (pixelformat::rgb888, size.width, size.height);
	size_t tempStride = dseed::get_bitmap_stride (dseed::pixelformat::rgb888, size.width)
		, srcStride = dseed::get_bitmap_stride (dseed::pixelformat::rgba8888, size.width);
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
		etc1_encode_image (temp.data () + (z * tempArr), size.width, size.height, 3, (etc1_uint32)tempStride, dest + (z * destArr));
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Conversions Dictionary
//
////////////////////////////////////////////////////////////////////////////////////////////

std::map<pctp, pcfn> g_pixelconvs = {
	////////////////////////////////////////////////////////////////////////////////////////
	// RGB/BGR/Grayscale/YUV series Conversions
	////////////////////////////////////////////////////////////////////////////////////////
	{ pctp (pixelformat::rgba8888, pixelformat::rgb888), pixelconv_plaincolor<rgba, rgb> },
	{ pctp (pixelformat::rgba8888, pixelformat::rgbaf), pixelconv_plaincolor<rgba, rgbaf> },
	{ pctp (pixelformat::rgba8888, pixelformat::bgra8888), pixelconv_plaincolor<rgba, bgra> },
	{ pctp (pixelformat::rgba8888, pixelformat::bgr888), pixelconv_plaincolor<rgba, bgr> },
	{ pctp (pixelformat::rgba8888, pixelformat::bgra4444), pixelconv_plaincolor<rgba, bgra4> },
	{ pctp (pixelformat::rgba8888, pixelformat::bgr565), pixelconv_plaincolor<rgba, bgr565> },
	{ pctp (pixelformat::rgba8888, pixelformat::grayscale8), pixelconv_plaincolor<rgba, grayscale> },
	{ pctp (pixelformat::rgba8888, pixelformat::grayscalef), pixelconv_plaincolor<rgba, grayscalef> },
	{ pctp (pixelformat::rgba8888, pixelformat::yuva8888), pixelconv_plaincolor<rgba, yuva> },
	{ pctp (pixelformat::rgba8888, pixelformat::yuv888), pixelconv_plaincolor<rgba, yuv> },
	{ pctp (pixelformat::rgba8888, pixelformat::hsva8888), pixelconv_plaincolor<rgba, hsva> },
	{ pctp (pixelformat::rgba8888, pixelformat::hsv888), pixelconv_plaincolor<rgba, hsv> },

	{ pctp (pixelformat::rgb888, pixelformat::rgba8888), pixelconv_plaincolor<rgb, rgba> },
	{ pctp (pixelformat::rgb888, pixelformat::rgbaf), pixelconv_plaincolor<rgb, rgbaf> },
	{ pctp (pixelformat::rgb888, pixelformat::bgra8888), pixelconv_plaincolor<rgb, bgra> },
	{ pctp (pixelformat::rgb888, pixelformat::bgr888), pixelconv_plaincolor<rgb, bgr> },
	{ pctp (pixelformat::rgb888, pixelformat::bgra4444), pixelconv_plaincolor<rgb, bgra4> },
	{ pctp (pixelformat::rgb888, pixelformat::bgr565), pixelconv_plaincolor<rgb, bgr565> },
	{ pctp (pixelformat::rgb888, pixelformat::grayscale8), pixelconv_plaincolor<rgb, grayscale> },
	{ pctp (pixelformat::rgb888, pixelformat::grayscalef), pixelconv_plaincolor<rgb, grayscalef> },
	{ pctp (pixelformat::rgb888, pixelformat::yuva8888), pixelconv_plaincolor<rgb, yuva> },
	{ pctp (pixelformat::rgb888, pixelformat::yuv888), pixelconv_plaincolor<rgb, yuv> },
	{ pctp (pixelformat::rgb888, pixelformat::hsva8888), pixelconv_plaincolor<rgb, hsva> },
	{ pctp (pixelformat::rgb888, pixelformat::hsv888), pixelconv_plaincolor<rgb, hsv> },

	{ pctp (pixelformat::rgbaf, pixelformat::rgba8888), pixelconv_plaincolor<rgbaf, rgba> },
	{ pctp (pixelformat::rgbaf, pixelformat::rgb888), pixelconv_plaincolor<rgbaf, rgb> },
	{ pctp (pixelformat::rgbaf, pixelformat::bgra8888), pixelconv_plaincolor<rgbaf, bgra> },
	{ pctp (pixelformat::rgbaf, pixelformat::bgr888), pixelconv_plaincolor<rgbaf, bgr> },
	{ pctp (pixelformat::rgbaf, pixelformat::bgra4444), pixelconv_plaincolor<rgbaf, bgra4> },
	{ pctp (pixelformat::rgbaf, pixelformat::bgr565), pixelconv_plaincolor<rgbaf, bgr565> },
	{ pctp (pixelformat::rgbaf, pixelformat::grayscale8), pixelconv_plaincolor<rgbaf, grayscale> },
	{ pctp (pixelformat::rgbaf, pixelformat::grayscalef), pixelconv_plaincolor<rgbaf, grayscalef> },
	{ pctp (pixelformat::rgbaf, pixelformat::yuva8888), pixelconv_plaincolor<rgbaf, yuva> },
	{ pctp (pixelformat::rgbaf, pixelformat::yuv888), pixelconv_plaincolor<rgbaf, yuv> },
	{ pctp (pixelformat::rgbaf, pixelformat::hsva8888), pixelconv_plaincolor<rgbaf, hsva> },
	{ pctp (pixelformat::rgbaf, pixelformat::hsv888), pixelconv_plaincolor<rgbaf, hsv> },

	{ pctp (pixelformat::bgra8888, pixelformat::rgba8888), pixelconv_plaincolor<bgra, rgba> },
	{ pctp (pixelformat::bgra8888, pixelformat::rgb888), pixelconv_plaincolor<bgra, rgb> },
	{ pctp (pixelformat::bgra8888, pixelformat::rgbaf), pixelconv_plaincolor<bgra, rgbaf> },
	{ pctp (pixelformat::bgra8888, pixelformat::bgr888), pixelconv_plaincolor<bgra, bgr> },
	{ pctp (pixelformat::bgra8888, pixelformat::bgra4444), pixelconv_plaincolor<bgra, bgra4> },
	{ pctp (pixelformat::bgra8888, pixelformat::bgr565), pixelconv_plaincolor<bgra, bgr565> },
	{ pctp (pixelformat::bgra8888, pixelformat::grayscale8), pixelconv_plaincolor<bgra, grayscale> },
	{ pctp (pixelformat::bgra8888, pixelformat::grayscalef), pixelconv_plaincolor<bgra, grayscalef> },
	{ pctp (pixelformat::bgra8888, pixelformat::yuva8888), pixelconv_plaincolor<bgra, yuva> },
	{ pctp (pixelformat::bgra8888, pixelformat::yuv888), pixelconv_plaincolor<bgra, yuv> },
	{ pctp (pixelformat::bgra8888, pixelformat::hsva8888), pixelconv_plaincolor<bgra, hsva> },
	{ pctp (pixelformat::bgra8888, pixelformat::hsv888), pixelconv_plaincolor<bgra, hsv> },

	{ pctp (pixelformat::bgr888, pixelformat::rgba8888), pixelconv_plaincolor<bgr, rgba> },
	{ pctp (pixelformat::bgr888, pixelformat::rgb888), pixelconv_plaincolor<bgr, rgb> },
	{ pctp (pixelformat::bgr888, pixelformat::rgbaf), pixelconv_plaincolor<bgr, rgbaf> },
	{ pctp (pixelformat::bgr888, pixelformat::bgra8888), pixelconv_plaincolor<bgr, bgra> },
	{ pctp (pixelformat::bgr888, pixelformat::bgra4444), pixelconv_plaincolor<bgr, bgra4> },
	{ pctp (pixelformat::bgr888, pixelformat::bgr565), pixelconv_plaincolor<bgr, bgr565> },
	{ pctp (pixelformat::bgr888, pixelformat::grayscale8), pixelconv_plaincolor<bgr, grayscale> },
	{ pctp (pixelformat::bgr888, pixelformat::grayscalef), pixelconv_plaincolor<bgr, grayscalef> },
	{ pctp (pixelformat::bgr888, pixelformat::yuva8888), pixelconv_plaincolor<bgr, yuva> },
	{ pctp (pixelformat::bgr888, pixelformat::yuv888), pixelconv_plaincolor<bgr, yuv> },
	{ pctp (pixelformat::bgr888, pixelformat::hsva8888), pixelconv_plaincolor<bgr, hsva> },
	{ pctp (pixelformat::bgr888, pixelformat::hsv888), pixelconv_plaincolor<bgr, hsv> },

	{ pctp (pixelformat::bgra4444, pixelformat::rgba8888), pixelconv_plaincolor<bgra4, rgba> },
	{ pctp (pixelformat::bgra4444, pixelformat::rgb888), pixelconv_plaincolor<bgra4, rgb> },
	{ pctp (pixelformat::bgra4444, pixelformat::rgbaf), pixelconv_plaincolor<bgra4, rgbaf> },
	{ pctp (pixelformat::bgra4444, pixelformat::bgra8888), pixelconv_plaincolor<bgra4, bgra> },
	{ pctp (pixelformat::bgra4444, pixelformat::bgr888), pixelconv_plaincolor<bgra4, bgr> },
	{ pctp (pixelformat::bgra4444, pixelformat::bgr565), pixelconv_plaincolor<bgra4, bgr565> },
	{ pctp (pixelformat::bgra4444, pixelformat::grayscale8), pixelconv_plaincolor<bgra4, grayscale> },
	{ pctp (pixelformat::bgra4444, pixelformat::grayscalef), pixelconv_plaincolor<bgra4, grayscalef> },
	{ pctp (pixelformat::bgra4444, pixelformat::yuva8888), pixelconv_plaincolor<bgra4, yuva> },
	{ pctp (pixelformat::bgra4444, pixelformat::yuv888), pixelconv_plaincolor<bgra4, yuv> },
	{ pctp (pixelformat::bgra4444, pixelformat::hsva8888), pixelconv_plaincolor<bgra4, hsva> },
	{ pctp (pixelformat::bgra4444, pixelformat::hsv888), pixelconv_plaincolor<bgra4, hsv> },

	{ pctp (pixelformat::bgr565, pixelformat::rgba8888), pixelconv_plaincolor<bgr565, rgba> },
	{ pctp (pixelformat::bgr565, pixelformat::rgb888), pixelconv_plaincolor<bgr565, rgb> },
	{ pctp (pixelformat::bgr565, pixelformat::rgbaf), pixelconv_plaincolor<bgr565, rgbaf> },
	{ pctp (pixelformat::bgr565, pixelformat::bgra8888), pixelconv_plaincolor<bgr565, bgra> },
	{ pctp (pixelformat::bgr565, pixelformat::bgr888), pixelconv_plaincolor<bgr565, bgr> },
	{ pctp (pixelformat::bgr565, pixelformat::bgra4444), pixelconv_plaincolor<bgr565, bgra4> },
	{ pctp (pixelformat::bgr565, pixelformat::grayscale8), pixelconv_plaincolor<bgr565, grayscale> },
	{ pctp (pixelformat::bgr565, pixelformat::grayscalef), pixelconv_plaincolor<bgr565, grayscalef> },
	{ pctp (pixelformat::bgr565, pixelformat::yuva8888), pixelconv_plaincolor<bgr565, yuva> },
	{ pctp (pixelformat::bgr565, pixelformat::yuv888), pixelconv_plaincolor<bgr565, yuv> },
	{ pctp (pixelformat::bgr565, pixelformat::hsva8888), pixelconv_plaincolor<bgr565, hsva> },
	{ pctp (pixelformat::bgr565, pixelformat::hsv888), pixelconv_plaincolor<bgr565, hsv> },

	{ pctp (pixelformat::grayscale8, pixelformat::rgba8888), pixelconv_plaincolor<grayscale, rgba> },
	{ pctp (pixelformat::grayscale8, pixelformat::rgb888), pixelconv_plaincolor<grayscale, rgb> },
	{ pctp (pixelformat::grayscale8, pixelformat::rgbaf), pixelconv_plaincolor<grayscale, rgbaf> },
	{ pctp (pixelformat::grayscale8, pixelformat::bgra8888), pixelconv_plaincolor<grayscale, bgra> },
	{ pctp (pixelformat::grayscale8, pixelformat::bgr888), pixelconv_plaincolor<grayscale, bgr> },
	{ pctp (pixelformat::grayscale8, pixelformat::bgra4444), pixelconv_plaincolor<grayscale, bgra4> },
	{ pctp (pixelformat::grayscale8, pixelformat::bgr565), pixelconv_plaincolor<grayscale, bgr565> },
	{ pctp (pixelformat::grayscale8, pixelformat::grayscalef), pixelconv_plaincolor<grayscale, grayscalef> },
	{ pctp (pixelformat::grayscale8, pixelformat::yuva8888), pixelconv_plaincolor<grayscale, yuva> },
	{ pctp (pixelformat::grayscale8, pixelformat::yuv888), pixelconv_plaincolor<grayscale, yuv> },
	{ pctp (pixelformat::grayscale8, pixelformat::hsva8888), pixelconv_plaincolor<grayscale, hsva> },
	{ pctp (pixelformat::grayscale8, pixelformat::hsv888), pixelconv_plaincolor<grayscale, hsv> },

	{ pctp (pixelformat::grayscalef, pixelformat::rgba8888), pixelconv_plaincolor<grayscalef, rgba> },
	{ pctp (pixelformat::grayscalef, pixelformat::rgb888), pixelconv_plaincolor<grayscalef, rgb> },
	{ pctp (pixelformat::grayscalef, pixelformat::rgbaf), pixelconv_plaincolor<grayscalef, rgbaf> },
	{ pctp (pixelformat::grayscalef, pixelformat::bgra8888), pixelconv_plaincolor<grayscalef, bgra> },
	{ pctp (pixelformat::grayscalef, pixelformat::bgr888), pixelconv_plaincolor<grayscalef, bgr> },
	{ pctp (pixelformat::grayscalef, pixelformat::bgra4444), pixelconv_plaincolor<grayscalef, bgra4> },
	{ pctp (pixelformat::grayscalef, pixelformat::bgr565), pixelconv_plaincolor<grayscalef, bgr565> },
	{ pctp (pixelformat::grayscalef, pixelformat::grayscale8), pixelconv_plaincolor<grayscalef, grayscale> },
	{ pctp (pixelformat::grayscalef, pixelformat::yuva8888), pixelconv_plaincolor<grayscalef, yuva> },
	{ pctp (pixelformat::grayscalef, pixelformat::yuv888), pixelconv_plaincolor<grayscalef, yuv> },
	{ pctp (pixelformat::grayscalef, pixelformat::hsva8888), pixelconv_plaincolor<grayscalef, hsva> },
	{ pctp (pixelformat::grayscalef, pixelformat::hsv888), pixelconv_plaincolor<grayscalef, hsv> },

	{ pctp (pixelformat::yuva8888, pixelformat::rgba8888), pixelconv_plaincolor<yuva, rgba> },
	{ pctp (pixelformat::yuva8888, pixelformat::rgb888), pixelconv_plaincolor<yuva, rgb> },
	{ pctp (pixelformat::yuva8888, pixelformat::rgbaf), pixelconv_plaincolor<yuva, rgbaf> },
	{ pctp (pixelformat::yuva8888, pixelformat::bgra8888), pixelconv_plaincolor<yuva, bgra> },
	{ pctp (pixelformat::yuva8888, pixelformat::bgr888), pixelconv_plaincolor<yuva, bgr> },
	{ pctp (pixelformat::yuva8888, pixelformat::bgra4444), pixelconv_plaincolor<yuva, bgra4> },
	{ pctp (pixelformat::yuva8888, pixelformat::bgr565), pixelconv_plaincolor<yuva, bgr565> },
	{ pctp (pixelformat::yuva8888, pixelformat::grayscale8), pixelconv_plaincolor<yuva, grayscale> },
	{ pctp (pixelformat::yuva8888, pixelformat::grayscalef), pixelconv_plaincolor<yuva, grayscalef> },
	{ pctp (pixelformat::yuva8888, pixelformat::yuv888), pixelconv_plaincolor<yuva, yuv> },
	{ pctp (pixelformat::yuva8888, pixelformat::hsva8888), pixelconv_plaincolor<yuva, hsva> },
	{ pctp (pixelformat::yuva8888, pixelformat::hsv888), pixelconv_plaincolor<yuva, hsv> },

	{ pctp (pixelformat::yuv888, pixelformat::rgba8888), pixelconv_plaincolor<yuv, rgba> },
	{ pctp (pixelformat::yuv888, pixelformat::rgb888), pixelconv_plaincolor<yuv, rgb> },
	{ pctp (pixelformat::yuv888, pixelformat::rgbaf), pixelconv_plaincolor<yuv, rgbaf> },
	{ pctp (pixelformat::yuv888, pixelformat::bgra8888), pixelconv_plaincolor<yuv, bgra> },
	{ pctp (pixelformat::yuv888, pixelformat::bgr888), pixelconv_plaincolor<yuv, bgr> },
	{ pctp (pixelformat::yuv888, pixelformat::bgra4444), pixelconv_plaincolor<yuv, bgra4> },
	{ pctp (pixelformat::yuv888, pixelformat::bgr565), pixelconv_plaincolor<yuv, bgr565> },
	{ pctp (pixelformat::yuv888, pixelformat::grayscale8), pixelconv_plaincolor<yuv, grayscale> },
	{ pctp (pixelformat::yuv888, pixelformat::grayscalef), pixelconv_plaincolor<yuv, grayscalef> },
	{ pctp (pixelformat::yuv888, pixelformat::yuva8888), pixelconv_plaincolor<yuv, yuva> },
	{ pctp (pixelformat::yuv888, pixelformat::hsva8888), pixelconv_plaincolor<yuv, hsva> },
	{ pctp (pixelformat::yuv888, pixelformat::hsv888), pixelconv_plaincolor<yuv, hsv> },

	{ pctp (pixelformat::hsva8888, pixelformat::rgba8888), pixelconv_plaincolor<hsva, rgba> },
	{ pctp (pixelformat::hsva8888, pixelformat::rgb888), pixelconv_plaincolor<hsva, rgb> },
	{ pctp (pixelformat::hsva8888, pixelformat::rgbaf), pixelconv_plaincolor<hsva, rgbaf> },
	{ pctp (pixelformat::hsva8888, pixelformat::bgra8888), pixelconv_plaincolor<hsva, bgra> },
	{ pctp (pixelformat::hsva8888, pixelformat::bgr888), pixelconv_plaincolor<hsva, bgr> },
	{ pctp (pixelformat::hsva8888, pixelformat::bgra4444), pixelconv_plaincolor<hsva, bgra4> },
	{ pctp (pixelformat::hsva8888, pixelformat::bgr565), pixelconv_plaincolor<hsva, bgr565> },
	{ pctp (pixelformat::hsva8888, pixelformat::grayscale8), pixelconv_plaincolor<hsva, grayscale> },
	{ pctp (pixelformat::hsva8888, pixelformat::grayscalef), pixelconv_plaincolor<hsva, grayscalef> },
	{ pctp (pixelformat::hsva8888, pixelformat::yuva8888), pixelconv_plaincolor<hsva, yuva> },
	{ pctp (pixelformat::hsva8888, pixelformat::yuv888), pixelconv_plaincolor<hsva, yuv> },
	{ pctp (pixelformat::hsva8888, pixelformat::hsv888), pixelconv_plaincolor<hsva, hsv> },

	{ pctp (pixelformat::hsv888, pixelformat::rgba8888), pixelconv_plaincolor<hsv, rgba> },
	{ pctp (pixelformat::hsv888, pixelformat::rgb888), pixelconv_plaincolor<hsv, rgb> },
	{ pctp (pixelformat::hsv888, pixelformat::rgbaf), pixelconv_plaincolor<hsv, rgbaf> },
	{ pctp (pixelformat::hsv888, pixelformat::bgra8888), pixelconv_plaincolor<hsv, bgra> },
	{ pctp (pixelformat::hsv888, pixelformat::bgr888), pixelconv_plaincolor<hsv, bgr> },
	{ pctp (pixelformat::hsv888, pixelformat::bgra4444), pixelconv_plaincolor<hsv, bgra4> },
	{ pctp (pixelformat::hsv888, pixelformat::bgr565), pixelconv_plaincolor<hsv, bgr565> },
	{ pctp (pixelformat::hsv888, pixelformat::grayscale8), pixelconv_plaincolor<hsv, grayscale> },
	{ pctp (pixelformat::hsv888, pixelformat::grayscalef), pixelconv_plaincolor<hsv, grayscalef> },
	{ pctp (pixelformat::hsv888, pixelformat::yuva8888), pixelconv_plaincolor<hsv, yuva> },
	{ pctp (pixelformat::hsv888, pixelformat::yuv888), pixelconv_plaincolor<hsv, yuv> },
	{ pctp (pixelformat::hsv888, pixelformat::hsva8888), pixelconv_plaincolor<hsv, hsva> },

	////////////////////////////////////////////////////////////////////////////////////////
	// Indexed Color Conversions
	////////////////////////////////////////////////////////////////////////////////////////
	{ pctp (pixelformat::rgba8888, pixelformat::bgra8888_indexed8), pixelconv_from_indexedcolor<rgba, bgra> },
	{ pctp (pixelformat::rgb888, pixelformat::bgra8888_indexed8), pixelconv_from_indexedcolor<rgb, bgra> },
	{ pctp (pixelformat::rgbaf, pixelformat::bgra8888_indexed8), pixelconv_from_indexedcolor<rgbaf, bgra> },
	{ pctp (pixelformat::bgra8888, pixelformat::bgra8888_indexed8), pixelconv_from_indexedcolor<bgra, bgra> },
	{ pctp (pixelformat::bgr888, pixelformat::bgra8888_indexed8), pixelconv_from_indexedcolor<bgr, bgra> },
	{ pctp (pixelformat::bgra4444, pixelformat::bgra8888_indexed8), pixelconv_from_indexedcolor<bgra4, bgra> },
	{ pctp (pixelformat::bgr565, pixelformat::bgra8888_indexed8), pixelconv_from_indexedcolor<bgr565, bgra> },
	{ pctp (pixelformat::grayscale8, pixelformat::bgra8888_indexed8), pixelconv_from_indexedcolor<grayscale, bgra> },
	{ pctp (pixelformat::grayscalef, pixelformat::bgra8888_indexed8), pixelconv_from_indexedcolor<grayscalef, bgra> },
	{ pctp (pixelformat::yuva8888, pixelformat::bgra8888_indexed8), pixelconv_from_indexedcolor<yuva, bgra> },
	{ pctp (pixelformat::yuv888, pixelformat::bgra8888_indexed8), pixelconv_from_indexedcolor<yuv, bgra> },
	{ pctp (pixelformat::hsva8888, pixelformat::bgra8888_indexed8), pixelconv_from_indexedcolor<hsva, bgra> },
	{ pctp (pixelformat::hsv888, pixelformat::bgra8888_indexed8), pixelconv_from_indexedcolor<hsv, bgra> },

	{ pctp (pixelformat::rgba8888, pixelformat::bgr888_indexed8), pixelconv_from_indexedcolor<rgba, bgr> },
	{ pctp (pixelformat::rgb888, pixelformat::bgr888_indexed8), pixelconv_from_indexedcolor<rgb, bgr> },
	{ pctp (pixelformat::rgbaf, pixelformat::bgr888_indexed8), pixelconv_from_indexedcolor<rgbaf, bgr> },
	{ pctp (pixelformat::bgra8888, pixelformat::bgr888_indexed8), pixelconv_from_indexedcolor<bgra, bgr> },
	{ pctp (pixelformat::bgr888, pixelformat::bgr888_indexed8), pixelconv_from_indexedcolor<bgr, bgr> },
	{ pctp (pixelformat::bgra4444, pixelformat::bgr888_indexed8), pixelconv_from_indexedcolor<bgra4, bgr> },
	{ pctp (pixelformat::bgr565, pixelformat::bgr888_indexed8), pixelconv_from_indexedcolor<bgr565, bgr> },
	{ pctp (pixelformat::grayscale8, pixelformat::bgr888_indexed8), pixelconv_from_indexedcolor<grayscale, bgr> },
	{ pctp (pixelformat::grayscalef, pixelformat::bgr888_indexed8), pixelconv_from_indexedcolor<grayscalef, bgr> },
	{ pctp (pixelformat::yuva8888, pixelformat::bgr888_indexed8), pixelconv_from_indexedcolor<yuva, bgr> },
	{ pctp (pixelformat::yuv888, pixelformat::bgr888_indexed8), pixelconv_from_indexedcolor<yuv, bgr> },
	{ pctp (pixelformat::hsva8888, pixelformat::bgr888_indexed8), pixelconv_from_indexedcolor<hsva, bgr> },
	{ pctp (pixelformat::hsv888, pixelformat::bgr888_indexed8), pixelconv_from_indexedcolor<hsv, bgr> },

	{ pctp (pixelformat::bgra8888_indexed8, pixelformat::rgba8888), pixelconv_to_indexedcolor<bgra, rgba> },
	{ pctp (pixelformat::bgra8888_indexed8, pixelformat::rgb888), pixelconv_to_indexedcolor<bgra, rgb> },
	{ pctp (pixelformat::bgra8888_indexed8, pixelformat::rgbaf), pixelconv_to_indexedcolor<bgra, rgbaf> },
	{ pctp (pixelformat::bgra8888_indexed8, pixelformat::bgra8888), pixelconv_to_indexedcolor<bgra, bgra> },
	{ pctp (pixelformat::bgra8888_indexed8, pixelformat::bgr888), pixelconv_to_indexedcolor<bgra, bgr> },
	{ pctp (pixelformat::bgra8888_indexed8, pixelformat::bgra4444), pixelconv_to_indexedcolor<bgra, bgra4> },
	{ pctp (pixelformat::bgra8888_indexed8, pixelformat::bgr565), pixelconv_to_indexedcolor<bgra, bgr565> },
	{ pctp (pixelformat::bgra8888_indexed8, pixelformat::grayscale8), pixelconv_to_indexedcolor<bgra, grayscale> },
	{ pctp (pixelformat::bgra8888_indexed8, pixelformat::grayscalef), pixelconv_to_indexedcolor<bgra, grayscalef> },
	{ pctp (pixelformat::bgra8888_indexed8, pixelformat::yuva8888), pixelconv_to_indexedcolor<bgra, yuva> },
	{ pctp (pixelformat::bgra8888_indexed8, pixelformat::yuv888), pixelconv_to_indexedcolor<bgra, yuv> },
	{ pctp (pixelformat::bgra8888_indexed8, pixelformat::hsva8888), pixelconv_to_indexedcolor<bgra, hsva> },
	{ pctp (pixelformat::bgra8888_indexed8, pixelformat::hsv888), pixelconv_to_indexedcolor<bgra, hsv> },

	{ pctp (pixelformat::bgr888_indexed8, pixelformat::rgba8888), pixelconv_to_indexedcolor<bgr, rgba> },
	{ pctp (pixelformat::bgr888_indexed8, pixelformat::rgb888), pixelconv_to_indexedcolor<bgr, rgb> },
	{ pctp (pixelformat::bgr888_indexed8, pixelformat::rgbaf), pixelconv_to_indexedcolor<bgr, rgbaf> },
	{ pctp (pixelformat::bgr888_indexed8, pixelformat::bgra8888), pixelconv_to_indexedcolor<bgr, bgra> },
	{ pctp (pixelformat::bgr888_indexed8, pixelformat::bgr888), pixelconv_to_indexedcolor<bgr, bgr> },
	{ pctp (pixelformat::bgr888_indexed8, pixelformat::bgra4444), pixelconv_to_indexedcolor<bgr, bgra4> },
	{ pctp (pixelformat::bgr888_indexed8, pixelformat::bgr565), pixelconv_to_indexedcolor<bgr, bgr565> },
	{ pctp (pixelformat::bgr888_indexed8, pixelformat::grayscale8), pixelconv_to_indexedcolor<bgr, grayscale> },
	{ pctp (pixelformat::bgr888_indexed8, pixelformat::grayscalef), pixelconv_to_indexedcolor<bgr, grayscalef> },
	{ pctp (pixelformat::bgr888_indexed8, pixelformat::yuva8888), pixelconv_to_indexedcolor<bgr, yuva> },
	{ pctp (pixelformat::bgr888_indexed8, pixelformat::yuv888), pixelconv_to_indexedcolor<bgr, yuv> },
	{ pctp (pixelformat::bgr888_indexed8, pixelformat::hsva8888), pixelconv_to_indexedcolor<bgr, hsva> },
	{ pctp (pixelformat::bgr888_indexed8, pixelformat::hsv888), pixelconv_to_indexedcolor<bgr, hsv> },

	////////////////////////////////////////////////////////////////////////////////////////
	// Subsampled YUV Color Conversions
	////////////////////////////////////////////////////////////////////////////////////////
	{ pctp (pixelformat::yuyv8888, pixelformat::rgba8888), pixelconv_to_chromasubsample_yuv422<rgba> },
	{ pctp (pixelformat::yuyv8888, pixelformat::rgb888), pixelconv_to_chromasubsample_yuv422<rgb> },
	{ pctp (pixelformat::yuyv8888, pixelformat::rgbaf), pixelconv_to_chromasubsample_yuv422<rgbaf> },
	{ pctp (pixelformat::yuyv8888, pixelformat::bgra8888), pixelconv_to_chromasubsample_yuv422<bgra> },
	{ pctp (pixelformat::yuyv8888, pixelformat::bgr888), pixelconv_to_chromasubsample_yuv422<bgr> },
	{ pctp (pixelformat::yuyv8888, pixelformat::bgra4444), pixelconv_to_chromasubsample_yuv422<bgra4> },
	{ pctp (pixelformat::yuyv8888, pixelformat::bgr565), pixelconv_to_chromasubsample_yuv422<bgr565> },
	{ pctp (pixelformat::yuyv8888, pixelformat::grayscale8), pixelconv_to_chromasubsample_yuv422<grayscale> },
	{ pctp (pixelformat::yuyv8888, pixelformat::grayscalef), pixelconv_to_chromasubsample_yuv422<grayscalef> },
	{ pctp (pixelformat::yuyv8888, pixelformat::yuva8888), pixelconv_to_chromasubsample_yuv422<yuva> },
	{ pctp (pixelformat::yuyv8888, pixelformat::yuv888), pixelconv_to_chromasubsample_yuv422<yuv> },
	{ pctp (pixelformat::yuyv8888, pixelformat::hsva8888), pixelconv_to_chromasubsample_yuv422<hsva> },
	{ pctp (pixelformat::yuyv8888, pixelformat::hsv888), pixelconv_to_chromasubsample_yuv422<hsv> },

	{ pctp (pixelformat::nv12, pixelformat::rgba8888), pixelconv_to_chromasubsample_nv12<rgba> },
	{ pctp (pixelformat::nv12, pixelformat::rgb888), pixelconv_to_chromasubsample_nv12<rgb> },
	{ pctp (pixelformat::nv12, pixelformat::rgbaf), pixelconv_to_chromasubsample_nv12<rgbaf> },
	{ pctp (pixelformat::nv12, pixelformat::bgra8888), pixelconv_to_chromasubsample_nv12<bgra> },
	{ pctp (pixelformat::nv12, pixelformat::bgr888), pixelconv_to_chromasubsample_nv12<bgr> },
	{ pctp (pixelformat::nv12, pixelformat::bgra4444), pixelconv_to_chromasubsample_nv12<bgra4> },
	{ pctp (pixelformat::nv12, pixelformat::bgr565), pixelconv_to_chromasubsample_nv12<bgr565> },
	{ pctp (pixelformat::nv12, pixelformat::grayscale8), pixelconv_to_chromasubsample_nv12<grayscale> },
	{ pctp (pixelformat::nv12, pixelformat::grayscalef), pixelconv_to_chromasubsample_nv12<grayscalef> },
	{ pctp (pixelformat::nv12, pixelformat::yuva8888), pixelconv_to_chromasubsample_nv12<yuva> },
	{ pctp (pixelformat::nv12, pixelformat::yuv888), pixelconv_to_chromasubsample_nv12<yuv> },
	{ pctp (pixelformat::nv12, pixelformat::hsva8888), pixelconv_to_chromasubsample_nv12<hsva> },
	{ pctp (pixelformat::nv12, pixelformat::hsv888), pixelconv_to_chromasubsample_nv12<hsv> },

	{ pctp (pixelformat::rgba8888, pixelformat::yuyv8888), pixelconv_from_chromasubsample_yuv422<rgba> },
	{ pctp (pixelformat::rgb888, pixelformat::yuyv8888), pixelconv_from_chromasubsample_yuv422<rgb> },
	{ pctp (pixelformat::rgbaf, pixelformat::yuyv8888), pixelconv_from_chromasubsample_yuv422<rgbaf> },
	{ pctp (pixelformat::bgra8888, pixelformat::yuyv8888), pixelconv_from_chromasubsample_yuv422<bgra> },
	{ pctp (pixelformat::bgr888, pixelformat::yuyv8888), pixelconv_from_chromasubsample_yuv422<bgr> },
	{ pctp (pixelformat::bgra4444, pixelformat::yuyv8888), pixelconv_from_chromasubsample_yuv422<bgra4> },
	{ pctp (pixelformat::bgr565, pixelformat::yuyv8888), pixelconv_from_chromasubsample_yuv422<bgr565> },
	{ pctp (pixelformat::grayscale8, pixelformat::yuyv8888), pixelconv_from_chromasubsample_yuv422<grayscale> },
	{ pctp (pixelformat::grayscalef, pixelformat::yuyv8888), pixelconv_from_chromasubsample_yuv422<grayscalef> },
	{ pctp (pixelformat::yuva8888, pixelformat::yuyv8888), pixelconv_from_chromasubsample_yuv422<yuva> },
	{ pctp (pixelformat::yuv888, pixelformat::yuyv8888), pixelconv_from_chromasubsample_yuv422<yuv> },
	{ pctp (pixelformat::hsva8888, pixelformat::yuyv8888), pixelconv_from_chromasubsample_yuv422<hsva> },
	{ pctp (pixelformat::hsv888, pixelformat::yuyv8888), pixelconv_from_chromasubsample_yuv422<hsv> },

	{ pctp (pixelformat::rgba8888, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<rgba> },
	{ pctp (pixelformat::rgb888, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<rgb> },
	{ pctp (pixelformat::rgbaf, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<rgbaf> },
	{ pctp (pixelformat::bgra8888, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<bgra> },
	{ pctp (pixelformat::bgr888, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<bgr> },
	{ pctp (pixelformat::bgra4444, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<bgra4> },
	{ pctp (pixelformat::bgr565, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<bgr565> },
	{ pctp (pixelformat::grayscale8, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<grayscale> },
	{ pctp (pixelformat::grayscalef, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<grayscalef> },
	{ pctp (pixelformat::yuva8888, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<yuva> },
	{ pctp (pixelformat::yuv888, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<yuv> },
	{ pctp (pixelformat::hsva8888, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<hsva> },
	{ pctp (pixelformat::hsv888, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<hsv> },

	////////////////////////////////////////////////////////////////////////////////////////
	// DXT Color Conversions
	////////////////////////////////////////////////////////////////////////////////////////
	{ pctp (pixelformat::rgba8888, pixelformat::bc1), pixelconv_to_dxt<BC1> },
	{ pctp (pixelformat::rgba8888, pixelformat::bc2), pixelconv_to_dxt<BC2> },
	{ pctp (pixelformat::rgba8888, pixelformat::bc3), pixelconv_to_dxt<BC3> },
	{ pctp (pixelformat::rgba8888, pixelformat::bc4), pixelconv_to_dxt<BC4> },
	{ pctp (pixelformat::rgba8888, pixelformat::bc5), pixelconv_to_dxt<BC5> },

	{ pctp (pixelformat::bc1, pixelformat::rgba8888), pixelconv_from_dxt<BC1> },
	{ pctp (pixelformat::bc2, pixelformat::rgba8888), pixelconv_from_dxt<BC2> },
	{ pctp (pixelformat::bc3, pixelformat::rgba8888), pixelconv_from_dxt<BC3> },
	{ pctp (pixelformat::bc4, pixelformat::rgba8888), pixelconv_from_dxt<BC4> },
	{ pctp (pixelformat::bc5, pixelformat::rgba8888), pixelconv_from_dxt<BC5> },

	////////////////////////////////////////////////////////////////////////////////////////
	// ETC1 Color Conversions
	////////////////////////////////////////////////////////////////////////////////////////
	{ pctp (pixelformat::rgba8888, pixelformat::etc1), pixelconv<pixelformat::rgba8888, pixelformat::etc1> },
	{ pctp (pixelformat::rgb888, pixelformat::etc1), pixelconv<pixelformat::rgb888, pixelformat::etc1> },

	{ pctp (pixelformat::etc1, pixelformat::rgba8888), pixelconv<pixelformat::etc1, pixelformat::rgba8888> },
	{ pctp (pixelformat::etc1, pixelformat::rgb888), pixelconv<pixelformat::etc1, pixelformat::rgb888> },

};

dseed::error_t dseed::reformat_bitmap (dseed::bitmap* original, dseed::pixelformat reformat, dseed::bitmap** bitmap)
{
	if (original == nullptr || bitmap == nullptr)
		return dseed::error_invalid_args;

	pixelformat originalFormat = original->format ();
	if (originalFormat == reformat)
	{
		*bitmap = original;
		original->retain ();
		return dseed::error_good;
	}

	bool isIndexedTarget = false;
	int bpp = 0;
	dseed::auto_object<dseed::palette> tempPalette;
	if (reformat == pixelformat::bgra8888_indexed8 || reformat == pixelformat::bgr888_indexed8)
	{
		isIndexedTarget = true;
		bpp = ((int)reformat & 0xff) * 8;

		if (dseed::failed (dseed::create_palette (nullptr, bpp, 256, &tempPalette)))
			return dseed::error_fail;
	}

	dseed::size3i size = original->size ();

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

	auto found = g_pixelconvs.find (pctp (reformat, originalFormat));
	if (found == g_pixelconvs.end ())
		return dseed::error_not_support;

	int paletteCount = found->second (destPtr, srcPtr, size, destPalettePtr, srcPalettePtr);
	if (paletteCount == -1) return dseed::error_not_support;
	if (isIndexedTarget) tempPalette->resize (paletteCount);

	*bitmap = temp.detach ();

	return dseed::error_good;
}