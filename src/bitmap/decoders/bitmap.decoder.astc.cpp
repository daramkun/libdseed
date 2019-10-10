#include <dseed.h>

#include "bitmap.decoder.hxx"

#if COMPILER_MSVC
#	pragma pack (push, 1)
#else
#	pragma pack (1)
#endif
struct ASTCHeader {
	uint32_t magic;
	unsigned char blockDimX;
	unsigned char blockDimY;
	unsigned char blockDimZ;
	unsigned char xSize[3];
	unsigned char ySize[3];
	unsigned char zSize[3];
};
#if COMPILER_MSVC
#	pragma pack (pop)
#else
#	pragma pack ()
#endif

dseed::error_t dseed::create_astc_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder)
{
	ASTCHeader header = {};
	if (sizeof (ASTCHeader) != stream->read (&header, sizeof (ASTCHeader)))
		return dseed::error_not_support_file_format;

	if (header.magic != 0x5CA1AB13)
		return dseed::error_not_support_file_format;

	dseed::size3i size (
		header.xSize[2] | (header.xSize[1] << 8) | (header.xSize[0] << 16),
		header.ySize[2] | (header.ySize[1] << 8) | (header.ySize[0] << 16),
		header.ySize[2] | (header.ySize[1] << 8) | (header.ySize[0] << 16));

	dseed::pixelformat format;
	if (header.blockDimX == 4 && header.blockDimY == 4) format = dseed::pixelformat::astc_4x4;
	else if (header.blockDimX == 5 && header.blockDimY == 4) format = dseed::pixelformat::astc_5x4;
	else if (header.blockDimX == 5 && header.blockDimY == 5) format = dseed::pixelformat::astc_5x5;
	else if (header.blockDimX == 6 && header.blockDimY == 5) format = dseed::pixelformat::astc_6x5;
	else if (header.blockDimX == 6 && header.blockDimY == 6) format = dseed::pixelformat::astc_6x6;
	else if (header.blockDimX == 8 && header.blockDimY == 5) format = dseed::pixelformat::astc_8x5;
	else if (header.blockDimX == 8 && header.blockDimY == 6) format = dseed::pixelformat::astc_8x6;
	else if (header.blockDimX == 8 && header.blockDimY == 8) format = dseed::pixelformat::astc_8x8;
	else if (header.blockDimX == 10 && header.blockDimY == 5) format = dseed::pixelformat::astc_10x5;
	else if (header.blockDimX == 10 && header.blockDimY == 6) format = dseed::pixelformat::astc_10x6;
	else if (header.blockDimX == 10 && header.blockDimY == 8) format = dseed::pixelformat::astc_10x8;
	else if (header.blockDimX == 10 && header.blockDimY == 10) format = dseed::pixelformat::astc_10x8;
	else if (header.blockDimX == 12 && header.blockDimY == 10) format = dseed::pixelformat::astc_12x10;
	else if (header.blockDimX == 12 && header.blockDimY == 12) format = dseed::pixelformat::astc_12x12;
	else return dseed::error_not_support;

	dseed::bitmaptype_t type;
	if (size.depth == 1)
		type = dseed::bitmaptype_2d;
	else if (size.depth == 6)
		type = dseed::bitmaptype_2d_cubemap;
	else
		type = dseed::bitmaptype_3d;

	dseed::auto_object<dseed::bitmap> bitmap;
	if (dseed::failed (dseed::create_bitmap (type, size, format, nullptr, &bitmap)))
		return dseed::error_fail;

	void* ptr;
	if (dseed::failed (bitmap->pixels_pointer (&ptr)))
		return dseed::error_fail;

	size_t totalImageSize = dseed::get_bitmap_plane_size (format, size.width, size.height) * size.depth;

	if (totalImageSize != stream->read (ptr, totalImageSize))
		return dseed::error_io;

	*decoder = new __common_bitmap_decoder (bitmap, dseed::frametype_mipmap);
	if (*decoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}