#include <dseed.h>

#include <vector>
#include "../libs/AudEarTypeConverter.hxx"

const __TC_SAMPLE_CONVERTERS& sample_converters = __TC_get_optimal_converter ();

class __pcm_to_float_stream : public dseed::media::audio_stream
{
public:
	__pcm_to_float_stream (dseed::media::audio_stream* base)
		: _refCount (1), _base (base)
	{
		_base->format (&_wf);

		switch (_wf.bits_per_sample)
		{
			case 8: _conv = sample_converters.i8_to_f32; break;
			case 16: _conv = sample_converters.i16_to_f32; break;
			case 24: _conv = sample_converters.i24_to_f32; break;
			case 32: _conv = sample_converters.i32_to_f32; break;
			default: _conv = nullptr; break;
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
	virtual size_t read (void* buffer, size_t length) noexcept override
	{
		int64_t readLen = length / 4 * (_wf.bits_per_sample / 8);
		std::vector<uint8_t> readBuffer (readLen);
		int64_t ret = _base->read (readBuffer.data (), readLen);
		if (ret == 0) return 0;

		if (!_conv (readBuffer.data (), buffer, ret, length))
			return 0;

		return ret / (_wf.bits_per_sample / 8) * 4;
	}

	virtual size_t write (const void* data, size_t length) noexcept override { return 0; }

	virtual bool seek (dseed::io::seekorigin origin, size_t offset) noexcept override
	{
		return _base->seek (origin, offset / 4 * (_wf.bits_per_sample / 8));
	}

	virtual void flush () noexcept override { }
	virtual dseed::error_t set_length (size_t length) noexcept override { return dseed::error_not_impl; }

	virtual size_t position () noexcept override { return _base->position () / (_wf.bits_per_sample / 8) * 4; }
	virtual size_t length () noexcept override { return _base->length () / (_wf.bits_per_sample / 8) * 4; }

	virtual bool readable () noexcept override { return _base->readable (); }
	virtual bool writable () noexcept override { return _base->writable (); }
	virtual bool seekable () noexcept override { return _base->seekable (); }

	virtual dseed::error_t format (dseed::media::audioformat* wf) override
	{
		wf->channels = _wf.channels;
		wf->bits_per_sample = 32;
		wf->samples_per_sec = _wf.samples_per_sec;
		wf->block_align = wf->channels * 4;
		wf->bytes_per_sec = wf->block_align * wf->samples_per_sec;
		wf->pulse_format = dseed::media::pulseformat::ieee_float;
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	dseed::autoref<dseed::media::audio_stream> _base;
	dseed::media::audioformat _wf;

	__TypeConverterFunction _conv;
};

class __float_to_pcm_stream : public dseed::media::audio_stream
{
public:
	__float_to_pcm_stream (dseed::media::audio_stream* base, int bitsPerSample)
		: _refCount (1), _base (base), _bps (bitsPerSample)
	{
		_base->format (&_wf);

		switch (bitsPerSample)
		{
			case 8: _conv = sample_converters.f32_to_i8; break;
			case 16: _conv = sample_converters.f32_to_i16; break;
			case 24: _conv = sample_converters.f32_to_i24; break;
			case 32: _conv = sample_converters.f32_to_i32; break;
			default: _conv = nullptr; break;
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
	virtual size_t read (void* buffer, size_t length) noexcept override
	{
		int64_t readLen = length / (_bps / 8) * 4;
		std::vector<uint8_t> readBuffer (readLen);
		int64_t ret = _base->read (readBuffer.data (), readLen);
		if (ret == 0) return 0;

		if (!_conv (readBuffer.data (), buffer, ret, length))
			return 0;

		return ret / 4 * (_bps / 8);
	}

	virtual size_t write (const void* data, size_t length) noexcept override { return 0; }

	virtual bool seek (dseed::io::seekorigin origin, size_t offset) noexcept override
	{
		return _base->seek (origin, offset / (_bps / 8) * 4);
	}

	virtual void flush () noexcept override { }
	virtual dseed::error_t set_length (size_t length) noexcept override { return dseed::error_not_impl; }

	virtual size_t position () noexcept override { return _base->position () / 4 * (_bps / 8); }
	virtual size_t length () noexcept override { return _base->length () / 4 * (_bps / 8); }

	virtual bool readable () noexcept override { return _base->readable (); }
	virtual bool writable () noexcept override { return _base->writable (); }
	virtual bool seekable () noexcept override { return _base->seekable (); }

	virtual dseed::error_t format (dseed::media::audioformat* wf) override
	{
		wf->channels = _wf.channels;
		wf->bits_per_sample = _bps;
		wf->samples_per_sec = _wf.samples_per_sec;
		wf->block_align = wf->channels * (_bps / 8);
		wf->bytes_per_sec = wf->block_align * wf->samples_per_sec;
		wf->pulse_format = dseed::media::pulseformat::pcm;
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	dseed::autoref<dseed::media::audio_stream> _base;
	dseed::media::audioformat _wf;
	int32_t _bps;

	__TypeConverterFunction _conv;
};

class __pcm_to_pcm_stream : public dseed::media::audio_stream
{
public:
	__pcm_to_pcm_stream (dseed::media::audio_stream* base, int bitsPerSample)
		: _refCount (1), _base (base), _bps (bitsPerSample)
	{
		_base->format (&_wf);

		switch (_wf.bits_per_sample)
		{
			case 8:
				switch (_bps)
				{
					case 16: _conv = sample_converters.i8_to_i16; break;
					case 24: _conv = sample_converters.i8_to_i24; break;
					case 32: _conv = sample_converters.i8_to_i32; break;
				}
				break;
			case 16:
				switch (_bps)
				{
					case 8: _conv = sample_converters.i16_to_i8; break;
					case 24: _conv = sample_converters.i16_to_i24; break;
					case 32: _conv = sample_converters.i16_to_i32; break;
				}
				break;
			case 24:
				switch (_bps)
				{
					case 8: _conv = sample_converters.i24_to_i8; break;
					case 16: _conv = sample_converters.i24_to_i16; break;
					case 32: _conv = sample_converters.i24_to_i32; break;
				}
				break;
			case 32:
				switch (_bps)
				{
					case 8: _conv = sample_converters.i32_to_i8; break;
					case 16: _conv = sample_converters.i32_to_i16; break;
					case 24: _conv = sample_converters.i32_to_i24; break;
				}
				break;
			default:
				_conv = nullptr;
				break;
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
	virtual size_t read (void* buffer, size_t length) noexcept override
	{
		int64_t readLen = length / (_bps / 8) * (_wf.bits_per_sample / 8);
		std::vector<uint8_t> readBuffer (readLen);
		int64_t ret = _base->read (readBuffer.data (), readLen);
		if (ret == 0) return 0;

		if (!_conv (readBuffer.data (), buffer, ret, length))
			return 0;

		return ret / (_wf.bits_per_sample / 8) * (_bps / 8);
	}

	virtual size_t write (const void* data, size_t length) noexcept override { return 0; }

	virtual bool seek (dseed::io::seekorigin origin, size_t offset) noexcept override
	{
		return _base->seek (origin, offset / (_bps / 8) * (_wf.bits_per_sample / 8));
	}

	virtual void flush () noexcept override { }
	virtual dseed::error_t set_length (size_t length) noexcept override { return dseed::error_not_impl; }

	virtual size_t position () noexcept override { return _base->position () / (_wf.bits_per_sample / 8) * (_bps / 8); }
	virtual size_t length () noexcept override { return _base->length () / (_wf.bits_per_sample / 8) * (_bps / 8); }

	virtual bool readable () noexcept override { return _base->readable (); }
	virtual bool writable () noexcept override { return _base->writable (); }
	virtual bool seekable () noexcept override { return _base->seekable (); }

	virtual dseed::error_t format (dseed::media::audioformat* wf) override
	{
		wf->channels = _wf.channels;
		wf->bits_per_sample = _bps;
		wf->samples_per_sec = _wf.samples_per_sec;
		wf->block_align = wf->channels * (_bps / 8);
		wf->bytes_per_sec = wf->block_align * wf->samples_per_sec;
		wf->pulse_format = dseed::media::pulseformat::pcm;
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	dseed::autoref<dseed::media::audio_stream> _base;
	dseed::media::audioformat _wf;
	int32_t _bps;

	__TypeConverterFunction _conv;
};

dseed::error_t dseed::media::reformat_audio (audio_stream* original, pulseformat fmt, int bits_per_sample, audio_stream** stream)
{
	if (original == nullptr || stream == nullptr)
		return dseed::error_invalid_args;
	if (!(fmt == pulseformat::pcm && (bits_per_sample == 8 || bits_per_sample == 16 || bits_per_sample == 24 || bits_per_sample == 32))
		&& !(fmt == pulseformat::ieee_float && bits_per_sample == 32))
		return dseed::error_invalid_args;

	audioformat original_format;
	original->format (&original_format);

	if (fmt == original_format.pulse_format && bits_per_sample == original_format.bits_per_sample)
	{
		*stream = original;
		original->retain ();
		return dseed::error_good;
	}
	else if (original_format.pulse_format == pulseformat::ieee_float && fmt == pulseformat::pcm)
		*stream = new __float_to_pcm_stream (original, bits_per_sample);
	else if (original_format.pulse_format == pulseformat::pcm && fmt == pulseformat::pcm)
		*stream = new __pcm_to_pcm_stream (original, bits_per_sample);
	else if (original_format.pulse_format == pulseformat::pcm && fmt == pulseformat::ieee_float)
		*stream = new __pcm_to_float_stream (original);
	else return dseed::error_fail;

	if (*stream == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}