#include <dseed.h>

#define DR_WAV_IMPLEMENTATION
#include "../../libs/dr_libs/dr_wav.h"

#include "common.hxx"

class __wav_decoder : public dseed::media::media_decoder
{
public:
	__wav_decoder ()
		: _refCount (1), _file (nullptr)
		, _fixedBps (0), _byterate (0)
	{ }

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

		_file = drwav_open (
			[](void* pUserData, void* pBufferOut, size_t bytesToRead) -> size_t
			{
				dseed::io::stream* stream = (dseed::io::stream*)pUserData;
				int64_t readed = stream->read ((uint8_t*)pBufferOut, bytesToRead);
				return (size_t)(readed);
			},
			[](void* pUserData, int offset, drwav_seek_origin origin) -> drwav_bool32
			{
				dseed::io::stream* stream = (dseed::io::stream*)pUserData;
				return stream->seek ((dseed::io::seekorigin)origin, offset);
			}, stream);

		if (_file == nullptr)
			return dseed::error_not_support;

		_fixedBps = _file->bitsPerSample;
		if (_fixedBps > 16)
			_fixedBps = 16;
		else if (_fixedBps < 16)
			_fixedBps = 32;

		_byterate = (_file->channels * (_fixedBps / 8) * _file->sampleRate);

		return dseed::error_good;
	}

public:
	virtual dseed::timespan duration () override
	{
		return dseed::timespan::from_seconds ((_file->totalSampleCount + 1) * (_fixedBps / 8) / (double)_byterate);
	}

public:
	virtual dseed::media::sampletype type (size_t streamIndex) override
	{
		if (streamIndex != 0) return dseed::media::sampletype_out_of_range;
		return dseed::media::sampletype_audio;
	}

	virtual dseed::error_t format (size_t streamIndex, void* buffer, size_t bufferSize) override
	{
		if (streamIndex != 0) return dseed::error_out_of_range;
		if (buffer == nullptr || bufferSize != sizeof (dseed::media::audioformat))
			return dseed::error_invalid_args;

		*((dseed::media::audioformat*)buffer) = dseed::media::audioformat (_file->channels, _fixedBps, _file->sampleRate, dseed::media::pulseformat::pcm);

		return dseed::error_good;
	}

public:
	virtual dseed::timespan reading_position () override
	{
		return dseed::timespan::from_seconds ((_file->dataChunkDataSize - _file->bytesRemaining) / (double)_byterate);
	}

	virtual dseed::error_t set_reading_position (dseed::timespan pos) override
	{
		drwav_seek_to_sample (_file, (uint64_t)(pos.total_seconds () * _byterate) / (_file->bitsPerSample / 8));
		return dseed::error_good;
	}

public:
	virtual dseed::error_t decode (size_t streamIndex, dseed::media::media_sample** sample) override
	{
		if (streamIndex != 0)
			return dseed::error_out_of_range;
		if (sample == nullptr)
			return dseed::error_invalid_args;

		int length = _byterate / 100;
		std::vector<uint8_t> buffer (length);
		dseed::timespan current = reading_position ();

		drwav_uint64 result = 0;
		if (_fixedBps == 32)
			result = drwav_read_s32 (_file, _byterate / 4, (drwav_int32*)buffer.size ());
		else if (_fixedBps == 16)
			result = drwav_read_s16 (_file, _byterate / 2, (drwav_int16*)buffer.size ());

		if (result == 0)
			return dseed::error_end_of_file;

		dseed::timespan duration = dseed::timespan::from_seconds (length / (double)_byterate);

		*sample = new __common_media_sample (dseed::media::sampletype_audio, buffer.data (), buffer.size (), current, duration);

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;

	dseed::autoref<dseed::io::stream> _stream;

	drwav* _file;
	int _byterate, _fixedBps;
};

dseed::error_t dseed::media::create_wav_media_decoder (dseed::io::stream* stream, dseed::media::media_decoder** decoder)
{
	if (stream == nullptr || decoder == nullptr)
		return dseed::error_invalid_args;

	dseed::autoref<__wav_decoder> temp = new __wav_decoder ();
	if (temp == nullptr)
		return dseed::error_out_of_memory;

	dseed::error_t err;
	if (dseed::failed (err = temp->initialize (stream)))
		return err;

	*decoder = temp.detach ();

	return dseed::error_good;
}