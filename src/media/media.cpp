#include <dseed.h>

constexpr int32_t MAXIMUM_MEDIA_DECODER_COUNT = 512;
std::function<dseed::error_t (dseed::stream*, dseed::media_decoder**)> g_media_decoder_creator[MAXIMUM_MEDIA_DECODER_COUNT] =
{
	dseed::create_wav_media_decoder,
	dseed::create_mp3_media_decoder,

	dseed::create_vorbis_media_decoder,
	dseed::create_opus_media_decoder,
	dseed::create_flac_media_decoder,

	dseed::create_media_foundation_media_decoder,
};
std::atomic<int32_t> g_media_decoder_creator_count = 6;

dseed::error_t dseed::add_media_decoder (createmediadecoder_fn fn)
{
	if (fn == nullptr)
		return dseed::error_invalid_args;

	g_media_decoder_creator[++g_media_decoder_creator_count] = fn;
	return dseed::error_good;
}
dseed::error_t dseed::detect_media_decoder (dseed::stream* stream, dseed::media_decoder** decoder)
{
	if (stream == nullptr || !stream->seekable () || decoder == nullptr)
		return dseed::error_invalid_args;

	for (int i = 0; i < g_media_decoder_creator_count; ++i)
	{
		stream->seek (dseed::seekorigin::begin, 0);
		auto err = g_media_decoder_creator[i] (stream, decoder);
		if (dseed::succeeded (err))
			return dseed::error_good;
		else
		{
			if (*decoder)
				(*decoder)->release ();
		}
	}

	return dseed::error_not_support;
}