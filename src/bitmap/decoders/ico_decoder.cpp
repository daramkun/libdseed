#include <dseed.h>

#include "../../libs/ICOHelper.hxx"

dseed::error_t __create_ico_cur_bitmap_decoder (dseed::io::stream* stream, bool ico, dseed::bitmaps::bitmap_array** decoder) noexcept
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

	std::vector<dseed::bitmaps::bitmap*> bitmaps;
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
		dseed::autoref<dseed::io::stream> bytesStream;
		if (dseed::failed (err = dseed::io::create_fixed_memorystream (bytes.data (), bytes.size (), &bytesStream)))
			return err;

		dseed::autoref<dseed::bitmaps::bitmap_array> inner_decoder;
		if (dseed::failed (dseed::bitmaps::create_dib_bitmap_decoder (bytesStream, &inner_decoder)))
		{
			bytesStream->seek (dseed::io::seekorigin::begin, 0);
			if (dseed::failed (dseed::bitmaps::create_png_bitmap_decoder (bytesStream, &inner_decoder)))
				return dseed::error_not_support_file_format;
		}

		dseed::autoref<dseed::bitmaps::bitmap> bitmap;
		if (dseed::failed (inner_decoder->at (0, &bitmap)))
			return dseed::error_fail;

		if (!ico)
		{
			dseed::autoref<dseed::attributes> attr;
			bitmap->extra_info (&attr);
			attr->set_size (dseed::attrkey_cursor_hotspot, entry.cur_hotspot_x, entry.cur_hotspot_y);
		}

		bitmaps.push_back (bitmap.detach ());
	}

	return create_bitmap_array(dseed::bitmaps::arraytype::plain, bitmaps, decoder);
}

dseed::error_t dseed::bitmaps::create_ico_bitmap_decoder (dseed::io::stream* stream, dseed::bitmaps::bitmap_array** decoder) noexcept
{
	return __create_ico_cur_bitmap_decoder (stream, true, decoder);
}
dseed::error_t dseed::bitmaps::create_cur_bitmap_decoder (dseed::io::stream* stream, dseed::bitmaps::bitmap_array** decoder) noexcept
{
	return __create_ico_cur_bitmap_decoder (stream, false, decoder);
}