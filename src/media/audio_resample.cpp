#include <dseed.h>

#include <map>

using namespace dseed;

using rsfn = std::function<bool (uint8_t * dest, const uint8_t * src, size_t srcLength, int ch, int sr, int nsr)>;
using rstp = std::tuple<dseed::media::resample, dseed::media::pulseformat, int8_t>;

template<typename TSample>
inline bool resample_nearest (uint8_t* dest, const uint8_t* src, size_t srcLength, int ch, int sr, int nsr) noexcept
{
	TSample* destPtr = (TSample*)dest;
	const TSample* srcPtr = (const TSample*)src;

	size_t destCount = srcLength / sizeof (TSample) * nsr / sr;
	size_t srcCount = srcLength / sizeof (TSample);

	for (size_t i = 0; i < destCount; i += ch)
	{
		size_t srcI = i * sr / nsr;

		for (size_t c = 0; c < ch; ++c)
		{
			dest[i + c] = src[srcI + c];
		}
	}

	return true;
}

std::map<rstp, rsfn> g_resamples = {
	{ rstp (dseed::media::resample::nearest, dseed::media::pulseformat::pcm, 8), resample_nearest<int8_t> },
	{ rstp (dseed::media::resample::nearest, dseed::media::pulseformat::pcm, 16), resample_nearest<int16_t> },
	{ rstp (dseed::media::resample::nearest, dseed::media::pulseformat::pcm, 24), resample_nearest<int24_t> },
	{ rstp (dseed::media::resample::nearest, dseed::media::pulseformat::pcm, 32), resample_nearest<int32_t> },
	{ rstp (dseed::media::resample::nearest, dseed::media::pulseformat::ieee_float, 32), resample_nearest<float> },
};

class __resample_stream : public dseed::media::audio_stream
{
public:
	__resample_stream (audio_stream* stream, int32_t samplerate, rsfn fn)
		: _refCount (1), _stream (stream), _samplerate (samplerate), _fn (fn)
	{
		_stream->format (&_format);
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
	virtual size_t read (void* buffer, size_t length) noexcept override
	{
		size_t readLength = length * _format.samples_per_sec / _samplerate;
		std::vector<int8_t> buf (readLength);
		size_t ret = _stream->read (buf.data (), readLength);
		if (ret <= 0)
			return ret;
		if (!_fn ((uint8_t*)buffer, (const uint8_t*)buf.data (), buf.size (), _format.channels, _format.samples_per_sec, _samplerate))
			return 0;
		return ret * _samplerate / _format.samples_per_sec;
	}

	virtual size_t write (const void* data, size_t length) noexcept override { return 0; }

	virtual bool seek (dseed::io::seekorigin origin, size_t offset) noexcept override
	{
		return _stream->seek (origin, offset * _format.samples_per_sec / _samplerate);
	}

	virtual void flush () noexcept override { _stream->flush (); }

	virtual error_t set_length (size_t length) noexcept override { return dseed::error_not_impl; }

	virtual size_t position () noexcept override { return _stream->position () * _samplerate / _format.samples_per_sec; }
	virtual size_t length () noexcept override { return _stream->length () * _samplerate / _format.samples_per_sec; }

	virtual bool readable () noexcept override { return _stream->readable (); }
	virtual bool writable () noexcept override { return false; }
	virtual bool seekable () noexcept override { return _stream->seekable (); }

	virtual error_t format (dseed::media::audioformat* wf) override
	{
		*wf = _format;
		wf->samples_per_sec = _samplerate;
		return error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	dseed::autoref<dseed::media::audio_stream> _stream;
	dseed::media::audioformat _format;
	int32_t _samplerate;

	rsfn _fn;
};

dseed::error_t dseed::media::resample_audio (audio_stream* original, int32_t samplerate, resample interpolation, audio_stream** stream)
{
	if (original == nullptr || samplerate <= 0 || stream == nullptr)
		return dseed::error_invalid_args;

	audioformat format;
	if (dseed::failed (original->format (&format)))
		return dseed::error_fail;

	if (format.samples_per_sec == samplerate)
	{
		original->retain ();
		*stream = original;
		return dseed::error_good;
	}

	auto fn = g_resamples.find (rstp (interpolation, format.pulse_format, format.bits_per_sample));
	if (fn == g_resamples.end ())
		return dseed::error_not_support;

	*stream = new __resample_stream (original, samplerate, fn->second);
	if (*stream == nullptr)
		return dseed::error_out_of_memory;

	return error_good;
}