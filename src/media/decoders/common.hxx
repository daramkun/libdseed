#ifndef __DSEED_INTERNAL_MEDIA_DECODER_COMMON_HXX__
#define __DSEED_INTERNAL_MEDIA_DECODER_COMMON_HXX__

#include <vector>

#include <vector>

class __common_media_sample : public dseed::media::media_sample
{
public:
	__common_media_sample (dseed::media::sampletype type, void* buffer, size_t bufferSize, dseed::timespan time, dseed::timespan duration)
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
	virtual dseed::media::sampletype type () override { return _sampleType; }

public:
	virtual dseed::timespan position () override { return _time; }
	virtual dseed::timespan duration () override { return _duration; }

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

	dseed::media::sampletype _sampleType;
	std::vector<uint8_t> _data;

	dseed::timespan _time, _duration;
};

#endif