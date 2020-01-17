#include <dseed.h>
#include "common.hxx"

#include "../../libs/DIBHelper.hxx"

dseed::error_t dseed::bitmaps::create_dib_bitmap_decoder (dseed::io::stream* stream, dseed::bitmaps::bitmap_array** decoder) noexcept
{
	if (stream == nullptr || decoder == nullptr)
		return dseed::error_invalid_args;

	bitmap_file_header fileHeader;
	if (stream->read (&fileHeader, sizeof (fileHeader)) != sizeof (fileHeader))
		return dseed::error_fail;

	if (memcmp (&fileHeader.bfType, "BM", 2) != 0)
		return dseed::error_fail;

	bitmap_info_header infoHeader;
	if (stream->read (&infoHeader, sizeof (infoHeader)) != sizeof (infoHeader))
		return dseed::error_fail;

	if (infoHeader.biPlanes != 1)
		return dseed::error_not_support;

	if (infoHeader.biCompression != bitmap_compression_rgb)
		return dseed::error_not_support;

	dseed::color::pixelformat format;
	switch (infoHeader.biBitCount)
	{
	case 8:
		if (infoHeader.biClrUsed > 0)
			format = dseed::color::pixelformat::bgra8_indexed8;
		else
			format = dseed::color::pixelformat::r8;
		break;
	case 16: format = dseed::color::pixelformat::bgr565; break;
	case 24: format = dseed::color::pixelformat::bgr8; break;
	case 32: format = dseed::color::pixelformat::bgra8; break;
	default: return dseed::error_not_support;
	}

	dseed::autoref<dseed::bitmaps::palette> palette;
	if (infoHeader.biClrUsed > 0)
	{
		std::vector<uint8_t> paletteData (infoHeader.biClrUsed * 4);
		stream->read (paletteData.data (), paletteData.size ());
		dseed::bitmaps::create_palette (paletteData.data (), 32, infoHeader.biClrUsed, &palette);
	}

	//stream->seek (dseed::seekorigin_begin, fileHeader.bfOffBits);

	size_t stride = dseed::color::calc_bitmap_stride (format, infoHeader.biWidth);

	dseed::autoref<dseed::bitmaps::bitmap> bitmap;
	if (dseed::failed (dseed::bitmaps::create_bitmap (bitmaptype::bitmap2d, dseed::size3i (infoHeader.biWidth, infoHeader.biHeight, 1),
		format, palette, &bitmap)))
		return dseed::error_fail;

	uint8_t* ptr;
	bitmap->lock ((void**)&ptr);

	for (auto y = 0; y < infoHeader.biHeight; ++y)
	{
		void* target = ptr + ((infoHeader.biHeight - y - 1) * stride);
		stream->read (target, stride);
	}

	bitmap->unlock ();

	*decoder = new dseed::__common_bitmap_array (bitmap);
	if (decoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}