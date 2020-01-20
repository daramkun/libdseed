#include <dseed.h>

#include <map>
#include <tuple>

#include "../libs/exoquant/exoquant.h"
#include "../libs/exoquant/exoquant.c"

//#if defined ( USE_SQUISH )
//#	include <squish.h>
//#endif

#include "../libs/etc1_utils/etc1_utils.h"
#include "../libs/etc1_utils/etc1_utils.c"

using namespace dseed::color;
using size2i = dseed::size2i;

#define PIXELCONV_ARGS										uint8_t* dest, const uint8_t* src, const dseed::size3i& size, uint8_t* destPalette, uint8_t* srcPalette
using pcfn = std::function<int (PIXELCONV_ARGS)>;
using pctp = std::tuple<dseed::color::pixelformat, dseed::color::pixelformat>;
template<dseed::color::pixelformat destformat, dseed::color::pixelformat srcformat>
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
	size_t destStride = calc_bitmap_stride (type2format<TDest> (), size.width)
		, srcStride = calc_bitmap_stride (type2format<TSrc> (), size.width);
	size_t destDepth = calc_bitmap_plane_size (type2format<TDest> (), size2i (size.width, size.height))
		, srcDepth = calc_bitmap_plane_size (type2format<TSrc> (), size2i (size.width, size.height));

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
	size_t destStride = calc_bitmap_stride (type2format<TDest> (), size.width)
		, srcStride = calc_bitmap_stride (type2indexedformat<TSrc> (), size.width);
	size_t destDepth = calc_bitmap_plane_size (type2format<TDest> (), size2i (size.width, size.height))
		, srcDepth = calc_bitmap_plane_size (type2indexedformat<TSrc> (), size2i (size.width, size.height));

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
	conved.resize (calc_bitmap_plane_size (dseed::color::pixelformat::bgra8, dseed::size2i (size.width, size.height)) * size.depth);
	pixelconv_plaincolor<bgra8, TSrc> (conved.data (), src, size, nullptr, nullptr);

	if (typeid(TDest) == typeid(bgra8))
		return pixelconv_to_indexedcolor<bgra8, bgra8> (dest, conved.data (), size, destPalette, nullptr);
	else if (typeid(TDest) == typeid(bgr8))
		return pixelconv_to_indexedcolor<bgr8, bgra8> (dest, conved.data (), size, destPalette, nullptr);
	return 0;
}

template<>
inline int pixelconv_to_indexedcolor<bgra8, bgra8> (PIXELCONV_ARGS) noexcept
{
	exq_data* pExq;
	pExq = exq_init ();
	exq_feed (pExq, (uint8_t*)src, size.width * size.height * size.depth);
	exq_quantize_hq (pExq, 256);
	std::vector<bgra8> palette (256);
	exq_get_palette (pExq, (uint8_t*)palette.data (), 256);
	memcpy (destPalette, palette.data (), sizeof (bgra8) * 256);

	exq_map_image_dither (pExq, size.width, size.height * size.depth, (uint8_t*)src, dest, 0);

	exq_free (pExq);

	return 256;
}
template<>
inline int pixelconv_to_indexedcolor<bgr8, bgra8> (PIXELCONV_ARGS) noexcept
{
	std::vector<bgra8> palette (256);
	size_t count = pixelconv_to_indexedcolor<bgra8, bgra8> (dest, src, size, (uint8_t*)palette.data (), nullptr);

	bgr8* destPaletteBGR = (bgr8*)destPalette;
	for (int i = 0; i < count; ++i)
		destPaletteBGR[i] = (bgr8)palette[i];

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
	size_t destStride = calc_bitmap_stride (pixelformat::yuyv8, size.width)
		, srcStride = calc_bitmap_stride (type2format<TSrc> (), size.width);
	size_t destDepth = calc_bitmap_plane_size (pixelformat::yuyv8, size2i (size.width, size.height))
		, srcDepth = calc_bitmap_plane_size (type2format<TSrc> (), size2i (size.width, size.height));

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
				yuv8 srcColor1 = *srcPtrX;
				r8 srcColor2 = { 0 };
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
	size_t destStride = calc_bitmap_stride (type2format<TDest> (), size.width)
		, srcStride = calc_bitmap_stride (pixelformat::yuyv8, size.width);
	size_t destDepth = calc_bitmap_plane_size (type2format<TDest> (), size2i (size.width, size.height))
		, srcDepth = calc_bitmap_plane_size (pixelformat::yuyv8, size2i (size.width, size.height));

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
				*destPtrX = yuv8 (srcPtrX->y1, srcPtrX->u, srcPtrX->v);
				if (x + 1 < size.width)
					*(destPtrX + 1) = yuv8 (srcPtrX->y2, srcPtrX->u, srcPtrX->v);
			}
		}
	}

	return 0;
}

template<class TSrc>
inline int pixelconv_to_chromasubsample_nv12 (PIXELCONV_ARGS) noexcept
{
	size_t destYStride = size.width
		, destUVStride = (size_t)ceil (size.width / 2.0) * 2
		, srcStride = calc_bitmap_stride (type2format<TSrc> (), size.width);
	size_t destDepth = calc_bitmap_plane_size (pixelformat::nv12, size2i (size.width, size.height))
		, srcDepth = calc_bitmap_plane_size (type2format<TSrc> (), size2i (size.width, size.height));
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
			r8* destYPtr = (r8*)(dest + destDepthZ + destYStrideY);
			uv* destUVPtr = (uv*)(dest + destDepthZ + ySize + destUVStrideY);
			const TSrc* srcPtr = (TSrc*)(src + srcDepthZ + srcStrideY);

			for (size_t x = 0; x < size.width; ++x)
			{
				r8* destYPtrX = destYPtr + x;
				const TSrc* srcPtrX = srcPtr + x;
				*destYPtrX = *srcPtrX;

				if ((x + 1) % 2 == 1 && (y + 1) % 2 == 1)
				{
					uv* destUVPtrX = destUVPtr + (x / 2);
					yuv8 yuv = *srcPtrX;
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
	size_t destStride = calc_bitmap_stride (type2format<TDest> (), size.width)
		, srcYStride = size.width
		, srcUVStride = (size_t)ceil (size.width / 2.0) * 2;
	size_t destDepth = calc_bitmap_plane_size (type2format<TDest> (), size2i (size.width, size.height))
		, srcDepth = calc_bitmap_plane_size (pixelformat::nv12, size2i (size.width, size.height));
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
			const r8* srcYPtr = (r8*)(src + srcDepthZ + srcYStrideY);
			const uv* srcUVPtr = (uv*)(src + srcDepthZ + ySize + srcUVStrideY);

			for (size_t x = 0; x < size.width; ++x)
			{
				TDest* destPtrX = destPtr + x;
				const r8* srcYPtrX = srcYPtr + x;
				const uv* srcUVPtrX = srcUVPtr + (x / 2);
				*destPtrX = yuv8 (srcYPtrX->color, srcUVPtrX->u, srcUVPtrX->v);
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
/*enum BCENUM { BC1 = (1 << 0), BC2 = (1 << 1), BC3 = (1 << 2), BC4 = (1 << 3), BC5 = (1 << 4) };
constexpr pixelformat BC_TO_PF (BCENUM compression) noexcept {
	switch (compression) {
		case BC1: return pixelformat::bc1; case BC2: return pixelformat::bc2;
		case BC3: return pixelformat::bc3; case BC4: return pixelformat::bc4; case BC5: return pixelformat::bc5; default: return pixelformat::unknown;
	}
}

template<BCENUM compression>
inline int pixelconv_to_dxt (PIXELCONV_ARGS) noexcept
{
#if defined ( USE_SQUISH )
	size_t destDepth = calc_bitmap_plane_size (pixelformat::rgba8, size2i (size.width, size.height)),
		srcDepth = calc_bitmap_plane_size (BC_TO_PF (compression), size2i (size.width, size.height));
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
	size_t destDepth = calc_bitmap_plane_size (BC_TO_PF (compression), size2i (size.width, size.height)),
		srcDepth = calc_bitmap_plane_size (pixelformat::rgba8, size2i (size.width, size.height));
	for (int z = 0; z < size.depth; ++z)
		squish::CompressImage (src + (srcDepth * z), size.width, size.height, dest + (destDepth * z)
			, compression | squish::kColourIterativeClusterFit);
	return 0;
#else
	return -1;
#endif
}*/

////////////////////////////////////////////////////////////////////////////////////////////
//
// ETC1 <-> RGB/RGBA series Conversions
//
////////////////////////////////////////////////////////////////////////////////////////////

template<> inline int pixelconv<pixelformat::rgb8, pixelformat::etc1> (PIXELCONV_ARGS)
{
	size_t destArr = calc_bitmap_plane_size (pixelformat::rgb8, size2i (size.width, size.height)),
		srcArr = calc_bitmap_plane_size (pixelformat::etc1, size2i (size.width, size.height));
	size_t destStride = calc_bitmap_stride (pixelformat::rgb8, size.width);
	for (int z = 0; z < size.depth; ++z)
		etc1_decode_image (src + (srcArr * z), dest + (destArr * z), size.width, size.height, 3, (uint32_t)destStride);
	return 0;
}
template<> inline int pixelconv<pixelformat::rgba8, pixelformat::etc1> (PIXELCONV_ARGS)
{
	size_t destArr = calc_bitmap_plane_size (pixelformat::rgba8, size2i (size.width, size.height)),
		tempArr = calc_bitmap_plane_size (pixelformat::rgb8, size2i (size.width, size.height));
	size_t tempStride = calc_bitmap_stride (pixelformat::rgb8, size.width)
		, destStride = calc_bitmap_stride (pixelformat::rgba8, size.width);
	std::vector<uint8_t> temp (tempStride * size.height * size.depth);
	pixelconv<pixelformat::rgb8, pixelformat::etc1> (temp.data (), src, size, destPalette, srcPalette);
	for (int z = 0; z < size.depth; ++z)
	{
		for (int y = 0; y < size.height; ++y)
		{
			for (int x = 0; x < size.width; ++x)
			{
				rgba8& destPixel = *((rgba8*)(dest + (z * destArr) + (y * destStride) + (x * 4)));
				rgb8& tempPixel = *((rgb8*)(temp.data () + (z * tempArr) + (y * tempStride) + (x * 3)));

				destPixel.r = tempPixel.r;
				destPixel.g = tempPixel.g;
				destPixel.b = tempPixel.b;
				destPixel.a = 255;
			}
		}
	}
	return 0;
}
template<> inline int pixelconv<pixelformat::etc1, pixelformat::rgb8> (PIXELCONV_ARGS)
{
	size_t destArr = calc_bitmap_plane_size (pixelformat::etc1, size2i (size.width, size.height)),
		srcArr = calc_bitmap_plane_size (pixelformat::rgb8, size2i (size.width, size.height));
	size_t srcStride = calc_bitmap_stride (pixelformat::rgb8, size.width);
	for (int z = 0; z < size.depth; ++z)
		etc1_encode_image (src + (z * srcArr), size.width, size.height, 3, (etc1_uint32)srcStride, dest + (z * destArr));
	return 0;
}
template<> inline int pixelconv<pixelformat::etc1, pixelformat::rgba8> (PIXELCONV_ARGS)
{
	size_t destArr = calc_bitmap_plane_size (pixelformat::etc1, size2i (size.width, size.height)),
		tempArr = calc_bitmap_plane_size (pixelformat::rgb8, size2i (size.width, size.height));
	size_t tempStride = calc_bitmap_stride (pixelformat::rgb8, size.width)
		, srcStride = calc_bitmap_stride (pixelformat::rgba8, size.width);
	std::vector<uint8_t> temp (tempStride * size.height * size.depth);
	for (int z = 0; z < size.depth; ++z)
	{
		for (int y = 0; y < size.height; ++y)
		{
			for (int x = 0; x < size.width; ++x)
			{
				const rgba8& srcPixel = *((const rgba8*)(dest + (y * srcStride) + (x * 4)));
				rgb8& tempPixel = *((rgb8*)(temp.data () + (y * tempStride) + (x * 3)));

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
	{ pctp (pixelformat::rgba8, pixelformat::rgb8), pixelconv_plaincolor<rgba8, rgb8> },
	{ pctp (pixelformat::rgba8, pixelformat::rgbaf), pixelconv_plaincolor<rgba8, rgbaf> },
	{ pctp (pixelformat::rgba8, pixelformat::bgra8), pixelconv_plaincolor<rgba8, bgra8> },
	{ pctp (pixelformat::rgba8, pixelformat::bgr8), pixelconv_plaincolor<rgba8, bgr8> },
	{ pctp (pixelformat::rgba8, pixelformat::bgra4), pixelconv_plaincolor<rgba8, bgra4> },
	{ pctp (pixelformat::rgba8, pixelformat::bgr565), pixelconv_plaincolor<rgba8, bgr565> },
	{ pctp (pixelformat::rgba8, pixelformat::r8), pixelconv_plaincolor<rgba8, r8> },
	{ pctp (pixelformat::rgba8, pixelformat::rf), pixelconv_plaincolor<rgba8, rf> },
	{ pctp (pixelformat::rgba8, pixelformat::yuva8), pixelconv_plaincolor<rgba8, yuva8> },
	{ pctp (pixelformat::rgba8, pixelformat::yuv8), pixelconv_plaincolor<rgba8, yuv8> },
	{ pctp (pixelformat::rgba8, pixelformat::hsva8), pixelconv_plaincolor<rgba8, hsva8> },
	{ pctp (pixelformat::rgba8, pixelformat::hsv8), pixelconv_plaincolor<rgba8, hsv8> },

	{ pctp (pixelformat::rgb8, pixelformat::rgba8), pixelconv_plaincolor<rgb8, rgba8> },
	{ pctp (pixelformat::rgb8, pixelformat::rgbaf), pixelconv_plaincolor<rgb8, rgbaf> },
	{ pctp (pixelformat::rgb8, pixelformat::bgra8), pixelconv_plaincolor<rgb8, bgra8> },
	{ pctp (pixelformat::rgb8, pixelformat::bgr8), pixelconv_plaincolor<rgb8, bgr8> },
	{ pctp (pixelformat::rgb8, pixelformat::bgra4), pixelconv_plaincolor<rgb8, bgra4> },
	{ pctp (pixelformat::rgb8, pixelformat::bgr565), pixelconv_plaincolor<rgb8, bgr565> },
	{ pctp (pixelformat::rgb8, pixelformat::r8), pixelconv_plaincolor<rgb8, r8> },
	{ pctp (pixelformat::rgb8, pixelformat::rf), pixelconv_plaincolor<rgb8, rf> },
	{ pctp (pixelformat::rgb8, pixelformat::yuva8), pixelconv_plaincolor<rgb8, yuva8> },
	{ pctp (pixelformat::rgb8, pixelformat::yuv8), pixelconv_plaincolor<rgb8, yuv8> },
	{ pctp (pixelformat::rgb8, pixelformat::hsva8), pixelconv_plaincolor<rgb8, hsva8> },
	{ pctp (pixelformat::rgb8, pixelformat::hsv8), pixelconv_plaincolor<rgb8, hsv8> },

	{ pctp (pixelformat::rgbaf, pixelformat::rgba8), pixelconv_plaincolor<rgbaf, rgba8> },
	{ pctp (pixelformat::rgbaf, pixelformat::rgb8), pixelconv_plaincolor<rgbaf, rgb8> },
	{ pctp (pixelformat::rgbaf, pixelformat::bgra8), pixelconv_plaincolor<rgbaf, bgra8> },
	{ pctp (pixelformat::rgbaf, pixelformat::bgr8), pixelconv_plaincolor<rgbaf, bgr8> },
	{ pctp (pixelformat::rgbaf, pixelformat::bgra4), pixelconv_plaincolor<rgbaf, bgra4> },
	{ pctp (pixelformat::rgbaf, pixelformat::bgr565), pixelconv_plaincolor<rgbaf, bgr565> },
	{ pctp (pixelformat::rgbaf, pixelformat::r8), pixelconv_plaincolor<rgbaf, r8> },
	{ pctp (pixelformat::rgbaf, pixelformat::rf), pixelconv_plaincolor<rgbaf, rf> },
	{ pctp (pixelformat::rgbaf, pixelformat::yuva8), pixelconv_plaincolor<rgbaf, yuva8> },
	{ pctp (pixelformat::rgbaf, pixelformat::yuv8), pixelconv_plaincolor<rgbaf, yuv8> },
	{ pctp (pixelformat::rgbaf, pixelformat::hsva8), pixelconv_plaincolor<rgbaf, hsva8> },
	{ pctp (pixelformat::rgbaf, pixelformat::hsv8), pixelconv_plaincolor<rgbaf, hsv8> },

	{ pctp (pixelformat::bgra8, pixelformat::rgba8), pixelconv_plaincolor<bgra8, rgba8> },
	{ pctp (pixelformat::bgra8, pixelformat::rgb8), pixelconv_plaincolor<bgra8, rgb8> },
	{ pctp (pixelformat::bgra8, pixelformat::rgbaf), pixelconv_plaincolor<bgra8, rgbaf> },
	{ pctp (pixelformat::bgra8, pixelformat::bgr8), pixelconv_plaincolor<bgra8, bgr8> },
	{ pctp (pixelformat::bgra8, pixelformat::bgra4), pixelconv_plaincolor<bgra8, bgra4> },
	{ pctp (pixelformat::bgra8, pixelformat::bgr565), pixelconv_plaincolor<bgra8, bgr565> },
	{ pctp (pixelformat::bgra8, pixelformat::r8), pixelconv_plaincolor<bgra8, r8> },
	{ pctp (pixelformat::bgra8, pixelformat::rf), pixelconv_plaincolor<bgra8, rf> },
	{ pctp (pixelformat::bgra8, pixelformat::yuva8), pixelconv_plaincolor<bgra8, yuva8> },
	{ pctp (pixelformat::bgra8, pixelformat::yuv8), pixelconv_plaincolor<bgra8, yuv8> },
	{ pctp (pixelformat::bgra8, pixelformat::hsva8), pixelconv_plaincolor<bgra8, hsva8> },
	{ pctp (pixelformat::bgra8, pixelformat::hsv8), pixelconv_plaincolor<bgra8, hsv8> },

	{ pctp (pixelformat::bgr8, pixelformat::rgba8), pixelconv_plaincolor<bgr8, rgba8> },
	{ pctp (pixelformat::bgr8, pixelformat::rgb8), pixelconv_plaincolor<bgr8, rgb8> },
	{ pctp (pixelformat::bgr8, pixelformat::rgbaf), pixelconv_plaincolor<bgr8, rgbaf> },
	{ pctp (pixelformat::bgr8, pixelformat::bgra8), pixelconv_plaincolor<bgr8, bgra8> },
	{ pctp (pixelformat::bgr8, pixelformat::bgra4), pixelconv_plaincolor<bgr8, bgra4> },
	{ pctp (pixelformat::bgr8, pixelformat::bgr565), pixelconv_plaincolor<bgr8, bgr565> },
	{ pctp (pixelformat::bgr8, pixelformat::r8), pixelconv_plaincolor<bgr8, r8> },
	{ pctp (pixelformat::bgr8, pixelformat::rf), pixelconv_plaincolor<bgr8, rf> },
	{ pctp (pixelformat::bgr8, pixelformat::yuva8), pixelconv_plaincolor<bgr8, yuva8> },
	{ pctp (pixelformat::bgr8, pixelformat::yuv8), pixelconv_plaincolor<bgr8, yuv8> },
	{ pctp (pixelformat::bgr8, pixelformat::hsva8), pixelconv_plaincolor<bgr8, hsva8> },
	{ pctp (pixelformat::bgr8, pixelformat::hsv8), pixelconv_plaincolor<bgr8, hsv8> },

	{ pctp (pixelformat::bgra4, pixelformat::rgba8), pixelconv_plaincolor<bgra4, rgba8> },
	{ pctp (pixelformat::bgra4, pixelformat::rgb8), pixelconv_plaincolor<bgra4, rgb8> },
	{ pctp (pixelformat::bgra4, pixelformat::rgbaf), pixelconv_plaincolor<bgra4, rgbaf> },
	{ pctp (pixelformat::bgra4, pixelformat::bgra8), pixelconv_plaincolor<bgra4, bgra8> },
	{ pctp (pixelformat::bgra4, pixelformat::bgr8), pixelconv_plaincolor<bgra4, bgr8> },
	{ pctp (pixelformat::bgra4, pixelformat::bgr565), pixelconv_plaincolor<bgra4, bgr565> },
	{ pctp (pixelformat::bgra4, pixelformat::r8), pixelconv_plaincolor<bgra4, r8> },
	{ pctp (pixelformat::bgra4, pixelformat::rf), pixelconv_plaincolor<bgra4, rf> },
	{ pctp (pixelformat::bgra4, pixelformat::yuva8), pixelconv_plaincolor<bgra4, yuva8> },
	{ pctp (pixelformat::bgra4, pixelformat::yuv8), pixelconv_plaincolor<bgra4, yuv8> },
	{ pctp (pixelformat::bgra4, pixelformat::hsva8), pixelconv_plaincolor<bgra4, hsva8> },
	{ pctp (pixelformat::bgra4, pixelformat::hsv8), pixelconv_plaincolor<bgra4, hsv8> },

	{ pctp (pixelformat::bgr565, pixelformat::rgba8), pixelconv_plaincolor<bgr565, rgba8> },
	{ pctp (pixelformat::bgr565, pixelformat::rgb8), pixelconv_plaincolor<bgr565, rgb8> },
	{ pctp (pixelformat::bgr565, pixelformat::rgbaf), pixelconv_plaincolor<bgr565, rgbaf> },
	{ pctp (pixelformat::bgr565, pixelformat::bgra8), pixelconv_plaincolor<bgr565, bgra8> },
	{ pctp (pixelformat::bgr565, pixelformat::bgr8), pixelconv_plaincolor<bgr565, bgr8> },
	{ pctp (pixelformat::bgr565, pixelformat::bgra4), pixelconv_plaincolor<bgr565, bgra4> },
	{ pctp (pixelformat::bgr565, pixelformat::r8), pixelconv_plaincolor<bgr565, r8> },
	{ pctp (pixelformat::bgr565, pixelformat::rf), pixelconv_plaincolor<bgr565, rf> },
	{ pctp (pixelformat::bgr565, pixelformat::yuva8), pixelconv_plaincolor<bgr565, yuva8> },
	{ pctp (pixelformat::bgr565, pixelformat::yuv8), pixelconv_plaincolor<bgr565, yuv8> },
	{ pctp (pixelformat::bgr565, pixelformat::hsva8), pixelconv_plaincolor<bgr565, hsva8> },
	{ pctp (pixelformat::bgr565, pixelformat::hsv8), pixelconv_plaincolor<bgr565, hsv8> },

	{ pctp (pixelformat::r8, pixelformat::rgba8), pixelconv_plaincolor<r8, rgba8> },
	{ pctp (pixelformat::r8, pixelformat::rgb8), pixelconv_plaincolor<r8, rgb8> },
	{ pctp (pixelformat::r8, pixelformat::rgbaf), pixelconv_plaincolor<r8, rgbaf> },
	{ pctp (pixelformat::r8, pixelformat::bgra8), pixelconv_plaincolor<r8, bgra8> },
	{ pctp (pixelformat::r8, pixelformat::bgr8), pixelconv_plaincolor<r8, bgr8> },
	{ pctp (pixelformat::r8, pixelformat::bgra4), pixelconv_plaincolor<r8, bgra4> },
	{ pctp (pixelformat::r8, pixelformat::bgr565), pixelconv_plaincolor<r8, bgr565> },
	{ pctp (pixelformat::r8, pixelformat::rf), pixelconv_plaincolor<r8, rf> },
	{ pctp (pixelformat::r8, pixelformat::yuva8), pixelconv_plaincolor<r8, yuva8> },
	{ pctp (pixelformat::r8, pixelformat::yuv8), pixelconv_plaincolor<r8, yuv8> },
	{ pctp (pixelformat::r8, pixelformat::hsva8), pixelconv_plaincolor<r8, hsva8> },
	{ pctp (pixelformat::r8, pixelformat::hsv8), pixelconv_plaincolor<r8, hsv8> },

	{ pctp (pixelformat::rf, pixelformat::rgba8), pixelconv_plaincolor<rf, rgba8> },
	{ pctp (pixelformat::rf, pixelformat::rgb8), pixelconv_plaincolor<rf, rgb8> },
	{ pctp (pixelformat::rf, pixelformat::rgbaf), pixelconv_plaincolor<rf, rgbaf> },
	{ pctp (pixelformat::rf, pixelformat::bgra8), pixelconv_plaincolor<rf, bgra8> },
	{ pctp (pixelformat::rf, pixelformat::bgr8), pixelconv_plaincolor<rf, bgr8> },
	{ pctp (pixelformat::rf, pixelformat::bgra4), pixelconv_plaincolor<rf, bgra4> },
	{ pctp (pixelformat::rf, pixelformat::bgr565), pixelconv_plaincolor<rf, bgr565> },
	{ pctp (pixelformat::rf, pixelformat::r8), pixelconv_plaincolor<rf, r8> },
	{ pctp (pixelformat::rf, pixelformat::yuva8), pixelconv_plaincolor<rf, yuva8> },
	{ pctp (pixelformat::rf, pixelformat::yuv8), pixelconv_plaincolor<rf, yuv8> },
	{ pctp (pixelformat::rf, pixelformat::hsva8), pixelconv_plaincolor<rf, hsva8> },
	{ pctp (pixelformat::rf, pixelformat::hsv8), pixelconv_plaincolor<rf, hsv8> },

	{ pctp (pixelformat::yuva8, pixelformat::rgba8), pixelconv_plaincolor<yuva8, rgba8> },
	{ pctp (pixelformat::yuva8, pixelformat::rgb8), pixelconv_plaincolor<yuva8, rgb8> },
	{ pctp (pixelformat::yuva8, pixelformat::rgbaf), pixelconv_plaincolor<yuva8, rgbaf> },
	{ pctp (pixelformat::yuva8, pixelformat::bgra8), pixelconv_plaincolor<yuva8, bgra8> },
	{ pctp (pixelformat::yuva8, pixelformat::bgr8), pixelconv_plaincolor<yuva8, bgr8> },
	{ pctp (pixelformat::yuva8, pixelformat::bgra4), pixelconv_plaincolor<yuva8, bgra4> },
	{ pctp (pixelformat::yuva8, pixelformat::bgr565), pixelconv_plaincolor<yuva8, bgr565> },
	{ pctp (pixelformat::yuva8, pixelformat::r8), pixelconv_plaincolor<yuva8, r8> },
	{ pctp (pixelformat::yuva8, pixelformat::rf), pixelconv_plaincolor<yuva8, rf> },
	{ pctp (pixelformat::yuva8, pixelformat::yuv8), pixelconv_plaincolor<yuva8, yuv8> },
	{ pctp (pixelformat::yuva8, pixelformat::hsva8), pixelconv_plaincolor<yuva8, hsva8> },
	{ pctp (pixelformat::yuva8, pixelformat::hsv8), pixelconv_plaincolor<yuva8, hsv8> },
	
	{ pctp (pixelformat::yuv8, pixelformat::rgba8), pixelconv_plaincolor<yuv8, rgba8> },
	{ pctp (pixelformat::yuv8, pixelformat::rgb8), pixelconv_plaincolor<yuv8, rgb8> },
	{ pctp (pixelformat::yuv8, pixelformat::rgbaf), pixelconv_plaincolor<yuv8, rgbaf> },
	{ pctp (pixelformat::yuv8, pixelformat::bgra8), pixelconv_plaincolor<yuv8, bgra8> },
	{ pctp (pixelformat::yuv8, pixelformat::bgr8), pixelconv_plaincolor<yuv8, bgr8> },
	{ pctp (pixelformat::yuv8, pixelformat::bgra4), pixelconv_plaincolor<yuv8, bgra4> },
	{ pctp (pixelformat::yuv8, pixelformat::bgr565), pixelconv_plaincolor<yuv8, bgr565> },
	{ pctp (pixelformat::yuv8, pixelformat::r8), pixelconv_plaincolor<yuv8, r8> },
	{ pctp (pixelformat::yuv8, pixelformat::rf), pixelconv_plaincolor<yuv8, rf> },
	{ pctp (pixelformat::yuv8, pixelformat::yuva8), pixelconv_plaincolor<yuv8, yuva8> },
	{ pctp (pixelformat::yuv8, pixelformat::hsva8), pixelconv_plaincolor<yuv8, hsva8> },
	{ pctp (pixelformat::yuv8, pixelformat::hsv8), pixelconv_plaincolor<yuv8, hsv8> },

	{ pctp (pixelformat::hsva8, pixelformat::rgba8), pixelconv_plaincolor<hsva8, rgba8> },
	{ pctp (pixelformat::hsva8, pixelformat::rgb8), pixelconv_plaincolor<hsva8, rgb8> },
	{ pctp (pixelformat::hsva8, pixelformat::rgbaf), pixelconv_plaincolor<hsva8, rgbaf> },
	{ pctp (pixelformat::hsva8, pixelformat::bgra8), pixelconv_plaincolor<hsva8, bgra8> },
	{ pctp (pixelformat::hsva8, pixelformat::bgr8), pixelconv_plaincolor<hsva8, bgr8> },
	{ pctp (pixelformat::hsva8, pixelformat::bgra4), pixelconv_plaincolor<hsva8, bgra4> },
	{ pctp (pixelformat::hsva8, pixelformat::bgr565), pixelconv_plaincolor<hsva8, bgr565> },
	{ pctp (pixelformat::hsva8, pixelformat::r8), pixelconv_plaincolor<hsva8, r8> },
	{ pctp (pixelformat::hsva8, pixelformat::rf), pixelconv_plaincolor<hsva8, rf> },
	{ pctp (pixelformat::hsva8, pixelformat::yuva8), pixelconv_plaincolor<hsva8, yuva8> },
	{ pctp (pixelformat::hsva8, pixelformat::yuv8), pixelconv_plaincolor<hsva8, yuv8> },
	{ pctp (pixelformat::hsva8, pixelformat::hsv8), pixelconv_plaincolor<hsva8, hsv8> },

	{ pctp (pixelformat::hsv8, pixelformat::rgba8), pixelconv_plaincolor<hsv8, rgba8> },
	{ pctp (pixelformat::hsv8, pixelformat::rgb8), pixelconv_plaincolor<hsv8, rgb8> },
	{ pctp (pixelformat::hsv8, pixelformat::rgbaf), pixelconv_plaincolor<hsv8, rgbaf> },
	{ pctp (pixelformat::hsv8, pixelformat::bgra8), pixelconv_plaincolor<hsv8, bgra8> },
	{ pctp (pixelformat::hsv8, pixelformat::bgr8), pixelconv_plaincolor<hsv8, bgr8> },
	{ pctp (pixelformat::hsv8, pixelformat::bgra4), pixelconv_plaincolor<hsv8, bgra4> },
	{ pctp (pixelformat::hsv8, pixelformat::bgr565), pixelconv_plaincolor<hsv8, bgr565> },
	{ pctp (pixelformat::hsv8, pixelformat::r8), pixelconv_plaincolor<hsv8, r8> },
	{ pctp (pixelformat::hsv8, pixelformat::rf), pixelconv_plaincolor<hsv8, rf> },
	{ pctp (pixelformat::hsv8, pixelformat::yuva8), pixelconv_plaincolor<hsv8, yuva8> },
	{ pctp (pixelformat::hsv8, pixelformat::yuv8), pixelconv_plaincolor<hsv8, yuv8> },
	{ pctp (pixelformat::hsv8, pixelformat::hsva8), pixelconv_plaincolor<hsv8, hsva8> },

	////////////////////////////////////////////////////////////////////////////////////////
	// Indexed Color Conversions
	////////////////////////////////////////////////////////////////////////////////////////
	{ pctp (pixelformat::rgba8, pixelformat::bgra8_indexed8), pixelconv_from_indexedcolor<rgba8, bgra8> },
	{ pctp (pixelformat::rgb8, pixelformat::bgra8_indexed8), pixelconv_from_indexedcolor<rgb8, bgra8> },
	{ pctp (pixelformat::rgbaf, pixelformat::bgra8_indexed8), pixelconv_from_indexedcolor<rgbaf, bgra8> },
	{ pctp (pixelformat::bgra8, pixelformat::bgra8_indexed8), pixelconv_from_indexedcolor<bgra8, bgra8> },
	{ pctp (pixelformat::bgr8, pixelformat::bgra8_indexed8), pixelconv_from_indexedcolor<bgr8, bgra8> },
	{ pctp (pixelformat::bgra4, pixelformat::bgra8_indexed8), pixelconv_from_indexedcolor<bgra4, bgra8> },
	{ pctp (pixelformat::bgr565, pixelformat::bgra8_indexed8), pixelconv_from_indexedcolor<bgr565, bgra8> },
	{ pctp (pixelformat::r8, pixelformat::bgra8_indexed8), pixelconv_from_indexedcolor<r8, bgra8> },
	{ pctp (pixelformat::rf, pixelformat::bgra8_indexed8), pixelconv_from_indexedcolor<rf, bgra8> },
	{ pctp (pixelformat::yuva8, pixelformat::bgra8_indexed8), pixelconv_from_indexedcolor<yuva8, bgra8> },
	{ pctp (pixelformat::yuv8, pixelformat::bgra8_indexed8), pixelconv_from_indexedcolor<yuv8, bgra8> },
	{ pctp (pixelformat::hsva8, pixelformat::bgra8_indexed8), pixelconv_from_indexedcolor<hsva8, bgra8> },
	{ pctp (pixelformat::hsv8, pixelformat::bgra8_indexed8), pixelconv_from_indexedcolor<hsv8, bgra8> },

	{ pctp (pixelformat::rgba8, pixelformat::bgr8_indexed8), pixelconv_from_indexedcolor<rgba8, bgr8> },
	{ pctp (pixelformat::rgb8, pixelformat::bgr8_indexed8), pixelconv_from_indexedcolor<rgb8, bgr8> },
	{ pctp (pixelformat::rgbaf, pixelformat::bgr8_indexed8), pixelconv_from_indexedcolor<rgbaf, bgr8> },
	{ pctp (pixelformat::bgra8, pixelformat::bgr8_indexed8), pixelconv_from_indexedcolor<bgra8, bgr8> },
	{ pctp (pixelformat::bgr8, pixelformat::bgr8_indexed8), pixelconv_from_indexedcolor<bgr8, bgr8> },
	{ pctp (pixelformat::bgra4, pixelformat::bgr8_indexed8), pixelconv_from_indexedcolor<bgra4, bgr8> },
	{ pctp (pixelformat::bgr565, pixelformat::bgr8_indexed8), pixelconv_from_indexedcolor<bgr565, bgr8> },
	{ pctp (pixelformat::r8, pixelformat::bgr8_indexed8), pixelconv_from_indexedcolor<r8, bgr8> },
	{ pctp (pixelformat::rf, pixelformat::bgr8_indexed8), pixelconv_from_indexedcolor<rf, bgr8> },
	{ pctp (pixelformat::yuva8, pixelformat::bgr8_indexed8), pixelconv_from_indexedcolor<yuva8, bgr8> },
	{ pctp (pixelformat::yuv8, pixelformat::bgr8_indexed8), pixelconv_from_indexedcolor<yuv8, bgr8> },
	{ pctp (pixelformat::hsva8, pixelformat::bgr8_indexed8), pixelconv_from_indexedcolor<hsva8, bgr8> },
	{ pctp (pixelformat::hsv8, pixelformat::bgr8_indexed8), pixelconv_from_indexedcolor<hsv8, bgr8> },

	{ pctp (pixelformat::bgra8_indexed8, pixelformat::rgba8), pixelconv_to_indexedcolor<bgra8, rgba8> },
	{ pctp (pixelformat::bgra8_indexed8, pixelformat::rgb8), pixelconv_to_indexedcolor<bgra8, rgb8> },
	{ pctp (pixelformat::bgra8_indexed8, pixelformat::rgbaf), pixelconv_to_indexedcolor<bgra8, rgbaf> },
	{ pctp (pixelformat::bgra8_indexed8, pixelformat::bgra8), pixelconv_to_indexedcolor<bgra8, bgra8> },
	{ pctp (pixelformat::bgra8_indexed8, pixelformat::bgr8), pixelconv_to_indexedcolor<bgra8, bgr8> },
	{ pctp (pixelformat::bgra8_indexed8, pixelformat::bgra4), pixelconv_to_indexedcolor<bgra8, bgra4> },
	{ pctp (pixelformat::bgra8_indexed8, pixelformat::bgr565), pixelconv_to_indexedcolor<bgra8, bgr565> },
	{ pctp (pixelformat::bgra8_indexed8, pixelformat::r8), pixelconv_to_indexedcolor<bgra8, r8> },
	{ pctp (pixelformat::bgra8_indexed8, pixelformat::rf), pixelconv_to_indexedcolor<bgra8, rf> },
	{ pctp (pixelformat::bgra8_indexed8, pixelformat::yuva8), pixelconv_to_indexedcolor<bgra8, yuva8> },
	{ pctp (pixelformat::bgra8_indexed8, pixelformat::yuv8), pixelconv_to_indexedcolor<bgra8, yuv8> },
	{ pctp (pixelformat::bgra8_indexed8, pixelformat::hsva8), pixelconv_to_indexedcolor<bgra8, hsva8> },
	{ pctp (pixelformat::bgra8_indexed8, pixelformat::hsv8), pixelconv_to_indexedcolor<bgra8, hsv8> },

	{ pctp (pixelformat::bgr8_indexed8, pixelformat::rgba8), pixelconv_to_indexedcolor<bgr8, rgba8> },
	{ pctp (pixelformat::bgr8_indexed8, pixelformat::rgb8), pixelconv_to_indexedcolor<bgr8, rgb8> },
	{ pctp (pixelformat::bgr8_indexed8, pixelformat::rgbaf), pixelconv_to_indexedcolor<bgr8, rgbaf> },
	{ pctp (pixelformat::bgr8_indexed8, pixelformat::bgra8), pixelconv_to_indexedcolor<bgr8, bgra8> },
	{ pctp (pixelformat::bgr8_indexed8, pixelformat::bgr8), pixelconv_to_indexedcolor<bgr8, bgr8> },
	{ pctp (pixelformat::bgr8_indexed8, pixelformat::bgra4), pixelconv_to_indexedcolor<bgr8, bgra4> },
	{ pctp (pixelformat::bgr8_indexed8, pixelformat::bgr565), pixelconv_to_indexedcolor<bgr8, bgr565> },
	{ pctp (pixelformat::bgr8_indexed8, pixelformat::r8), pixelconv_to_indexedcolor<bgr8, r8> },
	{ pctp (pixelformat::bgr8_indexed8, pixelformat::rf), pixelconv_to_indexedcolor<bgr8, rf> },
	{ pctp (pixelformat::bgr8_indexed8, pixelformat::yuva8), pixelconv_to_indexedcolor<bgr8, yuva8> },
	{ pctp (pixelformat::bgr8_indexed8, pixelformat::yuv8), pixelconv_to_indexedcolor<bgr8, yuv8> },
	{ pctp (pixelformat::bgr8_indexed8, pixelformat::hsva8), pixelconv_to_indexedcolor<bgr8, hsva8> },
	{ pctp (pixelformat::bgr8_indexed8, pixelformat::hsv8), pixelconv_to_indexedcolor<bgr8, hsv8> },

	////////////////////////////////////////////////////////////////////////////////////////
	// Subsampled YUV Color Conversions
	////////////////////////////////////////////////////////////////////////////////////////
	{ pctp (pixelformat::yuyv8, pixelformat::rgba8), pixelconv_to_chromasubsample_yuv422<rgba8> },
	{ pctp (pixelformat::yuyv8, pixelformat::rgb8), pixelconv_to_chromasubsample_yuv422<rgb8> },
	{ pctp (pixelformat::yuyv8, pixelformat::rgbaf), pixelconv_to_chromasubsample_yuv422<rgbaf> },
	{ pctp (pixelformat::yuyv8, pixelformat::bgra8), pixelconv_to_chromasubsample_yuv422<bgra8> },
	{ pctp (pixelformat::yuyv8, pixelformat::bgr8), pixelconv_to_chromasubsample_yuv422<bgr8> },
	{ pctp (pixelformat::yuyv8, pixelformat::bgra4), pixelconv_to_chromasubsample_yuv422<bgra4> },
	{ pctp (pixelformat::yuyv8, pixelformat::bgr565), pixelconv_to_chromasubsample_yuv422<bgr565> },
	{ pctp (pixelformat::yuyv8, pixelformat::r8), pixelconv_to_chromasubsample_yuv422<r8> },
	{ pctp (pixelformat::yuyv8, pixelformat::rf), pixelconv_to_chromasubsample_yuv422<rf> },
	{ pctp (pixelformat::yuyv8, pixelformat::yuva8), pixelconv_to_chromasubsample_yuv422<yuva8> },
	{ pctp (pixelformat::yuyv8, pixelformat::yuv8), pixelconv_to_chromasubsample_yuv422<yuv8> },
	{ pctp (pixelformat::yuyv8, pixelformat::hsva8), pixelconv_to_chromasubsample_yuv422<hsva8> },
	{ pctp (pixelformat::yuyv8, pixelformat::hsv8), pixelconv_to_chromasubsample_yuv422<hsv8> },

	{ pctp (pixelformat::nv12, pixelformat::rgba8), pixelconv_to_chromasubsample_nv12<rgba8> },
	{ pctp (pixelformat::nv12, pixelformat::rgb8), pixelconv_to_chromasubsample_nv12<rgb8> },
	{ pctp (pixelformat::nv12, pixelformat::rgbaf), pixelconv_to_chromasubsample_nv12<rgbaf> },
	{ pctp (pixelformat::nv12, pixelformat::bgra8), pixelconv_to_chromasubsample_nv12<bgra8> },
	{ pctp (pixelformat::nv12, pixelformat::bgr8), pixelconv_to_chromasubsample_nv12<bgr8> },
	{ pctp (pixelformat::nv12, pixelformat::bgra4), pixelconv_to_chromasubsample_nv12<bgra4> },
	{ pctp (pixelformat::nv12, pixelformat::bgr565), pixelconv_to_chromasubsample_nv12<bgr565> },
	{ pctp (pixelformat::nv12, pixelformat::r8), pixelconv_to_chromasubsample_nv12<r8> },
	{ pctp (pixelformat::nv12, pixelformat::rf), pixelconv_to_chromasubsample_nv12<rf> },
	{ pctp (pixelformat::nv12, pixelformat::yuva8), pixelconv_to_chromasubsample_nv12<yuva8> },
	{ pctp (pixelformat::nv12, pixelformat::yuv8), pixelconv_to_chromasubsample_nv12<yuv8> },
	{ pctp (pixelformat::nv12, pixelformat::hsva8), pixelconv_to_chromasubsample_nv12<hsva8> },
	{ pctp (pixelformat::nv12, pixelformat::hsv8), pixelconv_to_chromasubsample_nv12<hsv8> },

	{ pctp (pixelformat::rgba8, pixelformat::yuyv8), pixelconv_from_chromasubsample_yuv422<rgba8> },
	{ pctp (pixelformat::rgb8, pixelformat::yuyv8), pixelconv_from_chromasubsample_yuv422<rgb8> },
	{ pctp (pixelformat::rgbaf, pixelformat::yuyv8), pixelconv_from_chromasubsample_yuv422<rgbaf> },
	{ pctp (pixelformat::bgra8, pixelformat::yuyv8), pixelconv_from_chromasubsample_yuv422<bgra8> },
	{ pctp (pixelformat::bgr8, pixelformat::yuyv8), pixelconv_from_chromasubsample_yuv422<bgr8> },
	{ pctp (pixelformat::bgra4, pixelformat::yuyv8), pixelconv_from_chromasubsample_yuv422<bgra4> },
	{ pctp (pixelformat::bgr565, pixelformat::yuyv8), pixelconv_from_chromasubsample_yuv422<bgr565> },
	{ pctp (pixelformat::r8, pixelformat::yuyv8), pixelconv_from_chromasubsample_yuv422<r8> },
	{ pctp (pixelformat::rf, pixelformat::yuyv8), pixelconv_from_chromasubsample_yuv422<rf> },
	{ pctp (pixelformat::yuva8, pixelformat::yuyv8), pixelconv_from_chromasubsample_yuv422<yuva8> },
	{ pctp (pixelformat::yuv8, pixelformat::yuyv8), pixelconv_from_chromasubsample_yuv422<yuv8> },
	{ pctp (pixelformat::hsva8, pixelformat::yuyv8), pixelconv_from_chromasubsample_yuv422<hsva8> },
	{ pctp (pixelformat::hsv8, pixelformat::yuyv8), pixelconv_from_chromasubsample_yuv422<hsv8> },

	{ pctp (pixelformat::rgba8, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<rgba8> },
	{ pctp (pixelformat::rgb8, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<rgb8> },
	{ pctp (pixelformat::rgbaf, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<rgbaf> },
	{ pctp (pixelformat::bgra8, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<bgra8> },
	{ pctp (pixelformat::bgr8, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<bgr8> },
	{ pctp (pixelformat::bgra4, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<bgra4> },
	{ pctp (pixelformat::bgr565, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<bgr565> },
	{ pctp (pixelformat::r8, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<r8> },
	{ pctp (pixelformat::rf, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<rf> },
	{ pctp (pixelformat::yuva8, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<yuva8> },
	{ pctp (pixelformat::yuv8, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<yuv8> },
	{ pctp (pixelformat::hsva8, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<hsva8> },
	{ pctp (pixelformat::hsv8, pixelformat::nv12), pixelconv_from_chromasubsample_nv12<hsv8> },

	////////////////////////////////////////////////////////////////////////////////////////
	// DXT Color Conversions
	////////////////////////////////////////////////////////////////////////////////////////
//	{ pctp (pixelformat::rgba8, pixelformat::bc1), pixelconv_to_dxt<BC1> },
//	{ pctp (pixelformat::rgba8, pixelformat::bc2), pixelconv_to_dxt<BC2> },
//	{ pctp (pixelformat::rgba8, pixelformat::bc3), pixelconv_to_dxt<BC3> },
//	{ pctp (pixelformat::rgba8, pixelformat::bc4), pixelconv_to_dxt<BC4> },
//	{ pctp (pixelformat::rgba8, pixelformat::bc5), pixelconv_to_dxt<BC5> },

//	{ pctp (pixelformat::bc1, pixelformat::rgba8), pixelconv_from_dxt<BC1> },
//	{ pctp (pixelformat::bc2, pixelformat::rgba8), pixelconv_from_dxt<BC2> },
//	{ pctp (pixelformat::bc3, pixelformat::rgba8), pixelconv_from_dxt<BC3> },
//	{ pctp (pixelformat::bc4, pixelformat::rgba8), pixelconv_from_dxt<BC4> },
//	{ pctp (pixelformat::bc5, pixelformat::rgba8), pixelconv_from_dxt<BC5> },

	////////////////////////////////////////////////////////////////////////////////////////
	// ETC1 Color Conversions
	////////////////////////////////////////////////////////////////////////////////////////
	{ pctp (pixelformat::rgba8, pixelformat::etc1), pixelconv<pixelformat::rgba8, pixelformat::etc1> },
	{ pctp (pixelformat::rgb8, pixelformat::etc1), pixelconv<pixelformat::rgb8, pixelformat::etc1> },

	{ pctp (pixelformat::etc1, pixelformat::rgba8), pixelconv<pixelformat::etc1, pixelformat::rgba8> },
	{ pctp (pixelformat::etc1, pixelformat::rgb8), pixelconv<pixelformat::etc1, pixelformat::rgb8> },

};

dseed::error_t dseed::bitmaps::reformat_bitmap (dseed::bitmaps::bitmap* original, dseed::color::pixelformat reformat, dseed::bitmaps::bitmap** bitmap)
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
	dseed::autoref<dseed::bitmaps::palette> tempPalette;
	if (reformat == pixelformat::bgra8_indexed8 || reformat == pixelformat::bgr8_indexed8)
	{
		isIndexedTarget = true;
		bpp = ((int)reformat & 0xff) * 8;

		if (dseed::failed (dseed::bitmaps::create_palette (nullptr, bpp, 256, &tempPalette)))
			return dseed::error_fail;
	}

	dseed::size3i size = original->size ();

	dseed::autoref<dseed::bitmaps::bitmap> temp;
	if (dseed::failed (dseed::bitmaps::create_bitmap (dseed::bitmaps::bitmaptype::bitmap2d, size, reformat
		, tempPalette, &temp)))
		return dseed::error_fail;

	uint8_t* destPtr, * srcPtr, * destPalettePtr = nullptr, * srcPalettePtr = nullptr;
	original->lock ((void**)&srcPtr);
	temp->lock ((void**)&destPtr);

	dseed::autoref<dseed::bitmaps::palette> destPalette, srcPalette;
	if (dseed::succeeded (original->palette (&srcPalette)) && srcPalette != nullptr)
		srcPalette->lock ((void**)&srcPalettePtr);
	if (dseed::succeeded (temp->palette (&destPalette)) && destPalette != nullptr)
		destPalette->lock ((void**)&destPalettePtr);

	auto found = g_pixelconvs.find (pctp (reformat, originalFormat));
	if (found == g_pixelconvs.end ())
		return dseed::error_not_support;

	int paletteCount = found->second (destPtr, srcPtr, size, destPalettePtr, srcPalettePtr);
	if (paletteCount == -1)
		return dseed::error_not_support;

	if (destPalette != nullptr)
		destPalette->unlock ();
	if (srcPalette != nullptr)
		srcPalette->unlock ();

	temp->unlock ();
	original->unlock ();

	*bitmap = temp.detach ();

	return dseed::error_good;
}