#include <dseed.h>

#if defined ( USE_JPEG ) && defined ( USE_BITMAP_ENCODERS )
#	include <jpeglib.h>
#	include <assert.h>

constexpr int OUTPUT_BUF_SIZE = 4096;

void jpeg_stream_dest (j_compress_ptr cinfo, dseed::stream* stream)
{
	struct morisot_destination_mgr
	{
		jpeg_destination_mgr pub;
		dseed::stream* stream;
		JOCTET buffer[OUTPUT_BUF_SIZE];
	};

	morisot_destination_mgr* dest;

	if (cinfo->dest == nullptr)
		cinfo->dest = (jpeg_destination_mgr*)(*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof (morisot_destination_mgr));

	dest = (morisot_destination_mgr*)cinfo->dest;
	dest->pub.init_destination = [](j_compress_ptr cinfo)
	{
		morisot_destination_mgr* dest = (morisot_destination_mgr*)cinfo->dest;
		dest->pub.next_output_byte = dest->buffer;
		dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
	};
	dest->pub.empty_output_buffer = [](j_compress_ptr cinfo) -> boolean
	{
		morisot_destination_mgr* dest = (morisot_destination_mgr*)cinfo->dest;

		auto written = dest->stream->write (dest->buffer, OUTPUT_BUF_SIZE);
		if (written != OUTPUT_BUF_SIZE)
			return 0;

		dest->pub.next_output_byte = dest->buffer;
		dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

		return 1;
	};
	dest->pub.term_destination = [](j_compress_ptr cinfo)
	{
		morisot_destination_mgr* dest = (morisot_destination_mgr*)cinfo->dest;
		size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;
		size_t totalWritten = 0;

		while (datacount > 0) {
			auto written = dest->stream->write (dest->buffer + totalWritten, datacount);
			assert (!(datacount > 0 && written == 0));
			datacount -= written;
			totalWritten += written;
		}
	};
	dest->stream = stream;
}

class __jpeg_encoder : public dseed::bitmap_encoder
{
public:
	__jpeg_encoder (dseed::stream* stream, int quality)
		: _refCount (1), _stream (stream), _quality (quality), _cinfo ({}), _jerr ({})
		, _already_encoded (false)
	{

	}
	~__jpeg_encoder ()
	{

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
		if (_already_encoded)
			return dseed::error_invalid_op;

		dseed::size3i size = bitmap->size ();
		if (size.depth > 1)
			return dseed::error_not_support;

		dseed::pixelformat_t format = bitmap->format ();
		if (format != dseed::pixelformat_rgb888 &&
			format != dseed::pixelformat_rgba8888 &&
			format != dseed::pixelformat_bgr888 &&
			format != dseed::pixelformat_bgra8888 &&
			format != dseed::pixelformat_grayscale8 &&
			format != dseed::pixelformat_yuv888 &&
			format != dseed::pixelformat_bgr565)
			return dseed::error_not_support;

		size_t streamOriginPos = _stream->position ();

		_cinfo.err = jpeg_std_error (&_jerr);
		jpeg_create_compress (&_cinfo);
		jpeg_stream_dest (&_cinfo, _stream);

		_cinfo.image_width = size.width;
		_cinfo.image_height = size.height;
		switch (format)
		{
		case dseed::pixelformat_rgb888:
			_cinfo.in_color_space = JCS_EXT_RGB;
			_cinfo.input_components = 3;
			break;
		case dseed::pixelformat_rgba8888:
			_cinfo.in_color_space = JCS_EXT_RGBA;
			_cinfo.input_components = 4;
			break;
		case dseed::pixelformat_bgr888:
			_cinfo.in_color_space = JCS_EXT_BGR;
			_cinfo.input_components = 3;
			break;
		case dseed::pixelformat_bgra8888:
			_cinfo.in_color_space = JCS_EXT_BGRA;
			_cinfo.input_components = 4;
			break;
		case dseed::pixelformat_grayscale8:
			_cinfo.in_color_space = JCS_GRAYSCALE;
			_cinfo.input_components = 1;
			break;
		case dseed::pixelformat_yuv888:
			_cinfo.in_color_space = JCS_YCbCr;
			_cinfo.input_components = 3;
			break;
		case dseed::pixelformat_bgr565:
			_cinfo.in_color_space = JCS_RGB565;
			_cinfo.input_components = 3;
			break;
		}

		jpeg_set_defaults (&_cinfo);

		jpeg_set_quality (&_cinfo, _quality, true);

		jpeg_start_compress (&_cinfo, true);

		int row_stride = dseed::get_bitmap_stride (bitmap->format (), size.width);
		while (_cinfo.next_scanline < _cinfo.image_height)
		{
			void* ptr;
			bitmap->pixels_pointer_per_line (&ptr, _cinfo.next_scanline);

			JSAMPROW rows[1];
			rows[0] = (uint8_t*)ptr;

			jpeg_write_scanlines (&_cinfo, rows, 1);
		}

		jpeg_finish_compress (&_cinfo);
		jpeg_destroy_compress (&_cinfo);

		memset (&_jerr, 0, sizeof (_jerr));
		memset (&_cinfo, 0, sizeof (_cinfo));

		if (_stream->seekable ())
			_stream->seek (dseed::seekorigin_begin, streamOriginPos);

		_already_encoded = true;

		return dseed::error_good;
	}
	virtual dseed::frametype_t frame_type () override { return dseed::frametype_plain; }

public:
	virtual dseed::error_t commit () override
	{
		_stream->flush ();
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	dseed::auto_object<dseed::stream> _stream;
	int _quality;
	jpeg_compress_struct _cinfo;
	jpeg_error_mgr _jerr;

	bool _already_encoded;
};
#endif

dseed::error_t dseed::create_jpeg_bitmap_encoder (dseed::stream* stream, const dseed::bitmap_encoder_options* options, dseed::bitmap_encoder** encoder)
{
#if defined ( USE_JPEG ) && defined ( USE_BITMAP_ENCODERS )
	jpeg_encoder_options jpegOptions;
	if (options != nullptr)
	{
		if (options->options_size != sizeof (jpeg_encoder_options))
			return dseed::error_invalid_args;
		jpegOptions = *reinterpret_cast<const jpeg_encoder_options*>(options);
	}

	*encoder = new __jpeg_encoder (stream, jpegOptions.quality);
	if (*encoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}