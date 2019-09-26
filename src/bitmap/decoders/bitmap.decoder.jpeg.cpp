#include <dseed.h>

#if defined (USE_JPEG)
#	include <jpeglib.h>

constexpr int INPUT_BUF_SIZE = 4096;

void jpeg_stream_src (j_decompress_ptr cinfo, dseed::stream* stream)
{
	struct morisot_source_mgr
	{
		jpeg_source_mgr pub;
		dseed::stream* stream;
		JOCTET buffer[INPUT_BUF_SIZE];
	};
	const static JOCTET EOI_BUFFER[1] = { JPEG_EOI };

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
			src->stream->seek (dseed::seekorigin_current, num_bytes - src->pub.bytes_in_buffer);
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

#include "bitmap.decoder.hxx"

dseed::error_t __create_jpeg_bitmap_decoder_internal (dseed::stream* stream, bool yuv, dseed::bitmap_decoder** decoder)
{
#if defined ( USE_JPEG )
	if (stream == nullptr || decoder == nullptr)
		return dseed::error_invalid_args;

	uint8_t sig[3];
	stream->read (sig, 3);
	stream->seek (dseed::seekorigin_begin, 0);

	if (sig[0] != 0xff || sig[1] != 0xd8 || sig[2] != 0xff)
		return dseed::error_fail;

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error (&jerr);
	jpeg_create_decompress (&cinfo);

	jpeg_stream_src (&cinfo, stream);
	
	int r = jpeg_read_header (&cinfo, 1);
	if (r != JPEG_HEADER_OK)
		return dseed::error_fail;

	if (yuv && cinfo.jpeg_color_space == JCS_YCbCr)
		cinfo.out_color_space = JCS_YCbCr;

	jpeg_start_decompress (&cinfo);

	dseed::size3i size (cinfo.output_width, cinfo.output_height, 1);
	dseed::pixelformat_t pixelFormat;
	switch (cinfo.output_components)
	{
	case 3: pixelFormat = (!yuv) ? dseed::pixelformat_rgb888 : dseed::pixelformat_yuv888; break;
	case 4: pixelFormat = (!yuv) ? dseed::pixelformat_rgba8888 : dseed::pixelformat_yuva8888; break;
	case 1: pixelFormat = dseed::pixelformat_grayscale8; break;
	default: return dseed::error_not_support;
	}

	size_t stride = dseed::get_bitmap_stride (pixelFormat, size.width);

	dseed::auto_object<dseed::bitmap> bitmap;
	if (dseed::failed (dseed::create_bitmap (dseed::bitmaptype_2d, size, pixelFormat, nullptr, &bitmap)))
		return dseed::error_fail;

	while (cinfo.output_scanline < cinfo.output_height)
	{
		unsigned char* buffer_array[1];
		bitmap->pixels_pointer_per_line ((void**)& buffer_array[0], cinfo.output_scanline);
		jpeg_read_scanlines (&cinfo, buffer_array, 1);
	}

	jpeg_finish_decompress (&cinfo);
	jpeg_destroy_decompress (&cinfo);

	*decoder = new __common_bitmap_decoder (bitmap);
	if (*decoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
#else
	return dseed::error_feature_not_support;
#endif
}

dseed::error_t dseed::create_jpeg_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder)
{
	return __create_jpeg_bitmap_decoder_internal (stream, false, decoder);
}

dseed::error_t dseed::create_jpeg_bitmap_decoder_yuv (dseed::stream* stream, dseed::bitmap_decoder** decoder)
{
	return __create_jpeg_bitmap_decoder_internal (stream, true, decoder);
}