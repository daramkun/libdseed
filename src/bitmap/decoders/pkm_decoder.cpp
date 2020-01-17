#include <dseed.h>

#include "common.hxx"

#define PKM_HEADER_SIZE 16

#if COMPILER_MSVC
#	pragma pack (push, 1)
#else
#	pragma pack (1)
#endif
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
#if COMPILER_MSVC
#	pragma pack (pop)
#else
#	pragma pack ()
#endif

dseed::error_t dseed::bitmaps::create_pkm_bitmap_decoder (dseed::io::stream* stream, dseed::bitmaps::bitmap_array** decoder) noexcept
{
	PKMHeader header;
	stream->read (&header, sizeof (header));

	if (strcmp (header.aName, "PKM 10") != 0)
		return dseed::error_fail;

	int width = (header.iWidthMSB << 8) | header.iWidthLSB;
	int height = (header.iHeightMSB << 8) | header.iHeightLSB;

	size_t totalSize = dseed::color::calc_bitmap_total_size (dseed::color::pixelformat::etc1, dseed::size3i (width, height, 1));
	std::vector<uint8_t> pixels (totalSize);

	stream->read (pixels.data (), totalSize);

	dseed::autoref<dseed::bitmaps::bitmap> bitmap;
	if (dseed::failed (dseed::bitmaps::create_bitmap (bitmaptype::bitmap2d, dseed::size3i (width, height, 1),
		dseed::color::pixelformat::etc1, nullptr, &bitmap)))
		return dseed::error_fail;

	void* ptr;
	bitmap->lock (&ptr);
	memcpy (ptr, pixels.data (), pixels.size ());
	bitmap->unlock ();

	*decoder = new dseed::__common_bitmap_array (bitmap, dseed::bitmaps::arraytype::mipmap);
	if (*decoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}