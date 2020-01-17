#include <dseed.h>

#include <vector>
#include <map>
#include <tuple>

using namespace dseed::color;
using size2i = dseed::size2i;
using resize = dseed::bitmaps::resize;

using rzfn = std::function<bool (uint8_t * dest, const uint8_t * src, const dseed::size3i & destSize, const dseed::size3i & srcSize)>;
using rztp = std::tuple<dseed::bitmaps::resize, dseed::color::pixelformat>;

template<class TPixel>
inline bool bmprsz_nearest (uint8_t* dest, const uint8_t* src, const dseed::size3i& destSize, const dseed::size3i& srcSize) noexcept
{
	size_t destStride = calc_bitmap_stride (type2format<TPixel> (), destSize.width)
		, srcStride = calc_bitmap_stride (type2format<TPixel> (), srcSize.width);
	size_t destDepth = calc_bitmap_plane_size (type2format<TPixel> (), size2i (destSize.width, destSize.height))
		, srcDepth = calc_bitmap_plane_size (type2format<TPixel> (), size2i (srcSize.width, srcSize.height));

	for (size_t z = 0; z < destSize.depth; ++z)
	{
		size_t srcZ = (size_t)(z * srcSize.depth / destSize.depth);
		size_t destDepthZ = z * destDepth
			, srcDepthZ = srcZ * srcDepth;
		for (size_t y = 0; y < destSize.height; ++y)
		{
			size_t srcY = (size_t)(y * srcSize.height / destSize.height);
			size_t destStrideY = y * destStride
				, srcStrideY = srcY * srcStride;
			TPixel* destPtr = (TPixel*)(dest + destDepthZ + destStrideY);
			const TPixel* srcPtr = (TPixel*)(src + srcDepthZ + srcStrideY);

			for (size_t x = 0; x < destSize.width; ++x)
			{
				size_t srcX = (size_t)(x * srcSize.width / destSize.width);
				TPixel* destPtrX = destPtr + x;
				const TPixel* srcPtrX = srcPtr + srcX;
				*destPtrX = *srcPtrX;
			}
		}
	}

	return true;
}

template<class TPixel>
inline bool bmprsz_bilinear (uint8_t* dest, const uint8_t* src, const dseed::size3i& destSize, const dseed::size3i& srcSize) noexcept
{
	size_t destStride = calc_bitmap_stride (type2format<TPixel> (), destSize.width)
		, srcStride = calc_bitmap_stride (type2format<TPixel> (), srcSize.width);
	size_t destDepth = calc_bitmap_plane_size (type2format<TPixel> (), size2i (destSize.width, destSize.height))
		, srcDepth = calc_bitmap_plane_size (type2format<TPixel> (), size2i (srcSize.width, srcSize.height));

	double zRatio = srcSize.depth / (double)destSize.depth
		, yRatio = (srcSize.height - 1) / (double)destSize.height
		, xRatio = (srcSize.width - 1) / (double)destSize.width;

	for (size_t z = 0; z < destSize.depth; ++z)
	{
		size_t srcZ = (size_t)(z * zRatio);
		size_t destDepthZ = z * destDepth
			, srcDepthZ = srcZ * srcDepth;
		for (size_t y = 0; y < destSize.height; ++y)
		{
			size_t srcY1 = (size_t)(y * yRatio);
			size_t srcY2 = (size_t)((y + 1) * yRatio);
			double yDiff1 = (yRatio * y) - srcY1, yDiff2 = 1 - yDiff1;

			size_t destStrideY = y * destStride
				, srcStrideY1 = srcY1 * srcStride
				, srcStrideY2 = srcY2 * srcStride;
			TPixel* destPtr = (TPixel*)(dest + destDepthZ + destStrideY);
			const TPixel* srcPtr1 = (TPixel*)(src + srcDepthZ + srcStrideY1);
			const TPixel* srcPtr2 = (TPixel*)(src + srcDepthZ + srcStrideY2);

			for (size_t x = 0; x < destSize.width; ++x)
			{
				size_t srcX1 = (size_t)(x * xRatio);
				size_t srcX2 = (size_t)((x + 1) * xRatio);
				double xDiff1 = (xRatio * x) - srcX1, xDiff2 = 1 - xDiff1;

				TPixel srcY1XColorSum = ((*(srcPtr1 + srcX1) * xDiff2) + (*(srcPtr1 + srcX2) * xDiff1)) * yDiff2;
				TPixel srcY2XColorSum = ((*(srcPtr2 + srcX1) * xDiff2) + (*(srcPtr2 + srcX2) * xDiff1)) * yDiff1;

				TPixel* destPtrX = destPtr + x;
				*destPtrX = srcY1XColorSum + srcY2XColorSum;
			}
		}
	}

	return true;
}

colorv cubic_hermite (const colorv& a, const colorv& b, const colorv& c, const colorv& d, double factor) noexcept
{
	colorv _a = (-a / 2.0f) + ((b * 3.0f) / 2.0f) - (c * 3.0f) / 2.0f + (d / 2.0f);
	colorv _b = a - ((b * 5.0f) / 2.0f) + (c * 2.0f) - (d / 2.0f);
	colorv _c = (-a / 2.0f) + (c / 2.0f);
	colorv _d = b;

	return (_a * factor * factor * factor) + (_b * factor * factor) + (_c * factor) + d;
}
template<class TPixel>
inline bool bmprsz_bicubic (uint8_t* dest, const uint8_t* src, const dseed::size3i& destSize, const dseed::size3i& srcSize) noexcept
{
	size_t destStride = calc_bitmap_stride (type2format<TPixel> (), destSize.width)
		, srcStride = calc_bitmap_stride (type2format<TPixel> (), srcSize.width);
	size_t destDepth = calc_bitmap_plane_size (type2format<TPixel> (), size2i (destSize.width, destSize.height))
		, srcDepth = calc_bitmap_plane_size (type2format<TPixel> (), size2i (srcSize.width, srcSize.height));

	double zRatio = srcSize.depth / (double)destSize.depth
		, yRatio = (srcSize.height - 1) / (double)destSize.height
		, xRatio = (srcSize.width - 1) / (double)destSize.width;

	for (size_t z = 0; z < destSize.depth; ++z)
	{
		size_t srcZ = (size_t)(z * zRatio);
		size_t destDepthZ = z * destDepth
			, srcDepthZ = srcZ * srcDepth;

		const uint8_t* srcPtr = (uint8_t*)(src + srcDepthZ);

		for (size_t y = 0; y < destSize.height; ++y)
		{
			size_t srcY1 = dseed::clamp<size_t> ((size_t)((y - 1) * yRatio), srcSize.height - 1);
			size_t srcY2 = dseed::clamp<size_t> ((size_t)((y + 0) * yRatio), srcSize.height - 1);
			size_t srcY3 = dseed::clamp<size_t> ((size_t)((y + 1) * yRatio), srcSize.height - 1);
			size_t srcY4 = dseed::clamp<size_t> ((size_t)((y + 2) * yRatio), srcSize.height - 1);
			double yDiff1 = (yRatio * y) - srcY2, yDiff2 = 1 - yDiff1;

			size_t destStrideY = y * destStride;
			TPixel* destPtr = (TPixel*)(dest + destDepthZ + destStrideY);

			for (size_t x = 0; x < destSize.width; ++x)
			{
				size_t srcX1 = dseed::clamp<size_t> ((size_t)((x - 1) * xRatio), srcSize.width - 1);
				size_t srcX2 = dseed::clamp<size_t> ((size_t)((x + 0) * xRatio), srcSize.width - 1);
				size_t srcX3 = dseed::clamp<size_t> ((size_t)((x + 1) * xRatio), srcSize.width - 1);
				size_t srcX4 = dseed::clamp<size_t> ((size_t)((x + 2) * xRatio), srcSize.width - 1);
				double xDiff1 = (xRatio * x) - srcX2, xDiff2 = 1 - xDiff1;

				const TPixel* srcPtrY1 = (const TPixel*)(srcPtr + (srcY1 * srcStride))
					, * srcPtrY2 = (const TPixel*)(srcPtr + (srcY2 * srcStride))
					, * srcPtrY3 = (const TPixel*)(srcPtr + (srcY3 * srcStride))
					, * srcPtrY4 = (const TPixel*)(srcPtr + (srcY4 * srcStride));

				colorv
					p11 = *(srcPtrY1 + srcX1), p12 = *(srcPtrY1 + srcX2), p13 = *(srcPtrY1 + srcX3), p14 = *(srcPtrY1 + srcX4),
					p21 = *(srcPtrY2 + srcX1), p22 = *(srcPtrY2 + srcX2), p23 = *(srcPtrY2 + srcX3), p24 = *(srcPtrY2 + srcX4),
					p31 = *(srcPtrY3 + srcX1), p32 = *(srcPtrY3 + srcX2), p33 = *(srcPtrY3 + srcX3), p34 = *(srcPtrY3 + srcX4),
					p41 = *(srcPtrY4 + srcX1), p42 = *(srcPtrY4 + srcX2), p43 = *(srcPtrY4 + srcX3), p44 = *(srcPtrY4 + srcX4);

				colorv p1, p2, p3, p4;
				p1 = cubic_hermite (p11, p12, p13, p14, xDiff1);
				p2 = cubic_hermite (p21, p22, p23, p24, xDiff1);
				p3 = cubic_hermite (p31, p32, p33, p34, xDiff1);
				p4 = cubic_hermite (p41, p42, p43, p44, xDiff1);

				TPixel calced = cubic_hermite (p1, p2, p3, p4, yDiff1);

				TPixel* destPtrX = destPtr + x;
				*destPtrX = calced;
			}
		}
	}

	return true;
}

inline float sinc (float x) noexcept
{
	if (x == 0) return 1;
	return sin (dseed::pi * x) / (dseed::pi * x);
}
inline float lanczos_weight (float x, float dist) noexcept
{
	if (dist <= x)
		return sinc (dist) * sinc (dist / x);
	return 0;
}
constexpr int LANCZOS_WINDOW1 = 1;
constexpr int LANCZOS_WINDOW2 = 2;
constexpr int LANCZOS_WINDOW3 = 3;
constexpr int LANCZOS_WINDOW4 = 4;
constexpr int LANCZOS_WINDOW5 = 5;
template<class TPixel, int window>
inline bool bmprsz_lanczos (uint8_t* dest, const uint8_t* src, const dseed::size3i& destSize, const dseed::size3i& srcSize) noexcept
{
	size_t destStride = calc_bitmap_stride (type2format<TPixel> (), destSize.width)
		, srcStride = calc_bitmap_stride (type2format<TPixel> (), srcSize.width);
	size_t destDepth = calc_bitmap_plane_size (type2format<TPixel> (), size2i (destSize.width, destSize.height))
		, srcDepth = calc_bitmap_plane_size (type2format<TPixel> (), size2i (srcSize.width, srcSize.height));

	double zRatio = srcSize.depth / (double)destSize.depth
		, yRatio = srcSize.height / (double)destSize.height
		, xRatio = srcSize.width / (double)destSize.width;

	for (size_t z = 0; z < destSize.depth; ++z)
	{
		size_t srcZ = (size_t)(z * zRatio);
		size_t destDepthZ = z * destDepth
			, srcDepthZ = srcZ * srcDepth;

		const uint8_t* srcPtr = (uint8_t*)(src + srcDepthZ);

		for (size_t y = 0; y < destSize.height; ++y)
		{
			size_t destStrideY = y * destStride;
			TPixel* destPtr = (TPixel*)(dest + destDepthZ + destStrideY);

			for (size_t x = 0; x < destSize.width; ++x)
			{
				colorv sum;
				float samples = 0;

				int cx = (int)(x * xRatio);
				int cy = (int)(y * yRatio);

				for (int ly = -window + 1; ly <= window; ++ly)
				{
					for (int lx = -window + 1; lx <= window; ++lx)
					{
						int ix = cx + lx;
						int iy = cy + ly;

						if (ix < 0 || iy < 0 || ix >= srcSize.width || iy >= srcSize.height)
							continue;

						int deltaX = cx - ix;
						int deltaY = cy - iy;
						float weight = lanczos_weight ((float)window, (float)lx)
							* lanczos_weight ((float)window, (float)ly);

						colorv srcPixel = *((const TPixel*)(srcPtr + (srcStride * iy)) + ix);
						srcPixel = srcPixel * weight;
						sum += srcPixel;

						samples += weight;
					}
				}

				sum = sum / samples;

				TPixel* destPtrX = destPtr + x;
				*destPtrX = sum;
			}
		}
	}

	return true;
}

std::map<rztp, rzfn> g_resizes = {
	{ rztp (resize::nearest, pixelformat::rgba8), bmprsz_nearest<rgba8> },
	{ rztp (resize::nearest, pixelformat::rgb8), bmprsz_nearest<rgb8> },
	{ rztp (resize::nearest, pixelformat::rgbaf), bmprsz_nearest<rgbaf> },
	{ rztp (resize::nearest, pixelformat::bgra8), bmprsz_nearest<bgra8> },
	{ rztp (resize::nearest, pixelformat::bgr8), bmprsz_nearest<bgr8> },
	{ rztp (resize::nearest, pixelformat::bgra4), bmprsz_nearest<bgra4> },
	{ rztp (resize::nearest, pixelformat::bgr565), bmprsz_nearest<bgr565> },
	{ rztp (resize::nearest, pixelformat::r8), bmprsz_nearest<r8> },
	{ rztp (resize::nearest, pixelformat::rf), bmprsz_nearest<rf> },
	{ rztp (resize::nearest, pixelformat::yuva8), bmprsz_nearest<yuva8> },
	{ rztp (resize::nearest, pixelformat::yuv8), bmprsz_nearest<yuv8> },
	{ rztp (resize::nearest, pixelformat::hsva8), bmprsz_nearest<hsva8> },
	{ rztp (resize::nearest, pixelformat::hsv8), bmprsz_nearest<hsv8> },

	{ rztp (resize::bilinear, pixelformat::rgba8), bmprsz_bilinear<rgba8> },
	{ rztp (resize::bilinear, pixelformat::rgb8), bmprsz_bilinear<rgb8> },
	{ rztp (resize::bilinear, pixelformat::rgbaf), bmprsz_bilinear<rgbaf> },
	{ rztp (resize::bilinear, pixelformat::bgra8), bmprsz_bilinear<bgra8> },
	{ rztp (resize::bilinear, pixelformat::bgr8), bmprsz_bilinear<bgr8> },
	{ rztp (resize::bilinear, pixelformat::bgra4), bmprsz_bilinear<bgra4> },
	{ rztp (resize::bilinear, pixelformat::bgr565), bmprsz_bilinear<bgr565> },
	{ rztp (resize::bilinear, pixelformat::r8), bmprsz_bilinear<r8> },
	{ rztp (resize::bilinear, pixelformat::rf), bmprsz_bilinear<rf> },
	{ rztp (resize::bilinear, pixelformat::yuva8), bmprsz_bilinear<yuva8> },
	{ rztp (resize::bilinear, pixelformat::yuv8), bmprsz_bilinear<yuv8> },
	{ rztp (resize::bilinear, pixelformat::hsva8), bmprsz_bilinear<hsva8> },
	{ rztp (resize::bilinear, pixelformat::hsv8), bmprsz_bilinear<hsv8> },

	{ rztp (resize::bicubic, pixelformat::rgba8), bmprsz_bicubic<rgba8> },
	{ rztp (resize::bicubic, pixelformat::rgb8), bmprsz_bicubic<rgb8> },
	{ rztp (resize::bicubic, pixelformat::rgbaf), bmprsz_bicubic<rgbaf> },
	{ rztp (resize::bicubic, pixelformat::bgra8), bmprsz_bicubic<bgra8> },
	{ rztp (resize::bicubic, pixelformat::bgr8), bmprsz_bicubic<bgr8> },
	{ rztp (resize::bicubic, pixelformat::bgra4), bmprsz_bicubic<bgra4> },
	{ rztp (resize::bicubic, pixelformat::bgr565), bmprsz_bicubic<bgr565> },
	{ rztp (resize::bicubic, pixelformat::r8), bmprsz_bicubic<r8> },
	{ rztp (resize::bicubic, pixelformat::rf), bmprsz_bicubic<rf> },
	{ rztp (resize::bicubic, pixelformat::yuva8), bmprsz_bicubic<yuva8> },
	{ rztp (resize::bicubic, pixelformat::yuv8), bmprsz_bicubic<yuv8> },
	{ rztp (resize::bicubic, pixelformat::hsva8), bmprsz_bicubic<hsva8> },
	{ rztp (resize::bicubic, pixelformat::hsv8), bmprsz_bicubic<hsv8> },

	{ rztp (resize::lanczos, pixelformat::rgba8), bmprsz_lanczos<rgba8, LANCZOS_WINDOW1> },
	{ rztp (resize::lanczos, pixelformat::rgb8), bmprsz_lanczos<rgb8, LANCZOS_WINDOW1> },
	{ rztp (resize::lanczos, pixelformat::rgbaf), bmprsz_lanczos<rgbaf, LANCZOS_WINDOW1> },
	{ rztp (resize::lanczos, pixelformat::bgra8), bmprsz_lanczos<bgra8, LANCZOS_WINDOW1> },
	{ rztp (resize::lanczos, pixelformat::bgr8), bmprsz_lanczos<bgr8, LANCZOS_WINDOW1> },
	{ rztp (resize::lanczos, pixelformat::bgra4), bmprsz_lanczos<bgra4, LANCZOS_WINDOW1> },
	{ rztp (resize::lanczos, pixelformat::bgr565), bmprsz_lanczos<bgr565, LANCZOS_WINDOW1> },
	{ rztp (resize::lanczos, pixelformat::r8), bmprsz_lanczos<r8, LANCZOS_WINDOW1> },
	{ rztp (resize::lanczos, pixelformat::rf), bmprsz_lanczos<rf, LANCZOS_WINDOW1> },
	{ rztp (resize::lanczos, pixelformat::yuva8), bmprsz_lanczos<yuva8, LANCZOS_WINDOW1> },
	{ rztp (resize::lanczos, pixelformat::yuv8), bmprsz_lanczos<yuv8, LANCZOS_WINDOW1> },
	{ rztp (resize::lanczos, pixelformat::hsva8), bmprsz_lanczos<hsva8, LANCZOS_WINDOW1> },
	{ rztp (resize::lanczos, pixelformat::hsv8), bmprsz_lanczos<hsv8, LANCZOS_WINDOW1> },

	{ rztp (resize::lanczos2, pixelformat::rgba8), bmprsz_lanczos<rgba8, LANCZOS_WINDOW2> },
	{ rztp (resize::lanczos2, pixelformat::rgb8), bmprsz_lanczos<rgb8, LANCZOS_WINDOW2> },
	{ rztp (resize::lanczos2, pixelformat::rgbaf), bmprsz_lanczos<rgbaf, LANCZOS_WINDOW2> },
	{ rztp (resize::lanczos2, pixelformat::bgra8), bmprsz_lanczos<bgra8, LANCZOS_WINDOW2> },
	{ rztp (resize::lanczos2, pixelformat::bgr8), bmprsz_lanczos<bgr8, LANCZOS_WINDOW2> },
	{ rztp (resize::lanczos2, pixelformat::bgra4), bmprsz_lanczos<bgra4, LANCZOS_WINDOW2> },
	{ rztp (resize::lanczos2, pixelformat::bgr565), bmprsz_lanczos<bgr565, LANCZOS_WINDOW2> },
	{ rztp (resize::lanczos2, pixelformat::r8), bmprsz_lanczos<r8, LANCZOS_WINDOW2> },
	{ rztp (resize::lanczos2, pixelformat::rf), bmprsz_lanczos<rf, LANCZOS_WINDOW2> },
	{ rztp (resize::lanczos2, pixelformat::yuva8), bmprsz_lanczos<yuva8, LANCZOS_WINDOW2> },
	{ rztp (resize::lanczos2, pixelformat::yuv8), bmprsz_lanczos<yuv8, LANCZOS_WINDOW2> },
	{ rztp (resize::lanczos2, pixelformat::hsva8), bmprsz_lanczos<hsva8, LANCZOS_WINDOW2> },
	{ rztp (resize::lanczos2, pixelformat::hsv8), bmprsz_lanczos<hsv8, LANCZOS_WINDOW2> },

	{ rztp (resize::lanczos3, pixelformat::rgba8), bmprsz_lanczos<rgba8, LANCZOS_WINDOW3> },
	{ rztp (resize::lanczos3, pixelformat::rgb8), bmprsz_lanczos<rgb8, LANCZOS_WINDOW3> },
	{ rztp (resize::lanczos3, pixelformat::rgbaf), bmprsz_lanczos<rgbaf, LANCZOS_WINDOW3> },
	{ rztp (resize::lanczos3, pixelformat::bgra8), bmprsz_lanczos<bgra8, LANCZOS_WINDOW3> },
	{ rztp (resize::lanczos3, pixelformat::bgr8), bmprsz_lanczos<bgr8, LANCZOS_WINDOW3> },
	{ rztp (resize::lanczos3, pixelformat::bgra4), bmprsz_lanczos<bgra4, LANCZOS_WINDOW3> },
	{ rztp (resize::lanczos3, pixelformat::bgr565), bmprsz_lanczos<bgr565, LANCZOS_WINDOW3> },
	{ rztp (resize::lanczos3, pixelformat::r8), bmprsz_lanczos<r8, LANCZOS_WINDOW3> },
	{ rztp (resize::lanczos3, pixelformat::rf), bmprsz_lanczos<rf, LANCZOS_WINDOW3> },
	{ rztp (resize::lanczos3, pixelformat::yuva8), bmprsz_lanczos<yuva8, LANCZOS_WINDOW3> },
	{ rztp (resize::lanczos3, pixelformat::yuv8), bmprsz_lanczos<yuv8, LANCZOS_WINDOW3> },
	{ rztp (resize::lanczos3, pixelformat::hsva8), bmprsz_lanczos<hsva8, LANCZOS_WINDOW3> },
	{ rztp (resize::lanczos3, pixelformat::hsv8), bmprsz_lanczos<hsv8, LANCZOS_WINDOW3> },

	{ rztp (resize::lanczos4, pixelformat::rgba8), bmprsz_lanczos<rgba8, LANCZOS_WINDOW4> },
	{ rztp (resize::lanczos4, pixelformat::rgb8), bmprsz_lanczos<rgb8, LANCZOS_WINDOW4> },
	{ rztp (resize::lanczos4, pixelformat::rgbaf), bmprsz_lanczos<rgbaf, LANCZOS_WINDOW4> },
	{ rztp (resize::lanczos4, pixelformat::bgra8), bmprsz_lanczos<bgra8, LANCZOS_WINDOW4> },
	{ rztp (resize::lanczos4, pixelformat::bgr8), bmprsz_lanczos<bgr8, LANCZOS_WINDOW4> },
	{ rztp (resize::lanczos4, pixelformat::bgra4), bmprsz_lanczos<bgra4, LANCZOS_WINDOW4> },
	{ rztp (resize::lanczos4, pixelformat::bgr565), bmprsz_lanczos<bgr565, LANCZOS_WINDOW4> },
	{ rztp (resize::lanczos4, pixelformat::r8), bmprsz_lanczos<r8, LANCZOS_WINDOW4> },
	{ rztp (resize::lanczos4, pixelformat::rf), bmprsz_lanczos<rf, LANCZOS_WINDOW4> },
	{ rztp (resize::lanczos4, pixelformat::yuva8), bmprsz_lanczos<yuva8, LANCZOS_WINDOW4> },
	{ rztp (resize::lanczos4, pixelformat::yuv8), bmprsz_lanczos<yuv8, LANCZOS_WINDOW4> },
	{ rztp (resize::lanczos4, pixelformat::hsva8), bmprsz_lanczos<hsva8, LANCZOS_WINDOW4> },
	{ rztp (resize::lanczos4, pixelformat::hsv8), bmprsz_lanczos<hsv8, LANCZOS_WINDOW4> },

	{ rztp (resize::lanczos5, pixelformat::rgba8), bmprsz_lanczos<rgba8, LANCZOS_WINDOW5> },
	{ rztp (resize::lanczos5, pixelformat::rgb8), bmprsz_lanczos<rgb8, LANCZOS_WINDOW5> },
	{ rztp (resize::lanczos5, pixelformat::rgbaf), bmprsz_lanczos<rgbaf, LANCZOS_WINDOW5> },
	{ rztp (resize::lanczos5, pixelformat::bgra8), bmprsz_lanczos<bgra8, LANCZOS_WINDOW5> },
	{ rztp (resize::lanczos5, pixelformat::bgr8), bmprsz_lanczos<bgr8, LANCZOS_WINDOW5> },
	{ rztp (resize::lanczos5, pixelformat::bgra4), bmprsz_lanczos<bgra4, LANCZOS_WINDOW5> },
	{ rztp (resize::lanczos5, pixelformat::bgr565), bmprsz_lanczos<bgr565, LANCZOS_WINDOW5> },
	{ rztp (resize::lanczos5, pixelformat::r8), bmprsz_lanczos<r8, LANCZOS_WINDOW5> },
	{ rztp (resize::lanczos5, pixelformat::rf), bmprsz_lanczos<rf, LANCZOS_WINDOW5> },
	{ rztp (resize::lanczos5, pixelformat::yuva8), bmprsz_lanczos<yuva8, LANCZOS_WINDOW5> },
	{ rztp (resize::lanczos5, pixelformat::yuv8), bmprsz_lanczos<yuv8, LANCZOS_WINDOW5> },
	{ rztp (resize::lanczos5, pixelformat::hsva8), bmprsz_lanczos<hsva8, LANCZOS_WINDOW5> },
	{ rztp (resize::lanczos5, pixelformat::hsv8), bmprsz_lanczos<hsv8, LANCZOS_WINDOW5> },
};

dseed::error_t dseed::bitmaps::resize_bitmap (dseed::bitmaps::bitmap* original, resize resize_method, const dseed::size3i& size, dseed::bitmaps::bitmap** bitmap)
{
	if (original == nullptr || bitmap == nullptr)
		return dseed::error_invalid_args;
	if (size.width == 0 || size.height == 0 || size.depth == 0)
		return dseed::error_invalid_args;

	dseed::autoref<dseed::bitmaps::bitmap> temp;
	if (dseed::failed (dseed::bitmaps::create_bitmap (original->type (), size, original->format (), nullptr, &temp)))
		return dseed::error_fail;

	uint8_t* destPtr, * srcPtr;
	original->lock ((void**)&srcPtr);
	temp->lock ((void**)&destPtr);

	auto found = g_resizes.find (rztp (resize_method, original->format ()));
	if (found == g_resizes.end ())
		return dseed::error_not_support;

	if (!found->second (destPtr, srcPtr, size, original->size ()))
		return dseed::error_not_support;

	temp->unlock ();
	original->unlock ();

	*bitmap = temp.detach ();

	return dseed::error_good;
}

using cpfn = std::function<bool (uint8_t * dest, const uint8_t * src, const dseed::size3i & size, const dseed::rect2i & area)>;

template<class TPixel>
inline bool crop_pixels (uint8_t* dest, const uint8_t* src, const dseed::size3i& size, const dseed::rect2i& area) noexcept
{
	size_t destStride = calc_bitmap_stride (type2format<TPixel> (), area.width)
		, srcStride = calc_bitmap_stride (type2format<TPixel> (), size.width);
	size_t destDepth = calc_bitmap_plane_size (type2format<TPixel> (), size2i (area.width, area.height))
		, srcDepth = calc_bitmap_plane_size (type2format<TPixel> (), size2i (size.width, size.height));

	for (size_t z = 0; z < size.depth; ++z)
	{
		size_t destDepthZ = z * destDepth
			, srcDepthZ = z * srcDepth;
		for (size_t y = area.y; y < area.height; ++y)
		{
			size_t destStrideY = (y - area.y) * destStride
				, srcStrideY = y * srcStride;
			TPixel* destPtr = (TPixel*)(dest + destDepthZ + destStrideY);
			const TPixel* srcPtr = (TPixel*)(src + srcDepthZ + srcStrideY);

			for (size_t x = area.x; x < area.width; ++x)
			{
				TPixel* destPtrX = destPtr + (x - area.x);
				const TPixel* srcPtrX = srcPtr + x;
				*destPtrX = *srcPtrX;
			}
		}
	}

	return true;
}

std::unordered_map<pixelformat, cpfn> g_crops = {
	{ pixelformat::rgba8, crop_pixels<rgba8> },
	{ pixelformat::rgb8, crop_pixels<rgb8> },
	{ pixelformat::rgbaf, crop_pixels<rgbaf> },
	{ pixelformat::bgra8, crop_pixels<bgra8> },
	{ pixelformat::bgr8, crop_pixels<bgr8> },
	{ pixelformat::bgra4, crop_pixels<bgra4> },
	{ pixelformat::bgr565, crop_pixels<bgr565> },
	{ pixelformat::r8, crop_pixels<r8> },
	{ pixelformat::rf, crop_pixels<rf> },
	{ pixelformat::yuva8, crop_pixels<yuva8> },
	{ pixelformat::yuv8, crop_pixels<yuv8> },
};

dseed::error_t dseed::bitmaps::crop_bitmap (dseed::bitmaps::bitmap* original, const rect2i& area, dseed::bitmaps::bitmap** bitmap)
{
	if (original == nullptr || bitmap == nullptr)
		return dseed::error_invalid_args;

	auto size = original->size ();
	if (size.width < (area.width - area.x) ||
		size.height < (area.height - area.y) ||
		area.x < 0 || area.y < 0)
		return dseed::error_invalid_args;

	dseed::autoref<dseed::bitmaps::bitmap> temp;
	if (dseed::failed (dseed::bitmaps::create_bitmap (original->type (), dseed::size3i (area.width - area.x, area.height - area.y, size.depth), original->format (), nullptr, &temp)))
		return dseed::error_fail;

	uint8_t* destPtr, * srcPtr;
	original->lock ((void**)&srcPtr);
	temp->lock ((void**)&destPtr);

	auto found = g_crops.find (original->format ());
	if (found == g_crops.end ())
		return dseed::error_not_support;

	if (!found->second (destPtr, srcPtr, size, area))
		return dseed::error_not_support;

	temp->unlock ();
	original->unlock ();

	*bitmap = temp.detach ();

	return dseed::error_good;
}