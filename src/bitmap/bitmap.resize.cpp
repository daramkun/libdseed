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

			double yWeight1 = (srcY2 == 0) ? 0.5 : (srcY1 / srcY2);
			double yWeight2 = 1 - yWeight1;

			for (size_t x = 0; x < destSize.width; ++x)
			{
				size_t srcX1 = x * xRatio;
				size_t srcX2 = (x + 1) * xRatio;
				double xDiff1 = (xRatio * x) - srcX1, xDiff2 = 1 - xDiff1;
				
				TPixel* destPtrX = destPtr + x;
				const TPixel* srcPtrX1 = srcPtr1 + srcX1;
				const TPixel* srcPtrX2 = srcPtr1 + srcX2;

				TPixel srcY1XColor1 = (*srcPtrX1) * xDiff2;
				TPixel srcY1XColor2 = (*srcPtrX2) * xDiff1;
				TPixel srcY1XColorSum = srcY1XColor1 + srcY1XColor2;

				srcPtrX1 = srcPtr2 + srcX1;
				srcPtrX2 = srcPtr2 + srcX2;

				TPixel srcY2XColor1 = (*srcPtrX1) * xDiff2;
				TPixel srcY2XColor2 = (*srcPtrX2) * xDiff1;
				TPixel srcY2XColorSum = srcY2XColor1 + srcY2XColor2;

				TPixel srcColor1 = srcY1XColorSum * yDiff2;
				TPixel srcColor2 = srcY2XColorSum * yDiff1;
				TPixel srcColorSum = srcColor1 + srcColor2;

				*destPtrX = srcColorSum;
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