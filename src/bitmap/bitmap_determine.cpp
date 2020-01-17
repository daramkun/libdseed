#include <dseed.h>

#include <map>

using namespace dseed::color;
using size2i = dseed::size2i;

////////////////////////////////////////////////////////////////////////////////////////////
//
// Determine Bitmap Properties
//
////////////////////////////////////////////////////////////////////////////////////////////

using dbpfn = std::function<void (const uint8_t * src, const dseed::size3i & size, int threshold, dseed::bitmaps::bitmap_properties * prop)>;
using dbptp = dseed::color::pixelformat;

constexpr dseed::bitmaps::colorcount get_colorcount_t (size_t i)
{
	if (i <= 2) return dseed::bitmaps::colorcount::color_1bpp_palettable;
	else if (i <= 4) return dseed::bitmaps::colorcount::color_2bpp_palettable;
	else if (i <= 16) return dseed::bitmaps::colorcount::color_4bpp_palettable;
	else if (i <= 256) return dseed::bitmaps::colorcount::color_8bpp_palettable;
	else return dseed::bitmaps::colorcount::color_cannot_palettable;
}

template<class TPixel>
inline void determine_props (const uint8_t* src, const dseed::size3i& size, int threshold, dseed::bitmaps::bitmap_properties* prop)
{
	size_t stride = calc_bitmap_stride (type2format<TPixel> (), size.width);
	size_t depth = calc_bitmap_plane_size (type2format<TPixel> (), size2i (size.width, size.height));

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
				if (!prop->transparent && hasalpha<TPixel> ())
				{
					if (pixel[3] == TPixel::max_color ()[3])
						prop->transparent = true;
				}

				if (prop->grayscale && (type2format<TPixel> () != pixelformat::r8 && type2format<TPixel> () != pixelformat::rf))
				{
					rgb8 rgb = (rgb8) pixel;
					if (abs (rgb.r - rgb.g) > threshold || abs (rgb.r - rgb.b) > threshold)
						prop->grayscale = false;
				}

				if (pixelStore.size () <= 256)
				{
					if (std::find (pixelStore.begin (), pixelStore.end (), pixel) == pixelStore.end ())
						pixelStore.emplace_back (pixel);
				}

				if ((prop->transparent || !hasalpha<TPixel> ()) && !prop->grayscale && pixelStore.size () > 256)
				{
					prop->colours = dseed::bitmaps::colorcount::color_cannot_palettable;
					return;
				}
			}
		}
	}

	prop->colours = get_colorcount_t (pixelStore.size ());
}

template<class TPixel>
inline void determine_props_indexed (const uint8_t* src, int bpp, int elements, int threshold, dseed::bitmaps::bitmap_properties* prop)
{
	prop->transparent = false;
	prop->grayscale = true;
	prop->colours = get_colorcount_t (elements);

	for (int i = 0; i < elements; ++i)
	{
		TPixel& pixel = *(TPixel*)(src + i * bpp);
		rgba8 rgb = (rgba8) pixel;

		if (rgb.a < 255 && hasalpha<TPixel> ())
			prop->transparent = true;

		if (abs (rgb.r - rgb.g) > threshold || abs (rgb.r - rgb.b) > threshold)
			prop->grayscale = false;
	}
}

std::map<dbptp, dbpfn> g_dbps = {
	{ dseed::color::pixelformat::rgba8, determine_props<dseed::color::rgba8> },
	{ dseed::color::pixelformat::rgbaf, determine_props<dseed::color::rgbaf> },
	{ dseed::color::pixelformat::rgb8, determine_props<dseed::color::rgb8> },
	{ dseed::color::pixelformat::bgra8, determine_props<dseed::color::bgra8> },
	{ dseed::color::pixelformat::bgr8, determine_props<dseed::color::bgr8> },
	{ dseed::color::pixelformat::bgra4, determine_props<dseed::color::bgra4> },
	{ dseed::color::pixelformat::bgr565, determine_props<dseed::color::bgr565> },
	{ dseed::color::pixelformat::r8, determine_props<dseed::color::r8> },
	{ dseed::color::pixelformat::rf, determine_props<dseed::color::rf> },
	{ dseed::color::pixelformat::yuva8, determine_props<dseed::color::yuva8> },
	{ dseed::color::pixelformat::yuv8, determine_props<dseed::color::yuv8> },
	{ dseed::color::pixelformat::hsva8, determine_props<dseed::color::hsva8> },
	{ dseed::color::pixelformat::hsv8, determine_props<dseed::color::hsv8> },
};

dseed::error_t dseed::bitmaps::determine_bitmap_properties (dseed::bitmaps::bitmap* bitmap, dseed::bitmaps::bitmap_properties* prop, int threshold)
{
	if (bitmap == nullptr || prop == nullptr)
		return dseed::error_invalid_args;

	if (threshold >= 255)
		return dseed::error_invalid_args;

	auto format = bitmap->format ();

	if (format == dseed::color::pixelformat::bgra8_indexed8 || format == dseed::color::pixelformat::bgr8_indexed8)
	{
		dseed::autoref<dseed::bitmaps::palette> palette;
		bitmap->palette (&palette);

		uint8_t pixels[256 * 4];
		if (dseed::failed (palette->lock ((void**)&pixels)))
			return dseed::error_fail;

		if (format == dseed::color::pixelformat::bgra8_indexed8)
			determine_props_indexed<dseed::color::bgra8> (pixels, 4, (int)palette->size (), threshold, prop);
		else
			determine_props_indexed<dseed::color::bgr8> (pixels, 3, (int)palette->size (), threshold, prop);

		palette->unlock ();
	}
	else
	{
		auto size = bitmap->size ();

		uint8_t* srcPtr;
		if (dseed::failed (bitmap->lock ((void**)&srcPtr)))
			return dseed::error_fail;

		auto found = g_dbps.find (format);
		if (found == g_dbps.end ())
			return dseed::error_not_support;

		found->second (srcPtr, size, threshold, prop);

		bitmap->unlock ();
	}
	return dseed::error_good;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Transparent Detection
//
////////////////////////////////////////////////////////////////////////////////////////////
dseed::error_t dseed::bitmaps::detect_transparent (dseed::bitmaps::bitmap* bitmap, bool* transparent)
{
	if (bitmap == nullptr || transparent == nullptr)
		return dseed::error_invalid_args;

	dseed::bitmaps::bitmap_properties prop;
	if (dseed::failed (dseed::bitmaps::determine_bitmap_properties (bitmap, &prop)))
		return dseed::error_fail;
	*transparent = prop.transparent;

	return dseed::error_good;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Grayscale Detection
//
////////////////////////////////////////////////////////////////////////////////////////////
dseed::error_t dseed::bitmaps::detect_grayscale_bitmap (dseed::bitmaps::bitmap* bitmap, bool* grayscale, int threshold)
{
	if (bitmap == nullptr || grayscale == nullptr)
		return dseed::error_invalid_args;

	dseed::bitmaps::bitmap_properties prop;
	if (dseed::failed (dseed::bitmaps::determine_bitmap_properties (bitmap, &prop)))
		return dseed::error_fail;
	*grayscale = prop.grayscale;

	return dseed::error_good;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Get Total Colours Count
//
////////////////////////////////////////////////////////////////////////////////////////////
dseed::error_t dseed::bitmaps::get_total_colours (dseed::bitmaps::bitmap* bitmap, dseed::bitmaps::colorcount* colours)
{
	if (bitmap == nullptr || colours == nullptr)
		return dseed::error_invalid_args;

	dseed::bitmaps::bitmap_properties prop;
	if (dseed::failed (dseed::bitmaps::determine_bitmap_properties (bitmap, &prop)))
		return dseed::error_fail;
	*colours = prop.colours;

	return dseed::error_good;
}