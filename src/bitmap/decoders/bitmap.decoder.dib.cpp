#include <dseed.h>
#include "bitmap.decoder.hxx"

#include <memory>
#include <vector>
#include <string.h>

#if COMPILER_MSVC
#	pragma pack (push, 1)
#else
#	pragma pack (1)
#endif
struct bitmap_file_header
{
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;
};
struct bitmap_info_header
{
	uint32_t biSize;
	int32_t  biWidth;
	int32_t  biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	int32_t  biXPelsPerMeter;
	int32_t  biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
};
struct bitmap_info_header_v4 : public bitmap_info_header
{
	uint32_t bV4RedMask;
	uint32_t bV4GreenMask;
	uint32_t bV4BlueMask;
	uint32_t bV4AlphaMask;
	uint32_t bV4CSType;
	struct {
		struct CIEXYZ {
			int32_t ciexyzX;
			int32_t ciexyzY;
			int32_t ciexyzZ;
		};
		CIEXYZ  ciexyzRed;
		CIEXYZ  ciexyzGreen;
		CIEXYZ  ciexyzBlue;
	} bV4Endpoints;
	uint32_t bV4GammaRed;
	uint32_t bV4GammaGreen;
	uint32_t bV4GammaBlue;
};
struct bitmap_info_header_v5 : public bitmap_info_header_v4
{
	uint32_t bV5Intent;
	uint32_t bV5ProfileData;
	uint32_t bV5ProfileSize;
	uint32_t bV5Reserved;
};
#if COMPILER_MSVC
#	pragma pack (pop)
#else
#	pragma pack ()
#endif

enum bitmap_compression_t
{
	bitmap_compression_rgb = 0L,
	bitmap_compression_rle8 = 1L,
	bitmap_compression_rle4 = 2L,
	bitmap_compression_bitfields = 3L,
	bitmap_compression_jpeg = 4L,
	bitmap_compression_png = 5L,
};

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

	dseed::pixelformat_t format;
	switch (infoHeader.biBitCount)
	{
	case 8: format = dseed::pixelformat_grayscale8; break;
	case 16: format = dseed::pixelformat_bgr565; break;
	case 24: format = dseed::pixelformat_bgr888; break;
	case 32: format = dseed::pixelformat_bgra8888; break;
	default: return dseed::error_not_support;
	}

	stream->seek (dseed::seekorigin_begin, fileHeader.bfOffBits);

	size_t stride = dseed::get_bitmap_stride (format, infoHeader.biWidth);

	dseed::auto_object<dseed::bitmap> bitmap;
	if (dseed::failed (dseed::create_bitmap (bitmaptype_2d, dseed::size3i (infoHeader.biWidth, infoHeader.biHeight, 1),
		format, nullptr, &bitmap)))
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