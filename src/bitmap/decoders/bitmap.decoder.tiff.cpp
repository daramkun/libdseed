#include <dseed.h>

#if defined ( USE_TIFF )
#	include <tiff.h>
#	include <tiffio.h>
#	include "bitmap.decoder.hxx"
#endif

dseed::error_t dseed::create_tiff_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder)
{
#if defined ( USE_TIFF )
	if (stream == nullptr || decoder == nullptr)
		return dseed::error_invalid_args;

	auto tiff = TIFFClientOpen ("Stream", "r", stream,
		// Read
		[] (thandle_t handle, tdata_t data, tsize_t sz) -> tsize_t
		{
			return reinterpret_cast<dseed::stream*>(handle)->read (data, sz);
		},
		// Write
			[] (thandle_t handle, tdata_t data, tsize_t sz) -> tsize_t
		{
			return reinterpret_cast<dseed::stream*>(handle)->write (data, sz);
		},
			// Seek
			[] (thandle_t handle, toff_t offset, int origin) -> toff_t
		{
			auto stream = reinterpret_cast<dseed::stream*>(handle);
			if (!stream->seek ((dseed::seekorigin_t)origin, offset))
				return -1;
			return stream->position ();
		},
			// Close
			[](thandle_t handle) -> int { return 1; },
			// Size
			[](thandle_t handle) -> toff_t { return reinterpret_cast<dseed::stream*>(handle)->length (); },
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

	dseed::auto_object<dseed::bitmap> bitmap;
	if (dseed::failed (dseed::create_bitmap (dseed::bitmaptype_2d, 
		dseed::size3i (width, height, 1), dseed::pixelformat_rgba8888,
		nullptr, &bitmap)))
		return dseed::error_fail;

	for (size_t y = 0; y < height; ++y)
		bitmap->write_pixels (raster + ((height - y - 1) * width), dseed::point2i (0, (int32_t)y), dseed::size2i (width, 1));

	_TIFFfree (raster);

	*decoder = new __common_bitmap_decoder (bitmap);
	if (*decoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}