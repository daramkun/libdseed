#include <dseed.h>

#include <map>

using namespace dseed;

enum __HV { __HV_HORIZONTAL, __HV_VERTICAL };

using fpfn = std::function<bool (uint8_t* dest, const uint8_t* src, const dseed::size3i& size)>;
using fptp = std::tuple<__HV, dseed::pixelformat_t>;

template<class TPixel>
inline bool bmpfp_horizontal (uint8_t* dest, const uint8_t* src, const dseed::size3i& size) noexcept
{
	size_t stride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), size.width);
	size_t depth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), size.width, size.height);

	for (size_t z = 0; z < size.depth; ++z)
	{
		size_t depthZ = depth * z;

		for (size_t y = 0; y < size.height; ++y)
		{
			size_t strideY = y * stride;
			TPixel* destPtr = (TPixel*)(dest + depthZ + strideY);
			const TPixel* srcPtr = (const TPixel*)(dest + depthZ + strideY);

			for (size_t x = 0; x < ceil (size.width / 2.0); ++x)
			{
				size_t srcX = (size.width - x - 1);
				*(destPtr + x) = *(srcPtr + srcX);
			}
		}
	}

	return true;
}

template<class TPixel>
inline bool bmpfp_vertical (uint8_t* dest, const uint8_t* src, const dseed::size3i& size) noexcept
{
	size_t stride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), size.width);
	size_t depth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), size.width, size.height);

	for (size_t z = 0; z < size.depth; ++z)
	{
		size_t depthZ = depth * z;

		for (size_t y = 0; y < ceil(size.height / 2.0); ++y)
		{
			size_t destStrideY = y * stride;
			size_t srcStrideY = (size.height - y - 1) * stride;
			TPixel* destPtr = (TPixel*)(dest + depthZ + destStrideY);
			const TPixel* srcPtr = (const TPixel*)(src + depthZ + srcStrideY);

			memcpy (destPtr, srcPtr, stride);
		}
	}

	return true;
}

std::map<fptp, fpfn> g_flips = {
	{ fptp (__HV_HORIZONTAL, pixelformat_rgba8888), bmpfp_horizontal<rgba> },
	{ fptp (__HV_HORIZONTAL, pixelformat_rgb888), bmpfp_horizontal<rgb> },
	{ fptp (__HV_HORIZONTAL, pixelformat_rgbaf), bmpfp_horizontal<rgbaf> },
	{ fptp (__HV_HORIZONTAL, pixelformat_bgra8888), bmpfp_horizontal<bgra> },
	{ fptp (__HV_HORIZONTAL, pixelformat_bgr888), bmpfp_horizontal<bgr> },
	{ fptp (__HV_HORIZONTAL, pixelformat_bgra4444), bmpfp_horizontal<bgra4> },
	{ fptp (__HV_HORIZONTAL, pixelformat_bgr565), bmpfp_horizontal<bgr565> },
	{ fptp (__HV_HORIZONTAL, pixelformat_grayscale8), bmpfp_horizontal<grayscale> },
	{ fptp (__HV_HORIZONTAL, pixelformat_grayscalef), bmpfp_horizontal<grayscalef> },
	{ fptp (__HV_HORIZONTAL, pixelformat_yuva8888), bmpfp_horizontal<yuva> },
	{ fptp (__HV_HORIZONTAL, pixelformat_yuv888), bmpfp_horizontal<yuv> },

	{ fptp (__HV_VERTICAL, pixelformat_rgba8888), bmpfp_vertical<rgba> },
	{ fptp (__HV_VERTICAL, pixelformat_rgb888), bmpfp_vertical<rgb> },
	{ fptp (__HV_VERTICAL, pixelformat_rgbaf), bmpfp_vertical<rgbaf> },
	{ fptp (__HV_VERTICAL, pixelformat_bgra8888), bmpfp_vertical<bgra> },
	{ fptp (__HV_VERTICAL, pixelformat_bgr888), bmpfp_vertical<bgr> },
	{ fptp (__HV_VERTICAL, pixelformat_bgra4444), bmpfp_vertical<bgra4> },
	{ fptp (__HV_VERTICAL, pixelformat_bgr565), bmpfp_vertical<bgr565> },
	{ fptp (__HV_VERTICAL, pixelformat_grayscale8), bmpfp_vertical<grayscale> },
	{ fptp (__HV_VERTICAL, pixelformat_grayscalef), bmpfp_vertical<grayscalef> },
	{ fptp (__HV_VERTICAL, pixelformat_yuva8888), bmpfp_vertical<yuva> },
	{ fptp (__HV_VERTICAL, pixelformat_yuv888), bmpfp_vertical<yuv> },
};

dseed::error_t __internal_flip (dseed::bitmap* original, __HV hv, dseed::bitmap** bitmap)
{
	if (original == nullptr || bitmap == nullptr)
		return dseed::error_invalid_args;

	dseed::auto_object<dseed::bitmap> temp;
	if (dseed::failed (dseed::create_bitmap (original->type (), original->size (), original->format (), nullptr, &temp)))
		return dseed::error_fail;

	uint8_t* destPtr, * srcPtr;
	original->pixels_pointer ((void**)& srcPtr);
	temp->pixels_pointer ((void**)& destPtr);

	auto found = g_flips.find (fptp (hv, original->format ()));
	if (found == g_flips.end ())
		return dseed::error_not_support;

	if (!found->second (destPtr, srcPtr, original->size ()))
		return dseed::error_not_support;

	*bitmap = temp.detach ();

	return dseed::error_good;
}

dseed::error_t dseed::flip_horizontal_bitmap (dseed::bitmap* original, dseed::bitmap** bitmap)
{
	return __internal_flip (original, __HV_HORIZONTAL, bitmap);
}

dseed::error_t dseed::flip_vertical_bitmap (dseed::bitmap* original, dseed::bitmap** bitmap)
{
	return __internal_flip (original, __HV_VERTICAL, bitmap);
}