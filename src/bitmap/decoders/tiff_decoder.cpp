#include <dseed.h>

#include "common.hxx"

#if defined(USE_TIFF)
#	include <tiff.h>
#	include <tiffio.h>
#endif

dseed::error_t dseed::bitmaps::create_tiff_bitmap_decoder (dseed::io::stream* stream, dseed::bitmaps::bitmap_array** decoder) noexcept
{
#if defined(USE_TIFF)
	if (stream == nullptr || decoder == nullptr)
		return dseed::error_invalid_args;

	auto tiff = TIFFClientOpen ("Stream", "r", stream,
		// Read
		[] (thandle_t handle, tdata_t data, tsize_t sz) -> tsize_t
		{
			return reinterpret_cast<dseed::io::stream*>(handle)->read (data, sz);
		},
		// Write
		[] (thandle_t handle, tdata_t data, tsize_t sz) -> tsize_t
		{
			return reinterpret_cast<dseed::io::stream*>(handle)->write (data, sz);
		},
		// Seek
		[] (thandle_t handle, toff_t offset, int origin) -> toff_t
		{
			auto stream = reinterpret_cast<dseed::io::stream*>(handle);
			if (!stream->seek ((dseed::io::seekorigin)origin, offset))
				return -1;
			return stream->position ();
		},
		// Close
		[](thandle_t handle) -> int { return 1; },
		// Size
		[](thandle_t handle) -> toff_t { return reinterpret_cast<dseed::io::stream*>(handle)->length (); },
		// Map
		[](thandle_t, tdata_t*, toff_t*) -> int { return 0; },
		// Unmap
		[](thandle_t, tdata_t, toff_t) {}
		);
	if (tiff == nullptr)
		return dseed::error_fail;

	uint32_t width, height;
	TIFFGetField (tiff, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField (tiff, TIFFTAG_IMAGELENGTH, &height);
	size_t stride = width * sizeof (uint32_t);

	uint32* raster = (uint32_t*)_TIFFmalloc (stride * height);

	if (!TIFFReadRGBAImage (tiff, width, height, raster, 0))
	{
		_TIFFfree (raster);
		TIFFClose (tiff);
		return dseed::error_fail;
	}

	TIFFClose (tiff);

	dseed::autoref<dseed::bitmaps::bitmap> bitmap;
	if (dseed::failed (dseed::bitmaps::create_bitmap (dseed::bitmaps::bitmaptype::bitmap2d,
		dseed::size3i (width, height, 1), dseed::color::pixelformat::rgba8,
		nullptr, &bitmap)))
		return dseed::error_fail;

	for (size_t y = 0; y < height; ++y)
		bitmap->write_pixels (dseed::rect2i (0, (int32_t)y, width, 1), raster + ((height - y - 1) * width));

	_TIFFfree (raster);

	*decoder = new dseed::__common_bitmap_array (bitmap);
	if (*decoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}