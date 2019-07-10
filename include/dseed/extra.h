#ifndef __DSEED_EXTRA_H__
#define __DSEED_EXTRA_H__

namespace dseed
{
	// Memory Blob
	class DSEEDEXP blob : public object
	{
	public:
		virtual void* blob_pointer () = 0;
		virtual size_t blob_length () = 0;
	};

	// Create Empty Blob
	DSEEDEXP error_t create_memoryblob (size_t length, dseed::blob** blob);
	// Create Blob from Exists Buffer
	DSEEDEXP error_t create_bufferblob (void* buffer, size_t length, bool copy, dseed::blob** blob);
	// Create Blob from Stream
	DSEEDEXP error_t create_streamblob (dseed::stream* stream, dseed::blob** blob);

	// Attributes
	class DSEEDEXP attributes : public object
	{
	public:
		virtual error_t get_int32 (uint64_t key, int32_t* value) = 0;
		virtual error_t get_uint32 (uint64_t key, uint32_t* value) = 0;
		virtual error_t get_int64 (uint64_t key, int64_t* value) = 0;
		virtual error_t get_uint64 (uint64_t key, uint64_t* value) = 0;
		virtual error_t get_single (uint64_t key, float* value) = 0;
		virtual error_t get_double (uint64_t key, double* value) = 0;
		virtual error_t get_object (uint64_t key, object** value) = 0;

	public:
		virtual error_t set_int32 (uint64_t key, int32_t value) = 0;
		virtual error_t set_uint32 (uint64_t key, uint32_t value) = 0;
		virtual error_t set_int64 (uint64_t key, int64_t value) = 0;
		virtual error_t set_uint64 (uint64_t key, uint64_t value) = 0;
		virtual error_t set_single (uint64_t key, float value) = 0;
		virtual error_t set_double (uint64_t key, double value) = 0;
		virtual error_t set_object (uint64_t key, object* value) = 0;

	public:
		virtual error_t get_size (uint64_t key, uint32_t* width, uint32_t* height);
		virtual error_t set_size (uint64_t key, uint32_t width, uint32_t height);

	public:
		virtual error_t get_fraction (uint64_t key, int32_t* numerator, int32_t* denominator);
		virtual error_t set_fraction (uint64_t key, int32_t numerator, int32_t denominator);
	};

	// Create Empty Attributes
	DSEEDEXP error_t create_attributes (attributes** attr);
}

#endif