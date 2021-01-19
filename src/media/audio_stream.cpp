#include <dseed.h>

#include <cstring>
#include <vector>

class __buffered_stream : public dseed::media::audio_stream
{
public:
	__buffered_stream (dseed::media::media_decoder* decoder, int32_t streamIndex)
		: _refCount (1), _decoder (decoder), _streamIndex (streamIndex)
	{
		dseed::io::create_variable_memorystream (&_memstream, true);

		decoder->format (streamIndex, &_wf, sizeof (dseed::media::audioformat));
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
		if (_memstream->length () < length)
		{
			dseed::error_t err = buffering ();
			if (dseed::failed (err) && err != dseed::error_end_of_file)
				return 0;
		}

		if (_memstream->length () == 0)
			return 0;

		auto retValue = _memstream->read (buffer, length);
		_position += retValue;
		return retValue;
	}
	virtual size_t write (const void* data, size_t length) noexcept override { return 0; }
	virtual bool seek (dseed::io::seekorigin origin, size_t offset) noexcept override
	{
		int64_t unit;
		switch (origin)
		{
			case dseed::io::seekorigin::begin: unit = offset; break;
			case dseed::io::seekorigin::current: unit = _position + offset; break;
			case dseed::io::seekorigin::end: unit = length () - offset; break;
			default: return false;
		}

		if (unit > (int64_t)length ())
			unit = length ();
		if (unit < 0)
			unit = 0;

		if (dseed::failed (_decoder->set_reading_position (unit * _wf.bytes_per_sec)))
			return false;

		_memstream->set_length (0);

		_position = offset;

		return true;
	}
	virtual void flush () noexcept override { }
	virtual dseed::error_t set_length (size_t length) noexcept override
	{
		return dseed::error_not_impl;
	}
	virtual size_t position () noexcept override { return _position; }
	virtual size_t length () noexcept override { return (size_t)(_decoder->duration () * _wf.bytes_per_sec); }

public:
	virtual bool readable () noexcept override { return true; }
	virtual bool writable () noexcept override { return false; }
	virtual bool seekable () noexcept override { return true; }

public:
	virtual dseed::error_t format (dseed::media::audioformat* wf) override { *wf = _wf; return dseed::error_good; }

private:
	dseed::error_t buffering () noexcept
	{
		if ((_wf.bytes_per_sec / 100) > _memstream->length ())
		{
			while (_wf.bytes_per_sec > _memstream->length ())
			{
				dseed::error_t err;
				dseed::autoref<dseed::media::media_sample> as;
				err = _decoder->decode (_streamIndex, &as);
				if (dseed::failed (err))
				{
					if (err == dseed::error_end_of_file)
						return dseed::error_end_of_file;
					continue;
				}

				std::vector<uint8_t> bytes (as->sample_size ());
				as->copy_sample (bytes.data ());

				_memstream->write (bytes.data (), bytes.size ());
			}
		}
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;

	dseed::autoref<dseed::media::media_decoder> _decoder;
	dseed::autoref<dseed::io::stream> _memstream;
	size_t _position;
	dseed::media::audioformat _wf;
	size_t _streamIndex;
};

dseed::error_t dseed::media::create_audio_buffered_stream (media_decoder* decoder, audio_stream** stream, int32_t streamIndex)
{
	if (decoder == nullptr || stream == nullptr)
		return dseed::error_invalid_args;

	if (streamIndex == -1)
	{
		for (int i = 0;; ++i)
		{
			auto st = decoder->type (i);
			if (st == dseed::media::sampletype_audio)
			{
				streamIndex = i;
				break;
			}
			else if (st == dseed::media::sampletype_out_of_range)
				return dseed::error_not_support;
		}
	}
	else
	{
		if (decoder->type (streamIndex) != dseed::media::sampletype_audio)
			return dseed::error_invalid_args;
	}

	*stream = new __buffered_stream (decoder, streamIndex);
	if (*stream == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}

class __whole_stream : public dseed::media::audio_stream
{
public:
	__whole_stream (dseed::media::media_decoder* decoder, int32_t streamIndex)
		: _refCount (1), _position (0)
	{
		decoder->format (streamIndex, &_wf, sizeof (dseed::media::audioformat));
		_bytes.resize (decoder->duration () * _wf.bytes_per_sec);

		size_t written = 0;
		while (true)
		{
			dseed::autoref<dseed::media::media_sample> sample;
			if (dseed::failed (decoder->decode (streamIndex, &sample)))
				break;
			sample->copy_sample (_bytes.data () + written);
			written += sample->sample_size ();
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
		auto retValue = (_bytes.size () < _position + length) ? _bytes.size () - _position : length;
		memcpy (buffer, _bytes.data () + _position, (size_t)retValue);
		_position += retValue;
		return retValue;
	}
	virtual size_t write (const void* data, size_t length) noexcept override { return 0; }
	virtual bool seek (dseed::io::seekorigin origin, size_t offset) noexcept override
	{
		switch (origin)
		{
			case dseed::io::seekorigin::begin:
				if (offset > _bytes.size () || offset < 0) return false;
				_position = offset;
				break;

			case dseed::io::seekorigin::current:
				if (_position + offset > _bytes.size () || _position + offset < 0) return false;
				_position += offset;
				break;

			case dseed::io::seekorigin::end:
				if (_bytes.size () + offset > _bytes.size () || _bytes.size () + offset < 0) return false;
				_position = _bytes.size () - offset;
				break;
		}
		return true;
	}
	virtual void flush () noexcept override { }
	virtual dseed::error_t set_length (size_t length) noexcept override { return dseed::error_not_impl; }
	virtual size_t position () noexcept override { return _position; }
	virtual size_t length () noexcept override { return _bytes.size (); }

public:
	virtual bool readable () noexcept override { return true; }
	virtual bool writable () noexcept override { return false; }
	virtual bool seekable () noexcept override { return true; }

public:
	virtual dseed::error_t format (dseed::media::audioformat* wf) override { *wf = _wf; return dseed::error_good; }

private:
	std::atomic<int32_t> _refCount;
	std::vector<uint8_t> _bytes;
	size_t _position;
	dseed::media::audioformat _wf;
};

dseed::error_t dseed::media::create_audio_wholestored_stream (media_decoder* decoder, audio_stream** stream, int32_t streamIndex)
{
	if (decoder == nullptr || stream == nullptr)
		return dseed::error_invalid_args;

	if (streamIndex == -1)
	{
		for (int i = 0;; ++i)
		{
			auto st = decoder->type (i);
			if (st == dseed::media::sampletype_audio)
			{
				streamIndex = i;
				break;
			}
			else if (st == dseed::media::sampletype_out_of_range)
				return dseed::error_not_support;
		}
	}
	else
	{
		if (decoder->type (streamIndex) != dseed::media::sampletype_audio)
			return dseed::error_invalid_args;
	}

	*stream = new __whole_stream (decoder, streamIndex);
	if (*stream == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}

class __filter_stream : public dseed::media::audio_stream
{
public:
	__filter_stream (dseed::media::audio_stream* base, dseed::media::quadfilter* filters, size_t filtersCount)
		: _refCount (1), _base (base), _filters (filters, filters + filtersCount)
	{
		base->format (&_wf);
	}

public:
	virtual int32_t retain () override { return ++_refCount; }
	virtual int32_t release () override
	{
		auto ret = --_refCount;
		if (ret)
			delete this;
		return ret;
	}

public:
	virtual size_t read (void* buffer, size_t length) noexcept override
	{
		size_t ret = _base->read (buffer, length);
		if (ret <= 0) return 0;
		if (ret % _wf.block_align != 0)
			ret = ret / _wf.block_align * _wf.block_align;

		size_t len = ret / 4;
		float* tempBuffer = (float*)buffer;
		for (int i = 0; i < len; ++i)
		{
			int ch = i % _wf.channels;
			for (int f = 0; f < _filters.size (); ++f)
				tempBuffer[i] = _filters[f].process (tempBuffer[i], ch);
		}

		return ret;
	}
	virtual size_t write (const void* data, size_t length) noexcept override { return 0; }
	virtual bool seek (dseed::io::seekorigin origin, size_t offset) noexcept override { return _base->seek (origin, offset); }
	virtual void flush () noexcept override { }
	virtual dseed::error_t set_length (size_t length) noexcept override { return dseed::error_not_impl; }
	virtual size_t position () noexcept override { return size_t (); }
	virtual size_t length () noexcept override { return size_t (); }
	virtual bool readable () noexcept override { return true; }
	virtual bool writable () noexcept override { return false; }
	virtual bool seekable () noexcept override { return false; }
	virtual dseed::error_t format (dseed::media::audioformat* wf) override { *wf = _wf; return dseed::error_good; }

private:
	std::atomic<int32_t> _refCount;

	dseed::autoref<dseed::media::audio_stream> _base;
	std::vector<dseed::media::quadfilter> _filters;
	dseed::media::audioformat _wf;
};

dseed::error_t dseed::media::create_audio_filter_stream (dseed::media::audio_stream* original, dseed::media::quadfilter* filters, size_t filtersCount, dseed::media::audio_stream** stream)
{
	if (original == nullptr || filters == nullptr || filtersCount <= 0 || stream == nullptr)
		return dseed::error_invalid_args;

	dseed::media::audioformat wf;
	if (dseed::failed (original->format (&wf)))
		return dseed::error_fail;

	if (wf.pulse_format != dseed::media::pulseformat::ieee_float)
		return dseed::error_not_support;

	*stream = new __filter_stream (original, filters, filtersCount);
	if (*stream == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}