#include <dseed.h>

#include <vector>

#include "../../libs/WAVHelper.hxx"

class __wav_encoder : public dseed::media::media_encoder
{
public:
	__wav_encoder (dseed::io::stream* stream)
		: _refCount (1), _stream (stream), _inputStream (false), _bytesPerSec (0)
	{
		RIFF_HEADER riffHeader;
		_stream->write (&riffHeader, sizeof (riffHeader));
		WAVE_HEADER waveHeader;
		_stream->write (&waveHeader, sizeof (waveHeader));
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
	virtual dseed::error_t set_sample_format (size_t streamIndex, dseed::media::sampletype type, void* format, size_t formatSize) override
	{
		if (streamIndex != 0 || type != dseed::media::sampletype_audio || format == nullptr || formatSize != sizeof (dseed::media::audioformat))
			return dseed::error_invalid_args;
		if (_inputStream)
			return dseed::error_invalid_op;

		dseed::media::audioformat& wf = *((dseed::media::audioformat*)format);

		fmt_HEADER fmtHeader;
		fmtHeader.audio_type = wf.pulse_format == dseed::media::pulseformat::pcm
			? WAVE_FORMAT_PCM
			: (wf.pulse_format == dseed::media::pulseformat::ieee_float
				? WAVE_FORMAT_IEEE_FLOAT
				: 0);
		fmtHeader.audio_channels = wf.channels;
		fmtHeader.sample_rate = wf.samples_per_sec;
		fmtHeader.byte_rate = _bytesPerSec = wf.bytes_per_sec;
		fmtHeader.block_align = wf.block_align;
		fmtHeader.bits_per_sample = wf.bits_per_sample;
		_stream->write (&fmtHeader, sizeof (fmtHeader));

		data_HEADER dataHeader;
		_stream->write (&dataHeader, sizeof (dataHeader));

		_inputStream = true;

		return dseed::error_good;
	}

public:
	virtual dseed::error_t encode (size_t sampleIndex, dseed::media::media_sample* sample) override
	{
		if (!_inputStream) return dseed::error_invalid_op;
		if (sampleIndex != 0) return dseed::error_out_of_range;
		if (sample == nullptr) return dseed::error_invalid_args;

		size_t sampleBytesCount = dseed::media::get_audio_bytes_count (_bytesPerSec, sample->duration ());
		std::vector<uint8_t> buf (sampleBytesCount);
		sample->copy_sample (buf.data ());
		_stream->write (buf.data (), buf.size ());
		return dseed::error_good;
	}

	virtual dseed::error_t commit () override
	{
		_stream->seek (dseed::io::seekorigin::begin, 4);
		uint32_t totalSize = (uint32_t)_stream->length ();
		_stream->write (&totalSize, 4);
		uint32_t dataSeek = sizeof (RIFF_HEADER) + sizeof (WAVE_HEADER) + sizeof (fmt_HEADER) + 4;
		_stream->seek (dseed::io::seekorigin::begin, dataSeek);
		uint32_t pcmSize = totalSize - dataSeek;
		_stream->write (&pcmSize, 4);
		_stream->seek (dseed::io::seekorigin::end, 0);

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	dseed::autoref<dseed::io::stream> _stream;
	uint32_t _bytesPerSec;
	bool _inputStream;
};


dseed::error_t dseed::media::create_wav_media_encoder (dseed::io::stream* stream, const dseed::media::media_encoder_options* options, dseed::media::media_encoder** encoder)
{
	*encoder = new __wav_encoder (stream);
	if (*encoder == nullptr)
		return dseed::error_out_of_memory;
	return dseed::error_good;
}