#include <dseed.h>

#include "common.hxx"

#if defined ( USE_JPEG2000 )
#	define OPJ_STATIC
#	include <openjpeg.h>
#endif

#define JP2_RFC3745_MAGIC "\x00\x00\x00\x0c\x6a\x50\x20\x20\x0d\x0a\x87\x0a"
#define JP2_MAGIC "\x0d\x0a\x87\x0a"
/* position 45: "\xff\x52" */
#define J2K_CODESTREAM_MAGIC "\xff\x4f\xff\x51"

dseed::error_t dseed::bitmaps::create_jpeg2000_bitmap_decoder (dseed::io::stream* stream, dseed::bitmaps::bitmap_array** decoder) noexcept
{
#if defined ( USE_JPEG2000 )
	if (stream == nullptr || decoder == nullptr)
		return dseed::error_invalid_args;

	uint8_t sig[12];
	if (12 != stream->read (sig, 12))
		return dseed::error_fail;
	stream->seek (dseed::io::seekorigin::begin, 0);

	CODEC_FORMAT codecFormat;
	if (memcmp (sig, JP2_RFC3745_MAGIC, 12) == 0 || memcmp (sig, JP2_MAGIC, 4) == 0)
		codecFormat = OPJ_CODEC_JP2;
	else if (memcmp (sig, J2K_CODESTREAM_MAGIC, 4) == 0)
		codecFormat = OPJ_CODEC_J2K;
	else
		return dseed::error_not_support;

	opj_codec_t* dec = opj_create_decompress (codecFormat);
	if (dec == nullptr)
		return dseed::error_not_support;

	opj_dparameters_t params;
	opj_set_default_decoder_parameters (&params);
	params.decod_format = codecFormat == OPJ_CODEC_JP2 ? 1 : (codecFormat == OPJ_CODEC_J2K ? 0 : -1);

	if (!opj_setup_decoder (dec, &params))
	{
		opj_destroy_codec (dec);
		return dseed::error_fail;
	}

	opj_stream_t* opjStream = opj_stream_default_create (1);

	opj_stream_set_user_data (opjStream, stream, [](void* p_user_data) {});
	opj_stream_set_user_data_length (opjStream, stream->length ());
	opj_stream_set_read_function (opjStream, [](void* p_buffer, OPJ_SIZE_T p_nb_bytes, void* p_user_data) -> OPJ_SIZE_T {
		auto stream = reinterpret_cast<dseed::io::stream*>(p_user_data);
		size_t totalRead = 0;
		size_t currentPosition = stream->position ();
		while (totalRead != p_nb_bytes)
		{
			auto read = stream->read (((uint8_t*)p_buffer) + totalRead, p_nb_bytes - totalRead);
			if (read == 0) break;
			totalRead += read;
		}
		return totalRead == 0 ? -1 : totalRead;
		});
	opj_stream_set_seek_function (opjStream, [](OPJ_OFF_T p_nb_bytes, void* p_user_data) -> OPJ_BOOL {
		return reinterpret_cast<dseed::io::stream*>(p_user_data)->seek (dseed::io::seekorigin::begin, p_nb_bytes);
		});
	opj_stream_set_skip_function (opjStream, [](OPJ_OFF_T p_nb_bytes, void* p_user_data) -> OPJ_OFF_T {
		auto stream = reinterpret_cast<dseed::io::stream*>(p_user_data);
		auto lastPos = stream->position ();
		if (!stream->seek (dseed::io::seekorigin::current, p_nb_bytes))
			return -1;
		return stream->position () - lastPos;
		});

	opj_image_t* image;
	if (!opj_read_header (opjStream, dec, &image))
	{
		opj_stream_destroy (opjStream);
		opj_destroy_codec (dec);
		return dseed::error_fail;
	}

	if (!opj_decode (dec, opjStream, image))
	{
		opj_image_destroy (image);
		opj_stream_destroy (opjStream);
		opj_destroy_codec (dec);
		return dseed::error_fail;
	}

	if (!opj_end_decompress (dec, opjStream))
	{
		opj_image_destroy (image);
		opj_stream_destroy (opjStream);
		opj_destroy_codec (dec);
		return dseed::error_fail;
	}

	opj_stream_destroy (opjStream);
	opj_destroy_codec (dec);

	color::pixelformat format;
	switch (image->numcomps)
	{
	case 1: format = color::pixelformat::r8; break;
	case 3: format = color::pixelformat::rgb8; break;
	case 4: format = color::pixelformat::rgba8; break;
	default:
		return dseed::error_not_support;
	}

	dseed::size3i size (image->x1 - image->x0, image->y1 - image->y0, 1);

	dseed::autoref<dseed::bitmaps::bitmap> bitmap;
	if (dseed::failed (dseed::bitmaps::create_bitmap (dseed::bitmaps::bitmaptype::bitmap2d, size, format, nullptr, &bitmap)))
	{
		opj_image_destroy (image);
		return dseed::error_fail;
	}

	size_t stride = dseed::color::calc_bitmap_stride (format, size.width);

	uint8_t* ptr;
	if (dseed::failed (bitmap->lock ((void**)&ptr)))
	{
		bitmap->unlock ();
		opj_image_destroy (image);
		return dseed::error_fail;
	}

	for (auto y = 0; y < size.height; ++y)
	{
		uint8_t* linePtr = ptr + (stride * y);

		for (auto x = 0; x < size.width; ++x)
		{
			auto offset = x * image->numcomps;
			for (uint32_t z = 0; z < image->numcomps; ++z)
				linePtr[offset + z] = image->comps[z].data[y * size.width + x];
		}
	}

	bitmap->unlock ();

	opj_image_destroy (image);

	*decoder = new dseed::__common_bitmap_array (bitmap);
	if (*decoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}