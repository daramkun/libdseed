#define _CRT_SECURE_NO_WARNINGS
#include <dseed.h>

class __memorystream : public dseed::io::stream
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
	virtual size_t read (void* buffer, size_t length) noexcept override
	{
		if (_position + length < _length)
			length = _length - _position;
		if (length == 0)
			return 0;

		memcpy (buffer, ((const uint8_t*)_buffer) + _position, length);
		_position += length;

		return length;
	}
	virtual size_t write (const void* data, size_t length) noexcept override
	{
		if (_position + length < _length)
			length = _length - _position;
		if (length == 0)
			return 0;

		memcpy (_buffer, ((const uint8_t*)_buffer) + _position, length);
		_position += length;

		return length;
	}
	virtual bool seek (dseed::io::seekorigin origin, size_t offset) noexcept override
	{
		switch (origin)
		{
		case dseed::io::seekorigin::begin: break;
		case dseed::io::seekorigin::current: offset += _position; break;
		case dseed::io::seekorigin::end: offset = _length - offset; break;
		default: return false;
		}

		if (offset < 0) offset = 0;
		if (offset > _length) offset = _length;

		_position = offset;

		return true;
	}

	virtual void flush () noexcept override { }
	virtual dseed::error_t set_length (size_t length) noexcept override
	{
		seek (dseed::io::seekorigin::begin, length);
		_length = length;
		return dseed::error_good;
	}

public:
	virtual size_t position () noexcept override { return _position; }
	virtual size_t length () noexcept override { return _length; }

public:
	virtual bool readable () noexcept override { return true; }
	virtual bool writable () noexcept override { return true; }
	virtual bool seekable () noexcept override { return true; }

private:
	std::atomic<int32_t> _refCount;
	void* _buffer;
	size_t _length;

	size_t _position;
};

dseed::error_t dseed::io::create_fixed_memorystream (void* buffer, size_t length, dseed::io::stream** stream)
{
	if (buffer == nullptr || length <= 0 || stream == nullptr)
		return dseed::error_invalid_args;
	*stream = new __memorystream (buffer, length);
	if (*stream == nullptr)
		return dseed::error_out_of_memory;
	return dseed::error_good;
}

class __variable_memorystream : public dseed::io::stream
{
public:
	__variable_memorystream ()
		: _refCount (1), _position (0)
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
	virtual size_t read (void* buffer, size_t length) noexcept override
	{
		if (_position + length < _buffer.size ())
			length = _buffer.size () - _position;
		if (length == 0)
			return 0;

		memcpy (buffer, _buffer.data () + _position, length);
		_position += length;

		return length;
	}
	virtual size_t write (const void* data, size_t length) noexcept override
	{
		if (_position + length > _buffer.size ())
			_buffer.resize (_position + length);
		if (length == 0)
			return 0;

		memcpy (_buffer.data () + _position, data, length);
		_position += length;

		return length;
	}
	virtual bool seek (dseed::io::seekorigin origin, size_t offset) noexcept override
	{
		switch (origin)
		{
		case dseed::io::seekorigin::begin: break;
		case dseed::io::seekorigin::current: offset += _position; break;
		case dseed::io::seekorigin::end: offset = _buffer.size () - offset; break;
		default: return false;
		}

		if (offset < 0) offset = 0;
		if (offset > _buffer.size ()) _buffer.resize (offset);

		_position = offset;

		return true;
	}

	virtual void flush () noexcept override { }
	virtual dseed::error_t set_length (size_t length) noexcept override
	{
		seek (dseed::io::seekorigin::begin, length);
		_buffer.resize (length);
		return dseed::error_good;
	}

public:
	virtual size_t position () noexcept override { return _position; }
	virtual size_t length () noexcept override { return _buffer.size (); }

public:
	virtual bool readable () noexcept override { return true; }
	virtual bool writable () noexcept override { return true; }
	virtual bool seekable () noexcept override { return true; }

private:
	std::atomic<int32_t> _refCount;
	std::vector<uint8_t> _buffer;
	size_t _position;
};

class __variable_memorystream_remove_after_read : public dseed::io::stream
{
public:
	__variable_memorystream_remove_after_read ()
		: _refCount (1), _length (0)
	{
		_buffer.resize (48000 * 2 * 2);
		_tempBuffer.resize (48000 * 2 * 2);
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
		if (length > _length)
			length = _length;
		memcpy (buffer, _buffer.data (), (size_t)length);
		memcpy (_tempBuffer.data (), _buffer.data () + length, (size_t)_length - length);

		_buffer.swap (_tempBuffer);

		_length -= length;
		return length;
	}
	virtual size_t write (const void* data, size_t length) noexcept override
	{
		int64_t tempLength = _length;
		set_length (_length + length);
		memcpy (_buffer.data () + tempLength, data, (size_t)length);
		return length;
	}
	virtual bool seek (dseed::io::seekorigin origin, size_t offset) noexcept override
	{
		return false;
	}
	virtual void flush () noexcept override
	{
	}
	virtual dseed::error_t set_length (size_t length) noexcept override
	{
		if (_buffer.size () < length)
		{
			int64_t tempLength = _length;
			_buffer.resize ((length + 24) / 8 * 8);
			_tempBuffer.resize (_buffer.size ());
		}
		_length = length;
		return dseed::error_good;
	}
	virtual size_t position () noexcept override { return 0; }
	virtual size_t length () noexcept override { return _length; }

public:
	virtual bool readable () noexcept override { return true; }
	virtual bool writable () noexcept override { return true; }
	virtual bool seekable () noexcept override { return false; }

private:
	std::atomic<int32_t> _refCount;
	std::vector<uint8_t> _buffer, _tempBuffer;
	size_t _length;
};

dseed::error_t dseed::io::create_variable_memorystream (dseed::io::stream** stream, bool remove_after_read)
{
	if (!remove_after_read) *stream = new __variable_memorystream ();
	else *stream = new __variable_memorystream_remove_after_read ();
	if (*stream == nullptr)
		return dseed::error_out_of_memory;
	return dseed::error_good;
}

#if PLATFORM_WINDOWS
class __native_filestream_win32 : public dseed::io::stream
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
	virtual size_t read (void* buffer, size_t length) noexcept override
	{
		DWORD read;
		if (!ReadFile (_file, buffer, (DWORD)length, &read, nullptr))
			return -1;
		return read;
	}
	virtual size_t write (const void* data, size_t length) noexcept override
	{
		DWORD written;
		if (!WriteFile (_file, data, (DWORD)length, &written, nullptr))
			return -1;
		return written;
	}
	virtual bool seek (dseed::io::seekorigin origin, size_t offset) noexcept override
	{
		if (SetFilePointer (_file, (LONG)offset, nullptr, (int)origin) == INVALID_SET_FILE_POINTER)
			return false;
		return true;
	}
	virtual void flush () noexcept override
	{
		FlushFileBuffers (_file);
	}
	virtual dseed::error_t set_length (size_t length) noexcept override
	{
		SetFilePointer (_file, (LONG)length, nullptr, FILE_BEGIN);
		SetEndOfFile (_file);
		return dseed::error_good;
	}

public:
	virtual size_t position () noexcept override
	{
		return SetFilePointer (_file, 0, nullptr, FILE_CURRENT);
	}
	virtual size_t length () noexcept override
	{
		return GetFileSize (_file, nullptr);
	}

public:
	virtual bool readable () noexcept override { return true; }
	virtual bool writable () noexcept override { return true; }
	virtual bool seekable () noexcept override { return true; }

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
class __native_filestream_posix : public dseed::io::stream
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
	virtual size_t read (void* buffer, size_t length) noexcept override
	{
		return ::read (_fd, buffer, length);
	}
	virtual size_t write (const void* data, size_t length) noexcept override
	{
		return ::write (_fd, data, length);
	}
	virtual bool seek (dseed::io::seekorigin origin, size_t offset) noexcept override
	{
		return !lseek (_fd, offset, origin);
	}
	virtual void flush () noexcept override
	{
		fsync (_fd);
	}
	virtual dseed::error_t set_length (size_t length) noexcept override
	{
		ftruncate (_fd, length);
		return dseed::error_good;
	}

public:
	virtual size_t position () noexcept override
	{
		return lseek (_fd, 0, SEEK_CUR);
	}
	virtual size_t length () noexcept override
	{
		struct stat s;
		fstat (_fd, &s);
		return s.st_size;
	}

public:
	virtual bool readable () noexcept override { return true; }
	virtual bool writable () noexcept override { return true; }
	virtual bool seekable () noexcept override { return true; }

private:
	std::atomic<int32_t> _refCount;
	int _fd;
};
#endif

dseed::error_t dseed::io::create_native_filestream (const char* path, bool create, dseed::io::stream** stream)
{
	if (stream == nullptr)
		return dseed::error_invalid_args;
#if PLATFORM_WINDOWS
	char16_t filename[256];
	utf8_to_utf16 (path, filename, 256);
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
		? open (path, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO)
		: open (path, O_RDWR);
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