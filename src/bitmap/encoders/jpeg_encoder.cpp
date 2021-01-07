#include <dseed.h>

#if defined(USE_JPEG) && defined(USE_BITMAP_ENCODERS)
#	include <jpeglib.h>
#	include <assert.h>

constexpr int OUTPUT_BUF_SIZE = 4096;

void jpeg_stream_dest (j_compress_ptr cinfo, dseed::io::stream* stream)
{
	struct morisot_destination_mgr
	{
		jpeg_destination_mgr pub;
		dseed::io::stream* stream;
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

class __jpeg_encoder : public dseed::bitmaps::bitmap_encoder
{
public:
	__jpeg_encoder (dseed::io::stream* stream, int quality)
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
	virtual dseed::error_t encode_frame (dseed::bitmaps::bitmap* bitmap) noexcept override
	{
		if (_already_encoded)
			return dseed::error_invalid_op;

		dseed::size3i size = bitmap->size ();
		if (size.depth > 1)
			return dseed::error_not_support;

		dseed::color::pixelformat format = bitmap->format ();
		if (format != dseed::color::pixelformat::rgb8 &&
			format != dseed::color::pixelformat::rgba8 &&
			format != dseed::color::pixelformat::bgr8 &&
			format != dseed::color::pixelformat::bgra8 &&
			format != dseed::color::pixelformat::r8 &&
			format != dseed::color::pixelformat::yuv8 &&
			format != dseed::color::pixelformat::bgr565)
			return dseed::error_not_support;

		size_t streamOriginPos = _stream->position ();

		_cinfo.err = jpeg_std_error (&_jerr);
		jpeg_create_compress (&_cinfo);
		jpeg_stream_dest (&_cinfo, _stream);

		_cinfo.image_width = size.width;
		_cinfo.image_height = size.height;
		switch (format)
		{
			case dseed::color::pixelformat::rgb8:
				_cinfo.in_color_space = JCS_EXT_RGB;
				_cinfo.input_components = 3;
				break;
			case dseed::color::pixelformat::rgba8:
				_cinfo.in_color_space = JCS_EXT_RGBA;
				_cinfo.input_components = 4;
				break;
			case dseed::color::pixelformat::bgr8:
				_cinfo.in_color_space = JCS_EXT_BGR;
				_cinfo.input_components = 3;
				break;
			case dseed::color::pixelformat::bgra8:
				_cinfo.in_color_space = JCS_EXT_BGRA;
				_cinfo.input_components = 4;
				break;
			case dseed::color::pixelformat::r8:
				_cinfo.in_color_space = JCS_GRAYSCALE;
				_cinfo.input_components = 1;
				break;
			case dseed::color::pixelformat::yuv8:
				_cinfo.in_color_space = JCS_YCbCr;
				_cinfo.input_components = 3;
				break;
			case dseed::color::pixelformat::bgr565:
				_cinfo.in_color_space = JCS_RGB565;
				_cinfo.input_components = 3;
				break;
		}

		jpeg_set_defaults (&_cinfo);

		jpeg_set_quality (&_cinfo, _quality, true);

		jpeg_start_compress (&_cinfo, true);

		int stride = dseed::color::calc_bitmap_stride (bitmap->format (), size.width);
		uint8_t* ptr;
		bitmap->lock ((void**)&ptr);
		while (_cinfo.next_scanline < _cinfo.image_height)
		{
			JSAMPROW rows[1];
			rows[0] = ptr + stride * _cinfo.next_scanline;

			jpeg_write_scanlines (&_cinfo, rows, 1);
		}
		bitmap->unlock ();

		jpeg_finish_compress (&_cinfo);
		jpeg_destroy_compress (&_cinfo);

		memset (&_jerr, 0, sizeof (_jerr));
		memset (&_cinfo, 0, sizeof (_cinfo));

		if (_stream->seekable ())
			_stream->seek (dseed::io::seekorigin::begin, streamOriginPos);

		_already_encoded = true;

		return dseed::error_good;
	}
	virtual dseed::bitmaps::arraytype type () noexcept override { return dseed::bitmaps::arraytype::plain; }

public:
	virtual dseed::error_t commit () noexcept override
	{
		_stream->flush ();
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	dseed::autoref<dseed::io::stream> _stream;
	int _quality;
	jpeg_compress_struct _cinfo;
	jpeg_error_mgr _jerr;

	bool _already_encoded;
};
#endif

dseed::error_t dseed::bitmaps::create_jpeg_bitmap_encoder (dseed::io::stream* stream, const dseed::bitmaps::bitmap_encoder_options* options, dseed::bitmaps::bitmap_encoder** encoder)
{
#if defined(USE_JPEG) && defined(USE_BITMAP_ENCODERS)
	dseed::bitmaps::jpeg_encoder_options jpegOptions;
	if (options != nullptr)
	{
		if (options->options_size != sizeof (dseed::bitmaps::jpeg_encoder_options))
			return dseed::error_invalid_args;
		jpegOptions = *reinterpret_cast<const dseed::bitmaps::jpeg_encoder_options*>(options);
	}

	*encoder = new __jpeg_encoder (stream, jpegOptions.quality);
	if (*encoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}