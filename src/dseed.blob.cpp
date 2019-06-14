#include <dseed.h>

#include <memory>

class __memblob : public dseed::blob
{
public:
	__memblob (size_t length)
		: _refCount (1), _memLength (length), _mem (new uint8_t[length])
	{ }

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
	virtual void* blob_pointer () override { return &_mem[0]; }
	virtual size_t blob_length () override { return _memLength; }

private:
	std::atomic<int32_t> _refCount;
	std::shared_ptr<uint8_t[]> _mem;
	size_t _memLength;
};

dseed::error_t dseed::create_memoryblob (size_t length, blob** blob)
{
	*blob = new __memblob (length);
	if (*blob == nullptr)
		return error_out_of_memory;
	return error_good;
}

class __bufferblob : public dseed::blob
{
public:
	__bufferblob (void * buffer, size_t length)
		: _refCount (1), _length (length), _buf (buffer)
	{ }

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
	virtual void* blob_pointer () override { return _buf; }
	virtual size_t blob_length () override { return _length; }

private:
	std::atomic<int32_t> _refCount;
	void* _buf;
	size_t _length;
};

dseed::error_t dseed::create_bufferblob (void* buffer, size_t length, bool copy, blob** blob)
{
	if (!copy)
	{
		*blob = new __bufferblob (buffer, length);
		if (*blob == nullptr)
			return error_out_of_memory;
	}
	else
	{
		*blob = new __memblob (length);
		if (*blob == nullptr)
			return error_out_of_memory;
		memcpy ((*blob)->blob_pointer (), buffer, length);
	}
	return error_good;
}