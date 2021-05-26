#include <dseed.h>

#if defined (USE_JPEG)
#	include <jpeglib.h>

constexpr int INPUT_BUF_SIZE = 4096;

void jpeg_stream_src (j_decompress_ptr cinfo, dseed::io::stream* stream)
{
	struct morisot_source_mgr
	{
		jpeg_source_mgr pub;
		dseed::io::stream* stream;
		JOCTET buffer[INPUT_BUF_SIZE];
	};

	morisot_source_mgr* src;

	if (cinfo->src == nullptr)
		cinfo->src = (jpeg_source_mgr*)(*cinfo->mem->alloc_small) ((j_common_ptr)cinfo,
			JPOOL_PERMANENT, sizeof (morisot_source_mgr));

	src = (morisot_source_mgr*)cinfo->src;
	src->pub.init_source = [](j_decompress_ptr cinfo)
	{
		morisot_source_mgr* src = (morisot_source_mgr*)cinfo->src;
		src->pub.next_input_byte = src->buffer;
		src->pub.bytes_in_buffer = 0;
	};
	src->pub.fill_input_buffer = [](j_decompress_ptr cinfo) -> boolean
	{
		morisot_source_mgr* src = (morisot_source_mgr*)cinfo->src;

		auto read = src->stream->read (src->buffer, INPUT_BUF_SIZE);
		if (read == 0)
			return 0;

		src->pub.next_input_byte = src->buffer;
		src->pub.bytes_in_buffer = read;

		return 1;
	};
	src->pub.skip_input_data = [](j_decompress_ptr cinfo, long num_bytes)
	{
		morisot_source_mgr* src = (morisot_source_mgr*)cinfo->src;

		if (num_bytes > src->pub.bytes_in_buffer)
		{
			src->stream->seek (dseed::io::seekorigin::current, num_bytes - src->pub.bytes_in_buffer);
			src->pub.next_input_byte = src->buffer;
			src->pub.bytes_in_buffer = 0;
		}
		else
		{
			src->pub.next_input_byte += num_bytes;
			src->pub.bytes_in_buffer -= num_bytes;
		}
	};
	src->pub.resync_to_restart = jpeg_resync_to_restart;
	src->pub.term_source = [](j_decompress_ptr cinfo)
	{

	};
	src->stream = stream;
}
#endif

dseed::error_t __create_jpeg_bitmap_decoder_internal (dseed::io::stream* stream, bool yuv, dseed::bitmaps::bitmap_array** decoder)
{
#if defined(USE_JPEG)
	if (stream == nullptr || decoder == nullptr)
		return dseed::error_invalid_args;

	uint8_t sig[3];
	stream->read (sig, 3);
	stream->seek (dseed::io::seekorigin::begin, 0);

	if (sig[0] != 0xff || sig[1] != 0xd8 || sig[2] != 0xff)
		return dseed::error_fail;

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error (&jerr);
	jerr.error_exit = [](j_common_ptr cinfo) {};
	jpeg_create_decompress (&cinfo);

	jpeg_stream_src (&cinfo, stream);

	int r = jpeg_read_header (&cinfo, 1);
	if (r != JPEG_HEADER_OK)
		return dseed::error_fail;

	if (yuv && cinfo.jpeg_color_space == JCS_YCbCr)
		cinfo.out_color_space = JCS_YCbCr;

	jpeg_start_decompress (&cinfo);

	dseed::size3i size (cinfo.output_width, cinfo.output_height, 1);
	dseed::color::pixelformat pixelFormat;
	switch (cinfo.output_components)
	{
	case 3: pixelFormat = (!yuv) ? dseed::color::pixelformat::rgb8 : dseed::color::pixelformat::yuv8; break;
	case 4: pixelFormat = (!yuv) ? dseed::color::pixelformat::rgba8 : dseed::color::pixelformat::yuva8; break;
	case 1: pixelFormat = dseed::color::pixelformat::r8; break;
	default: return dseed::error_not_support;
	}

	size_t stride = dseed::color::calc_bitmap_stride (pixelFormat, size.width);

	dseed::autoref<dseed::bitmaps::bitmap> bitmap;
	if (dseed::failed (dseed::bitmaps::create_bitmap (dseed::bitmaps::bitmaptype::bitmap2d, size, pixelFormat, nullptr, &bitmap)))
		return dseed::error_fail;

	uint8_t* ptr;
	bitmap->lock ((void**)&ptr);

	while (cinfo.output_scanline < cinfo.output_height)
	{
		unsigned char* buffer_array[1];
		buffer_array[0] = ptr + (cinfo.output_scanline * stride);
		if (jpeg_read_scanlines (&cinfo, buffer_array, 1) != 1)
		{
			jpeg_skip_scanlines (&cinfo, 1);
			break;
		}
	}

	bitmap->unlock ();

	jpeg_finish_decompress (&cinfo);
	jpeg_destroy_decompress (&cinfo);

	return create_bitmap_array(dseed::bitmaps::arraytype::plain, bitmap, decoder);
#else
	return dseed::error_not_support_file_format;
#endif
}

dseed::error_t dseed::bitmaps::create_jpeg_bitmap_decoder (dseed::io::stream* stream, dseed::bitmaps::bitmap_array** decoder) noexcept
{
	return __create_jpeg_bitmap_decoder_internal (stream, false, decoder);
}

dseed::error_t dseed::bitmaps::create_jpeg_bitmap_decoder_yuv (dseed::io::stream* stream, dseed::bitmaps::bitmap_array** decoder) noexcept
{
	return __create_jpeg_bitmap_decoder_internal (stream, true, decoder);
}