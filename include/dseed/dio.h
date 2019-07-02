#ifndef __DSEED_DIO_H__
#define __DSEED_DIO_H__

#include <dseed/dcommon.h>

namespace dseed
{
	// Stream Seeking Origin
	enum DSEEDEXP seekorigin_t : int32_t
	{
		// Seek from Begin of Stream
		seekorigin_begin,
		// Seek from Current of Stream
		seekorigin_current,
		// Seek from End of Stream
		seekorigin_end,
	};

	// Stream
	class DSEEDEXP stream : public object
	{
	public:
		// Read from Stream
		virtual size_t read (void* buffer, size_t length) = 0;
		// Write to Stream
		virtual size_t write (const void* data, size_t length) = 0;
		// Seeking Position
		virtual bool seek (seekorigin_t origin, size_t offset) = 0;
		// Flush Stream
		virtual void flush () = 0;
		// Set Length
		virtual error_t set_length (size_t length) = 0;

	public:
		// Get Stream Read/Write Position
		virtual size_t position () = 0;
		// Get Stream Length
		virtual size_t length () = 0;

	public:
		// Is Stream Readable?
		virtual bool readable () = 0;
		// Is Stream Writable?
		virtual bool writable () = 0;
		// Is Stream Seekable?
		virtual bool seekable () = 0;
	};

	// Create Memory Stream from Exists Buffer
	DSEEDEXP error_t create_memorystream (void* buffer, size_t length, stream** stream);
	// Create Variable Empty Memory Stream
	DSEEDEXP error_t create_variable_memorystream (stream** stream, bool remove_after_read = false);
	// Create File Stream (Maybe some platform not support)
	DSEEDEXP error_t create_native_filestream (const char* path, bool create, stream** stream);

	// File System
	class DSEEDEXP filesystem : public object
	{
	public:
		// Create New Directory
		virtual error_t create_directory (const char* path) PURE;
		// Create File Stream
		virtual error_t create_file (const char* path, bool create, stream** stream) PURE;
		// Delete File
		virtual error_t delete_file (const char* path) PURE;

		// Is File Exists?
		virtual bool file_exists (const char* path) PURE;
		// Is Directory Exists?
		virtual bool directory_exists (const char* path) PURE;
	};

	// Native File System Directories
	enum DSEEDEXP nativefilesystem_t : int32_t
	{
		// Document Directory
		nativefilesystem_documents,
		// Assets Directory
		nativefilesystem_assets,
		// Temporary Directory
		nativefilesystem_temporary,
	};

	// Create Native File System
	DSEEDEXP error_t create_native_filesystem (nativefilesystem_t fst, filesystem** fileSystem);

	// Memory Blob
	class DSEEDEXP blob : public object
	{
	public:
		virtual void* blob_pointer () = 0;
		virtual size_t blob_length () = 0;
	};

	// Create Empty Blob
	DSEEDEXP error_t create_memoryblob (size_t length, blob** blob);
	// Create Blob from Exists Buffer
	DSEEDEXP error_t create_bufferblob (void* buffer, size_t length, bool copy, blob** blob);
	// Create Blob from Stream
	DSEEDEXP error_t create_streamblob (stream* stream, blob** blob);
}

#endif