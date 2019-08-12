#include <dseed.h>

#include <map>

using namespace dseed;

using rsfn = std::function<bool (uint8_t* dest, const uint8_t* src, size_t srcLength, int ch, int sr, int nsr)>;
using rstp = std::tuple<dseed::resample_t, dseed::pulseformat_t, int8_t>;

template<typename TSample>
inline bool resampl_nearest (uint8_t* dest, const uint8_t* src, size_t srcLength, int ch, int sr, int nsr) noexcept
{
	TSample* destPtr = (TSample*)dest;
	const TSample* srcPtr = (const TSample*)src;

	size_t destCount = srcLength / sizeof (TSample) * nsr / sr;
	size_t srcCount = srcLength / sizeof (TSample);

	for (size_t i = 0; i < destCount; i += ch)
	{
		size_t srcI = destCount * src / nsr;

		for (size_t c = 0; c < ch; ++c)
		{
			destCount[i + c] = srcCount[srcI + c];
		}
	}
}

std::map<rstp, rsfn> g_resamples = {
	{ rstp (resample_nearest, pulseformat_pcm, 8), resampl_nearest<int8_t> },
	{ rstp (resample_nearest, pulseformat_pcm, 16), resampl_nearest<int16_t> },
	{ rstp (resample_nearest, pulseformat_pcm, 24), resampl_nearest<int24_t> },
	{ rstp (resample_nearest, pulseformat_pcm, 32), resampl_nearest<int32_t> },
	{ rstp (resample_nearest, pulseformat_ieee_float, 32), resampl_nearest<float> },
};

class __resample_stream : public dseed::audio_stream
{
public:
	__resample_stream (audio_stream* stream, int32_t samplerate, rsfn fn)
		: _refCount (1), _stream (stream), _samplerate (samplerate), _fn (fn)
	{
		_stream->audioformat (&_format);
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
	virtual size_t read (void* buffer, size_t length) override
	{
		size_t readLength = length * _format.samples_per_sec / _samplerate;
		std::vector<int8_t> buf (readLength);
		size_t ret = _stream->read (buf.data (), readLength);
		if (!_fn ((uint8_t*)buffer, (const uint8_t*)buf.data (), buf.size (), _format.channels, _format.samples_per_sec, _samplerate))
			return 0;
		return ret * _samplerate / _format.samples_per_sec;
	}

	virtual size_t write (const void* data, size_t length) override { return 0; }

	virtual bool seek (seekorigin_t origin, size_t offset) override
	{
		return _stream->seek (origin, offset * _format.samples_per_sec / _samplerate);
	}

	virtual void flush () override { _stream->flush (); }

	virtual error_t set_length (size_t length) override { return dseed::error_not_impl; }

	virtual size_t position () override { return _stream->position () * _samplerate / _format.samples_per_sec; }
	virtual size_t length () override { return _stream->length () * _samplerate / _format.samples_per_sec; }

	virtual bool readable () override { return _stream->readable (); }
	virtual bool writable () override { return false; }
	virtual bool seekable () override { return _stream->seekable (); }

	virtual error_t audioformat (dseed::audioformat* wf) override
	{
		*wf = _format;
		wf->samples_per_sec = _samplerate;
		return error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	dseed::auto_object<dseed::audio_stream> _stream;
	dseed::audioformat _format;
	int32_t _samplerate;

	rsfn _fn;
};

dseed::error_t dseed::resample_audio (audio_stream* original, int32_t samplerate, resample_t interpolation, audio_stream** stream)
{
	if (original == nullptr || samplerate <= 0 || stream == nullptr)
		return dseed::error_invalid_args;

	audioformat format;
	if (dseed::failed (original->audioformat (&format)))
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