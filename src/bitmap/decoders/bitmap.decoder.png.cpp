#include <dseed.h>

#if defined ( USE_PNG )
#	include <png.h>
#endif

#include "bitmap.decoder.hxx"

constexpr int PNG_BYTES_TO_CHECK = 8;

dseed::error_t dseed::create_png_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder)
{
#if defined ( USE_PNG )
	if (stream == nullptr || decoder == nullptr)
		return dseed::error_invalid_args;

	png_byte pngsig[PNG_BYTES_TO_CHECK];
	if (PNG_BYTES_TO_CHECK != stream->read (pngsig, PNG_BYTES_TO_CHECK))
		return dseed::error_invalid_args;

	if (0 != png_sig_cmp (pngsig, (png_size_t)0, PNG_BYTES_TO_CHECK))
		return dseed::error_invalid_args;

	png_structp png = png_create_read_struct (PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (png == nullptr)
		return dseed::error_out_of_memory;
	png_infop info = png_create_info_struct (png);
	if (info == nullptr)
	{
		png_destroy_read_struct (&png, nullptr, nullptr);
		return dseed::error_out_of_memory;
	}

	png_set_read_fn (png, stream, [](png_structp png, png_bytep buf, size_t size)
		{
			reinterpret_cast<dseed::stream*> (png_get_io_ptr (png))->read (buf, size);
		});

	png_set_sig_bytes (png, PNG_BYTES_TO_CHECK);
	png_read_info (png, info);

	auto colorType = png_get_color_type (png, info);
	switch (colorType)
	{
	case PNG_COLOR_TYPE_PALETTE:
		png_set_palette_to_rgb (png);
		break;

	case PNG_COLOR_TYPE_GRAY:
		png_set_expand_gray_1_2_4_to_8 (png);
		break;
	}

	auto bitDepth = png_get_bit_depth (png, info);
	if (bitDepth == 16)
		png_set_strip_16 (png);

	png_read_update_info (png, info);

	dseed::size3i size (png_get_image_width (png, info), png_get_image_height (png, info), 1);

	bitDepth = png_get_bit_depth (png, info);
	colorType = png_get_color_type (png, info);
	auto channels = png_get_channels (png, info);

	dseed::pixelformat_t format;
	switch (channels)
	{
	case 1:
		if (colorType == PNG_COLOR_TYPE_GRAY)
			format = dseed::pixelformat_grayscale8;
		else if (colorType == PNG_COLOR_TYPE_PALETTE)
		{
			png_set_bgr (png);
			if (png_get_valid (png, info, PNG_INFO_tRNS))
				format = dseed::pixelformat_bgra8888_indexed8;
			else
				format = dseed::pixelformat_bgr888_indexed8;
		}
		else
		{
			png_destroy_read_struct (&png, nullptr, nullptr);
			return dseed::error_not_support;
		}
		break;

	case 3:
		format = dseed::pixelformat_rgb888;
		break;

	case 4:
		if (bitDepth == 8)
			format = dseed::pixelformat_rgba8888;
		else
		{
			png_destroy_read_struct (&png, nullptr, nullptr);
			return dseed::error_not_support;
		}
		break;

	default:
		png_destroy_read_struct (&png, nullptr, nullptr);
		return dseed::error_not_support;
	}

	size_t stride = dseed::get_bitmap_stride (format, size.width);
	size_t totalBytes = stride * size.height;

	dseed::auto_object<dseed::bitmap> bitmap;
	if (format == dseed::pixelformat_rgba8888 || format == dseed::pixelformat_rgb888 ||
		format == dseed::pixelformat_grayscale8)
	{
		create_bitmap (dseed::bitmaptype_2d, size, format, nullptr, &bitmap);
	}
	else if (format == dseed::pixelformat_bgr888_indexed8 || format == dseed::pixelformat_bgra8888_indexed8)
	{
		png_colorp palette;
		int numPalette;
		png_get_PLTE (png, info, &palette, &numPalette);

		if (format == dseed::pixelformat_bgr888)
		{
			for (int i = 0; i < numPalette; ++i)
			{
				auto temp = palette[i].red;
				palette[i].red = palette[i].blue;
				palette[i].blue = temp;
			}

			dseed::auto_object<dseed::palette> paletteObj;
			dseed::create_palette (palette, 24, numPalette, &paletteObj);

			create_bitmap (dseed::bitmaptype_2d, size, format, paletteObj, &bitmap);
		}
		else if (format == dseed::pixelformat_bgra8888)
		{
			png_bytep alpha;
			int numAlpha;
			png_get_tRNS (png, info, &alpha, &numAlpha, nullptr);

			std::vector<uint8_t> newPalette;
			newPalette.resize (numPalette * 4);

			for (int i = 0; i < numPalette; ++i)
			{
				newPalette[i * 4 + 0] = palette[i].blue;
				newPalette[i * 4 + 0] = palette[i].green;
				newPalette[i * 4 + 0] = palette[i].red;
				newPalette[i * 4 + 0] = alpha[i];
			}

			dseed::auto_object<dseed::palette> paletteObj;
			dseed::create_palette (palette, 32, numPalette, &paletteObj);

			create_bitmap (dseed::bitmaptype_2d, size, format, paletteObj, &bitmap);
		}
	}

	std::vector<png_bytep> rows;
	rows.resize (size.height);
	for (size_t i = 0; i < size.height; ++i)
	{
		png_uint_32 q = (uint32_t)(i * stride);
		bitmap->pixels_pointer_per_line ((void**)&((png_bytep)rows[i]), i);
	}
	png_read_image (png, rows.data ());

	*decoder = new __common_bitmap_decoder (bitmap);
	if (*decoder == nullptr)
		return dseed::error_out_of_memory;

	return error_good;
#else
	return dseed::error_feature_not_support;
#endif
}