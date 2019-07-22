#include <dseed.h>

#include <vector>
#include <map>

using namespace dseed;

using rzfn = std::function<bool (uint8_t* dest, const uint8_t* src, const dseed::size3i& destSize, const dseed::size3i& srcSize)>;
using rztp = std::tuple<dseed::resize_t, dseed::pixelformat_t>;

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
			size_t srcY1 = y * yRatio;
			size_t srcY2 = (y + 1) * yRatio;
			double yDiff1 = (yRatio * y) - srcY1, yDiff2 = 1 - yDiff1;

			size_t destStrideY = y * destStride
				, srcStrideY1 = srcY1 * srcStride
				, srcStrideY2 = srcY2 * srcStride;
			TPixel* destPtr = (TPixel*)(dest + destDepthZ + destStrideY);
			const TPixel* srcPtr1 = (TPixel*)(src + srcDepthZ + srcStrideY1);
			const TPixel* srcPtr2 = (TPixel*)(src + srcDepthZ + srcStrideY2);

			for (size_t x = 0; x < destSize.width; ++x)
			{
				size_t srcX1 = x * xRatio;
				size_t srcX2 = (x + 1) * xRatio;
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

rgbaf cubic_hermite (const rgbaf& a, const rgbaf& b, const rgbaf& c, const rgbaf& d, float factor) noexcept
{
	rgbaf _a = (-a / 2.0f) + ((b * 3.0f) / 2.0f) - (c * 3.0f) / 2.0f + (d / 2.0f);
	rgbaf _b = a - ((b * 5.0f) / 2.0f) + (c * 2.0f) - (d / 2.0f);
	rgbaf _c = (-a / 2.0f) + (c / 2.0f);
	rgbaf _d = b;

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
			size_t srcY1 = clamp<size_t> ((y - 1) * yRatio, srcSize.height);
			size_t srcY2 = clamp<size_t> ((y + 0) * yRatio, srcSize.height);
			size_t srcY3 = clamp<size_t> ((y + 1) * yRatio, srcSize.height);
			size_t srcY4 = clamp<size_t> ((y + 2) * yRatio, srcSize.height);
			double yDiff1 = (yRatio * y) - srcY2, yDiff2 = 1 - yDiff1;

			size_t destStrideY = y * destStride;
			TPixel* destPtr = (TPixel*)(dest + destDepthZ + destStrideY);

			for (size_t x = 0; x < destSize.width; ++x)
			{
				size_t srcX1 = clamp<size_t> ((x - 1) * xRatio, srcSize.width);
				size_t srcX2 = clamp<size_t> ((x + 0) * xRatio, srcSize.width);
				size_t srcX3 = clamp<size_t> ((x + 1) * xRatio, srcSize.width);
				size_t srcX4 = clamp<size_t> ((x + 2) * xRatio, srcSize.width);
				double xDiff1 = (xRatio * x) - srcX2, xDiff2 = 1 - xDiff1;

				const TPixel* srcPtrY1 = (const TPixel*)(srcPtr + (srcY1 * srcStride))
					, * srcPtrY2 = (const TPixel*)(srcPtr + (srcY2 * srcStride))
					, * srcPtrY3 = (const TPixel*)(srcPtr + (srcY3 * srcStride))
					, * srcPtrY4 = (const TPixel*)(srcPtr + (srcY4 * srcStride));

				rgbaf
					p11 = *(srcPtrY1 + srcX1), p12 = *(srcPtrY1 + srcX2), p13 = *(srcPtrY1 + srcX3), p14 = *(srcPtrY1 + srcX4),
					p21 = *(srcPtrY2 + srcX1), p22 = *(srcPtrY2 + srcX2), p23 = *(srcPtrY2 + srcX3), p24 = *(srcPtrY2 + srcX4),
					p31 = *(srcPtrY3 + srcX1), p32 = *(srcPtrY3 + srcX2), p33 = *(srcPtrY3 + srcX3), p34 = *(srcPtrY3 + srcX4),
					p41 = *(srcPtrY4 + srcX1), p42 = *(srcPtrY4 + srcX2), p43 = *(srcPtrY4 + srcX3), p44 = *(srcPtrY4 + srcX4);

				rgbaf p1, p2, p3, p4;
				p1 = cubic_hermite (p11, p12, p13, p14, xDiff1);
				p2 = cubic_hermite (p21, p22, p23, p24, xDiff1);
				p3 = cubic_hermite (p31, p32, p33, p34, xDiff1);
				p4 = cubic_hermite (p41, p42, p43, p44, xDiff1);
				rgbaf calced = cubic_hermite (p1, p2, p3, p4, yDiff1);
				calced.r = saturate (calced.r);
				calced.g = saturate (calced.g);
				calced.b = saturate (calced.b);
				calced.a = saturate (calced.a);

				TPixel* destPtrX = destPtr + x;
				*destPtrX = calced;
			}
		}
	}

	return true;
}

std::map<rztp, rzfn> g_resizes = {
	{ rztp (resize_nearest, pixelformat_rgba8888), bmprsz_nearest<rgba> },
	{ rztp (resize_nearest, pixelformat_rgb888), bmprsz_nearest<rgb> },
	{ rztp (resize_nearest, pixelformat_rgbaf), bmprsz_nearest<rgbaf> },
	{ rztp (resize_nearest, pixelformat_bgra8888), bmprsz_nearest<bgra> },
	{ rztp (resize_nearest, pixelformat_bgr888), bmprsz_nearest<bgr> },
	{ rztp (resize_nearest, pixelformat_bgra4444), bmprsz_nearest<bgra4> },
	{ rztp (resize_nearest, pixelformat_bgr565), bmprsz_nearest<bgr565> },
	{ rztp (resize_nearest, pixelformat_grayscale8), bmprsz_nearest<grayscale> },
	{ rztp (resize_nearest, pixelformat_grayscalef), bmprsz_nearest<grayscalef> },
	{ rztp (resize_nearest, pixelformat_yuva8888), bmprsz_nearest<yuva> },
	{ rztp (resize_nearest, pixelformat_yuv888), bmprsz_nearest<yuv> },

	{ rztp (resize_bilinear, pixelformat_rgba8888), bmprsz_bilinear<rgba> },
	{ rztp (resize_bilinear, pixelformat_rgb888), bmprsz_bilinear<rgb> },
	{ rztp (resize_bilinear, pixelformat_rgbaf), bmprsz_bilinear<rgbaf> },
	{ rztp (resize_bilinear, pixelformat_bgra8888), bmprsz_bilinear<bgra> },
	{ rztp (resize_bilinear, pixelformat_bgr888), bmprsz_bilinear<bgr> },
	{ rztp (resize_bilinear, pixelformat_bgra4444), bmprsz_bilinear<bgra4> },
	{ rztp (resize_bilinear, pixelformat_bgr565), bmprsz_bilinear<bgr565> },
	{ rztp (resize_bilinear, pixelformat_grayscale8), bmprsz_bilinear<grayscale> },
	{ rztp (resize_bilinear, pixelformat_grayscalef), bmprsz_bilinear<grayscalef> },
	{ rztp (resize_bilinear, pixelformat_yuva8888), bmprsz_bilinear<yuva> },
	{ rztp (resize_bilinear, pixelformat_yuv888), bmprsz_bilinear<yuv> },

	{ rztp (resize_bicubic, pixelformat_rgba8888), bmprsz_bicubic<rgba> },
	{ rztp (resize_bicubic, pixelformat_rgb888), bmprsz_bicubic<rgb> },
	{ rztp (resize_bicubic, pixelformat_rgbaf), bmprsz_bicubic<rgbaf> },
	{ rztp (resize_bicubic, pixelformat_bgra8888), bmprsz_bicubic<bgra> },
	{ rztp (resize_bicubic, pixelformat_bgr888), bmprsz_bicubic<bgr> },
	{ rztp (resize_bicubic, pixelformat_bgra4444), bmprsz_bicubic<bgra4> },
	{ rztp (resize_bicubic, pixelformat_bgr565), bmprsz_bicubic<bgr565> },
	{ rztp (resize_bicubic, pixelformat_grayscale8), bmprsz_bicubic<grayscale> },
	{ rztp (resize_bicubic, pixelformat_grayscalef), bmprsz_bicubic<grayscalef> },
	{ rztp (resize_bicubic, pixelformat_yuva8888), bmprsz_bicubic<yuva> },
	{ rztp (resize_bicubic, pixelformat_yuv888), bmprsz_bicubic<yuv> },
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