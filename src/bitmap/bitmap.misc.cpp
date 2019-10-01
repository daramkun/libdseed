#include <dseed.h>

#include <map>

////////////////////////////////////////////////////////////////////////////////////////////
//
// Transparent Detection
//
////////////////////////////////////////////////////////////////////////////////////////////

using dtfn = std::function<bool (const uint8_t * src, const dseed::size3i & size)>;
using dttp = dseed::pixelformat_t;

template<class TPixel>
inline bool detect_transparent (const uint8_t* src, const dseed::size3i& size)
{
	size_t stride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), size.width);
	size_t depth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), size.width, size.height);

	for (size_t z = 0; z < size.depth; ++z)
	{
		size_t depthZ = z * depth;
		for (size_t y = 0; y < size.height; ++y)
		{
			size_t strideY = y * stride;

			const TPixel* srcPtr = (const TPixel*)(src + depthZ + strideY);

			for (size_t x = 0; x < size.width; ++x)
			{
				const TPixel& pixel = *(srcPtr + x);
				if (pixel[3] == TPixel::max_color ()[3])
					return true;
			}
		}
	}

	return false;
}

std::map<dttp, dtfn> g_dts = {
	{ dseed::pixelformat_rgba8888, detect_transparent<dseed::rgba> },
	{ dseed::pixelformat_rgbaf, detect_transparent<dseed::rgbaf> },
	{ dseed::pixelformat_bgra8888, detect_transparent<dseed::bgra> },
	{ dseed::pixelformat_yuva8888, detect_transparent<dseed::yuva> },
	{ dseed::pixelformat_hsva8888, detect_transparent<dseed::hsva> },
};

dseed::error_t dseed::bitmap_detect_transparent (dseed::bitmap* bitmap, bool* transparent)
{
	if (bitmap == nullptr || transparent == nullptr)
		return dseed::error_invalid_args;

	auto format = bitmap->format ();
	if (!(format == dseed::pixelformat_rgba8888 || format == dseed::pixelformat_rgbaf
		|| format == dseed::pixelformat_bgra8888 || format == dseed::pixelformat_bgra4444
		|| format == dseed::pixelformat_yuva8888 || format == dseed::pixelformat_hsva8888
		|| format == dseed::pixelformat_bgra8888_indexed8))
	{
		*transparent = false;
		return dseed::error_good;
	}

	if (format == dseed::pixelformat_bgra8888_indexed8)
	{
		dseed::auto_object<dseed::palette> palette;
		bitmap->palette (&palette);

		dseed::bgra pixels[256];
		if (dseed::failed (palette->pixels_pointer ((void**)&pixels)))
			return dseed::error_fail;

		for (dseed::bgra& pixel : pixels)
		{
			if (pixel.a != 255)
			{
				*transparent = true;
				return dseed::error_good;
			}
		}
	}
	else
	{
		uint8_t* srcPtr;
		if (dseed::failed (bitmap->pixels_pointer ((void**)&srcPtr)))
			return dseed::error_fail;

		auto size = bitmap->size ();

		auto found = g_dts.find (format);
		if (found == g_dts.end ())
			return dseed::error_not_support;

		*transparent = found->second (srcPtr, size);
	}
	return dseed::error_good;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Grayscale Detection
//
////////////////////////////////////////////////////////////////////////////////////////////

using dgfn = std::function<bool (const uint8_t * src, const dseed::size3i & size, int threshold)>;
using dgtp = dseed::pixelformat_t;

template<class TPixel>
bool detect_grayscale (const uint8_t* src, const dseed::size3i& size, int threshold)
{
	size_t stride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), size.width);
	size_t depth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), size.width, size.height);

	for (size_t z = 0; z < size.depth; ++z)
	{
		size_t depthZ = z * depth;
		for (size_t y = 0; y < size.height; ++y)
		{
			size_t strideY = y * stride;

			const TPixel* srcPtr = (const TPixel*)(src + depthZ + strideY);

			for (size_t x = 0; x < size.width; ++x)
			{
				const TPixel& pixel = *(srcPtr + x);
				dseed::rgb rgb = (dseed::rgb) pixel;
				if (abs (rgb.r - rgb.g) > threshold)
					return false;
				if (abs (rgb.r - rgb.b) > threshold)
					return false;
			}
		}
	}

	return true;
}

std::map<dgtp, dgfn> g_dgs = {
	{ dseed::pixelformat_rgba8888, detect_grayscale<dseed::rgba> },
	{ dseed::pixelformat_rgbaf, detect_grayscale<dseed::rgbaf> },
	{ dseed::pixelformat_rgb888, detect_grayscale<dseed::rgb> },
	{ dseed::pixelformat_bgra8888, detect_grayscale<dseed::bgra> },
	{ dseed::pixelformat_bgr888, detect_grayscale<dseed::bgr> },
	{ dseed::pixelformat_bgra4444, detect_grayscale<dseed::bgra4> },
	{ dseed::pixelformat_bgr565, detect_grayscale<dseed::bgr565> },
	{ dseed::pixelformat_yuva8888, detect_grayscale<dseed::yuva> },
	{ dseed::pixelformat_yuv888, detect_grayscale<dseed::yuv> },
	{ dseed::pixelformat_hsva8888, detect_grayscale<dseed::hsva> },
	{ dseed::pixelformat_hsv888, detect_grayscale<dseed::hsv> },
};

template<class TPixel>
bool detect_grayscale_indexed (const uint8_t* src, int bpp, int elements, int threshold)
{
	for (int i = 0; i < elements; ++i)
	{
		TPixel& pixel = *(TPixel*)(src + i * bpp);
		dseed::rgb rgb = (dseed::rgb) pixel;
		if (abs (rgb.r - rgb.g) > threshold)
			return false;
		if (abs (rgb.r - rgb.b) > threshold)
			return false;
	}
	return true;
}

dseed::error_t dseed::bitmap_detect_grayscale_bitmap (dseed::bitmap* bitmap, bool* grayscale, int threshold)
{
	if (bitmap == nullptr || grayscale == nullptr)
		return dseed::error_invalid_args;

	if (threshold >= 255)
		return dseed::error_invalid_args;

	auto format = bitmap->format ();
	if (format == dseed::pixelformat_grayscale8 || format == dseed::pixelformat_grayscalef)
	{
		*grayscale = true;
		return dseed::error_good;
	}

	if (format == dseed::pixelformat_bgra8888_indexed8)
	{
		dseed::auto_object<dseed::palette> palette;
		bitmap->palette (&palette);

		dseed::bgra pixels[256];
		if (dseed::failed (palette->pixels_pointer ((void**)&pixels)))
			return dseed::error_fail;

		*grayscale = detect_grayscale_indexed<dseed::bgra> ((uint8_t*)pixels, 4, 256, threshold);
	}
	else if (format == dseed::pixelformat_bgr888_indexed8)
	{
		dseed::auto_object<dseed::palette> palette;
		bitmap->palette (&palette);

		dseed::bgr pixels[256];
		if (dseed::failed (palette->pixels_pointer ((void**)&pixels)))
			return dseed::error_fail;

		*grayscale = detect_grayscale_indexed<dseed::bgr> ((uint8_t*)pixels, 3, 256, threshold);
	}
	else
	{
		uint8_t* srcPtr;
		if (dseed::failed (bitmap->pixels_pointer ((void**)&srcPtr)))
			return dseed::error_fail;

		auto size = bitmap->size ();

		auto found = g_dgs.find (format);
		if (found == g_dgs.end ())
			return dseed::error_not_support;

		*grayscale = found->second (srcPtr, size, threshold);
	}
	return dseed::error_good;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Get Total Colours Count
//
////////////////////////////////////////////////////////////////////////////////////////////

using gtcfn = std::function<dseed::colorcount_t (const uint8_t * src, const dseed::size3i & size)>;
using gtctp = dseed::pixelformat_t;
constexpr dseed::colorcount_t get_colorcount_t (size_t i)
{
	if (i <= 2) return dseed::colorcount_1bpp;
	else if (i <= 4) return dseed::colorcount_under2bpp;
	else if (i <= 16) return dseed::colorcount_under4bpp;
	else if (i <= 256) return dseed::colorcount_under8bpp;
	else return dseed::colorcount_over8bpp;
}

template<class TPixel>
dseed::colorcount_t get_total_colours_count (const uint8_t* src, const dseed::size3i& size)
{
	std::vector<TPixel> pixelStore;

	size_t stride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), size.width);
	size_t depth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), size.width, size.height);

	for (size_t z = 0; z < size.depth; ++z)
	{
		size_t depthZ = z * depth;
		for (size_t y = 0; y < size.height; ++y)
		{
			size_t strideY = y * stride;

			const TPixel* srcPtr = (const TPixel*)(src + depthZ + strideY);

			for (size_t x = 0; x < size.width; ++x)
			{
				const TPixel& pixel = *(srcPtr + x);
				
				if (std::find (pixelStore.begin (), pixelStore.end (), pixel) == pixelStore.end ())
					pixelStore.push_back (pixel);
				if (pixelStore.size () > 256)
					return dseed::colorcount_over8bpp;
			}
		}
	}

	return get_colorcount_t (pixelStore.size ());
}

std::map<gtctp, gtcfn> g_gtcs = {
	{ dseed::pixelformat_rgba8888, get_total_colours_count<dseed::rgba> },
	{ dseed::pixelformat_rgbaf, get_total_colours_count<dseed::rgbaf> },
	{ dseed::pixelformat_rgb888, get_total_colours_count<dseed::rgb> },
	{ dseed::pixelformat_bgra8888, get_total_colours_count<dseed::bgra> },
	{ dseed::pixelformat_bgr888, get_total_colours_count<dseed::bgr> },
	{ dseed::pixelformat_bgra4444, get_total_colours_count<dseed::bgra4> },
	{ dseed::pixelformat_bgr565, get_total_colours_count<dseed::bgr565> },
	{ dseed::pixelformat_yuva8888, get_total_colours_count<dseed::yuva> },
	{ dseed::pixelformat_yuv888, get_total_colours_count<dseed::yuv> },
	{ dseed::pixelformat_hsva8888, get_total_colours_count<dseed::hsva> },
	{ dseed::pixelformat_hsv888, get_total_colours_count<dseed::hsv> },
};

dseed::error_t dseed::bitmap_get_total_colours (dseed::bitmap* bitmap, colorcount_t* colours)
{
	if (bitmap == nullptr || colours == nullptr)
		return dseed::error_invalid_args;

	auto format = bitmap->format ();
	if (format == dseed::pixelformat_bgra8888_indexed8 || format == dseed::pixelformat_bgr888_indexed8)
	{
		dseed::auto_object<dseed::palette> palette;
		if (dseed::failed (bitmap->palette (&palette)))
			return dseed::error_fail;
		*colours = get_colorcount_t (palette->size ());
		return dseed::error_good;
	}

	uint8_t* srcPtr;
	if (dseed::failed (bitmap->pixels_pointer ((void**)&srcPtr)))
		return dseed::error_fail;

	auto size = bitmap->size ();

	auto found = g_gtcs.find (format);
	if (found == g_gtcs.end ())
		return dseed::error_not_support;

	*colours = found->second (srcPtr, size);

	return dseed::error_good;
}
