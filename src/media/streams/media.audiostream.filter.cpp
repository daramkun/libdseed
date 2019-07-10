#include <dseed.h>

#include <vector>

class __filter_stream : public dseed::audio_stream
{
public:
	__filter_stream (dseed::audio_stream* base, dseed::quadfilter* filters, size_t filtersCount)
		: _refCount (1), _base (base), _filters (filters, filters + filtersCount)
	{
		base->audioformat (&_wf);
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
	virtual size_t read (void* buffer, size_t length) override
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
	virtual size_t write (const void* data, size_t length) override { return 0; }
	virtual bool seek (dseed::seekorigin_t origin, size_t offset) override { return _base->seek (origin, offset); }
	virtual void flush () override { }
	virtual dseed::error_t set_length (size_t length) override { return dseed::error_not_impl; }
	virtual size_t position () override { return size_t (); }
	virtual size_t length () override { return size_t (); }
	virtual bool readable () override { return true; }
	virtual bool writable () override { return false; }
	virtual bool seekable () override { return false; }
	virtual dseed::error_t audioformat (dseed::audioformat* wf) override { *wf = _wf; return dseed::error_good; }

private:
	std::atomic<int32_t> _refCount;

	dseed::auto_object<dseed::audio_stream> _base;
	std::vector<dseed::quadfilter> _filters;
	dseed::audioformat _wf;
};

dseed::error_t dseed::create_audio_filter_stream (dseed::audio_stream* original, dseed::quadfilter* filters, size_t filtersCount, dseed::audio_stream** stream)
{
	if (original == nullptr || filters == nullptr || filtersCount <= 0 || stream == nullptr)
		return dseed::error_invalid_args;

	dseed::audioformat wf;
	if (dseed::failed (original->audioformat (&wf)))
		return dseed::error_fail;

	if (wf.pulse_format != dseed::pulseformat_ieee_float)
		return dseed::error_not_support;

	*stream = new __filter_stream (original, filters, filtersCount);
	if (*stream == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}