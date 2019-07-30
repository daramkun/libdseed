#include <dseed.h>

#define DR_MP3_IMPLEMENTATION
#define DR_MP3_NO_STDIO
#include "../../libs/dr_libs/dr_mp3.h"

#include "media.decoder.hxx"

class __mp3_decoder : public dseed::media_decoder
{
public:
	__mp3_decoder ()
		: _refCount (1), _file ()
	{ }
	~__mp3_decoder ()
	{
		drmp3_uninit (&_file);
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
	virtual dseed::timespan_t duration () override
	{
		return dseed::timespan_t::from_seconds (drmp3_get_pcm_frame_count (&_file) / (double)_file.sampleRate);
	}

public:
	virtual dseed::sampletype_t sample_type (size_t streamIndex) override
	{
		if (streamIndex != 0) return dseed::sampletype_out_of_range;
		return dseed::sampletype_audio;
	}
	virtual dseed::error_t sample_format (size_t streamIndex, void* buffer, size_t bufferSize) override
	{
		*((dseed::audioformat*)buffer) = dseed::audioformat (_file.channels, 16, _file.sampleRate, dseed::pulseformat_pcm);
		return dseed::error_good;
	}

public:
	virtual dseed::timespan_t reading_position () override
	{
		return dseed::timespan_t::from_seconds (_file.currentPCMFrame / (double)_file.sampleRate);
	}
	virtual dseed::error_t set_reading_position (dseed::timespan_t pos) override
	{
		if (!drmp3_seek_to_pcm_frame (&_file, (uint64_t)(pos.total_seconds () * _file.sampleRate)))
			return dseed::error_not_support;
		return dseed::error_good;
	}

public:
	virtual dseed::error_t decode (size_t streamIndex, dseed::media_sample** sample) override
	{
		if (streamIndex != 0)
			return dseed::error_out_of_range;
		if (sample == nullptr)
			return dseed::error_invalid_args;

		int length = _file.sampleRate;
		std::vector<uint8_t> buffer (length * _file.channels * 2);
		dseed::timespan_t current = reading_position ();

		drmp3_uint64 result = drmp3_read_pcm_frames_s16 (&_file, length, (drmp3_int16*)buffer.data ());

		if (result == 0)
			return dseed::error_end_of_file;

		length = (int)result;
		dseed::timespan_t duration = dseed::timespan_t::from_seconds (length * _file.channels * 2 / (double)_byterate);

		*sample = new __common_media_sample (dseed::sampletype_audio, buffer.data (), buffer.size (), current, duration);

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;

	dseed::auto_object<dseed::stream> _stream;

	drmp3 _file;
	int _byterate, _fixedBps;
};

dseed::error_t dseed::create_mp3_media_decoder (dseed::stream* stream, dseed::media_decoder** decoder)
{
	return DSEEDEXP error_t ();
}