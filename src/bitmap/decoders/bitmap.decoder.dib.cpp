#include <dseed.h>
#include "bitmap.decoder.hxx"

#include <memory>
#include <vector>
#include <string.h>

#include "../../libs/Microsoft/DIBCommon.hxx"

dseed::error_t dseed::create_dib_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder)
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

	dseed::pixelformat format;
	switch (infoHeader.biBitCount)
	{
	case 8:
		if (infoHeader.biClrUsed > 0)
			format = dseed::pixelformat::bgra8888_indexed8;
		else
			format = dseed::pixelformat::grayscale8;
		break;
	case 16: format = dseed::pixelformat::bgr565; break;
	case 24: format = dseed::pixelformat::bgr888; break;
	case 32: format = dseed::pixelformat::bgra8888; break;
	default: return dseed::error_not_support;
	}

	dseed::auto_object<dseed::palette> palette;
	if (infoHeader.biClrUsed > 0)
	{
		std::vector<uint8_t> paletteData (infoHeader.biClrUsed * 4);
		stream->read (paletteData.data (), paletteData.size ());
		dseed::create_palette (paletteData.data (), 32, infoHeader.biClrUsed, &palette);
	}

	//stream->seek (dseed::seekorigin_begin, fileHeader.bfOffBits);

	size_t stride = dseed::get_bitmap_stride (format, infoHeader.biWidth);

	dseed::auto_object<dseed::bitmap> bitmap;
	if (dseed::failed (dseed::create_bitmap (bitmaptype_2d, dseed::size3i (infoHeader.biWidth, infoHeader.biHeight, 1),
		format, palette, &bitmap)))
		return dseed::error_fail;

	for (auto y = 0; y < infoHeader.biHeight; ++y)
	{
		void* target;
		if (dseed::failed (bitmap->pixels_pointer_per_line (&target, infoHeader.biHeight - y - 1)))
			continue;
		stream->read (target, stride);
	}

	*decoder = new __common_bitmap_decoder (bitmap);
	if (decoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}