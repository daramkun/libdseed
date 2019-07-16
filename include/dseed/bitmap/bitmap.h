#ifndef __DSEED_BITMAP_BITMAP_H__
#define __DSEED_BITMAP_BITMAP_H__

namespace dseed
{
	// Bitmap Type
	enum bitmaptype_t
	{
		// 2D (width * height)
		bitmaptype_2d,
		// 2D Cube-map(width * height * 6)
		bitmaptype_2d_cubemap,
		// 3D (width * height * depth)
		bitmaptype_3d,
	};

	// Cube-map Index
	enum cubemapindex_t
	{
		cubemapindex_positive_x = 0,
		cubemapindex_negative_x = 1,
		cubemapindex_positive_y = 2,
		cubemapindex_negative_y = 3,
		cubemapindex_positive_z = 4,
		cubemapindex_negative_z = 5,
		cubemapindex_size = 6,
	};

	// Palette Object
	class DSEEDEXP palette : public object
	{
	public:
		virtual size_t size () = 0;
		virtual int bits_per_pixel () = 0;

	public:
		virtual error_t copy_palette (void* dest) = 0;

	public:
		virtual void resize (size_t size) = 0;
		virtual error_t pixels_pointer (void** ptr) = 0;
	};

	DSEEDEXP error_t create_palette (const void* pixels, int bpp, size_t pixelsCount, palette** palette);
	
	// Bitmap Object
	class DSEEDEXP bitmap : public object
	{
	public:
		virtual bitmaptype_t type () = 0;

	public:
		virtual pixelformat_t format () = 0;
		virtual size3i size () = 0;

	public:
		virtual error_t palette (palette** palette) = 0;

	public:
		virtual error_t copy_pixels_per_line (void* dest, size_t y, size_t depth = 0) = 0;
		virtual error_t copy_pixels (void* dest, size_t depth = 0) = 0;

	public:
		virtual error_t pixels_pointer_per_line (void** ptr, size_t y, size_t depth = 0) = 0;
		virtual error_t pixels_pointer (void** ptr, size_t depth = 0) = 0;

	public:
		virtual error_t read_pixels (void* buffer, const point2i& pos, const size2i& size, size_t depth = 0) = 0;
		virtual error_t write_pixels (const void* data, const point2i& pos, const size2i& size, size_t depth = 0) = 0;

	public:
		virtual error_t extra_info (attributes** attr) = 0;
	};

	DSEEDEXP error_t create_bitmap (bitmaptype_t type, const size3i& size, pixelformat_t format, palette* palette, bitmap** bitmap);

	enum frametype_t
	{
		frametype_plain,
		frametype_mipmap,
	};

	class DSEEDEXP bitmap_decoder : public object
	{
	public:
		virtual error_t decode_frame (size_t i, bitmap** bitmap, timespan_t* duration) = 0;
		virtual size_t frame_count () = 0;
		virtual frametype_t frame_type () = 0;
	};

	// Decoder Creation Function Prototype
	using createbitmapdecoder_fn = error_t (*) (dseed::stream*, bitmap_decoder**);
	// Add Bitmap Decoder to Detection Queue
	DSEEDEXP error_t add_bitmap_decoder (createbitmapdecoder_fn fn);
	// Detect Bitmap Decoder from Stream
	DSEEDEXP error_t detect_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder);

	// Bitmap Encoder
	class DSEEDEXP bitmap_encoder : public object
	{
	public:
		virtual error_t encode_frame (bitmap* bitmap, timespan_t duration) = 0;
		virtual frametype_t frame_type () = 0;

	public:
		virtual error_t commit () = 0;
	};

	// Bitmap Encoder Options Base Object
	struct DSEEDEXP bitmap_encoder_options
	{
		size_t options_size;
	protected:
		bitmap_encoder_options (size_t options_size) : options_size (options_size) { }
	};

	// Encoder Creation Function Prototype
	using createbitmapencoder_fn = error_t (*) (dseed::stream*, const bitmap_encoder_options*, bitmap_encoder**);

	// Bitmap Pixel Reformatting
	//  : RGBA, RGB, BGRA, BGR, Grayscale color formats can be converted to each other(without Legacy RGB/BGR formats).
	//  : BGR565 can be converted from/to BGR only.
	//  : Palette formats can be converted from/to BGRA, BGR only.
	//  : YCbCr series formats can be converted from/to BGRA, BGR only.
	//  : Packed YCbCr series formats can be converted from/to BGR only.
	//  : Compressed color formats can be converted from/to RGBA only. (BC6H, BC7, ETC2, PVRTC, ASTC not implemented now)
	DSEEDEXP error_t reformat_bitmap (dseed::bitmap* original, dseed::pixelformat_t reformat, dseed::bitmap** bitmap);
}

#include <dseed/bitmap/bitmap.decoders.h>
#include <dseed/bitmap/bitmap.encoders.h>

#endif