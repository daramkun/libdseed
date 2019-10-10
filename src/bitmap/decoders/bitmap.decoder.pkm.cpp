#include <dseed.h>

#include <string.h>

#include "bitmap.decoder.hxx"

#define PKM_HEADER_SIZE 16
struct PKMHeader
{
	char aName[6];
	unsigned short iBlank;
	unsigned char iPaddedWidthMSB;
	unsigned char iPaddedWidthLSB;
	unsigned char iPaddedHeightMSB;
	unsigned char iPaddedHeightLSB;
	unsigned char iWidthMSB;
	unsigned char iWidthLSB;
	unsigned char iHeightMSB;
	unsigned char iHeightLSB;
};

dseed::error_t dseed::create_pkm_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder)
{
	PKMHeader header;
	stream->read (&header, sizeof (header));

	if (strcmp (header.aName, "PKM 10") != 0)
		return dseed::error_fail;

	int width = (header.iWidthMSB << 8) | header.iWidthLSB;
	int height = (header.iHeightMSB << 8) | header.iHeightLSB;

	size_t totalSize = dseed::get_bitmap_total_size (dseed::pixelformat::etc1, dseed::size3i (width, height, 1));
	std::vector<uint8_t> pixels (totalSize);

	stream->read (pixels.data (), totalSize);

	dseed::auto_object<dseed::bitmap> bitmap;
	if (dseed::failed (dseed::create_bitmap (bitmaptype_2d, dseed::size3i (width, height, 1),
		dseed::pixelformat::etc1, nullptr, &bitmap)))
		return dseed::error_fail;

	void* ptr;
	bitmap->pixels_pointer (&ptr);

	memcpy (ptr, pixels.data (), pixels.size ());

	*decoder = new __common_bitmap_decoder (bitmap, dseed::frametype_mipmap);
	if (*decoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}