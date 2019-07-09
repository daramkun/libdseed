#include <dseed.h>

#if defined ( USE_FLAC )
#	define FLAC__NO_DLL
#	include "FLAC/all.h"

#	include "media.decoder.hxx"

typedef FLAC__StreamDecoderInitStatus (*FLAC_INIT) (
	FLAC__StreamDecoder* decoder,
	FLAC__StreamDecoderReadCallback read_callback,
	FLAC__StreamDecoderSeekCallback seek_callback,
	FLAC__StreamDecoderTellCallback tell_callback,
	FLAC__StreamDecoderLengthCallback length_callback,
	FLAC__StreamDecoderEofCallback eof_callback,
	FLAC__StreamDecoderWriteCallback write_callback,
	FLAC__StreamDecoderMetadataCallback metadata_callback,
	FLAC__StreamDecoderErrorCallback error_callback,
	void* client_data
	);

#include <vector>

inline void WriteToBuffer16 (int16_t* buffer, FLAC__int16 data)
{
	union {
		int16_t word;
		int8_t arr[2];
	} temp;
	temp.arr[0] = (int8_t)(data & 0xff);
	temp.arr[1] = (int8_t)((data >> 8) & 0xff);
	*buffer = temp.word;
}

class __flac_decoder : public dseed::media_decoder
{
public:
	__flac_decoder (bool isOggContainer)
		: _refCount (1), _file (nullptr), _byterate (0)
		, _initFunc (isOggContainer ? FLAC__stream_decoder_init_ogg_stream : FLAC__stream_decoder_init_stream)
	{

	}
	~__flac_decoder ()
	{
		if (_file)
		{
			FLAC__stream_decoder_delete (_file);
			_file = nullptr;
		}
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
	dseed::error_t initialize (dseed::stream* stream)
	{
		_stream = stream;

		_file = FLAC__stream_decoder_new ();
		auto result = _initFunc (_file,
			[] (const FLAC__StreamDecoder* decoder, FLAC__byte buffer[], size_t* bytes, void* client_data)
			-> FLAC__StreamDecoderReadStatus
			{
				dseed::stream* stream = ((__flac_decoder*)client_data)->_stream;
				int64_t readed = stream->read (buffer, *bytes);
				if (readed == 0)
					return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
				else if (readed < 0)
					return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
				*bytes = readed;

				return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
			},
			[] (const FLAC__StreamDecoder* decoder, FLAC__uint64 absolute_byte_offset, void* client_data)
				-> FLAC__StreamDecoderSeekStatus
			{
				dseed::stream* stream = ((__flac_decoder*)client_data)->_stream;
				if (stream->seek (dseed::seekorigin_begin, absolute_byte_offset) == -1)
					return FLAC__STREAM_DECODER_SEEK_STATUS_UNSUPPORTED;
				return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
			},
				[] (const FLAC__StreamDecoder* decoder, FLAC__uint64* absolute_byte_offset, void* client_data)
				-> FLAC__StreamDecoderTellStatus
			{
				dseed::stream* stream = ((__flac_decoder*)client_data)->_stream;
				*absolute_byte_offset = stream->position ();
				if (*absolute_byte_offset == -1)
					return FLAC__STREAM_DECODER_TELL_STATUS_UNSUPPORTED;
				return FLAC__STREAM_DECODER_TELL_STATUS_OK;
			},
				[] (const FLAC__StreamDecoder* decoder, FLAC__uint64* stream_length, void* client_data)
				-> FLAC__StreamDecoderLengthStatus
			{
				dseed::stream* stream = ((__flac_decoder*)client_data)->_stream;
				*stream_length = stream->length ();
				if (*stream_length == -1)
					return FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED;
				return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
			},
				[](const FLAC__StreamDecoder* decoder, void* client_data) -> FLAC__bool
			{
				dseed::stream* stream = ((__flac_decoder*)client_data)->_stream;
				return stream->position () == stream->length ();
			},
				[] (const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* client_data)
				-> FLAC__StreamDecoderWriteStatus
			{
				__flac_decoder* jrmdecoder = (__flac_decoder*)client_data;
				dseed::stream* stream = jrmdecoder->_stream;

				int16_t* tempBuffer = (int16_t*)jrmdecoder->_bytes;
				for (unsigned i = 0; i < frame->header.blocksize; ++i)
				{
					for (unsigned c = 0; c < frame->header.channels; ++c)
						WriteToBuffer16 (tempBuffer++, (FLAC__int16)buffer[c][i]);
				}
				jrmdecoder->_bytesRead = frame->header.blocksize * frame->header.channels * 2;
				return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
			},
				[](const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata, void* client_data)
			{
				__flac_decoder* jrmdecoder = (__flac_decoder*)client_data;
				jrmdecoder->_totalSamples = (int64_t)metadata->data.stream_info.total_samples;
				jrmdecoder->_samplerate = metadata->data.stream_info.sample_rate;
				jrmdecoder->_channels = metadata->data.stream_info.channels;
				jrmdecoder->_bitsPerSample = metadata->data.stream_info.bits_per_sample;

				jrmdecoder->_byterate = (jrmdecoder->_channels * (jrmdecoder->_bitsPerSample / 8) * jrmdecoder->_samplerate);
			},
				[](const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status, void* client_data)
			{

			}, this);

		if (result != FLAC__STREAM_DECODER_INIT_STATUS_OK)
		{
			FLAC__stream_decoder_delete (_file);
			_file = nullptr;
			return dseed::error_fail;
		}

		FLAC__stream_decoder_process_until_end_of_metadata (_file);

		if (_channels == 0)
			return dseed::error_fail;

		if (!FLAC__stream_decoder_process_single (_file))
		{
			FLAC__stream_decoder_delete (_file);
			_file = nullptr;
			return dseed::error_fail;
		}

		FLAC__stream_decoder_seek_absolute (_file, 0);

		return dseed::error_good;
	}

public:
	virtual dseed::sampletype_t sample_type (size_t streamIndex) override
	{
		if (streamIndex != 0)
			return dseed::sampletype_out_of_range;
		return dseed::sampletype_audio;
	}
	virtual dseed::error_t sample_format (size_t streamIndex, void* buffer, size_t bufferSize) override
	{
		if (streamIndex != 0)
			return dseed::error_out_of_range;
		if (buffer == nullptr || bufferSize != sizeof (dseed::audioformat))
			return dseed::error_invalid_args;

		*((dseed::audioformat*)buffer) = dseed::audioformat (_channels, _bitsPerSample, _samplerate, dseed::pulseformat_pcm);

		return dseed::error_good;
	}

public:
	virtual dseed::timespan_t duration () override
	{
		uint64_t bytes = FLAC__stream_decoder_get_total_samples (_file);
		return dseed::timespan_t::from_seconds ((bytes * _channels * (_bitsPerSample / 8)) / (double)_byterate);
	}

public:
	virtual dseed::timespan_t reading_position () override
	{
		uint64_t bytesPos;
		FLAC__stream_decoder_get_decode_position (_file, &bytesPos);
		return dseed::timespan_t::from_seconds (bytesPos / (double)_byterate);
	}
	virtual dseed::error_t set_reading_position (dseed::timespan_t pos) override
	{
		if (_initFunc == FLAC__stream_decoder_init_ogg_stream)
			return dseed::error_not_support;
		return FLAC__stream_decoder_seek_absolute (_file, (int64_t)(pos.total_seconds () * _byterate))
			? dseed::error_good : dseed::error_fail;
	}

public:
	virtual dseed::error_t decode (size_t streamIndex, dseed::media_sample** sample) override
	{
		if (streamIndex != 0)
			return dseed::error_out_of_range;
		if (sample == nullptr)
			return dseed::error_invalid_args;

		uint64_t bytesPos;
		if (_initFunc == FLAC__stream_decoder_init_ogg_stream)
			bytesPos = 0;
		else
			if (!FLAC__stream_decoder_get_decode_position (_file, &bytesPos))
				return dseed::error_fail;
		dseed::timespan_t current = dseed::timespan_t::from_seconds (bytesPos / (double)_byterate);

		bool loop = true;
		while (loop)
		{
			if (!FLAC__stream_decoder_process_single (_file))
				return dseed::error_fail;

			auto state = FLAC__stream_decoder_get_state (_file);
			switch (state)
			{
			case FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC:
				loop = false;
				break;
			case FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM:
				return dseed::error_end_of_file;
			default:
				continue;
			}
		}

		dseed::timespan_t duration = dseed::timespan_t::from_seconds (_bytesRead / (double)_byterate);
		*sample = new __common_media_sample (dseed::sampletype_audio, _bytes, current, duration, _byterate);

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;

	dseed::auto_object<dseed::stream> _stream;

	FLAC_INIT _initFunc;
	FLAC__StreamDecoder* _file;

	int8_t _bytes[1048576];
	int64_t _bytesRead;

	int _channels, _bitsPerSample, _samplerate, _byterate;
	int64_t _totalSamples;
};
#endif

dseed::error_t dseed::create_flac_media_decoder (dseed::stream* stream, dseed::media_decoder** decoder)
{
#if defined ( USE_FLAC )
	dseed::auto_object<__flac_decoder> temp = new __flac_decoder (false);
	if (temp == nullptr)
		return dseed::error_out_of_memory;

	dseed::error_t err;
	if (dseed::failed (err = temp->initialize (stream)))
	{
		temp.release ();
		temp = new __flac_decoder (false);
		if (temp == nullptr)
			return dseed::error_out_of_memory;
		if (dseed::failed (err = temp->initialize (stream)))
			return err;
	}

	*decoder = temp.detach ();

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}