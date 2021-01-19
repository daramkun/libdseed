#include <dseed.h>

#include <map>

using namespace dseed;

using mtmfn = std::function<bool (uint8_t * dest, const uint8_t * src, size_t srcLength, int ch)>;
using mtmtp = std::tuple<dseed::media::pulseformat, int8_t>;

template<typename TSample>
inline bool many_to_mono_ch (uint8_t* dest, const uint8_t* src, size_t srcLength, int ch) noexcept
{
	TSample* destPtr = (TSample*)dest;
	const TSample* srcPtr = (const TSample*)src;

	size_t destCount = srcLength / sizeof (TSample) / ch;
	size_t srcCount = srcLength / sizeof (TSample);

	for (size_t i = 0; i < destCount; ++i)
	{
		size_t srcI = i * ch;

		float sum = 0;
		int div = 0;
		for (size_t c = 0; c < ch; ++c)
		{
			sum += src[srcI + c];
			if (src[srcI + c] != 0)
				++div;
		}

		if (div != 0)
			dest[i] = (TSample)(sum / div);
		else
			dest[i] = 0;
	}

	return true;
}

std::map<mtmtp, mtmfn> g_mtmchs = {
	{ mtmtp (dseed::media::pulseformat::pcm, 8), many_to_mono_ch<int8_t> },
	{ mtmtp (dseed::media::pulseformat::pcm, 16), many_to_mono_ch<int16_t> },
	{ mtmtp (dseed::media::pulseformat::pcm, 24), many_to_mono_ch<int24_t> },
	{ mtmtp (dseed::media::pulseformat::pcm, 32), many_to_mono_ch<int32_t> },
	{ mtmtp (dseed::media::pulseformat::ieee_float, 32), many_to_mono_ch<float> },
};

class __many_to_mono_channel : public dseed::media::audio_stream
{
public:
	__many_to_mono_channel (dseed::media::audio_stream* original)
		: _refCount (1), _original (original)
	{
		_original->format (&_format);

		_fn = g_mtmchs[mtmtp (_format.pulse_format, _format.bits_per_sample)];
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
		size_t readLength = length * _format.channels;
		std::vector<int8_t> buf (readLength);
		size_t ret = _original->read (buf.data (), readLength);
		if (ret <= 0) return ret;

		if(!_fn ((uint8_t*)buffer, (const uint8_t*)buf.data (), ret, (int)_format.channels))
			return 0;

		return ret / _format.channels;
	}
	virtual size_t write (const void* data, size_t length) noexcept override { return 0; }
	virtual bool seek (dseed::io::seekorigin origin, size_t offset) noexcept override
	{
		return _original->seek (origin, offset / _format.channels);
	}
	virtual void flush () noexcept override { }
	virtual dseed::error_t set_length (size_t length) noexcept override { return dseed::error_not_impl; }

public:
	virtual size_t position () noexcept override { return _original->position () / _format.channels; }
	virtual size_t length () noexcept override { return _original->length () / _format.channels; }

public:
	virtual bool readable () noexcept override { return _original->readable (); }
	virtual bool writable () noexcept override { return false; }
	virtual bool seekable () noexcept override { return _original->seekable (); }

private:
	std::atomic<int32_t> _refCount;

	dseed::autoref<dseed::media::audio_stream> _original;
	dseed::media::audioformat _format;

	mtmfn _fn;
};