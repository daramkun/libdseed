#include <dseed.h>

#include <map>

////////////////////////////////////////////////////////////////////////////////////////////
//
// Determine Bitmap Properties
//
////////////////////////////////////////////////////////////////////////////////////////////

using dbpfn = std::function<void (const uint8_t * src, const dseed::size3i & size, int threshold, dseed::bitmap_properties * prop)>;
using dbptp = dseed::pixelformat_t;

constexpr dseed::colorcount_t get_colorcount_t (size_t i)
{
	if (i <= 2) return dseed::colorcount_1bpp_palettable;
	else if (i <= 4) return dseed::colorcount_2bpp_palettable;
	else if (i <= 16) return dseed::colorcount_4bpp_palettable;
	else if (i <= 256) return dseed::colorcount_8bpp_palettable;
	else return dseed::colorcount_cannot_palettable;
}

template<class TPixel>
inline void determine_props (const uint8_t* src, const dseed::size3i& size, int threshold, dseed::bitmap_properties* prop)
{
	size_t stride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), size.width);
	size_t depth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), size.width, size.height);

	prop->transparent = false;
	prop->grayscale = true;

	std::vector<TPixel> pixelStore;

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
				if (!prop->transparent && dseed::hasalpha<TPixel> ())
				{
					if (pixel[3] == TPixel::max_color ()[3])
						prop->transparent = true;
				}

				if (prop->grayscale && (dseed::type2format<TPixel> () != dseed::pixelformat_grayscale8 && dseed::type2format<TPixel> () != dseed::pixelformat_grayscalef))
				{
					dseed::rgb rgb = (dseed::rgb) pixel;
					if (abs (rgb.r - rgb.g) > threshold || abs (rgb.r - rgb.b) > threshold)
						prop->grayscale = false;
				}

				if (pixelStore.size () <= 256)
				{
					if (std::find (pixelStore.begin (), pixelStore.end (), pixel) == pixelStore.end ())
						pixelStore.emplace_back (pixel);
				}

				if ((prop->transparent || !dseed::hasalpha<TPixel> ()) && !prop->grayscale&& pixelStore.size () > 256)
				{
					prop->colours = dseed::colorcount_cannot_palettable;
					return;
				}
			}
		}
	}

	prop->colours = get_colorcount_t (pixelStore.size ());
}

template<class TPixel>
inline void determine_props_indexed (const uint8_t* src, int bpp, int elements, int threshold, dseed::bitmap_properties* prop)
{
	prop->transparent = false;
	prop->grayscale = true;
	prop->colours = get_colorcount_t (elements);

	for (int i = 0; i < elements; ++i)
	{
		TPixel& pixel = *(TPixel*)(src + i * bpp);
		dseed::rgba rgb = (dseed::rgba) pixel;

		if (rgb.a < 255 && dseed::hasalpha<TPixel> ())
			prop->transparent = true;

		if (abs (rgb.r - rgb.g) > threshold || abs (rgb.r - rgb.b) > threshold)
			prop->grayscale = false;
	}
}

std::map<dbptp, dbpfn> g_dbps = {
	{ dseed::pixelformat_rgba8888, determine_props<dseed::rgba> },
	{ dseed::pixelformat_rgbaf, determine_props<dseed::rgbaf> },
	{ dseed::pixelformat_rgb888, determine_props<dseed::rgb> },
	{ dseed::pixelformat_bgra8888, determine_props<dseed::bgra> },
	{ dseed::pixelformat_bgr888, determine_props<dseed::bgr> },
	{ dseed::pixelformat_bgra4444, determine_props<dseed::bgra4> },
	{ dseed::pixelformat_bgr565, determine_props<dseed::bgr565> },
	{ dseed::pixelformat_grayscale8, determine_props<dseed::grayscale> },
	{ dseed::pixelformat_grayscalef, determine_props<dseed::grayscalef> },
	{ dseed::pixelformat_yuva8888, determine_props<dseed::yuva> },
	{ dseed::pixelformat_yuv888, determine_props<dseed::yuv> },
	{ dseed::pixelformat_hsva8888, determine_props<dseed::hsva> },
	{ dseed::pixelformat_hsv888, determine_props<dseed::hsv> },
};

dseed::error_t dseed::bitmap_determine_bitmap_properties (dseed::bitmap* bitmap, dseed::bitmap_properties* prop, int threshold)
{
	if (bitmap == nullptr || prop == nullptr)
		return dseed::error_invalid_args;

	if (threshold >= 255)
		return dseed::error_invalid_args;

	auto format = bitmap->format ();

	if (format == dseed::pixelformat_bgra8888_indexed8 || format == dseed::pixelformat_bgr888_indexed8)
	{
		dseed::auto_object<dseed::palette> palette;
		bitmap->palette (&palette);

		uint8_t pixels[256 * 4];
		if (dseed::failed (palette->pixels_pointer ((void**)&pixels)))
			return dseed::error_fail;

		if (format == dseed::pixelformat_bgra8888_indexed8)
			determine_props_indexed<dseed::bgra> (pixels, 4, palette->size (), threshold, prop);
		else
			determine_props_indexed<dseed::bgr> (pixels, 3, palette->size (), threshold, prop);
	}
	else
	{
		uint8_t* srcPtr;
		if (dseed::failed (bitmap->pixels_pointer ((void**)&srcPtr)))
			return dseed::error_fail;

		auto size = bitmap->size ();

		auto found = g_dbps.find (format);
		if (found == g_dbps.end ())
			return dseed::error_not_support;

		found->second (srcPtr, size, threshold, prop);
	}
	return dseed::error_good;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Transparent Detection
//
////////////////////////////////////////////////////////////////////////////////////////////
dseed::error_t dseed::bitmap_detect_transparent (dseed::bitmap* bitmap, bool* transparent)
{
	if (bitmap == nullptr || transparent == nullptr)
		return dseed::error_invalid_args;

	dseed::bitmap_properties prop;
	if (dseed::failed (dseed::bitmap_determine_bitmap_properties (bitmap, &prop)))
		return dseed::error_fail;
	*transparent = prop.transparent;

	return dseed::error_good;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Grayscale Detection
//
////////////////////////////////////////////////////////////////////////////////////////////
dseed::error_t dseed::bitmap_detect_grayscale_bitmap (dseed::bitmap* bitmap, bool* grayscale, int threshold)
{
	if (bitmap == nullptr || grayscale == nullptr)
		return dseed::error_invalid_args;

	dseed::bitmap_properties prop;
	if (dseed::failed (dseed::bitmap_determine_bitmap_properties (bitmap, &prop)))
		return dseed::error_fail;
	*grayscale = prop.grayscale;

	return dseed::error_good;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Get Total Colours Count
//
////////////////////////////////////////////////////////////////////////////////////////////
dseed::error_t dseed::bitmap_get_total_colours (dseed::bitmap* bitmap, colorcount_t* colours)
{
	if (bitmap == nullptr || colours == nullptr)
		return dseed::error_invalid_args;

	dseed::bitmap_properties prop;
	if (dseed::failed (dseed::bitmap_determine_bitmap_properties (bitmap, &prop)))
		return dseed::error_fail;
	*colours = prop.colours;

	return dseed::error_good;
}
