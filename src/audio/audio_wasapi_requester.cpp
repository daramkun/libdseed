#include <dseed.h>

class __wasapi_audiorequester : public dseed::audio::audiorequester
{
public:
	virtual int32_t retain () override
	{
		return int32_t ();
	}
	virtual int32_t release () override
	{
		return int32_t ();
	}

public:
	virtual dseed::error_t native_object (void** nativeObject) override
	{
		return dseed::error_t ();
	}

public:
	virtual dseed::error_t format (dseed::media::audioformat* format) override
	{
		return dseed::error_t ();
	}

public:
	virtual dseed::error_t start () override
	{
		return dseed::error_t ();
	}
	virtual dseed::error_t stop () override
	{
		return dseed::error_t ();
	}

public:
	virtual dseed::error_t request (void* buffer, size_t bufferMaxSize) override
	{
		return dseed::error_t ();
	}
};