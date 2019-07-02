#define _CRT_SECURE_NO_WARNINGS
#include <dseed.h>

#include <cstring>
#include <vector>

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

		memcpy (buffer, ((const uint8_t*)_buffer) + _position, length);
		_position += length;

		return length;
	}
	virtual size_t write (const void* data, size_t length) override
	{
		if (_position + length < _length)
			length = _length - _position;
		if (length == 0)
			return 0;

		memcpy (_buffer, ((const uint8_t*)_buffer) + _position, length);
		_position += length;

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

class __variable_memorystream : public dseed::stream
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
	virtual size_t read (void* buffer, size_t length) override
	{
		if (_position + length < _buffer.size ())
			length = _buffer.size () - _position;
		if (length == 0)
			return 0;

		memcpy (buffer, _buffer.data () + _position, length);
		_position += length;

		return length;
	}
	virtual size_t write (const void* data, size_t length) override
	{
		if (_position + length > _buffer.size ())
			_buffer.resize (_position + length);
		if (length == 0)
			return 0;

		memcpy (_buffer.data () + _position, data, length);
		_position += length;

		return length;
	}
	virtual bool seek (dseed::seekorigin_t origin, size_t offset) override
	{
		switch (origin)
		{
		case dseed::seekorigin_begin: break;
		case dseed::seekorigin_current: offset += _position; break;
		case dseed::seekorigin_end: offset = _buffer.size () - offset; break;
		default: return false;
		}

		if (offset < 0) offset = 0;
		if (offset > _buffer.size ()) _buffer.resize (offset);

		_position = offset;

		return true;
	}

	virtual void flush () override { }
	virtual dseed::error_t set_length (size_t length) override
	{
		seek (dseed::seekorigin_begin, length);
		_buffer.resize (length);
		return dseed::error_good;
	}

public:
	virtual size_t position () override { return _position; }
	virtual size_t length () override { return _buffer.size (); }

public:
	virtual bool readable () override { return true; }
	virtual bool writable () override { return true; }
	virtual bool seekable () override { return true; }

private:
	std::atomic<int32_t> _refCount;
	std::vector<uint8_t> _buffer;
	size_t _position;
};
class __variable_memorystream_remove_after_read : public dseed::stream
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
	virtual size_t read (void* buffer, size_t length) override
	{
		if (length > _length)
			length = _length;
		memcpy (buffer, _buffer.data (), (size_t)length);
		memcpy (_tempBuffer.data (), _buffer.data () + length, (size_t)_length - length);

		_buffer.swap (_tempBuffer);

		_length -= length;
		return length;
	}
	virtual size_t write (const void* data, size_t length) override
	{
		int64_t tempLength = _length;
		set_length (_length + length);
		memcpy (_buffer.data () + tempLength, data, (size_t)length);
		return length;
	}
	virtual bool seek (dseed::seekorigin_t origin, size_t offset) override
	{
		return false;
	}
	virtual void flush () override
	{
	}
	virtual dseed::error_t set_length (size_t length) override
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
	virtual size_t position () override { return 0; }
	virtual size_t length () override { return _length; }

public:
	virtual bool readable () override { return true; }
	virtual bool writable () override { return true; }
	virtual bool seekable () override { return false; }

private:
	std::atomic<int32_t> _refCount;
	std::vector<uint8_t> _buffer, _tempBuffer;
	size_t _length;
};

dseed::error_t dseed::create_variable_memorystream (stream** stream, bool remove_after_read)
{
	if (!remove_after_read) *stream = new __variable_memorystream ();
	else *stream = new __variable_memorystream_remove_after_read ();
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

#if PLATFORM_MICROSOFT
#	include <Shlwapi.h>
#	include <ShlObj.h>
#	include <atlconv.h>
#	pragma comment ( lib, "Shlwapi.lib" )
#endif
#include <filesystem>

std::string ____GetDocumentsDirectory ()
{
#if PLATFORM_WINDOWS
	wchar_t path[MAX_PATH];
	HRESULT result = SHGetFolderPathW (NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path);

	if (PathCombineW (path, path, L"My Games") == nullptr)
		return nullptr;

	wchar_t filename[1024];
	wchar_t* shortname;
	GetModuleFileNameW (GetModuleHandle (nullptr), filename, 1024);
	shortname = PathFindFileNameW (filename);
	if (PathCombineW (path, path, shortname) == nullptr)
		return std::string ("");

	USES_CONVERSION;
	return std::string (W2A (path));
#elif PLATFORM_UWP
	wchar_t path[MAX_PATH];
	wcscat_s (path, MAX_PATH, Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data ());
	USES_CONVERSION;
	return W2A (path);
#elif PLATFORM_APPLE
	return [[[[ NSFileManager defaultManager ]URLsForDirectory:NSDocumentDirectory inDomains : NSUserDomainMask] lastObject] .absoluteString UTF8String] ;
#elif PLATFORM_ANDROID

#elif PLATFORM_UNIX

#else
	return "";
#endif
}

std::string ____GetAssetsDirectory ()
{
#if PLATFORM_WINDOWS
	wchar_t temp[MAX_PATH];
	GetModuleFileNameW (GetModuleHandle (nullptr), temp, MAX_PATH);
	PathRemoveFileSpecW (temp);
	USES_CONVERSION;
	return std::string (W2A (temp));
#elif PLATFORM_UWP
	wchar_t path[MAX_PATH + 1] = { 0, };
	wcscat_s (path, MAX_PATH, Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data ());
	if (path[wcslen (path) - 1] != KB_PATH_SEPARATOR)
	{
		size_t len = wcslen (path);
		path[len] = KB_PATH_SEPARATOR;
		path[len + 1] = '\0';
	}
	//wcscat_s ( path, MAX_PATH, L"Assets\\" );
	USES_CONVERSION;
	return W2A (path);
#elif PLATFORM_APPLE
	CFBundleRef mainBundle = CFBundleGetMainBundle ();
	CFURLRef resourceURL = CFBundleCopyResourcesDirectoryURL (mainBundle);
	char path[4096];
	if (!CFURLGetFileSystemRepresentation (resourceURL, true, (UInt8*)path, 4096)) {
		CFRelease (resourceURL);
		return nullptr;
	}
	CFRelease (resourceURL);

	return path;
#elif PLATFORM_ANDROID

#elif PLATFORM_UNIX

#else
	return "";
#endif
}

std::string ____GetTemporaryDirectory ()
{
#if PLATFORM_WINDOWS
	wchar_t path[MAX_PATH];
	GetTempPathW (MAX_PATH, path);
	USES_CONVERSION;
	return std::string (W2A (path));
#elif PLATFORM_UWP
	wchar_t path[MAX_PATH + 1];
	wcscat_s (path, MAX_PATH, Windows::Storage::ApplicationData::Current->TemporaryFolder->Path->Data ());
	USES_CONVERSION;
	return W2A (path);
#elif PLATFORM_MACOS
	return getenv ("TMPDIR");
#elif PLATFORM_IOS
	return[NSTemporaryDirectory () UTF8String];
#elif PLATFORM_ANDROID

#elif PLATFORM_UNIX
	return return pathCombine (getenv ("TMPDIR"), getprogname ());
#else
	return "";
#endif
}

class __nativefilesystem : public dseed::filesystem
{
public:
	__nativefilesystem (dseed::nativefilesystem_t fst)
		: _refCount (1), _fst (fst)
	{
		switch (fst)
		{
		case dseed::nativefilesystem_documents:
			_baseDir = ____GetDocumentsDirectory ();
			break;
		case dseed::nativefilesystem_assets:
			_baseDir = ____GetAssetsDirectory ();
			break;
		case dseed::nativefilesystem_temporary:
			_baseDir = ____GetTemporaryDirectory ();
			break;
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
	virtual dseed::error_t create_directory (const char* path) override
	{
		if (_fst == dseed::nativefilesystem_assets)
			return dseed::error_invalid_op;

		std::string fullPath = dseed::path_combine (_baseDir, path);
		return std::filesystem::create_directory (fullPath) ? dseed::error_good : dseed::error_fail;
	}
	virtual dseed::error_t create_file (const char* path, bool create, dseed::stream** stream) override
	{
		if (_fst == dseed::nativefilesystem_assets && create)
			return dseed::error_invalid_op;
		if (!create && !file_exists (path))
			return dseed::error_file_not_found;

		std::string fullPath = dseed::path_combine (_baseDir, path);

		dseed::create_native_filestream (fullPath.c_str (), create, stream);

		if (*stream == nullptr)
			return dseed::error_out_of_memory;

		return dseed::error_good;
	}
	virtual dseed::error_t delete_file (const char* path) override
	{
		if (_fst == dseed::nativefilesystem_assets)
			return dseed::error_invalid_op;

		std::string fullPath = dseed::path_combine (_baseDir, path);
		return std::filesystem::remove (fullPath) ? dseed::error_good : dseed::error_fail;
	}

	virtual bool file_exists (const char* path) override
	{
		std::string fullPath = dseed::path_combine (_baseDir, path);
		return std::filesystem::exists (fullPath)
			&& !std::filesystem::is_directory (fullPath);
	}
	virtual bool directory_exists (const char* path) override
	{
		std::string fullPath = dseed::path_combine (_baseDir, path);
		return std::filesystem::is_directory (fullPath);
	}

private:
	std::atomic<int32_t> _refCount;
	dseed::nativefilesystem_t _fst;
	std::string _baseDir;
};

dseed::error_t dseed::create_native_filesystem (nativefilesystem_t fst, filesystem** fileSystem)
{
	*fileSystem = new __nativefilesystem (fst);
	if (*fileSystem == nullptr)
		return dseed::error_out_of_memory;
	return dseed::error_good;
}

class __memblob : public dseed::blob
{
public:
	__memblob (size_t length)
		: _refCount (1), _memLength (length), _mem (length)
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
	virtual void* blob_pointer () override { return _mem.data (); }
	virtual size_t blob_length () override { return _memLength; }

private:
	std::atomic<int32_t> _refCount;
	std::vector<uint8_t> _mem;
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
	__bufferblob (void* buffer, size_t length)
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

dseed::error_t dseed::create_bufferblob (void* buffer, size_t length, bool copy, dseed::blob** blob)
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

dseed::error_t dseed::create_streamblob (stream* stream, blob** blob)
{
	dseed::auto_object<dseed::blob> tempBlob;
	if (dseed::failed (dseed::create_memoryblob (stream->length (), &tempBlob)))
		return dseed::error_fail;

	if (stream->read (tempBlob->blob_pointer (), tempBlob->blob_length ()) != tempBlob->blob_length ())
		return dseed::error_io;

	*blob = tempBlob.detach ();

	return error_good;
}