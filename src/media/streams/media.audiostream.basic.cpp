#include <dseed.h>

#include <vector>

class __buffered_stream : public dseed::audio_stream
{
public:
	__buffered_stream (dseed::media_decoder* decoder, int32_t streamIndex)
		: _refCount (1), _decoder (decoder), _streamIndex (streamIndex)
	{
		dseed::create_variable_memorystream (&_memstream, true);

		decoder->sample_format (streamIndex, &_wf, sizeof (dseed::audioformat));
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
	virtual size_t write (const void* data, size_t length) override { return 0; }
	virtual bool seek (dseed::seekorigin origin, size_t offset) override
	{
		int64_t unit;
		switch (origin)
		{
		case dseed::seekorigin::begin: unit = offset; break;
		case dseed::seekorigin::current: unit = _position + offset; break;
		case dseed::seekorigin::end: unit = length () - offset; break;
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
	virtual void flush () override { }
	virtual dseed::error_t set_length (size_t length) override
	{
		return dseed::error_not_impl;
	}
	virtual size_t position () override { return _position; }
	virtual size_t length () override { return (size_t)(_decoder->duration () * _wf.bytes_per_sec); }

public:
	virtual bool readable () override { return true; }
	virtual bool writable () override { return false; }
	virtual bool seekable () override { return true; }

public:
	virtual dseed::error_t audioformat (dseed::audioformat* wf) override { *wf = _wf; return dseed::error_good; }

private:
	dseed::error_t buffering () noexcept
	{
		if ((_wf.bytes_per_sec / 100) > _memstream->length ())
		{
			while (_wf.bytes_per_sec > _memstream->length ())
			{
				dseed::error_t err;
				dseed::auto_object<dseed::media_sample> as;
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

	dseed::auto_object<dseed::media_decoder> _decoder;
	dseed::auto_object<dseed::stream> _memstream;
	size_t _position;
	dseed::audioformat _wf;
	size_t _streamIndex;
};

dseed::error_t dseed::create_audio_buffered_stream (media_decoder* decoder, audio_stream** stream, int32_t streamIndex)
{
	if (decoder == nullptr || stream == nullptr)
		return dseed::error_invalid_args;

	if (streamIndex == -1)
	{
		for (int i = 0;; ++i)
		{
			auto st = decoder->sample_type (i);
			if (st == dseed::sampletype_audio)
			{
				streamIndex = i;
				break;
			}
			else if (st == dseed::sampletype_out_of_range)
				return dseed::error_not_support;
		}
	}
	else
	{
		if (decoder->sample_type (streamIndex) != dseed::sampletype_audio)
			return dseed::error_invalid_args;
	}

	*stream = new __buffered_stream (decoder, streamIndex);
	if (*stream == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}

class __whole_stream : public dseed::audio_stream
{
public:
	__whole_stream (dseed::media_decoder* decoder, int32_t streamIndex)
		: _refCount (1), _position (0)
	{
		decoder->sample_format (streamIndex, &_wf, sizeof (dseed::audioformat));
		_bytes.resize (decoder->duration () * _wf.bytes_per_sec);

		size_t written = 0;
		while (true)
		{
			dseed::auto_object<dseed::media_sample> sample;
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
	virtual size_t read (void* buffer, size_t length) override
	{
		auto retValue = (_bytes.size () < _position + length) ? _bytes.size () - _position : length;
		memcpy (buffer, _bytes.data () + _position, (size_t)retValue);
		_position += retValue;
		return retValue;
	}
	virtual size_t write (const void* data, size_t length) override { return 0; }
	virtual bool seek (dseed::seekorigin origin, size_t offset) override
	{
		switch (origin)
		{
		case dseed::seekorigin::begin:
			if (offset > _bytes.size () || offset < 0) return false;
			_position = offset;
			break;

		case dseed::seekorigin::current:
			if (_position + offset > _bytes.size () || _position + offset < 0) return false;
			_position += offset;
			break;

		case dseed::seekorigin::end:
			if (_bytes.size () + offset > _bytes.size () || _bytes.size () + offset < 0) return false;
			_position = _bytes.size () - offset;
			break;
		}
		return true;
	}
	virtual void flush () override { }
	virtual dseed::error_t set_length (size_t length) override { return dseed::error_not_impl; }
	virtual size_t position () override { return _position; }
	virtual size_t length () override { return _bytes.size (); }

public:
	virtual bool readable () override { return true; }
	virtual bool writable () override { return false; }
	virtual bool seekable () override { return true; }

public:
	virtual dseed::error_t audioformat (dseed::audioformat* wf) override { *wf = _wf; return dseed::error_good; }

private:
	std::atomic<int32_t> _refCount;
	std::vector<uint8_t> _bytes;
	size_t _position;
	dseed::audioformat _wf;
};

dseed::error_t dseed::create_audio_wholestored_stream (media_decoder* decoder, audio_stream** stream, int32_t streamIndex)
{
	if (decoder == nullptr || stream == nullptr)
		return dseed::error_invalid_args;

	if (streamIndex == -1)
	{
		for (int i = 0;; ++i)
		{
			auto st = decoder->sample_type (i);
			if (st == dseed::sampletype_audio)
			{
				streamIndex = i;
				break;
			}
			else if (st == dseed::sampletype_out_of_range)
				return dseed::error_not_support;
		}
	}
	else
	{
		if (decoder->sample_type (streamIndex) != dseed::sampletype_audio)
			return dseed::error_invalid_args;
	}

	*stream = new __whole_stream (decoder, streamIndex);
	if (*stream == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}