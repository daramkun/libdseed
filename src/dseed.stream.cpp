#define _CRT_SECURE_NO_WARNINGS
#include <dseed.h>

#include <cstring>

class __memorystream : public dseed::stream
{
public:
	__memorystream (void* buffer, size_t length)
		: _refCount (1), _buffer (buffer), _length (length), _position (0)
	{ }

public:
	virtual int32_t retain () override
	{
		return ++_refCount;
	}
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
		if (_position + length < _length)
			length = _length - _position;
		if (length == 0)
			return 0;

		memcpy (buffer, _buffer, length);

		return length;
	}
	virtual size_t write (const void* data, size_t length) override
	{
		if (_position + length < _length)
			length = _length - _position;
		if (length == 0)
			return 0;

		memcpy (_buffer, data, length);

		return length;
	}
	virtual bool seek (dseed::seekorigin_t origin, size_t offset) override
	{
		switch (origin)
		{
		case dseed::seekorigin_begin: break;
		case dseed::seekorigin_current: offset += _position; break;
		case dseed::seekorigin_end: offset = _length - offset; break;
		default: return false;
		}

		if (offset < 0) offset = 0;
		if (offset > _length) offset = _length;

		_position = offset;

		return true;
	}

	virtual void flush () override { }
	virtual dseed::error_t set_length (size_t length) override
	{
		seek (dseed::seekorigin_begin, length);
		_length = length;
		return dseed::error_good;
	}

public:
	virtual size_t position () override { return _position; }
	virtual size_t length () override { return _length; }

public:
	virtual bool readable () override { return true; }
	virtual bool writable () override { return true; }
	virtual bool seekable () override { return true; }

private:
	std::atomic<int32_t> _refCount;
	void* _buffer;
	size_t _length;

	size_t _position;
};

dseed::error_t dseed::create_memorystream (void* buffer, size_t length, stream** stream)
{
	if (buffer == nullptr || length <= 0 || stream == nullptr)
		return dseed::error_invalid_args;
	*stream = new __memorystream (buffer, length);
	if (*stream == nullptr)
		return dseed::error_out_of_memory;
	return dseed::error_good;
}

#if PLATFORM_WINDOWS
class __native_filestream_win32 : public dseed::stream
{
public:
	__native_filestream_win32 (HANDLE file)
		: _refCount (1), _file (file)
	{ }
	~__native_filestream_win32 ()
	{
		CloseHandle (_file);
		_file = nullptr;
	}

public:
	virtual int32_t retain () override
	{
		return ++_refCount;
	}
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
		DWORD read;
		if (!ReadFile (_file, buffer, (DWORD)length, &read, nullptr))
			return -1;
		return read;
	}
	virtual size_t write (const void* data, size_t length) override
	{
		DWORD written;
		if (!WriteFile (_file, data, (DWORD)length, &written, nullptr))
			return -1;
		return written;
	}
	virtual bool seek (dseed::seekorigin_t origin, size_t offset) override
	{
		if (SetFilePointer (_file, (LONG)offset, nullptr, origin) == INVALID_SET_FILE_POINTER)
			return false;
		return true;
	}
	virtual void flush () override
	{
		FlushFileBuffers (_file);
	}
	virtual dseed::error_t set_length (size_t length) override
	{
		SetFilePointer (_file, (LONG)length, nullptr, FILE_BEGIN);
		SetEndOfFile (_file);
		return dseed::error_good;
	}

public:
	virtual size_t position () override
	{
		return SetFilePointer (_file, 0, nullptr, FILE_CURRENT);
	}
	virtual size_t length () override
	{
		return GetFileSize (_file, nullptr);
	}

public:
	virtual bool readable () override { return true; }
	virtual bool writable () override { return true; }
	virtual bool seekable () override { return true; }

private:
	std::atomic<int32_t> _refCount;
	HANDLE _file;
};
#elif PLATFORM_UWP
/*class __native_filestream_uwp : public dseed::stream
{
public:
	__native_filestream_uwp (Windows::Storage::Streams::IRandomAccessStream^ stream)
		: _refCount ( 1 )
	{

	}

public:


private:
	std::atomic<int32_t> _refCount;
};*/
#else
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <fcntl.h>
#	include <unistd.h>
class __native_filestream_posix : public dseed::stream
{
public:
	__native_filestream_posix (int fd)
		: _refCount (1), _fd (fd)
	{

	}
	~__native_filestream_posix ()
	{
		close (_fd);
		_fd = 0;
	}

public:
	virtual int32_t retain () override
	{
		return ++_refCount;
	}
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
		return ::read (_fd, buffer, length);
	}
	virtual size_t write (const void* data, size_t length) override
	{
		return ::write (_fd, data, length);
	}
	virtual bool seek (dseed::seekorigin_t origin, size_t offset) override
	{
		return !lseek (_fd, offset, origin);
	}
	virtual void flush () override
	{
		fsync (_fd);
	}
	virtual dseed::error_t set_length (size_t length) override
	{
		ftruncate (_fd, length);
		return dseed::error_good;
	}

public:
	virtual size_t position () override
	{
		return lseek (_fd, 0, SEEK_CUR);
	}
	virtual size_t length () override
	{
		struct stat s;
		fstat (_fd, &s);
		return s.st_size;
	}

public:
	virtual bool readable () override { return true; }
	virtual bool writable () override { return true; }
	virtual bool seekable () override { return true; }

private:
	std::atomic<int32_t> _refCount;
	int _fd;
};
#endif

dseed::error_t dseed::create_native_filestream (const char* path, bool create, stream** stream)
{
	if (stream == nullptr)
		return dseed::error_invalid_args;
#if PLATFORM_WINDOWS
	char16_t filename[256];
	utf8toutf16 (path, filename, 256);
	HANDLE file = CreateFile2 ((LPCWSTR)filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, create ? CREATE_ALWAYS : OPEN_EXISTING, nullptr);
	if (file == INVALID_HANDLE_VALUE)
	{
		auto error = GetLastError ();
		if (error == ERROR_ACCESS_DENIED)
			return dseed::error_access_denied;
		else if (error == ERROR_FILE_NOT_FOUND)
			return dseed::error_file_not_found;
		else return dseed::error_io;
	}

	*stream = new __native_filestream_win32 (file);
#elif PLATFORM_UWP
	return dseed::error_not_support;
#else
	int fd = create
		? open (path.c_str (), O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO)
		: open (path.c_str (), O_RDWR);
	if (fd == -1)
	{
		if (errno == EACCES)
			return dseed::error_access_denied;
		else if (errno == ENOENT)
			return dseed::error_file_not_found;
		else return dseed::error_io;
	}

	*stream = new __native_filestream_posix (fd);
#endif
	if (*stream == nullptr)
		return dseed::error_out_of_memory;
	return dseed::error_good;
}

#include <sstream>
void path_combine (const char* path1, const char* path2, char* ret, size_t retsize)
{
	std::stringstream ss;
	ss << path1;

	size_t path1len = strlen (path1);

	char lastword = path1[path1len - 1];
	if (!(lastword == '/' || lastword == '\\'))
		ss << PATH_SEPARATOR;

	ss << path2;

	auto retstr = ss.str ();
	memcpy (ret, retstr.c_str (), min (retsize, retstr.length () + 1) * sizeof (char));
}