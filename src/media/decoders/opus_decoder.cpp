#include <dseed.h>

#if defined ( USE_OPUS )
#	include "../../libs/opusfile/opusfile.h"
#	include "../../libs/opusfile/opusfile.c"
#	include "../../libs/opusfile/internal.c"
#	include "../../libs/opusfile/info.c"
#	include "../../libs/opusfile/stream.c"

#	include "common.hxx"

#include <vector>

class __opus_decoder : public dseed::media::media_decoder
{
public:
	__opus_decoder ()
		: _refCount (1), _file (nullptr), _byterate (0)
	{
		_callbacks.read = [](void* datasource, unsigned char* ptr, int nbytes) -> int
		{
			dseed::io::stream* stream = (dseed::io::stream*)datasource;
			return (int)stream->read (ptr, nbytes);
		};
		_callbacks.seek = [](void* datasource, int64_t offset, int whence) -> int
		{
			dseed::io::stream* stream = (dseed::io::stream*)datasource;
			return (int)stream->seek ((dseed::io::seekorigin)whence, offset);
		};
		_callbacks.tell = [](void* datasource) -> int64_t
		{
			dseed::io::stream* stream = (dseed::io::stream*)datasource;
			return stream->position ();
		};
		_callbacks.close = [](void* datasource) -> int
		{
			return 0;
		};
	}
	~__opus_decoder ()
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
	dseed::error_t initialize (dseed::io::stream* stream)
	{
		_stream = stream;

		int error;
		_file = op_open_callbacks (stream, &_callbacks, nullptr, 0, &error);

		if (error != 0)
		{
			op_free (_file);
			switch (error)
			{
				case OP_EREAD: return dseed::error_io;
				case OP_ENOTAUDIO: return dseed::error_not_support;
				case OP_EVERSION: return dseed::error_not_support;
				case OP_EBADHEADER: return dseed::error_not_support;
				case OP_EFAULT: return dseed::error_fail;
				default: return dseed::error_fail;
			}
		}

		const OpusHead* head = op_head (_file, 0);
		_channels = head->channel_count;
		_samplerate = head->input_sample_rate;

		_byterate = (_channels * 2 * _samplerate);

		return dseed::error_good;
	}

public:
	virtual dseed::media::sampletype type (size_t streamIndex) override
	{
		if (streamIndex != 0)
			return dseed::media::sampletype_out_of_range;
		return dseed::media::sampletype_audio;
	}
	virtual dseed::error_t format (size_t streamIndex, void* buffer, size_t bufferSize) override
	{
		if (streamIndex != 0)
			return dseed::error_out_of_range;
		if (buffer == nullptr || bufferSize != sizeof (dseed::media::audioformat))
			return dseed::error_invalid_args;

		*((dseed::media::audioformat*)buffer) = dseed::media::audioformat (_channels, 16, _samplerate, dseed::media::pulseformat::pcm);

		return dseed::error_good;
	}

public:
	virtual dseed::timespan duration () override
	{
		return dseed::timespan::from_seconds (op_pcm_total (_file, -1) / (double)_samplerate);
	}

public:
	virtual dseed::timespan reading_position () override
	{
		return dseed::timespan::from_seconds (op_pcm_tell (_file) / (double)_samplerate);
	}
	virtual dseed::error_t set_reading_position (dseed::timespan pos) override
	{
		op_pcm_seek (_file, (int64_t)(pos.total_seconds () * _samplerate));
		return dseed::error_good;
	}

public:
	virtual dseed::error_t decode (size_t streamIndex, dseed::media::media_sample** sample) override
	{
		int length = _byterate / 100;
		std::vector<uint8_t> buffer (length);
		int bitstream;
		dseed::timespan current = dseed::timespan::from_seconds (op_pcm_tell (_file) / (double)_samplerate);
		long result = op_read (_file, (opus_int16*)buffer.data (), length, &bitstream);

		if (result == 0)
			return dseed::error_end_of_file;
		else if (result == OP_HOLE || result == OP_EBADLINK || result == OP_EINVAL)
			return dseed::error_fail;

		dseed::timespan duration = dseed::timespan::from_seconds (length / (double)_byterate);

		*sample = new __common_media_sample (dseed::media::sampletype_audio, buffer.data (), current, duration, _byterate);

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;

	dseed::autoref<dseed::io::stream> _stream;

	OggOpusFile* _file;
	OpusFileCallbacks _callbacks;

	int _channels, _samplerate, _byterate;
};
#endif

dseed::error_t dseed::media::create_opus_media_decoder (dseed::io::stream* stream, dseed::media::media_decoder** decoder)
{
#if defined ( USE_OPUS )
	dseed::autoref<__opus_decoder> temp = new __opus_decoder ();
	if (temp == nullptr)
		return dseed::error_out_of_memory;

	dseed::error_t err;
	if (dseed::failed (err = temp->initialize (stream)))
		return err;

	*decoder = temp.detach ();

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}