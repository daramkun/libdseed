#include <dseed.h>

#include <thread>
#include <iomanip>
#include <string.h>

#if PLATFORM_MICROSOFT
#	include <atlconv.h>
#endif

std::string ____assembleLogMessage(dseed::diagnostics::loglevel level, const char* tag, const char* message, bool includeCarrageReturn)
{
	auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::tm localtime = *std::localtime(&now);

	const char* levelStr;
	switch (level)
	{
	case dseed::diagnostics::loglevel::debug_only: levelStr = "DEBUG"; break;
	case dseed::diagnostics::loglevel::information: levelStr = "INFOR"; break;
	case dseed::diagnostics::loglevel::warning: levelStr = "WARNN"; break;
	case dseed::diagnostics::loglevel::error: levelStr = "ERROR"; break;
	case dseed::diagnostics::loglevel::fatal: levelStr = "FATAL"; break;
	default: levelStr = "UNKNN"; break;
	}

#if PLATFORM_MICROSOFT
	const char* carrageReturn = "\r\n";
#else
	const char* carrageReturn = "\n";
#endif

	char assemble[25600];
	sprintf(assemble,
#if ARCH_AMD64 || ARCH_ARM64
		"[%02d-%02d-%02d %02d:%02d:%02d][%s][0x%08llx][%s]%s%s",
#else
		"[%02d-%02d-%02d %02d:%02d:%02d][%s][0x%08x][%s]%s%s",
#endif
		1900 + localtime.tm_year, localtime.tm_mon + 1, localtime.tm_mday, localtime.tm_hour, localtime.tm_min, localtime.tm_sec,
		levelStr, std::hash<std::thread::id>{}(std::this_thread::get_id()), tag, message, includeCarrageReturn ? carrageReturn : "");

	return assemble;
}

class stream_log_writer : public dseed::diagnostics::logwriter
{
public:
	stream_log_writer(dseed::io::stream* stream)
		: _refCount(1), _stream(stream)
	{ }

public:
	virtual int32_t retain() override { return ++_refCount; }
	virtual int32_t release() override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual void write(dseed::diagnostics::loglevel level, const char* tag, const char* message) override
	{
		auto log = ____assembleLogMessage(level, tag, message, true);
#if PLATFORM_MICROSOFT
		USES_CONVERSION;
		LPCWSTR tempString = A2W(log.c_str());
		const char* text = W2A_CP(tempString, CP_UTF8);
#else
		const char* text = log.c_str();
#endif

		_stream->write(text, strlen(text));
	}

private:
	std::atomic<int32_t> _refCount;
	dseed::autoref<dseed::io::stream> _stream;
};

dseed::error_t dseed::diagnostics::create_stream_logwriter(dseed::io::stream* stream, dseed::diagnostics::logwriter** writer)
{
	if (stream == nullptr || writer == nullptr)
		return dseed::error_invalid_args;
	if (!stream->writable())
		return dseed::error_invalid_args;

	*writer = new stream_log_writer(stream);
	if (*writer == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}

class debug_log_writer : public dseed::diagnostics::logwriter
{
public:
	debug_log_writer()
		: _refCount(1)
	{

	}

public:
	virtual int32_t retain() override { return ++_refCount; }
	virtual int32_t release() override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual void write(dseed::diagnostics::loglevel level, const char* tag, const char* message) override
	{
#if _DEBUG || defined(DEBUG)
#	if PLATFORM_MICROSOFT || PLATFORM_UNIX
		auto log = ____assembleLogMessage(level, tag, message, true);
#		if PLATFORM_MICROSOFT
		USES_CONVERSION;
		OutputDebugStringW(A2W(log.c_str()));
#		else
		std::cerr << ss.str();
#		endif
#	endif
#endif
	}

private:
	std::atomic<int32_t> _refCount;
};

class internal_logger : public dseed::diagnostics::logger
{
public:
	internal_logger()
		: _minPrintLevel(dseed::diagnostics::loglevel::debug_only)
		, _maxPrintLevel(dseed::diagnostics::loglevel::fatal)
	{
		dseed::autoref<dseed::diagnostics::logwriter> debugLogWriter;
		*&debugLogWriter = new debug_log_writer();
		register_writer(debugLogWriter);
	}
	~internal_logger()
	{
		for (dseed::diagnostics::logwriter* writer : _writers)
			writer->release();
		_writers.clear();
	}

public:
	virtual void register_writer(dseed::diagnostics::logwriter* writer) override
	{
		if (std::find(_writers.begin(), _writers.end(), writer) == _writers.end())
		{
			_writers.push_back(writer);
			writer->retain();
		}
	}
	virtual void unregister_writer(dseed::diagnostics::logwriter* writer) override
	{
		auto found = std::find(_writers.begin(), _writers.end(), writer);
		if (found != _writers.end())
		{
			(*found)->release();
			_writers.erase(found);
		}
	}

public:
	virtual void set_minimum_print_level(dseed::diagnostics::loglevel logLevel) override
	{
		if (logLevel > dseed::diagnostics::loglevel::debug_only)
			return;
		_minPrintLevel = logLevel;
	}
	virtual void set_maximum_print_level(dseed::diagnostics::loglevel logLevel) override
	{
		if (logLevel > dseed::diagnostics::loglevel::fatal)
			return;
		_maxPrintLevel = logLevel;
	}

public:
	virtual void write(dseed::diagnostics::loglevel level, const char* tag, const char* message) override
	{
		if (level > _maxPrintLevel || level > dseed::diagnostics::loglevel::fatal)
			return;
		if (level < _minPrintLevel || level < dseed::diagnostics::loglevel::debug_only)
			return;

		for (auto writer : _writers)
			writer->write(level, tag, message);
	}

private:
	std::vector<dseed::diagnostics::logwriter*> _writers;
	dseed::diagnostics::loglevel _minPrintLevel, _maxPrintLevel;
};

dseed::diagnostics::logger* dseed::diagnostics::logger::shared_logger()
{
	static internal_logger logger;
	return &logger;
}