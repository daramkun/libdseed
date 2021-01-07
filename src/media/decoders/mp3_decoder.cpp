#include <dseed.h>

#define DR_MP3_IMPLEMENTATION
#define DR_MP3_NO_STDIO
#if ARCH_ARM64
#	define __ARM_NEON
#endif
#include "../../libs/dr_libs/dr_mp3.h"

#include "common.hxx"

class __mp3_decoder : public dseed::media::media_decoder
{
public:
	__mp3_decoder ()
		: _refCount (1), _file ()
	{
		
	}
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
	dseed::error_t initialize (dseed::io::stream* stream)
	{
		_stream = stream;

		drmp3_config config = {};
		config.outputChannels = 2;
		config.outputSampleRate = 48000;

		if(!drmp3_init (&_file, [](void* pUserData, void* pBufferOut, size_t bytesToRead) -> size_t
			{
				dseed::io::stream* stream = (dseed::io::stream*)pUserData;
				int64_t readed = stream->read ((uint8_t*)pBufferOut, bytesToRead);
				return (size_t)(readed);
			},
			[](void* pUserData, int offset, drmp3_seek_origin origin) -> drmp3_bool32
			{
				dseed::io::stream* stream = (dseed::io::stream*)pUserData;
				return stream->seek ((dseed::io::seekorigin)origin, offset);
			},
				stream, & config))
			return dseed::error_not_support;

		_byterate = (_file.channels * 2 * _file.sampleRate);

		return dseed::error_good;
	}

public:
	virtual dseed::timespan duration () override
	{
		return dseed::timespan::from_seconds (drmp3_get_pcm_frame_count (&_file) / (double)_file.sampleRate);
	}

public:
	virtual dseed::media::sampletype type (size_t streamIndex) override
	{
		if (streamIndex != 0) return dseed::media::sampletype_out_of_range;
		return dseed::media::sampletype_audio;
	}
	virtual dseed::error_t format (size_t streamIndex, void* buffer, size_t bufferSize) override
	{
		*((dseed::media::audioformat*)buffer) = dseed::media::audioformat (_file.channels, 16, _file.sampleRate, dseed::media::pulseformat::pcm);
		return dseed::error_good;
	}

public:
	virtual dseed::timespan reading_position () override
	{
		return dseed::timespan::from_seconds (_file.currentPCMFrame / (double)_file.sampleRate);
	}
	virtual dseed::error_t set_reading_position (dseed::timespan pos) override
	{
		if (!drmp3_seek_to_pcm_frame (&_file, (uint64_t)(pos.total_seconds () * _file.sampleRate)))
			return dseed::error_not_support;
		return dseed::error_good;
	}

public:
	virtual dseed::error_t decode (size_t streamIndex, dseed::media::media_sample** sample) override
	{
		if (streamIndex != 0)
			return dseed::error_out_of_range;
		if (sample == nullptr)
			return dseed::error_invalid_args;

		int length = _file.sampleRate;
		std::vector<uint8_t> buffer (length * _file.channels * 2);
		dseed::timespan current = reading_position ();

		drmp3_uint64 result = drmp3_read_pcm_frames_s16 (&_file, length, (drmp3_int16*)buffer.data ());

		if (result == 0)
			return dseed::error_end_of_file;

		length = (int)result;
		dseed::timespan duration = dseed::timespan::from_seconds (length * _file.channels * 2 / (double)_byterate);

		*sample = new __common_media_sample (dseed::media::sampletype_audio, buffer.data (), buffer.size (), current, duration);

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;

	dseed::autoref<dseed::io::stream> _stream;

	drmp3 _file;
	int _byterate;
};

dseed::error_t dseed::media::create_mp3_media_decoder (dseed::io::stream* stream, dseed::media::media_decoder** decoder)
{
	if (stream == nullptr || decoder == nullptr)
		return dseed::error_invalid_args;

	dseed::autoref<__mp3_decoder> temp = new __mp3_decoder ();
	if (temp == nullptr)
		return dseed::error_out_of_memory;

	dseed::error_t err;
	if (dseed::failed (err = temp->initialize (stream)))
		return err;

	*decoder = temp.detach ();

	return dseed::error_good;
}