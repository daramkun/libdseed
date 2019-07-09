#ifndef __DSEED_INTERNAL_MEDIA_DECODER_HXX__
#define __DSEED_INTERNAL_MEDIA_DECODER_HXX__

#include <vector>

class __common_media_sample : public dseed::media_sample
{
public:
	__common_media_sample (dseed::sampletype_t type, void * buffer, size_t bufferSize, dseed::timespan_t time, dseed::timespan_t duration )
		: _refCount (1)
		, _sampleType (type), _data (bufferSize), _time (time), _duration (duration)
	{
		memcpy (_data.data (), buffer, bufferSize);
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
	virtual dseed::sampletype_t sample_type () override { return _sampleType; }

public:
	virtual dseed::timespan_t sample_time () override { return _time; }
	virtual dseed::timespan_t sample_duration () override { return _duration; }

public:
	virtual dseed::error_t copy_sample (void* buffer) override
	{
		if (buffer == nullptr) return dseed::error_invalid_args;

		memcpy (buffer, _data.data (), _data.size ());

		return dseed::error_good;
	}

	virtual size_t sample_size () override { return _data.size (); }

public:
	std::atomic<int32_t> _refCount;

	dseed::sampletype_t _sampleType;
	std::vector<uint8_t> _data;

	dseed::timespan_t _time, _duration;
};

#endif