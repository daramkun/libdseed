#include <dseed.h>

#include <string.h>

class __palette : public dseed::palette
{
public:
	__palette (const void* pixels, size_t size, int bpp)
		: _refCount (1), _bpp (bpp)
	{
		_palette.resize (size * (bpp / 8));
		if (pixels != nullptr)
			memcpy (_palette.data (), pixels, _palette.size ());
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
	virtual size_t size () override { return _palette.size () / (_bpp / 8); }
	virtual int bits_per_pixel () override { return _bpp; }

public:
	virtual dseed::error_t copy_palette (void* dest) override
	{
		if (dest == nullptr) return dseed::error_invalid_args;
		memcpy (dest, _palette.data (), _palette.size ());
		return dseed::error_good;
	}

public:
	virtual void resize (size_t size) override
	{
		_palette.resize (size * (_bpp / 8));
	}
	virtual dseed::error_t pixels_pointer (void** ptr) override
	{
		if (ptr == nullptr) return dseed::error_invalid_args;
		*ptr = _palette.data ();
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	std::vector<uint8_t> _palette;
	int _bpp;
};

dseed::error_t dseed::create_palette (const void* pixels, int bpp, size_t pixelsCount, palette** palette)
{
	if (!(pixelsCount > 0 && pixelsCount <= 256) || palette == nullptr)
		return dseed::error_invalid_args;

	*palette = new __palette (pixels, pixelsCount, bpp);
	if (*palette == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}

class __bitmap : public dseed::bitmap
{
public:
	__bitmap (dseed::bitmaptype_t type, dseed::pixelformat_t format, const dseed::size3i& size, dseed::palette* palette)
		: _refCount (1)
		, _type (type), _format (format), _size (size), _palette (palette)
	{
		_stride = dseed::get_bitmap_stride (format, size.width);
		_planeSize = dseed::get_bitmap_plane_size (format, size.width, size.height);
		
		size_t bufferSize = dseed::get_bitmap_total_size (format, size);
		_pixels.resize (bufferSize);

		dseed::create_attributes (&_extraInfo);
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
	virtual dseed::bitmaptype_t type () override { return _type; }
	virtual dseed::pixelformat_t format () override { return _format; }
	virtual dseed::size3i size () override { return _size; }

public:
	virtual dseed::error_t palette (dseed::palette** palette) override
	{
		if (_palette == nullptr)
			return dseed::error_feature_not_support;
		*palette = _palette;
		_palette->retain ();
		return dseed::error_good;
	}

public:
	virtual dseed::error_t copy_pixels_per_line (void* dest, size_t y, size_t depth = 0) override
	{
		if (_pixels.size () == 0)
			return dseed::error_not_support;
		if (_format >= dseed::pixelformat_bc1)
			return dseed::error_not_support;
		if ((depth >= _size.depth || depth < 0) || (y >= _size.height || y < 0) || dest == nullptr)
			return dseed::error_invalid_args;

		size_t stride = dseed::get_bitmap_stride (format (), _size.width), start = (depth * _planeSize) * (y * stride);
		memcpy (dest, _pixels.data () + start, stride);

		return dseed::error_good;
	}

	virtual dseed::error_t copy_pixels (void* dest, size_t depth = 0) override
	{
		if (_pixels.size () == 0)
			return dseed::error_not_support;
		if ((depth >= _size.depth || depth < 0) || dest == nullptr)
			return dseed::error_invalid_args;

		memcpy (dest, _pixels.data () + (depth * _planeSize), _pixels.size ());

		return dseed::error_good;
	}

public:
	virtual dseed::error_t pixels_pointer_per_line (void** ptr, size_t y, size_t depth = 0) override
	{
		if (_format >= dseed::pixelformat_bc1)
			return dseed::error_not_support;
		if ((depth >= _size.depth || depth < 0) || (y >= _size.height || y < 0))
			return dseed::error_invalid_args;
		*ptr = _pixels.data () + (depth * _planeSize) + (y * _stride);
		return dseed::error_good;
	}

	virtual dseed::error_t pixels_pointer (void** ptr, size_t depth = 0) override
	{
		if (depth >= _size.depth || depth < 0)
			return dseed::error_invalid_args;
		*ptr = _pixels.data () + (depth * _planeSize);
		return dseed::error_good;
	}

public:
	virtual dseed::error_t read_pixels (void* buffer, const dseed::point2i& pos, const dseed::size2i& size, size_t depth = 0) override
	{
		if (_format >= dseed::pixelformat_bc1)
			return dseed::error_not_support;
		
		size_t startPoint = dseed::get_bitmap_total_size (_format, _size) * depth;
		size_t pixelStride = ((((_format >> 16) & 0xffff) != 0x0002)
			? (_format & 0xf)
			: (_format >> 8) & 0xff);

		for (auto y = pos.y; y < pos.y + size.height; ++y)
		{
			uint8_t* bufferY = ((uint8_t*)buffer) + startPoint + dseed::get_bitmap_stride (_format, size.width) * (y - pos.y);
			uint8_t* pixelsY = _pixels.data () + startPoint + _stride * y + pixelStride * pos.x;

			memcpy (bufferY, pixelsY, pixelStride * size.width);
		}

		return dseed::error_good;
	}

	virtual dseed::error_t write_pixels (const void* data, const dseed::point2i& pos, const dseed::size2i& size, size_t depth = 0) override
	{
		if (_format >= dseed::pixelformat_bc1)
			return dseed::error_not_support;

		size_t startPoint = dseed::get_bitmap_total_size (_format, _size) * depth;
		size_t pixelStride = ((((_format >> 16) & 0xffff) != 0x0002)
			? (_format & 0xf)
			: (_format >> 8) & 0xff);

		for (auto y = pos.y; y < pos.y + size.height; ++y)
		{
			const uint8_t* bufferY = ((const uint8_t*)data) + startPoint + dseed::get_bitmap_stride (_format, size.width) * (y - pos.y);
			uint8_t* pixelsY = _pixels.data () + startPoint + _stride * y + pixelStride * pos.x;

			memcpy (pixelsY, bufferY, pixelStride * size.width);
		}

		return dseed::error_good;
	}

public:
	virtual dseed::error_t extra_info (dseed::attributes** attr) override
	{
		if (attr == nullptr) return dseed::error_invalid_args;
		*attr = _extraInfo;
		(*attr)->retain ();
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;

	dseed::bitmaptype_t _type;
	dseed::pixelformat_t _format;
	dseed::size3i _size;

	dseed::auto_object<dseed::palette> _palette;

	std::vector<uint8_t> _pixels;
	size_t _stride, _planeSize;

	dseed::auto_object<dseed::attributes> _extraInfo;
};

dseed::error_t dseed::create_bitmap (bitmaptype_t type, const size3i& size, pixelformat_t format, palette* palette, bitmap** bitmap)
{
	if (size.width <= 0 || size.height <= 0 || size.depth <= 0 || bitmap == nullptr
		|| !(type >= bitmaptype_2d && type <= bitmaptype_3d))
		return dseed::error_invalid_args;
	if (format == pixelformat_bgra8888_indexed8 || format == pixelformat_bgr888_indexed8)
	{
		if (palette == nullptr)
			return dseed::error_invalid_args;
		else
		{
			if ((format == pixelformat_bgra8888_indexed8 && palette->bits_per_pixel () == 32)
				&& (format == pixelformat_bgr888_indexed8 && palette->bits_per_pixel () == 24))
				return dseed::error_invalid_args;
		}
	}

	*bitmap = new __bitmap (type, format, size, palette);
	if (*bitmap == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}

constexpr int32_t MAXIMUM_BITMAP_DECODER_COUNT = 512;
std::function<dseed::error_t (dseed::stream*, dseed::bitmap_decoder**)> g_bitmap_decoder_creator[MAXIMUM_BITMAP_DECODER_COUNT];
std::atomic<int32_t> g_bitmap_decoder_creator_count;

dseed::error_t dseed::add_bitmap_decoder (createbitmapdecoder_fn fn)
{
	if (fn == nullptr)
		return dseed::error_invalid_args;

	g_bitmap_decoder_creator[++g_bitmap_decoder_creator_count] = fn;
	return dseed::error_good;
}

dseed::error_t dseed::detect_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder)
{
	if (stream == nullptr || !stream->seekable () || decoder == nullptr)
		return dseed::error_invalid_args;

	for (int i = 0; i < g_bitmap_decoder_creator_count; ++i)
	{
		stream->seek (dseed::seekorigin_begin, 0);
		auto err = g_bitmap_decoder_creator[i] (stream, decoder);
		if (dseed::succeeded (err))
			return dseed::error_good;
		else
		{
			if (*decoder)
				(*decoder)->release ();
		}
	}

	return dseed::error_not_support;
}
