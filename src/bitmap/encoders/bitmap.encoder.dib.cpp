#include <dseed.h>

#include "../../Microsoft/DIBCommon.hxx"

class __dib_encoder : public dseed::bitmap_encoder
{
public:
	__dib_encoder (dseed::stream* stream)
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
	virtual dseed::error_t encode_frame (dseed::bitmap* bitmap, dseed::timespan_t duration) override
	{
		auto size = bitmap->size ();
		if (size.depth > 1 || size.width > 256 || size.height > 256)
			return dseed::error_not_support;

		auto format = bitmap->format ();
		if (!(format == dseed::pixelformat_bgra8888 || format == dseed::pixelformat_bgr888
			|| format == dseed::pixelformat_bgr565 || format == dseed::pixelformat_grayscale8
			|| format == dseed::pixelformat_bgra8888_indexed8))
			return dseed::error_not_support;

		bitmap_file_header fileHeader;
		memcpy (&fileHeader.bfType, "BM", 2);
		fileHeader.bfReserved1 = fileHeader.bfReserved2 = 0;
		
		bitmap_info_header infoHeader;
		infoHeader.biSize = sizeof (bitmap_info_header);
		infoHeader.biWidth = size.width;
		infoHeader.biHeight = size.height;
		infoHeader.biPlanes = 1;
		switch (format)
		{
		case dseed::pixelformat_bgra8888:
			infoHeader.biBitCount = 32;
			break;
		case dseed::pixelformat_bgr888:
			infoHeader.biBitCount = 24;
			break;
		case dseed::pixelformat_bgr565:
			infoHeader.biBitCount = 16;
			break;
		case dseed::pixelformat_grayscale8:
		case dseed::pixelformat_bgra8888_indexed8:
			infoHeader.biBitCount = 8;
			break;
		}
		infoHeader.biCompression = BI_RGB;
		infoHeader.biClrImportant = 0;
		infoHeader.biXPelsPerMeter = 96;
		infoHeader.biYPelsPerMeter = 96;

		dseed::auto_object<dseed::palette> palette;
		if (format == dseed::pixelformat_bgra8888_indexed8)
		{
			bitmap->palette (&palette);
			infoHeader.biClrUsed = palette->size ();
		}

		fileHeader.bfSize = sizeof (bitmap_file_header) + sizeof (bitmap_info_header)
			+ sizeof (dseed::bgra) * infoHeader.biClrUsed
			+ infoHeader.biSizeImage;
		fileHeader.bfOffBits = sizeof (bitmap_file_header) + sizeof (bitmap_info_header)
			+ sizeof (dseed::bgra) * infoHeader.biClrUsed;
		infoHeader.biSizeImage = dseed::get_bitmap_plane_size (format, size.width, size.height);

		_stream->write (&fileHeader, sizeof (fileHeader));
		_stream->write (&infoHeader, sizeof (infoHeader));
		if (palette != nullptr)
		{
			void* palettePtr;
			palette->pixels_pointer (&palettePtr);
			_stream->write (palettePtr, infoHeader.biClrUsed * 4);
		}

		size_t stride = dseed::get_bitmap_stride (format, size.width);

		for (auto i = 0; i < size.height; ++i)
		{
			void* pixelsPtr;
			bitmap->pixels_pointer_per_line (&pixelsPtr, size.height - i - 1);
			_stream->write (pixelsPtr, stride);
		}

		return dseed::error_good;
	}

public:
	virtual dseed::error_t commit () override
	{
		_stream->flush ();
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	dseed::auto_object<dseed::stream> _stream;
};

dseed::error_t dseed::create_dib_bitmap_encoder (dseed::stream* stream, const dseed::bitmap_encoder_options* options, dseed::bitmap_encoder** encoder)
{
	if (stream == nullptr || encoder == nullptr)
		return dseed::error_invalid_args;

	*encoder = new __dib_encoder (stream);
	if (*encoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}