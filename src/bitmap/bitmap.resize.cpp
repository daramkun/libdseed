#include <dseed.h>

#include <vector>
#include <map>

using namespace dseed;

using rzfn = std::function<bool (uint8_t* dest, const uint8_t* src, const dseed::size3i& destSize, const dseed::size3i& srcSize)>;
using rztp = std::tuple<dseed::resize_t, dseed::pixelformat>;

template<class TPixel>
inline bool bmprsz_nearest (uint8_t* dest, const uint8_t* src, const dseed::size3i& destSize, const dseed::size3i& srcSize) noexcept
{
	size_t destStride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), destSize.width)
		, srcStride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), srcSize.width);
	size_t destDepth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), destSize.width, destSize.height)
		, srcDepth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), srcSize.width, srcSize.height);

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
	size_t destStride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), destSize.width)
		, srcStride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), srcSize.width);
	size_t destDepth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), destSize.width, destSize.height)
		, srcDepth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), srcSize.width, srcSize.height);

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

color4f cubic_hermite (const color4f& a, const color4f& b, const color4f& c, const color4f& d, double factor) noexcept
{
	color4f _a = (-a / 2.0f) + ((b * 3.0f) / 2.0f) - (c * 3.0f) / 2.0f + (d / 2.0f);
	color4f _b = a - ((b * 5.0f) / 2.0f) + (c * 2.0f) - (d / 2.0f);
	color4f _c = (-a / 2.0f) + (c / 2.0f);
	color4f _d = b;

	return (_a * factor * factor * factor) + (_b * factor * factor) + (_c * factor) + d;
}
template<class TPixel>
inline bool bmprsz_bicubic (uint8_t* dest, const uint8_t* src, const dseed::size3i& destSize, const dseed::size3i& srcSize) noexcept
{
	size_t destStride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), destSize.width)
		, srcStride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), srcSize.width);
	size_t destDepth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), destSize.width, destSize.height)
		, srcDepth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), srcSize.width, srcSize.height);

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
			size_t srcY1 = clamp<size_t> ((size_t)((y - 1) * yRatio), srcSize.height - 1);
			size_t srcY2 = clamp<size_t> ((size_t)((y + 0) * yRatio), srcSize.height - 1);
			size_t srcY3 = clamp<size_t> ((size_t)((y + 1) * yRatio), srcSize.height - 1);
			size_t srcY4 = clamp<size_t> ((size_t)((y + 2) * yRatio), srcSize.height - 1);
			double yDiff1 = (yRatio * y) - srcY2, yDiff2 = 1 - yDiff1;

			size_t destStrideY = y * destStride;
			TPixel* destPtr = (TPixel*)(dest + destDepthZ + destStrideY);

			for (size_t x = 0; x < destSize.width; ++x)
			{
				size_t srcX1 = clamp<size_t> ((size_t)((x - 1) * xRatio), srcSize.width - 1);
				size_t srcX2 = clamp<size_t> ((size_t)((x + 0) * xRatio), srcSize.width - 1);
				size_t srcX3 = clamp<size_t> ((size_t)((x + 1) * xRatio), srcSize.width - 1);
				size_t srcX4 = clamp<size_t> ((size_t)((x + 2) * xRatio), srcSize.width - 1);
				double xDiff1 = (xRatio * x) - srcX2, xDiff2 = 1 - xDiff1;

				const TPixel* srcPtrY1 = (const TPixel*)(srcPtr + (srcY1 * srcStride))
					, * srcPtrY2 = (const TPixel*)(srcPtr + (srcY2 * srcStride))
					, * srcPtrY3 = (const TPixel*)(srcPtr + (srcY3 * srcStride))
					, * srcPtrY4 = (const TPixel*)(srcPtr + (srcY4 * srcStride));

				color4f
					p11 = *(srcPtrY1 + srcX1), p12 = *(srcPtrY1 + srcX2), p13 = *(srcPtrY1 + srcX3), p14 = *(srcPtrY1 + srcX4),
					p21 = *(srcPtrY2 + srcX1), p22 = *(srcPtrY2 + srcX2), p23 = *(srcPtrY2 + srcX3), p24 = *(srcPtrY2 + srcX4),
					p31 = *(srcPtrY3 + srcX1), p32 = *(srcPtrY3 + srcX2), p33 = *(srcPtrY3 + srcX3), p34 = *(srcPtrY3 + srcX4),
					p41 = *(srcPtrY4 + srcX1), p42 = *(srcPtrY4 + srcX2), p43 = *(srcPtrY4 + srcX3), p44 = *(srcPtrY4 + srcX4);

				color4f p1, p2, p3, p4;
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
	size_t destStride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), destSize.width)
		, srcStride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), srcSize.width);
	size_t destDepth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), destSize.width, destSize.height)
		, srcDepth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), srcSize.width, srcSize.height);

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
				color4f sum;
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

						color4f srcPixel = *((const TPixel*)(srcPtr + (srcStride * iy)) + ix);
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
	{ rztp (resize_nearest, pixelformat::rgba8888), bmprsz_nearest<rgba> },
	{ rztp (resize_nearest, pixelformat::rgb888), bmprsz_nearest<rgb> },
	{ rztp (resize_nearest, pixelformat::rgbaf), bmprsz_nearest<rgbaf> },
	{ rztp (resize_nearest, pixelformat::bgra8888), bmprsz_nearest<bgra> },
	{ rztp (resize_nearest, pixelformat::bgr888), bmprsz_nearest<bgr> },
	{ rztp (resize_nearest, pixelformat::bgra4444), bmprsz_nearest<bgra4> },
	{ rztp (resize_nearest, pixelformat::bgr565), bmprsz_nearest<bgr565> },
	{ rztp (resize_nearest, pixelformat::grayscale8), bmprsz_nearest<grayscale> },
	{ rztp (resize_nearest, pixelformat::grayscalef), bmprsz_nearest<grayscalef> },
	{ rztp (resize_nearest, pixelformat::yuva8888), bmprsz_nearest<yuva> },
	{ rztp (resize_nearest, pixelformat::yuv888), bmprsz_nearest<yuv> },
	{ rztp (resize_nearest, pixelformat::hsva8888), bmprsz_nearest<hsva> },
	{ rztp (resize_nearest, pixelformat::hsv888), bmprsz_nearest<hsv> },

	{ rztp (resize_bilinear, pixelformat::rgba8888), bmprsz_bilinear<rgba> },
	{ rztp (resize_bilinear, pixelformat::rgb888), bmprsz_bilinear<rgb> },
	{ rztp (resize_bilinear, pixelformat::rgbaf), bmprsz_bilinear<rgbaf> },
	{ rztp (resize_bilinear, pixelformat::bgra8888), bmprsz_bilinear<bgra> },
	{ rztp (resize_bilinear, pixelformat::bgr888), bmprsz_bilinear<bgr> },
	{ rztp (resize_bilinear, pixelformat::bgra4444), bmprsz_bilinear<bgra4> },
	{ rztp (resize_bilinear, pixelformat::bgr565), bmprsz_bilinear<bgr565> },
	{ rztp (resize_bilinear, pixelformat::grayscale8), bmprsz_bilinear<grayscale> },
	{ rztp (resize_bilinear, pixelformat::grayscalef), bmprsz_bilinear<grayscalef> },
	{ rztp (resize_bilinear, pixelformat::yuva8888), bmprsz_bilinear<yuva> },
	{ rztp (resize_bilinear, pixelformat::yuv888), bmprsz_bilinear<yuv> },
	{ rztp (resize_bilinear, pixelformat::hsva8888), bmprsz_bilinear<hsva> },
	{ rztp (resize_bilinear, pixelformat::hsv888), bmprsz_bilinear<hsv> },

	{ rztp (resize_bicubic, pixelformat::rgba8888), bmprsz_bicubic<rgba> },
	{ rztp (resize_bicubic, pixelformat::rgb888), bmprsz_bicubic<rgb> },
	{ rztp (resize_bicubic, pixelformat::rgbaf), bmprsz_bicubic<rgbaf> },
	{ rztp (resize_bicubic, pixelformat::bgra8888), bmprsz_bicubic<bgra> },
	{ rztp (resize_bicubic, pixelformat::bgr888), bmprsz_bicubic<bgr> },
	{ rztp (resize_bicubic, pixelformat::bgra4444), bmprsz_bicubic<bgra4> },
	{ rztp (resize_bicubic, pixelformat::bgr565), bmprsz_bicubic<bgr565> },
	{ rztp (resize_bicubic, pixelformat::grayscale8), bmprsz_bicubic<grayscale> },
	{ rztp (resize_bicubic, pixelformat::grayscalef), bmprsz_bicubic<grayscalef> },
	{ rztp (resize_bicubic, pixelformat::yuva8888), bmprsz_bicubic<yuva> },
	{ rztp (resize_bicubic, pixelformat::yuv888), bmprsz_bicubic<yuv> },
	{ rztp (resize_bicubic, pixelformat::hsva8888), bmprsz_bicubic<hsva> },
	{ rztp (resize_bicubic, pixelformat::hsv888), bmprsz_bicubic<hsv> },

	{ rztp (resize_lanczos, pixelformat::rgba8888), bmprsz_lanczos<rgba, LANCZOS_WINDOW1> },
	{ rztp (resize_lanczos, pixelformat::rgb888), bmprsz_lanczos<rgb, LANCZOS_WINDOW1> },
	{ rztp (resize_lanczos, pixelformat::rgbaf), bmprsz_lanczos<rgbaf, LANCZOS_WINDOW1> },
	{ rztp (resize_lanczos, pixelformat::bgra8888), bmprsz_lanczos<bgra, LANCZOS_WINDOW1> },
	{ rztp (resize_lanczos, pixelformat::bgr888), bmprsz_lanczos<bgr, LANCZOS_WINDOW1> },
	{ rztp (resize_lanczos, pixelformat::bgra4444), bmprsz_lanczos<bgra4, LANCZOS_WINDOW1> },
	{ rztp (resize_lanczos, pixelformat::bgr565), bmprsz_lanczos<bgr565, LANCZOS_WINDOW1> },
	{ rztp (resize_lanczos, pixelformat::grayscale8), bmprsz_lanczos<grayscale, LANCZOS_WINDOW1> },
	{ rztp (resize_lanczos, pixelformat::grayscalef), bmprsz_lanczos<grayscalef, LANCZOS_WINDOW1> },
	{ rztp (resize_lanczos, pixelformat::yuva8888), bmprsz_lanczos<yuva, LANCZOS_WINDOW1> },
	{ rztp (resize_lanczos, pixelformat::yuv888), bmprsz_lanczos<yuv, LANCZOS_WINDOW1> },
	{ rztp (resize_lanczos, pixelformat::hsva8888), bmprsz_lanczos<hsva, LANCZOS_WINDOW1> },
	{ rztp (resize_lanczos, pixelformat::hsv888), bmprsz_lanczos<hsv, LANCZOS_WINDOW1> },

	{ rztp (resize_lanczos2, pixelformat::rgba8888), bmprsz_lanczos<rgba, LANCZOS_WINDOW2> },
	{ rztp (resize_lanczos2, pixelformat::rgb888), bmprsz_lanczos<rgb, LANCZOS_WINDOW2> },
	{ rztp (resize_lanczos2, pixelformat::rgbaf), bmprsz_lanczos<rgbaf, LANCZOS_WINDOW2> },
	{ rztp (resize_lanczos2, pixelformat::bgra8888), bmprsz_lanczos<bgra, LANCZOS_WINDOW2> },
	{ rztp (resize_lanczos2, pixelformat::bgr888), bmprsz_lanczos<bgr, LANCZOS_WINDOW2> },
	{ rztp (resize_lanczos2, pixelformat::bgra4444), bmprsz_lanczos<bgra4, LANCZOS_WINDOW2> },
	{ rztp (resize_lanczos2, pixelformat::bgr565), bmprsz_lanczos<bgr565, LANCZOS_WINDOW2> },
	{ rztp (resize_lanczos2, pixelformat::grayscale8), bmprsz_lanczos<grayscale, LANCZOS_WINDOW2> },
	{ rztp (resize_lanczos2, pixelformat::grayscalef), bmprsz_lanczos<grayscalef, LANCZOS_WINDOW2> },
	{ rztp (resize_lanczos2, pixelformat::yuva8888), bmprsz_lanczos<yuva, LANCZOS_WINDOW2> },
	{ rztp (resize_lanczos2, pixelformat::yuv888), bmprsz_lanczos<yuv, LANCZOS_WINDOW2> },
	{ rztp (resize_lanczos2, pixelformat::hsva8888), bmprsz_lanczos<hsva, LANCZOS_WINDOW2> },
	{ rztp (resize_lanczos2, pixelformat::hsv888), bmprsz_lanczos<hsv, LANCZOS_WINDOW2> },

	{ rztp (resize_lanczos3, pixelformat::rgba8888), bmprsz_lanczos<rgba, LANCZOS_WINDOW3> },
	{ rztp (resize_lanczos3, pixelformat::rgb888), bmprsz_lanczos<rgb, LANCZOS_WINDOW3> },
	{ rztp (resize_lanczos3, pixelformat::rgbaf), bmprsz_lanczos<rgbaf, LANCZOS_WINDOW3> },
	{ rztp (resize_lanczos3, pixelformat::bgra8888), bmprsz_lanczos<bgra, LANCZOS_WINDOW3> },
	{ rztp (resize_lanczos3, pixelformat::bgr888), bmprsz_lanczos<bgr, LANCZOS_WINDOW3> },
	{ rztp (resize_lanczos3, pixelformat::bgra4444), bmprsz_lanczos<bgra4, LANCZOS_WINDOW3> },
	{ rztp (resize_lanczos3, pixelformat::bgr565), bmprsz_lanczos<bgr565, LANCZOS_WINDOW3> },
	{ rztp (resize_lanczos3, pixelformat::grayscale8), bmprsz_lanczos<grayscale, LANCZOS_WINDOW3> },
	{ rztp (resize_lanczos3, pixelformat::grayscalef), bmprsz_lanczos<grayscalef, LANCZOS_WINDOW3> },
	{ rztp (resize_lanczos3, pixelformat::yuva8888), bmprsz_lanczos<yuva, LANCZOS_WINDOW3> },
	{ rztp (resize_lanczos3, pixelformat::yuv888), bmprsz_lanczos<yuv, LANCZOS_WINDOW3> },
	{ rztp (resize_lanczos3, pixelformat::hsva8888), bmprsz_lanczos<hsva, LANCZOS_WINDOW3> },
	{ rztp (resize_lanczos3, pixelformat::hsv888), bmprsz_lanczos<hsv, LANCZOS_WINDOW3> },

	{ rztp (resize_lanczos4, pixelformat::rgba8888), bmprsz_lanczos<rgba, LANCZOS_WINDOW4> },
	{ rztp (resize_lanczos4, pixelformat::rgb888), bmprsz_lanczos<rgb, LANCZOS_WINDOW4> },
	{ rztp (resize_lanczos4, pixelformat::rgbaf), bmprsz_lanczos<rgbaf, LANCZOS_WINDOW4> },
	{ rztp (resize_lanczos4, pixelformat::bgra8888), bmprsz_lanczos<bgra, LANCZOS_WINDOW4> },
	{ rztp (resize_lanczos4, pixelformat::bgr888), bmprsz_lanczos<bgr, LANCZOS_WINDOW4> },
	{ rztp (resize_lanczos4, pixelformat::bgra4444), bmprsz_lanczos<bgra4, LANCZOS_WINDOW4> },
	{ rztp (resize_lanczos4, pixelformat::bgr565), bmprsz_lanczos<bgr565, LANCZOS_WINDOW4> },
	{ rztp (resize_lanczos4, pixelformat::grayscale8), bmprsz_lanczos<grayscale, LANCZOS_WINDOW4> },
	{ rztp (resize_lanczos4, pixelformat::grayscalef), bmprsz_lanczos<grayscalef, LANCZOS_WINDOW4> },
	{ rztp (resize_lanczos4, pixelformat::yuva8888), bmprsz_lanczos<yuva, LANCZOS_WINDOW4> },
	{ rztp (resize_lanczos4, pixelformat::yuv888), bmprsz_lanczos<yuv, LANCZOS_WINDOW4> },
	{ rztp (resize_lanczos4, pixelformat::hsva8888), bmprsz_lanczos<hsva, LANCZOS_WINDOW4> },
	{ rztp (resize_lanczos4, pixelformat::hsv888), bmprsz_lanczos<hsv, LANCZOS_WINDOW4> },

	{ rztp (resize_lanczos5, pixelformat::rgba8888), bmprsz_lanczos<rgba, LANCZOS_WINDOW5> },
	{ rztp (resize_lanczos5, pixelformat::rgb888), bmprsz_lanczos<rgb, LANCZOS_WINDOW5> },
	{ rztp (resize_lanczos5, pixelformat::rgbaf), bmprsz_lanczos<rgbaf, LANCZOS_WINDOW5> },
	{ rztp (resize_lanczos5, pixelformat::bgra8888), bmprsz_lanczos<bgra, LANCZOS_WINDOW5> },
	{ rztp (resize_lanczos5, pixelformat::bgr888), bmprsz_lanczos<bgr, LANCZOS_WINDOW5> },
	{ rztp (resize_lanczos5, pixelformat::bgra4444), bmprsz_lanczos<bgra4, LANCZOS_WINDOW5> },
	{ rztp (resize_lanczos5, pixelformat::bgr565), bmprsz_lanczos<bgr565, LANCZOS_WINDOW5> },
	{ rztp (resize_lanczos5, pixelformat::grayscale8), bmprsz_lanczos<grayscale, LANCZOS_WINDOW5> },
	{ rztp (resize_lanczos5, pixelformat::grayscalef), bmprsz_lanczos<grayscalef, LANCZOS_WINDOW5> },
	{ rztp (resize_lanczos5, pixelformat::yuva8888), bmprsz_lanczos<yuva, LANCZOS_WINDOW5> },
	{ rztp (resize_lanczos5, pixelformat::yuv888), bmprsz_lanczos<yuv, LANCZOS_WINDOW5> },
	{ rztp (resize_lanczos5, pixelformat::hsva8888), bmprsz_lanczos<hsva, LANCZOS_WINDOW5> },
	{ rztp (resize_lanczos5, pixelformat::hsv888), bmprsz_lanczos<hsv, LANCZOS_WINDOW5> },
};

dseed::error_t dseed::resize_bitmap (dseed::bitmap* original, resize_t resize_method, const size3i& size, dseed::bitmap** bitmap)
{
	if (original == nullptr || bitmap == nullptr)
		return dseed::error_invalid_args;
	if (size.width == 0 || size.height == 0 || size.depth == 0)
		return dseed::error_invalid_args;

	dseed::auto_object<dseed::bitmap> temp;
	if (dseed::failed (dseed::create_bitmap (original->type (), size, original->format (), nullptr, &temp)))
		return dseed::error_fail;

	uint8_t* destPtr, * srcPtr;
	original->pixels_pointer ((void**)& srcPtr);
	temp->pixels_pointer ((void**)& destPtr);

	auto found = g_resizes.find (rztp (resize_method, original->format ()));
	if (found == g_resizes.end ())
		return dseed::error_not_support;

	if (!found->second (destPtr, srcPtr, size, original->size ()))
		return dseed::error_not_support;

	*bitmap = temp.detach ();

	return dseed::error_good;
}

using cpfn = std::function<bool (uint8_t* dest, const uint8_t* src, const dseed::size3i& size, const dseed::rectangle& area)>;

template<class TPixel>
inline bool crop_pixels (uint8_t* dest, const uint8_t* src, const dseed::size3i& size, const dseed::rectangle& area) noexcept
{
	size_t destStride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), area.width)
		, srcStride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), size.width);
	size_t destDepth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), area.width, area.height)
		, srcDepth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), size.width, size.height);

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

std::map<dseed::pixelformat, cpfn> g_crops = {
	{ pixelformat::rgba8888, crop_pixels<rgba> },
	{ pixelformat::rgb888, crop_pixels<rgb> },
	{ pixelformat::rgbaf, crop_pixels<rgbaf> },
	{ pixelformat::bgra8888, crop_pixels<bgra> },
	{ pixelformat::bgr888, crop_pixels<bgr> },
	{ pixelformat::bgra4444, crop_pixels<bgra4> },
	{ pixelformat::bgr565, crop_pixels<bgr565> },
	{ pixelformat::grayscale8, crop_pixels<grayscale> },
	{ pixelformat::grayscalef, crop_pixels<grayscalef> },
	{ pixelformat::yuva8888, crop_pixels<yuva> },
	{ pixelformat::yuv888, crop_pixels<yuv> },
};

error_t dseed::crop_bitmap (dseed::bitmap* original, const rectangle& area, dseed::bitmap** bitmap)
{
	if (original == nullptr || bitmap == nullptr)
		return dseed::error_invalid_args;

	auto size = original->size ();
	if (size.width < (area.width - area.x) ||
		size.height < (area.height - area.y) ||
		area.x < 0 || area.y < 0)
		return dseed::error_invalid_args;

	dseed::auto_object<dseed::bitmap> temp;
	if (dseed::failed (dseed::create_bitmap (original->type (), dseed::size3i (area.width - area.x, area.height - area.y, size.depth), original->format (), nullptr, &temp)))
		return dseed::error_fail;

	uint8_t* destPtr, * srcPtr;
	original->pixels_pointer ((void**)& srcPtr);
	temp->pixels_pointer ((void**)& destPtr);

	auto found = g_crops.find (original->format ());
	if (found == g_crops.end ())
		return dseed::error_not_support;

	if (!found->second (destPtr, srcPtr, size, area))
		return dseed::error_not_support;

	*bitmap = temp.detach ();

	return dseed::error_good;
}