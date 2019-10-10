#include <dseed.h>

#include "../../libs/Microsoft/ICOCURCommon.hxx"
#include "bitmap.decoder.hxx"

dseed::error_t __create_ico_cur_bitmap_decoder (dseed::stream* stream, bool ico, dseed::bitmap_decoder** decoder)
{
	dseed::error_t err;

	IcoCurHeader header;
	if (sizeof (IcoCurHeader) != stream->read (&header, sizeof (IcoCurHeader)))
		return dseed::error_io;

	if (header.reserved != 0)
		return dseed::error_not_support_file_format;

	if ((ico && header.image_type != ICO_CUR_IMAGE_TYPE_ICON)
		|| (!ico && header.image_type != ICO_CUR_IMAGE_TYPE_CURSOR))
		return dseed::error_not_support_file_format;

	std::vector<dseed::bitmap*> bitmaps;
	for (auto i = 0; i < header.images; ++i)
	{
		IcoCurEntry entry;
		if (sizeof (IcoCurEntry) != stream->read (&entry, sizeof (IcoCurEntry)))
			return dseed::error_io;

		if (!(entry.reserved == 0 || entry.reserved == 255))
			return dseed::error_not_support_file_format;

		if (entry.offset != stream->position ())
			return dseed::error_not_support_file_format;

		std::vector<uint8_t> bytes (entry.bytes);
		if (entry.bytes != stream->read (bytes.data (), entry.bytes))
			return dseed::error_io;
		dseed::auto_object<dseed::stream> bytesStream;
		if (dseed::failed (err = dseed::create_memorystream (bytes.data (), bytes.size (), &bytesStream)))
			return err;

		dseed::auto_object<dseed::bitmap_decoder> inner_decoder;
		if (dseed::failed (dseed::create_dib_bitmap_decoder (bytesStream, &inner_decoder)))
		{
			bytesStream->seek (dseed::seekorigin::begin, 0);
			if (dseed::failed (dseed::create_png_bitmap_decoder (bytesStream, &inner_decoder)))
				return dseed::error_not_support_file_format;
		}

		dseed::auto_object<dseed::bitmap> bitmap;
		if (dseed::failed (inner_decoder->decode_frame (0, &bitmap, nullptr)))
			return dseed::error_fail;

		if (!ico)
		{
			dseed::auto_object<dseed::attributes> attr;
			bitmap->extra_info (&attr);
			attr->set_size (dseed::attrkey_cursor_hotspot, entry.cur_hotspot_x, entry.cur_hotspot_y);
		}

		bitmaps.push_back (bitmap.detach ());
	}

	*decoder = new __common_bitmap_decoder (bitmaps);
	if (*decoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}

dseed::error_t dseed::create_ico_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder)
{
	return __create_ico_cur_bitmap_decoder (stream, true, decoder);
}
dseed::error_t dseed::create_cur_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder)
{
	return __create_ico_cur_bitmap_decoder (stream, false, decoder);
}
