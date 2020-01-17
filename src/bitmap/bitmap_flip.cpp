#include <dseed.h>

#include <map>
#include <tuple>

using namespace dseed::color;
using size2i = dseed::size2i;

enum __HV { __HV_HORIZONTAL, __HV_VERTICAL };

using fpfn = std::function<bool (uint8_t * dest, const uint8_t * src, const dseed::size3i & size)>;
using fptp = std::tuple<__HV, pixelformat>;

template<class TPixel>
inline bool bmpfp_horizontal (uint8_t* dest, const uint8_t* src, const dseed::size3i& size) noexcept
{
	size_t stride = calc_bitmap_stride (type2format<TPixel> (), size.width);
	size_t depth = calc_bitmap_plane_size (type2format<TPixel> (), size2i (size.width, size.height));

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
	size_t stride = calc_bitmap_stride (type2format<TPixel> (), size.width);
	size_t depth = calc_bitmap_plane_size (type2format<TPixel> (), size2i (size.width, size.height));

	for (size_t z = 0; z < size.depth; ++z)
	{
		size_t depthZ = depth * z;

		for (size_t y = 0; y < ceil (size.height / 2.0); ++y)
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
	{ fptp (__HV_HORIZONTAL, pixelformat::rgba8), bmpfp_horizontal<rgba8> },
	{ fptp (__HV_HORIZONTAL, pixelformat::rgb8), bmpfp_horizontal<rgb8> },
	{ fptp (__HV_HORIZONTAL, pixelformat::rgbaf), bmpfp_horizontal<rgbaf> },
	{ fptp (__HV_HORIZONTAL, pixelformat::bgra8), bmpfp_horizontal<bgra8> },
	{ fptp (__HV_HORIZONTAL, pixelformat::bgr8), bmpfp_horizontal<bgr8> },
	{ fptp (__HV_HORIZONTAL, pixelformat::bgra4), bmpfp_horizontal<bgra4> },
	{ fptp (__HV_HORIZONTAL, pixelformat::bgr565), bmpfp_horizontal<bgr565> },
	{ fptp (__HV_HORIZONTAL, pixelformat::r8), bmpfp_horizontal<r8> },
	{ fptp (__HV_HORIZONTAL, pixelformat::rf), bmpfp_horizontal<rf> },
	{ fptp (__HV_HORIZONTAL, pixelformat::yuva8), bmpfp_horizontal<yuva8> },
	{ fptp (__HV_HORIZONTAL, pixelformat::yuv8), bmpfp_horizontal<yuv8> },
	{ fptp (__HV_HORIZONTAL, pixelformat::hsva8), bmpfp_horizontal<hsva8> },
	{ fptp (__HV_HORIZONTAL, pixelformat::hsv8), bmpfp_horizontal<hsv8> },

	{ fptp (__HV_VERTICAL, pixelformat::rgba8), bmpfp_vertical<rgba8> },
	{ fptp (__HV_VERTICAL, pixelformat::rgb8), bmpfp_vertical<rgb8> },
	{ fptp (__HV_VERTICAL, pixelformat::rgbaf), bmpfp_vertical<rgbaf> },
	{ fptp (__HV_VERTICAL, pixelformat::bgra8), bmpfp_vertical<bgra8> },
	{ fptp (__HV_VERTICAL, pixelformat::bgr8), bmpfp_vertical<bgr8> },
	{ fptp (__HV_VERTICAL, pixelformat::bgra4), bmpfp_vertical<bgra4> },
	{ fptp (__HV_VERTICAL, pixelformat::bgr565), bmpfp_vertical<bgr565> },
	{ fptp (__HV_VERTICAL, pixelformat::r8), bmpfp_vertical<r8> },
	{ fptp (__HV_VERTICAL, pixelformat::rf), bmpfp_vertical<rf> },
	{ fptp (__HV_VERTICAL, pixelformat::yuva8), bmpfp_vertical<yuva8> },
	{ fptp (__HV_VERTICAL, pixelformat::yuv8), bmpfp_vertical<yuv8> },
	{ fptp (__HV_VERTICAL, pixelformat::hsva8), bmpfp_vertical<hsva8> },
	{ fptp (__HV_VERTICAL, pixelformat::hsv8), bmpfp_vertical<hsv8> },
};

dseed::error_t __internal_flip (dseed::bitmaps::bitmap* original, __HV hv, dseed::bitmaps::bitmap** bitmap)
{
	if (original == nullptr || bitmap == nullptr)
		return dseed::error_invalid_args;

	dseed::autoref<dseed::bitmaps::bitmap> temp;
	if (dseed::failed (dseed::bitmaps::create_bitmap (original->type (), original->size (), original->format (), nullptr, &temp)))
		return dseed::error_fail;

	uint8_t* destPtr, * srcPtr;
	original->lock ((void**)&srcPtr);
	temp->lock ((void**)&destPtr);

	auto found = g_flips.find (fptp (hv, original->format ()));
	if (found == g_flips.end ())
		return dseed::error_not_support;

	if (!found->second (destPtr, srcPtr, original->size ()))
		return dseed::error_not_support;

	temp->unlock ();
	original->unlock ();

	*bitmap = temp.detach ();

	return dseed::error_good;
}

dseed::error_t dseed::bitmaps::flip_horizontal_bitmap (dseed::bitmaps::bitmap* original, dseed::bitmaps::bitmap** bitmap)
{
	return __internal_flip (original, __HV_HORIZONTAL, bitmap);
}

dseed::error_t dseed::bitmaps::flip_vertical_bitmap (dseed::bitmaps::bitmap* original, dseed::bitmaps::bitmap** bitmap)
{
	return __internal_flip (original, __HV_VERTICAL, bitmap);
}