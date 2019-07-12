#include <dseed.h>

#if defined ( USE_PNG ) && defined ( USE_BITMAP_ENCODERS )
#	include <png.h>
#	include <zlib.h>
#endif

#if defined ( USE_ZOPFLI ) && defined ( USE_BITMAP_ENCODERS )
#	include <zopflipng_lib.h>
#endif

#include <vector>

#if defined ( USE_PNG ) && defined ( USE_BITMAP_ENCODERS )
class __png_encoder : public dseed::bitmap_encoder
{
public:
	__png_encoder (png_structp png, png_infop info, dseed::stream* stream, bool use_zopfli)
		: _refCount (1), _png (png), _info (info), _stream (stream), _use_zopfli (use_zopfli)
		, _alreadyEncoded (false)
	{
		png_set_write_fn (png, stream,
			[](png_structp png, png_bytep buf, size_t size)
			{
				reinterpret_cast<dseed::stream*> (png_get_io_ptr (png))->write (buf, size);
			},
			[](png_structp png)
			{
				reinterpret_cast<dseed::stream*> (png_get_io_ptr (png))->flush ();
			}
			);
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
		if (_alreadyEncoded)
			return dseed::error_invalid_op;

		dseed::size3i size = bitmap->size ();
		dseed::pixelformat_t format = bitmap->format ();
		size_t stride = dseed::get_bitmap_stride (format, size.width);

		if (size.depth > 1)
			return dseed::error_not_support;
		if (!(format == dseed::pixelformat_rgba8888 || format == dseed::pixelformat_rgb888
			|| format == dseed::pixelformat_bgra8888 || format == dseed::pixelformat_bgr888
			|| format == dseed::pixelformat_bgr888_indexed8 || format == dseed::pixelformat_bgra8888_indexed8
			|| format == dseed::pixelformat_grayscale8))
			return dseed::error_not_support;

		std::vector<uint8_t> pixels;
		pixels.resize (stride * size.height);
		size_t pixelsLength;
		bitmap->copy_pixels (pixels.data ());

		std::vector<png_bytep> pixelsRows;
		pixelsRows.resize (size.height);
		for (int y = 0; y < size.height; ++y)
			pixelsRows[y] = pixels.data () + y * stride;

		if (format == dseed::pixelformat_bgr888_indexed8)
		{
			std::vector<uint8_t> palette;
			palette.resize (256 * 3);

			dseed::auto_object<dseed::palette> paletteObj;
			if (dseed::failed (bitmap->palette (&paletteObj)))
				return dseed::error_fail;

			size_t paletteSize = paletteObj->size () / 3;
			if (dseed::failed (paletteObj->copy_palette (palette.data ())))
				return dseed::error_fail;

			for (auto i = 0; i < paletteSize / 3; ++i)
			{
				uint8_t temp = palette[i * 3 + 0];
				palette[i * 3 + 0] = palette[i * 3 + 2];
				palette[i * 3 + 2] = temp;
			}

			png_set_IHDR (_png, _info, size.width, size.height, 8, PNG_COLOR_TYPE_PALETTE,
				PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
			png_set_PLTE (_png, _info, (png_const_colorp)palette.data (), paletteSize / 3);
			png_set_rows (_png, _info, pixelsRows.data ());

			png_write_png (_png, _info, PNG_TRANSFORM_IDENTITY, nullptr);
			png_write_flush (_png);

			_alreadyEncoded = true;
			return dseed::error_good;
		}
		else if (format == dseed::pixelformat_bgra8888_indexed8)
		{
			std::vector<uint8_t> palette;
			palette.resize (256 * 4);

			dseed::auto_object<dseed::palette> paletteObj;
			if (dseed::failed (bitmap->palette (&paletteObj)))
				return dseed::error_fail;

			size_t paletteSize = paletteObj->size () * 4;
			if (dseed::failed (paletteObj->copy_palette (palette.data ())))
				return dseed::error_fail;

			std::vector<uint8_t> newPalette;
			newPalette.resize (paletteSize / 4 * 3);
			for (auto i = 0; i < newPalette.size () / 3; ++i)
			{
				newPalette[i * 3 + 0] = palette[i * 4 + 2];
				newPalette[i * 3 + 1] = palette[i * 4 + 1];
				newPalette[i * 3 + 2] = palette[i * 4 + 0];
			}

			std::vector<uint8_t> alpha;
			alpha.resize (paletteSize / 4);
			for (int i = 0; i < alpha.size (); ++i)
				alpha[i] = palette[i * 4 + 3];

			png_set_IHDR (_png, _info, size.width, size.height, 8, PNG_COLOR_TYPE_PALETTE,
				PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
			png_set_tRNS (_png, _info, alpha.data (), alpha.size (), nullptr);
			png_set_PLTE (_png, _info, (png_const_colorp)newPalette.data (), newPalette.size () / 3);
			png_set_rows (_png, _info, pixelsRows.data ());

			png_write_png (_png, _info, PNG_TRANSFORM_IDENTITY, nullptr);
			png_write_flush (_png);

			_alreadyEncoded = true;
			return dseed::error_good;
		}
		else if (format == dseed::pixelformat_rgba8888 || format == dseed::pixelformat_bgra8888
			|| format == dseed::pixelformat_rgb888 || format == dseed::pixelformat_bgr888
			|| format == dseed::pixelformat_grayscale8)
		{
			int colorType = 0;
			switch (format)
			{
			case dseed::pixelformat_bgra8888:
				png_set_bgr (_png);
			case dseed::pixelformat_rgba8888:
				colorType = PNG_COLOR_TYPE_RGBA;
				break;

			case dseed::pixelformat_bgr888:
				png_set_bgr (_png);
			case dseed::pixelformat_rgb888:
				colorType = PNG_COLOR_TYPE_RGB;
				break;

			case dseed::pixelformat_grayscale8:
				colorType = PNG_COLOR_TYPE_GRAY;
				break;
			}

			png_set_IHDR (_png, _info, size.width, size.height, 8, colorType,
				PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
			png_set_rows (_png, _info, pixelsRows.data ());

			png_write_png (_png, _info, PNG_TRANSFORM_IDENTITY, nullptr);

			_alreadyEncoded = true;
			return dseed::error_good;
		}

		return dseed::error_not_support;
	}
	virtual dseed::error_t commit () override
	{
		png_write_flush (_png);

#if defined ( USE_ZOPFLI )
		if (_use_zopfli)
		{
			_stream->seek (dseed::seekorigin_begin, 0);

			ZopfliPNGOptions options;
			options.lossy_8bit = png_get_color_type (_png, _info) == PNG_COLOR_TYPE_PALETTE ? true : false;
			options.use_zopfli = true;
			options.auto_filter_strategy = true;
			options.lossy_transparent = true;
			options.num_iterations *= 4;
			options.num_iterations_large *= 4;

			std::vector<uint8_t> src;
			src.resize (_stream->length ());
			size_t totalRead = 0;
			while (totalRead != src.size ())
			{
				auto read = _stream->read (src.data () + totalRead, src.size () - totalRead);
				totalRead += read;
			}
			_stream->set_length (0);

			std::vector<byte> dest;
			if (0 != ZopfliPNGOptimize (src, options, false, &dest))
				return dseed::error_not_support;

			size_t totalWritten = 0;
			while (totalWritten != dest.size ())
			{
				auto written = _stream->write (dest.data () + totalWritten, dest.size () - totalWritten);
				totalWritten += written;
			}
		}
#endif

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	png_structp _png;
	png_infop _info;
	dseed::auto_object<dseed::stream> _stream;
	bool _use_zopfli;

	bool _alreadyEncoded;
};
#endif

dseed::error_t dseed::create_png_bitmap_encoder (dseed::stream* stream, const dseed::bitmap_encoder_options* options, dseed::bitmap_encoder** encoder)
{
#if defined ( USE_PNG ) && defined ( USE_BITMAP_ENCODERS )
	png_encoder_options pngOptions;
	if (options != nullptr)
	{
		if (options->options_size != sizeof (png_encoder_options))
			return dseed::error_invalid_args;
		pngOptions = *reinterpret_cast<const png_encoder_options*>(options);
#if !(defined ( USE_ZOPFLI ) && defined ( USE_BITMAP_ENCODERS ) )
		if (pngOptions.use_zopfli_optimization)
			return dseed::error_not_support;
#endif
	}

	png_structp png = png_create_write_struct (PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (png == nullptr)
		return dseed::error_out_of_memory;
	png_infop info = png_create_info_struct (png);
	if (info == nullptr)
	{
		png_destroy_read_struct (&png, nullptr, nullptr);
		return dseed::error_out_of_memory;
	}

	png_set_filter (png, PNG_FILTER_TYPE_BASE, PNG_ALL_FILTERS);
	png_set_compression_level (png, Z_BEST_COMPRESSION);

	*encoder = new __png_encoder (png, info, stream, pngOptions.use_zopfli_optimization);
	if (*encoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}