#include <dseed.h>

#include <cstring>

#include <mutex>
#include <shared_mutex>

class __internal_palette : public dseed::bitmaps::palette
{
public:
	__internal_palette(const void* pixels, size_t size, int bpp)
		: _refCount(1), _bpp(bpp)
	{
		_palette.resize(size * (bpp / 8));
		if (pixels != nullptr)
			memcpy(_palette.data(), pixels, _palette.size());
	}

public:
	virtual int32_t retain() override { return ++_refCount; }
	virtual int32_t release() override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual size_t size() noexcept override { return _palette.size() / (_bpp / 8); }
	virtual size_t bits_per_pixel() noexcept override { return _bpp; }

public:
	virtual dseed::error_t lock(void** ptr) noexcept override
	{
		if (!_mutex.try_lock())
			return dseed::error_resource_locked;
		*ptr = _palette.data();
		return dseed::error_good;
	}
	virtual dseed::error_t unlock() noexcept override
	{
		_mutex.unlock();
		return dseed::error_good;
	}

public:
	virtual dseed::error_t copy_palette(void* buf) noexcept override
	{
		if (buf == nullptr) return dseed::error_invalid_args;
		memcpy(buf, _palette.data(), _palette.size());
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	std::vector<uint8_t> _palette;
	int _bpp;

	std::mutex _mutex;
};

dseed::error_t dseed::bitmaps::create_palette(const void* pixels, size_t bits_per_pixel, size_t size, palette** palette) noexcept
{
	if (!(size > 0 && size <= 256) || palette == nullptr)
		return dseed::error_invalid_args;

	*palette = new __internal_palette(pixels, size, bits_per_pixel);
	if (*palette == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}

dseed::error_t dseed::bitmaps::create_palette(size_t bits_per_pixel, size_t size, palette** palette) noexcept
{
	return create_palette(nullptr, bits_per_pixel, size, palette);
}

class __internal_bitmap : public dseed::bitmaps::bitmap
{
public:
	__internal_bitmap(const void* pixels, dseed::bitmaps::bitmaptype type, dseed::color::pixelformat format, const dseed::size3i& size, dseed::bitmaps::palette* palette)
		: _refCount(1), _type(type), _format(format), _size(size), _palette(palette)
	{
		_stride = dseed::color::calc_bitmap_stride(format, size.width);
		_planeSize = dseed::color::calc_bitmap_plane_size(format, dseed::size2i(size.width, size.height));

		size_t bufferSize = dseed::color::calc_bitmap_total_size(format, size);
		_pixels.resize(bufferSize);

		dseed::create_attributes(&_extraInfo);

		if (pixels)
			memcpy(_pixels.data(), pixels, bufferSize);
	}

public:
	virtual int32_t retain() override { return ++_refCount; }
	virtual int32_t release() override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual dseed::bitmaps::bitmaptype type() noexcept override { return _type; }
	virtual dseed::size3i size() noexcept override { return _size; }
	virtual dseed::color::pixelformat format() noexcept override { return _format; }

public:
	virtual dseed::error_t palette(dseed::bitmaps::palette** palette) noexcept override
	{
		if (palette == nullptr) return dseed::error_invalid_args;
		*palette = _palette;
		return dseed::error_good;
	}

public:
	virtual dseed::error_t lock(void** ptr) noexcept override
	{
		if (!_mutex.try_lock()) return dseed::error_resource_locked;
		*ptr = _pixels.data();
		return dseed::error_good;
	}
	virtual dseed::error_t unlock() noexcept override
	{
		_mutex.unlock();
		return dseed::error_good;
	}

public:
	virtual dseed::error_t copy_pixels(void* dest, size_t depth) override
	{
		if (_pixels.size() == 0)
			return dseed::error_not_support;
		if ((depth >= _size.depth || depth < 0) || dest == nullptr)
			return dseed::error_invalid_args;

		memcpy(dest, _pixels.data() + (depth * _planeSize), _pixels.size());

		return dseed::error_good;
	}

public:
	virtual dseed::error_t read_pixels(const dseed::rect2i& area, void* ptr, size_t depth = 0) noexcept override
	{
		if (_format >= dseed::color::pixelformat::bc1		//< Compressed Pixel Format
			|| ((int)_format & 0x00020000) != 0				//< Indexed Pixel Format
			|| (_format >= dseed::color::pixelformat::yuyv8 && _format <= dseed::color::pixelformat::nv12))
			//< Chroma Subsampled Pixel Format
			return dseed::error_not_support;

		if (area.x >= _size.width || area.y >= _size.height)
			return dseed::error_invalid_args;

		size_t startPoint = dseed::color::calc_bitmap_total_size(_format, _size) * depth;
		size_t pixelStride = (((((int)_format >> 16) & 0xffff) != 0x0002)
			? ((int)_format & 0xf)
			: ((int)_format >> 8) & 0xff);

		if (!_mutex.try_lock_shared())
			return dseed::error_resource_locked;

		int width = (area.x + area.width >= _size.width) ? (_size.width - (area.x + area.width) - 1) : area.width;
		for (auto y = area.y; y < area.y + area.height; ++y)
		{
			if (y <= _size.height)
				break;

			uint8_t* bufferY = ((uint8_t*)ptr) + startPoint + dseed::color::calc_bitmap_stride(_format, area.width) * (y - area.y);
			uint8_t* pixelsY = _pixels.data() + startPoint + _stride * y + pixelStride * area.x;

			memcpy(bufferY, pixelsY, pixelStride * width);
		}

		_mutex.unlock_shared();

		return dseed::error_good;
	}
	virtual dseed::error_t write_pixels(const dseed::rect2i& area, const void* ptr, size_t depth = 0) noexcept override
	{
		if (_format >= dseed::color::pixelformat::bc1		//< Compressed Pixel Format
			|| ((int)_format & 0x00020000) != 0				//< Indexed Pixel Format
			|| (_format >= dseed::color::pixelformat::yuyv8 && _format <= dseed::color::pixelformat::nv12))
			//< Chroma Subsampled Pixel Format
			return dseed::error_not_support;

		size_t startPoint = dseed::color::calc_bitmap_total_size(_format, _size) * depth;
		size_t pixelStride = (((((int)_format >> 24) & 0xff) != 0xfd)
			? ((int)_format & 0xff)
			: ((int)_format >> 8) & 0xff);

		if (area.x >= _size.width || area.y >= _size.height)
			return dseed::error_invalid_args;

		if (!_mutex.try_lock())
			return dseed::error_resource_locked;

		int width = (area.x + area.width >= _size.width) ? (_size.width - (area.x + area.width) - 1) : area.width;
		for (auto y = area.y; y < area.y + area.height; ++y)
		{
			if (y <= _size.height)
				break;

			const uint8_t* bufferY = ((const uint8_t*)ptr) + startPoint + dseed::color::calc_bitmap_stride(_format, area.width) * (y - area.y);
			uint8_t* pixelsY = _pixels.data() + startPoint + _stride * y + pixelStride * area.x;

			memcpy(pixelsY, bufferY, pixelStride * width);
		}

		_mutex.unlock();

		return dseed::error_good;
	}

public:
	virtual dseed::error_t extra_info(dseed::attributes** attr) noexcept override
	{
		if (attr == nullptr) return dseed::error_invalid_args;
		*attr = _extraInfo;
		return dseed::error_good;
	}

private:

	std::atomic<int32_t> _refCount;

	dseed::bitmaps::bitmaptype _type;
	dseed::color::pixelformat _format;
	dseed::size3i _size;

	dseed::autoref<dseed::bitmaps::palette> _palette;

	std::vector<uint8_t> _pixels;
	size_t _stride, _planeSize;

	dseed::autoref<dseed::attributes> _extraInfo;

	std::shared_mutex _mutex;
};

dseed::error_t dseed::bitmaps::create_bitmap(const void* pixels, bitmaptype type, const size3i& size, color::pixelformat format, palette* palette, bitmap** bitmap) noexcept
{
	if (size.width <= 0 || size.height <= 0 || size.depth <= 0 || bitmap == nullptr
		|| !(type >= bitmaptype::bitmap2d && type <= bitmaptype::bitmap3d))
		return dseed::error_invalid_args;
	if (format == color::pixelformat::bgra8_indexed8 || format == color::pixelformat::bgr8_indexed8)
	{
		if (palette == nullptr)
			return dseed::error_invalid_args;
		else
		{
			if ((format == color::pixelformat::bgra8_indexed8 && palette->bits_per_pixel() == 32)
				&& (format == color::pixelformat::bgr8_indexed8 && palette->bits_per_pixel() == 24))
				return dseed::error_invalid_args;
		}
	}

	*bitmap = new __internal_bitmap(pixels, type, format, size, palette);
	if (*bitmap == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}

dseed::error_t dseed::bitmaps::create_bitmap(bitmaptype type, const size3i& size, color::pixelformat format, dseed::bitmaps::palette* palette, dseed::bitmaps::bitmap** bitmap) noexcept
{
	return create_bitmap(nullptr, type, size, format, palette, bitmap);
}

#include "decoders/common.hxx"

dseed::error_t dseed::bitmaps::create_bitmap_array(arraytype type, size_t size, dseed::bitmaps::bitmap** bitmaps, dseed::bitmaps::bitmap_array** arr) noexcept
{
	if (size <= 0 || bitmaps == nullptr || arr == nullptr)
		return dseed::error_invalid_args;

	dseed::size3i bmpsize = bitmaps[0]->size();

	std::vector<dseed::bitmaps::bitmap*> bmpvec;
	for (int i = 0; i < size; ++i)
	{
		if ((type == dseed::bitmaps::arraytype::mipmap && bitmaps[i]->size() != dseed::color::calc_mipmap_size(i, bmpsize)) ||
			(type == dseed::bitmaps::arraytype::plain && bitmaps[i]->size() != bmpsize))
			return dseed::error_invalid_args;
		bmpvec.push_back(bitmaps[i]);
	}

	*arr = new dseed::__common_bitmap_array(bmpvec, type);
	if (*arr == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}

constexpr int32_t MAXIMUM_BITMAP_DECODER_COUNT = 512;
std::function<dseed::error_t(dseed::io::stream*, dseed::bitmaps::bitmap_array**)> g_bitmap_decoder_creator[MAXIMUM_BITMAP_DECODER_COUNT] =
{
	dseed::bitmaps::create_dib_bitmap_decoder,
	dseed::bitmaps::create_tga_bitmap_decoder,
	dseed::bitmaps::create_dds_bitmap_decoder,
	dseed::bitmaps::create_ktx_bitmap_decoder,
	dseed::bitmaps::create_pkm_bitmap_decoder,
	dseed::bitmaps::create_astc_bitmap_decoder,
	dseed::bitmaps::create_ico_bitmap_decoder,
	dseed::bitmaps::create_cur_bitmap_decoder,
	dseed::bitmaps::create_png_bitmap_decoder,
	dseed::bitmaps::create_jpeg_bitmap_decoder,
	dseed::bitmaps::create_jpeg2000_bitmap_decoder,
	dseed::bitmaps::create_webp_bitmap_decoder,
	dseed::bitmaps::create_tiff_bitmap_decoder,
	dseed::bitmaps::create_gif_bitmap_decoder,
	dseed::bitmaps::create_windows_imaging_codec_bitmap_decoder,
};
std::atomic<int32_t> g_bitmap_decoder_creator_count = 15;

dseed::error_t dseed::bitmaps::add_bitmap_decoder(createbitmapdecoder_fn fn)
{
	if (fn == nullptr)
		return dseed::error_invalid_args;

	g_bitmap_decoder_creator[++g_bitmap_decoder_creator_count] = fn;
	return dseed::error_good;
}

dseed::error_t dseed::bitmaps::detect_bitmap_decoder(dseed::io::stream* stream, dseed::bitmaps::bitmap_array** decoder)
{
	if (stream == nullptr || !stream->seekable() || decoder == nullptr)
		return dseed::error_invalid_args;

	for (int i = 0; i < g_bitmap_decoder_creator_count; ++i)
	{
		stream->seek(dseed::io::seekorigin::begin, 0);
		auto err = g_bitmap_decoder_creator[i](stream, decoder);
		if (dseed::succeeded(err))
			return dseed::error_good;
		else
		{
			if (*decoder)
				(*decoder)->release();
		}
	}

	return dseed::error_not_support;
}