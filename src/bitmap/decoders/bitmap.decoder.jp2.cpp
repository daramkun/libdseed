#include <dseed.h>

#if defined ( USE_JPEG2000 )
#	define OPJ_STATIC
#	include <openjpeg.h>
#	include "bitmap.decoder.hxx"
#endif

#define JP2_RFC3745_MAGIC "\x00\x00\x00\x0c\x6a\x50\x20\x20\x0d\x0a\x87\x0a"
#define JP2_MAGIC "\x0d\x0a\x87\x0a"
/* position 45: "\xff\x52" */
#define J2K_CODESTREAM_MAGIC "\xff\x4f\xff\x51"

dseed::error_t dseed::create_jpeg2000_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder)
{
#if defined ( USE_JPEG2000 )
	if (stream == nullptr || decoder == nullptr)
		return dseed::error_invalid_args;

	uint8_t sig[12];
	if (12 != stream->read (sig, 12))
		return dseed::error_fail;
	stream->seek (dseed::seekorigin_begin, 0);

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
		auto stream = reinterpret_cast<dseed::stream*>(p_user_data);
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
		return reinterpret_cast<dseed::stream*>(p_user_data)->seek (dseed::seekorigin_begin, p_nb_bytes);
		});
	opj_stream_set_skip_function (opjStream, [](OPJ_OFF_T p_nb_bytes, void* p_user_data) -> OPJ_OFF_T {
		auto stream = reinterpret_cast<dseed::stream*>(p_user_data);
		auto lastPos = stream->position ();
		if (!stream->seek (dseed::seekorigin_current, p_nb_bytes))
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

	pixelformat format;
	switch (image->numcomps)
	{
	case 1: format = pixelformat::grayscale8; break;
	case 3: format = pixelformat::rgb888; break;
	case 4: format = pixelformat::rgba8888; break;
	default:
		return dseed::error_not_support;
	}

	dseed::size3i size (image->x1 - image->x0, image->y1 - image->y0, 1);

	dseed::auto_object<dseed::bitmap> bitmap;
	if (dseed::failed (dseed::create_bitmap (dseed::bitmaptype_2d, size, format, nullptr, &bitmap)))
	{
		opj_image_destroy (image);
		return dseed::error_fail;
	}

	for (auto y = 0; y < size.height; ++y)
	{
		uint8_t* linePtr;
		if (dseed::failed (bitmap->pixels_pointer_per_line ((void**)& linePtr, y)))
		{
			opj_image_destroy (image);
			return dseed::error_fail;
		}

		for (auto x = 0; x < size.width; ++x)
		{
			auto offset = x * image->numcomps;
			for (uint32_t z = 0; z < image->numcomps; ++z)
				linePtr[offset + z] = image->comps[z].data[y * size.width + x];
		}
	}

	opj_image_destroy (image);

	*decoder = new __common_bitmap_decoder (bitmap);
	if (*decoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}