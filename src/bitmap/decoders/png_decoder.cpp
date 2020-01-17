#include <dseed.h>

#if defined (USE_PNG)
#	include <png.h>
#endif

#include "common.hxx"

constexpr int PNG_BYTES_TO_CHECK = 8;

dseed::error_t dseed::bitmaps::create_png_bitmap_decoder (dseed::io::stream* stream, dseed::bitmaps::bitmap_array** decoder) noexcept
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

	if (setjmp (png_jmpbuf (png)))
	{
		png_destroy_read_struct (&png, nullptr, nullptr);
		return dseed::error_out_of_memory;
	}

	png_set_read_fn (png, stream, [](png_structp png, png_bytep buf, size_t size)
		{
			reinterpret_cast<dseed::io::stream*> (png_get_io_ptr (png))->read (buf, size);
		});

	png_set_crc_action (png, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);
	png_set_check_for_invalid_index (png, 0);
	png_set_keep_unknown_chunks (png, PNG_HANDLE_CHUNK_ALWAYS, nullptr, 0);
	png_set_benign_errors (png, 1);

	png_voidp user_chunkp = png_get_user_chunk_ptr (png);
	png_set_read_user_chunk_fn (png, user_chunkp, [](png_structp png, png_unknown_chunkp chunk)
		{
			return 1;
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

	dseed::color::pixelformat format;
	switch (channels)
	{
	case 1:
		if (colorType == PNG_COLOR_TYPE_GRAY)
			format = dseed::color::pixelformat::r8;
		else if (colorType == PNG_COLOR_TYPE_PALETTE)
		{
			png_set_bgr (png);
			if (png_get_valid (png, info, PNG_INFO_tRNS))
				format = dseed::color::pixelformat::bgra8_indexed8;
			else
				format = dseed::color::pixelformat::bgr8_indexed8;
		}
		else
		{
			png_destroy_read_struct (&png, nullptr, nullptr);
			return dseed::error_not_support;
		}
		break;

	case 3:
		format = dseed::color::pixelformat::rgb8;
		break;

	case 4:
		if (bitDepth == 8)
			format = dseed::color::pixelformat::rgba8;
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

	size_t stride = dseed::color::calc_bitmap_stride (format, size.width);
	size_t totalBytes = stride * size.height;

	dseed::autoref<dseed::bitmaps::bitmap> bitmap;
	if (format == dseed::color::pixelformat::rgba8 || format == dseed::color::pixelformat::rgb8 ||
		format == dseed::color::pixelformat::r8)
	{
		create_bitmap (dseed::bitmaps::bitmaptype::bitmap2d, size, format, nullptr, &bitmap);
	}
	else if (format == dseed::color::pixelformat::bgr8_indexed8 || format == dseed::color::pixelformat::bgra8_indexed8)
	{
		png_colorp palette;
		int numPalette;
		png_get_PLTE (png, info, &palette, &numPalette);

		if (format == dseed::color::pixelformat::bgr8)
		{
			for (int i = 0; i < numPalette; ++i)
			{
				auto temp = palette[i].red;
				palette[i].red = palette[i].blue;
				palette[i].blue = temp;
			}

			dseed::autoref<dseed::bitmaps::palette> paletteObj;
			dseed::bitmaps::create_palette (palette, 24, numPalette, &paletteObj);

			create_bitmap (dseed::bitmaps::bitmaptype::bitmap2d, size, format, paletteObj, &bitmap);
		}
		else if (format == dseed::color::pixelformat::bgra8)
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

			dseed::autoref<dseed::bitmaps::palette> paletteObj;
			dseed::bitmaps::create_palette (palette, 32, numPalette, &paletteObj);

			create_bitmap (dseed::bitmaps::bitmaptype::bitmap2d, size, format, paletteObj, &bitmap);
		}
	}

	uint8_t* ptr;
	if (dseed::failed (bitmap->lock ((void**)&ptr)))
		return dseed::error_fail;

	std::vector<png_bytep> rows;
	rows.resize (size.height);
	for (size_t i = 0; i < size.height; ++i)
	{
		png_uint_32 q = (uint32_t)(i * stride);
		rows[i] = ptr + (i * stride);
	}
	png_read_image (png, rows.data ());

	bitmap->unlock ();

	*decoder = new __common_bitmap_array (bitmap);
	if (*decoder == nullptr)
		return dseed::error_out_of_memory;

	return error_good;
#else
	return dseed::error_not_support;
#endif
}