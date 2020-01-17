#include <dseed.h>

#include <map>
#include <tuple>

using namespace dseed::color;
using size2i = dseed::size2i;

using ghfn = std::function<bool (dseed::bitmaps::histogram*, const uint8_t*, const dseed::size3i&, uint32_t, dseed::bitmaps::histogram_color)>;
using ghtp = pixelformat;

template<class TPixel>
inline bool gen_histogram (dseed::bitmaps::histogram* histogram, const uint8_t* src, const dseed::size3i& size, uint32_t targetDepth, dseed::bitmaps::histogram_color c) noexcept
{
	size_t stride = calc_bitmap_stride (type2format<TPixel> (), size.width);
	size_t depth = calc_bitmap_plane_size (type2format<TPixel> (), size2i (size.width, size.height));

	histogram->total_pixels = size.width * size.height;

	size_t depthZ = targetDepth * depth;
	for (size_t y = 0; y < size.height; ++y)
	{
		size_t strideY = y * stride;

		const TPixel* srcPtr = (const TPixel*)(src + depthZ + strideY);

		for (size_t x = 0; x < size.width; ++x)
		{
			const TPixel& pixel = *(srcPtr + x);
			++histogram->histogram[pixel[(int)c]];
		}
	}

	return true;
}

std::map<ghtp, ghfn> g_ghs = {
	{ pixelformat::rgba8, gen_histogram<rgba8> },
	{ pixelformat::rgb8, gen_histogram<rgb8> },
	{ pixelformat::bgra8, gen_histogram<bgra8> },
	{ pixelformat::bgr8, gen_histogram<bgr8> },
	{ pixelformat::r8, gen_histogram<r8> },
	{ pixelformat::yuva8, gen_histogram<yuva8> },
	{ pixelformat::yuv8, gen_histogram<yuv8> },
	{ pixelformat::hsva8, gen_histogram<hsva8> },
	{ pixelformat::hsv8, gen_histogram<hsv8> },
};

dseed::error_t dseed::bitmaps::bitmap_generate_histogram (dseed::bitmaps::bitmap* original, histogram_color color, uint32_t depth, histogram* histogram)
{
	if (original == nullptr || histogram == nullptr)
		return dseed::error_invalid_args;

	auto format = original->format ();
	if (color > histogram_color::fourth || color < histogram_color::first)
		return dseed::error_invalid_args;
	if (color >= histogram_color::second && (format == pixelformat::r8))
		return dseed::error_invalid_args;
	if (color >= histogram_color::fourth && (format == pixelformat::rgb8
		|| format == pixelformat::bgr8 || format == pixelformat::yuv8
		|| format == pixelformat::hsv8))
		return dseed::error_invalid_args;

	const uint8_t* srcPtr;
	original->lock ((void**)&srcPtr);

	auto size = original->size ();

	auto found = g_ghs.find (format);
	if (found == g_ghs.end ())
		return dseed::error_not_support;

	if (!found->second (histogram, srcPtr, size, depth, color))
		return dseed::error_fail;

	original->unlock ();

	return dseed::error_good;
}

dseed::error_t dseed::bitmaps::histogram_equalization (histogram* histogram)
{
	int total = 0;
	for (int i = 0; i < 256; ++i)
	{
		int current = histogram->histogram[i];
		total += current;
		histogram->histogram_table[i] = (int)roundf (total / (double)histogram->total_pixels * 255);
	}
	histogram->calced_table = true;

	return dseed::error_good;
}

using ahfn = std::function<bool (const dseed::bitmaps::histogram*, uint8_t*, const uint8_t*, const dseed::size3i&, uint32_t, dseed::bitmaps::histogram_color)>;
using ahtp = pixelformat;

template<class TPixel>
inline bool apply_histogram (const dseed::bitmaps::histogram* histogram, uint8_t* dest, const uint8_t* src, const dseed::size3i& size, uint32_t targetDepth, dseed::bitmaps::histogram_color c) noexcept
{
	size_t stride = calc_bitmap_stride (type2format<TPixel> (), size.width);
	size_t depth = calc_bitmap_plane_size (type2format<TPixel> (), size2i (size.width, size.height));

	size_t depthZ = targetDepth * depth;
	for (size_t y = 0; y < size.height; ++y)
	{
		size_t strideY = y * stride;

		TPixel* destPtr = (TPixel*)(dest + depthZ + strideY);
		const TPixel* srcPtr = (const TPixel*)(src + depthZ + strideY);

		for (size_t x = 0; x < size.width; ++x)
		{
			const TPixel& pixel = *(srcPtr + x);
			TPixel& dest = *(destPtr + x);
			dest = pixel;
			dest[(int)c] = histogram->histogram_table[pixel[(int)c]];
		}
	}

	return true;
}

std::map<ahtp, ahfn> g_ahs = {
	{ pixelformat::rgba8, apply_histogram<rgba8> },
	{ pixelformat::rgb8, apply_histogram<rgb8> },
	{ pixelformat::bgra8, apply_histogram<bgra8> },
	{ pixelformat::bgr8, apply_histogram<bgr8> },
	{ pixelformat::r8, apply_histogram<r8> },
	{ pixelformat::yuva8, apply_histogram<yuva8> },
	{ pixelformat::yuv8, apply_histogram<yuv8> },
	{ pixelformat::hsva8, apply_histogram<hsva8> },
	{ pixelformat::hsv8, apply_histogram<hsv8> },
};

dseed::error_t dseed::bitmaps::bitmap_apply_histogram (dseed::bitmaps::bitmap* original, histogram_color color, uint32_t depth, const histogram* histogram, dseed::bitmaps::bitmap** bitmap)
{
	if (original == nullptr || histogram == nullptr || bitmap == nullptr)
		return dseed::error_invalid_args;
	if (!histogram->calced_table)
		return dseed::error_invalid_args;

	auto format = original->format ();
	if (color > histogram_color::fourth || color < histogram_color::first)
		return dseed::error_invalid_args;
	if (color >= histogram_color::second && (format == pixelformat::r8))
		return dseed::error_invalid_args;
	if (color >= histogram_color::fourth && (format == pixelformat::rgb8
		|| format == pixelformat::bgr8 || format == pixelformat::yuv8
		|| format == pixelformat::hsv8))
		return dseed::error_invalid_args;

	auto size = original->size ();

	dseed::autoref<dseed::bitmaps::bitmap> temp;
	if (dseed::failed (dseed::bitmaps::create_bitmap (original->type (), size, format, nullptr, &temp)))
		return dseed::error_fail;

	uint8_t* destPtr;
	const uint8_t* srcPtr;
	original->lock ((void**)&srcPtr);
	temp->lock ((void**)&destPtr);

	auto found = g_ahs.find (format);
	if (found == g_ahs.end ())
		return dseed::error_not_support;

	if (!found->second (histogram, destPtr, srcPtr, size, depth, color))
		return dseed::error_fail;

	temp->unlock ();
	original->unlock ();

	*bitmap = temp.detach ();

	return dseed::error_good;
}

dseed::error_t dseed::bitmaps::bitmap_auto_histogram_equalization (dseed::bitmaps::bitmap* original, histogram_color color, uint32_t depth, dseed::bitmaps::bitmap** bitmap)
{
	dseed::bitmaps::histogram histogram = {};

	if (dseed::failed (dseed::bitmaps::bitmap_generate_histogram (original, color, depth, &histogram)))
		return dseed::error_fail;

	if (dseed::failed (dseed::bitmaps::histogram_equalization (&histogram)))
		return dseed::error_fail;

	if (dseed::failed (dseed::bitmaps::bitmap_apply_histogram (original, color, depth, &histogram, bitmap)))
		return dseed::error_fail;

	return dseed::error_good;
}