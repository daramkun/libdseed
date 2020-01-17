#ifndef __DSEED_DIB_HELPER_HXX__
#define __DSEED_DIB_HELPER_HXX__

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

enum bitmap_compression
{
	bitmap_compression_rgb = 0L,
	bitmap_compression_rle8 = 1L,
	bitmap_compression_rle4 = 2L,
	bitmap_compression_bitfields = 3L,
	bitmap_compression_jpeg = 4L,
	bitmap_compression_png = 5L,
};

#endif