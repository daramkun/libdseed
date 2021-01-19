#include <dseed.h>

#include <cstring>

#include "../../libs/DIBHelper.hxx"

#ifndef _WINGDI_
#	define BI_RGB        0L
#	define BI_RLE8       1L
#	define BI_RLE4       2L
#	define BI_BITFIELDS  3L
#	define BI_JPEG       4L
#	define BI_PNG        5L
#endif

class __dib_encoder : public dseed::bitmaps::bitmap_encoder
{
public:
	__dib_encoder (dseed::io::stream* stream)
		: _refCount (1), _stream (stream)
	{ }

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
	virtual dseed::error_t encode_frame (dseed::bitmaps::bitmap* bitmap) noexcept override
	{
		auto size = bitmap->size ();
		if (size.depth > 1 || size.width > 256 || size.height > 256)
			return dseed::error_not_support;

		auto format = bitmap->format ();
		if (!(format == dseed::color::pixelformat::bgra8 || format == dseed::color::pixelformat::bgr8
			|| format == dseed::color::pixelformat::bgr565 || format == dseed::color::pixelformat::r8
			|| format == dseed::color::pixelformat::bgra8_indexed8))
			return dseed::error_not_support;

		bitmap_file_header fileHeader = {};
		memcpy (&fileHeader.bfType, "BM", 2);
		fileHeader.bfReserved1 = fileHeader.bfReserved2 = 0;

		bitmap_info_header infoHeader = {};
		infoHeader.biSize = sizeof (bitmap_info_header);
		infoHeader.biWidth = size.width;
		infoHeader.biHeight = size.height;
		infoHeader.biPlanes = 1;
		switch (format)
		{
			case dseed::color::pixelformat::bgra8:
				infoHeader.biBitCount = 32;
				break;
			case dseed::color::pixelformat::bgr8:
				infoHeader.biBitCount = 24;
				break;
			case dseed::color::pixelformat::bgr565:
				infoHeader.biBitCount = 16;
				break;
			case dseed::color::pixelformat::r8:
			case dseed::color::pixelformat::bgra8_indexed8:
				infoHeader.biBitCount = 8;
				break;
			default:
				return dseed::error_not_support;
		}
		infoHeader.biCompression = BI_RGB;
		infoHeader.biClrImportant = 0;
		infoHeader.biXPelsPerMeter = 96;
		infoHeader.biYPelsPerMeter = 96;

		dseed::autoref<dseed::bitmaps::palette> palette;
		if (format == dseed::color::pixelformat::bgra8_indexed8)
		{
			bitmap->palette (&palette);
			infoHeader.biClrUsed = (uint32_t)palette->size ();
		}

		fileHeader.bfSize = sizeof (bitmap_file_header) + sizeof (bitmap_info_header)
			+ sizeof (dseed::color::bgra8) * infoHeader.biClrUsed
			+ infoHeader.biSizeImage;
		fileHeader.bfOffBits = sizeof (bitmap_file_header) + sizeof (bitmap_info_header)
			+ sizeof (dseed::color::bgra8) * infoHeader.biClrUsed;
		infoHeader.biSizeImage = (uint32_t)dseed::color::calc_bitmap_plane_size (format, dseed::size2i (size.width, size.height));

		_stream->write (&fileHeader, sizeof (fileHeader));
		_stream->write (&infoHeader, sizeof (infoHeader));
		if (palette != nullptr)
		{
			void* palettePtr;
			palette->lock (&palettePtr);
			_stream->write (palettePtr, infoHeader.biClrUsed * 4);
			palette->unlock ();
		}

		size_t stride = dseed::color::calc_bitmap_stride (format, size.width);

		uint8_t* pixelsPtr;
		bitmap->lock ((void**)&pixelsPtr);
		for (auto i = 0; i < size.height; ++i)
			_stream->write (pixelsPtr +(( size.height - i - 1) * stride), stride);
		bitmap->unlock ();

		return dseed::error_good;
	}
	virtual dseed::bitmaps::arraytype type () noexcept override { return dseed::bitmaps::arraytype::plain; }

public:
	virtual dseed::error_t commit () noexcept override
	{
		_stream->flush ();
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	dseed::autoref<dseed::io::stream> _stream;
};

dseed::error_t dseed::bitmaps::create_dib_bitmap_encoder (dseed::io::stream* stream, const dseed::bitmaps::bitmap_encoder_options* options, dseed::bitmaps::bitmap_encoder** encoder)
{
	if (stream == nullptr || encoder == nullptr)
		return dseed::error_invalid_args;

	*encoder = new __dib_encoder (stream);
	if (*encoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}