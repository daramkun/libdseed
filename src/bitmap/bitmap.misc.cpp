#include <dseed.h>

#include <map>

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
		palette->pixels_pointer ((void**)&pixels);

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
		bitmap->pixels_pointer ((void**)&srcPtr);

		auto size = bitmap->size ();

		auto found = g_dts.find (format);
		if (found == g_dts.end ())
			return dseed::error_not_support;

		*transparent = found->second (srcPtr, size);
	}
	return dseed::error_good;
}