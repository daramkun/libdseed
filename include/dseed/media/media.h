#ifndef __DSEED_MEDIA_MEDIA_H__
#define __DSEED_MEDIA_MEDIA_H__

namespace dseed
{
	using sampletype_t = uint32_t;
	enum
	{
		sampletype_unknown = 0,
		
		sampletype_image = 0x00000001,
		sampletype_audio = 0x00000002,

		sampletype_decoded = 0x00000000,
		sampletype_encoded = 0x00010000,

		sampletype_out_of_range = 0xffffffff,
	};

	// Image Format
	struct DSEEDEXP imageformat
	{
		size2i size;
		pixelformat_t image_format;
		fraction frame_rate;

		imageformat () = default;
		imageformat (const size2i& size, pixelformat_t format, fraction framerate)
			: size (size), image_format (format), frame_rate (framerate)
		{ }
	};

	// Audio Pulse Format
	enum pulseformat_t : int32_t
	{
		// Unknown Format
		pulseformat_unknown,

		// Integer PCM Format
		pulseformat_pcm,
		// IEEE Float PCM Format
		pulseformat_ieee_float,
	};

	// Audio Format
	struct DSEEDEXP audioformat
	{
		// Audio Channels (1: Mono, 2: Stereo, 3: 2.1ch, 6: 5.1ch, 8: 7.1ch)
		uint8_t channels;
		// Bits per Sample (8, 16, 24, 32)
		uint8_t bits_per_sample;
		// Block Alignment (Audio Channels * Bits per Sample / 8)
		uint16_t block_align;
		// Sample-Rate (44100, 48000, ...)
		uint32_t samples_per_sec;
		// Bytes per Second (Block Alignment * Sample-Rate)
		uint32_t bytes_per_sec;
		// Audio Pulse Sample Format
		pulseformat_t pulse_format;

		audioformat () = default;
		audioformat (int channels, int bits_per_sample, int samples_per_sec, pulseformat_t format)
			: channels (channels), bits_per_sample (bits_per_sample), samples_per_sec (samples_per_sec)
			, pulse_format (format)
			, block_align (channels* (bits_per_sample / 8)), bytes_per_sec (block_align* samples_per_sec)
		{ }
	};

	DSEEDEXP timespan_t get_audio_duration (const audioformat& wf, size_t bytes);
	DSEEDEXP size_t get_audio_bytes_count (const audioformat& wf, dseed::timespan_t duration);

	class DSEEDEXP media_sample : public dseed::object
	{
	public:
		virtual sampletype_t sample_type () = 0;

	public:
		// Get Sample Position
		virtual dseed::timespan_t sample_time () = 0;
		// Get Sample Length
		virtual dseed::timespan_t sample_duration () = 0;

	public:
		// Copy Sample Data to buffer
		virtual dseed::error_t copy_sample (void* buffer) = 0;
		// Get Sample BUffer Size
		virtual size_t sample_size () = 0;
	};

	class DSEEDEXP media_decoder : public dseed::object
	{
	public:
		// Get Total Duration
		virtual dseed::timespan_t duration () = 0;

	public:
		virtual sampletype_t sample_type (size_t streamIndex) = 0;

	public:
		virtual dseed::error_t sample_format (size_t streamIndex, void* buffer, size_t bufferSize) = 0;

	public:
		// Get Reading Position
		virtual dseed::timespan_t reading_position () = 0;
		// Set Reading Position
		virtual dseed::error_t set_reading_position (dseed::timespan_t pos) = 0;

	public:
		virtual dseed::error_t decode (size_t streamIndex, media_sample** sample) = 0;
	};

	using createmediadecoder_fn = error_t (*) (dseed::stream*, media_decoder**);
	DSEEDEXP error_t add_media_decoder (createmediadecoder_fn fn);
	DSEEDEXP error_t detect_media_decoder (dseed::stream* stream, dseed::media_decoder** decoder);

	class DSEEDEXP media_encoder : public dseed::object
	{
	public:
		virtual dseed::error_t set_sample_format (size_t streamIndex, sampletype_t type, void* format, size_t formatSize) = 0;

	public:
		virtual dseed::error_t encode (size_t streamIndex, media_sample* sample) = 0;
	};
}

#include "media.decoders.h"

#endif