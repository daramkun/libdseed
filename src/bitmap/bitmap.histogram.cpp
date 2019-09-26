#include <dseed.h>

#include <map>

using ghfn = std::function<bool (dseed::histogram*, const uint8_t*, const dseed::size3i&, uint32_t, dseed::histogram_color_t)>;
using ghtp = dseed::pixelformat_t;

template<class TPixel>
inline bool gen_histogram (dseed::histogram* histogram, const uint8_t* src, const dseed::size3i& size, uint32_t targetDepth, dseed::histogram_color_t c) noexcept
{
	size_t stride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), size.width);
	size_t depth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), size.width, size.height);

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
	{ dseed::pixelformat_rgba8888, gen_histogram<dseed::rgba> },
	{ dseed::pixelformat_rgb888, gen_histogram<dseed::rgb> },
	{ dseed::pixelformat_bgra8888, gen_histogram<dseed::bgra> },
	{ dseed::pixelformat_bgr888, gen_histogram<dseed::bgr> },
	{ dseed::pixelformat_grayscale8, gen_histogram<dseed::grayscale> },
	{ dseed::pixelformat_yuva8888, gen_histogram<dseed::yuva> },
	{ dseed::pixelformat_yuv888, gen_histogram<dseed::yuv> },
	{ dseed::pixelformat_hsva8888, gen_histogram<dseed::hsva> },
	{ dseed::pixelformat_hsv888, gen_histogram<dseed::hsv> },
};

dseed::error_t dseed::bitmap_generate_histogram (dseed::bitmap* original, histogram_color_t color, uint32_t depth, histogram* histogram)
{
	if (original == nullptr || histogram == nullptr)
		return dseed::error_invalid_args;

	auto format = original->format ();
	if (color > dseed::histogram_color_fourth || color < dseed::histogram_color_first)
		return dseed::error_invalid_args;
	if (color >= dseed::histogram_color_second && (format == dseed::pixelformat_grayscale8))
		return dseed::error_invalid_args;
	if (color >= dseed::histogram_color_fourth && (format == dseed::pixelformat_rgb888
		|| format == dseed::pixelformat_bgr888 || format == dseed::pixelformat_yuv888
		|| format == dseed::pixelformat_hsv888))
		return dseed::error_invalid_args;

	const uint8_t* srcPtr;
	original->pixels_pointer ((void**)&srcPtr);

	auto size = original->size ();

	auto found = g_ghs.find (format);
	if (found == g_ghs.end ())
		return dseed::error_not_support;

	if (!found->second (histogram, srcPtr, size, depth, color))
		return dseed::error_fail;

	return dseed::error_good;
}

dseed::error_t dseed::histogram_equalization (histogram* histogram)
{
	int total = 0;
	for (int i = 0; i < 256; ++i)
	{
		int current = histogram->histogram[i];
		total += current;
		histogram->histogram_table[i] = (int)round (total / (double)histogram->total_pixels);
	}
	histogram->calced_table = true;

	memset (histogram->histogram, 0, sizeof (histogram->histogram));

	for (int i = 0; i < 256; ++i)
		++histogram->histogram[histogram->histogram_table[i]];

	for (int i = 0; i < 256; ++i)
		histogram->histogram[i] = (int)((histogram->histogram[i] / 255.0f) * histogram->total_pixels);

	return dseed::error_good;
}

using ahfn = std::function<bool (const dseed::histogram*, uint8_t*, const uint8_t*, const dseed::size3i&, uint32_t, dseed::histogram_color_t)>;
using ahtp = dseed::pixelformat_t;

template<class TPixel>
inline bool apply_histogram (const dseed::histogram* histogram, uint8_t* dest, const uint8_t* src, const dseed::size3i& size, uint32_t targetDepth, dseed::histogram_color_t c) noexcept
{
	size_t stride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), size.width);
	size_t depth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), size.width, size.height);

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
	{ dseed::pixelformat_rgba8888, apply_histogram<dseed::rgba> },
	{ dseed::pixelformat_rgb888, apply_histogram<dseed::rgb> },
	{ dseed::pixelformat_bgra8888, apply_histogram<dseed::bgra> },
	{ dseed::pixelformat_bgr888, apply_histogram<dseed::bgr> },
	{ dseed::pixelformat_grayscale8, apply_histogram<dseed::grayscale> },
	{ dseed::pixelformat_yuva8888, apply_histogram<dseed::yuva> },
	{ dseed::pixelformat_yuv888, apply_histogram<dseed::yuv> },
	{ dseed::pixelformat_hsva8888, apply_histogram<dseed::hsva> },
	{ dseed::pixelformat_hsv888, apply_histogram<dseed::hsv> },
};

dseed::error_t dseed::bitmap_apply_histogram (dseed::bitmap* original, histogram_color_t color, uint32_t depth, const histogram* histogram, dseed::bitmap** bitmap)
{
	if (original == nullptr || histogram == nullptr || bitmap == nullptr)
		return dseed::error_invalid_args;
	if (!histogram->calced_table)
		return dseed::error_invalid_args;

	auto format = original->format ();
	if (color > dseed::histogram_color_fourth || color < dseed::histogram_color_first)
		return dseed::error_invalid_args;
	if (color >= dseed::histogram_color_second && (format == dseed::pixelformat_grayscale8))
		return dseed::error_invalid_args;
	if (color >= dseed::histogram_color_fourth && (format == dseed::pixelformat_rgb888
		|| format == dseed::pixelformat_bgr888 || format == dseed::pixelformat_yuv888
		|| format == dseed::pixelformat_hsv888))
		return dseed::error_invalid_args;

	auto size = original->size ();

	dseed::auto_object<dseed::bitmap> temp;
	if (dseed::failed (dseed::create_bitmap (original->type (), size, format, nullptr, &temp)))
		return dseed::error_fail;

	uint8_t* destPtr;
	const uint8_t* srcPtr;
	original->pixels_pointer ((void**)&srcPtr);
	temp->pixels_pointer ((void**)&destPtr);

	auto found = g_ahs.find (format);
	if (found == g_ahs.end ())
		return dseed::error_not_support;

	if (!found->second (histogram, destPtr, srcPtr, size, depth, color))
		return dseed::error_fail;

	*bitmap = temp.detach ();

	return dseed::error_good;
}
