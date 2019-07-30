#include <dseed.h>

#include "../../libs/Microsoft/ICOCURCommon.hxx"

class __ico_cur_encoder : public dseed::bitmap_encoder
{
public:
	__ico_cur_encoder (dseed::stream* stream, bool ico, const dseed::png_encoder_options* options)
		: _refCount (1), _stream (stream), _ico (ico)
	{
		if (options)
		{
			_haveOptions = true;
			_options = *options;
		}
		else
			_haveOptions = false;
	}

public:
	virtual int32_t retain () override { return ++_refCount; }
	virtual int32_t release () override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual dseed::error_t encode_frame (dseed::bitmap* bitmap, dseed::timespan_t duration) override
	{
		auto size = bitmap->size ();
		if (size.depth > 1 || size.width > 256 || size.height > 256)
			return dseed::error_not_support;

		auto format = bitmap->format ();
		if (!(format == dseed::pixelformat_bgra8888 || format == dseed::pixelformat_bgr888
			|| format == dseed::pixelformat_bgra8888_indexed8))
			return dseed::error_not_support;

		for (auto& containedBitmap : _bitmaps)
		{
			auto containedSize = containedBitmap->size ();
			auto containedFormat = containedBitmap->format ();

			if (size.width == containedSize.width && size.height == containedSize.height
				&& format == containedFormat)
				return dseed::error_invalid_op;
		}

		_bitmaps.push_back (bitmap);

		return dseed::error_good;
	}

public:
	virtual dseed::error_t commit () override
	{
		IcoCurHeader header = { 0, (uint16_t)(_ico ? ICO_CUR_IMAGE_TYPE_ICON : ICO_CUR_IMAGE_TYPE_CURSOR), (uint16_t)_bitmaps.size () };
		_stream->write (&header, sizeof (IcoCurHeader));

		for (auto& bitmap : _bitmaps)
		{
			auto size = bitmap->size ();
			auto format = bitmap->format ();

			IcoCurEntry entry;
			entry.width = size.width == 256 ? 0 : size.width;
			entry.height = size.width == 256 ? 0 : size.height;
			if (format & 0x00020000)
			{
				dseed::auto_object<dseed::palette> palette;
				bitmap->palette (&palette);
				entry.palettes = (uint8_t)palette->size ();
			}
			else entry.palettes = 0;
			entry.reserved = 0;
			if (_ico)
			{
				entry.ico_planes = 1;
				entry.ico_bits_per_pixels = (format & 0x00020000) ? 1 : format & 0x000000ff;
			}
			else
			{
				dseed::auto_object<dseed::attributes> attr;
				bitmap->extra_info (&attr);

				uint32_t x, y;
				if (dseed::succeeded (attr->get_size (dseed::attrkey_cursor_hotspot, &x, &y)))
				{
					entry.cur_hotspot_x = x;
					entry.cur_hotspot_y = y;
				}
				else
				{
					entry.cur_hotspot_x = entry.cur_hotspot_y = 0;
				}
			}
			entry.offset = (uint32_t)(_stream->position () + sizeof (IcoCurEntry));

			dseed::auto_object<dseed::stream> encodeStream;
			dseed::create_variable_memorystream (&encodeStream);

			dseed::auto_object<dseed::bitmap_encoder> encoder;
			if (_haveOptions)
				dseed::create_png_bitmap_encoder (encodeStream, &_options, &encoder);
			else
				dseed::create_dib_bitmap_encoder (encodeStream, nullptr, &encoder);

			encoder->encode_frame (bitmap, 0);
			encoder->commit ();

			encodeStream->seek (dseed::seekorigin_begin, 0);

			std::vector<uint8_t> bytes (encodeStream->length ());
			encodeStream->read (bytes.data (), bytes.size ());

			entry.bytes = (uint32_t)bytes.size ();

			_stream->write (&entry, sizeof (IcoCurEntry));
			_stream->write (bytes.data (), bytes.size ());
		}

		_stream->flush ();

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	dseed::auto_object<dseed::stream> _stream;
	bool _ico;

	std::vector<dseed::auto_object<dseed::bitmap>> _bitmaps;

	bool _haveOptions;
	dseed::png_encoder_options _options;
};

dseed::error_t dseed::create_ico_bitmap_encoder (dseed::stream* stream, const dseed::bitmap_encoder_options* options, dseed::bitmap_encoder** encoder)
{
	return dseed::error_good;
}
dseed::error_t dseed::create_cur_bitmap_encoder (dseed::stream* stream, const dseed::bitmap_encoder_options* options, dseed::bitmap_encoder** encoder)
{
	return dseed::error_good;
}