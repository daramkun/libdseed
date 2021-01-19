#include <dseed.h>

#include <string>
#include <string.h>
#include <locale>
#include <codecvt>

void dseed::utf8_to_utf16(const char* u8, char16_t* u16, size_t u16size) noexcept
{
#if COMPILER_MSVC
	size_t nLen = MultiByteToWideChar(CP_UTF8, 0, u8, (int)strlen(u8), nullptr, 0);
	std::shared_ptr<char16_t[]> ret(new char16_t[nLen + 1]);
	MultiByteToWideChar(CP_UTF8, 0, u8, (int)strlen(u8), (LPWSTR)&ret[0], nLen);
	ret[nLen] = u'\0';
	memcpy(u16, &ret[0], dseed::minimum(u16size, nLen + 1) * sizeof(char16_t));
#else
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	auto ret = converter.from_bytes(u8);
	memcpy(u16, ret.c_str(), dseed::minimum(u16size, ret.length() + 1) * sizeof(char16_t));
#endif
}

void dseed::utf16_to_utf8(const char16_t* u16, char* u8, size_t u8size) noexcept
{
#if COMPILER_MSVC
	size_t nLen = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)u16, (int)wcslen((LPCWSTR)u16), nullptr, 0, nullptr, nullptr);
	std::shared_ptr<char[]> ret(new char[nLen + 1]);
	WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)u16, (int)wcslen((LPCWSTR)u16), &ret[0], nLen, nullptr, nullptr);
	ret[nLen] = '\0';
	memcpy(u8, &ret[0], dseed::minimum(u8size, nLen + 1) * sizeof(char));
#else
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	auto ret = converter.to_bytes(u16);
	memcpy(u8, ret.c_str(), dseed::minimum(u8size, ret.length() + 1) * sizeof(char));
#endif
}

void dseed::utf16_to_wchar(const char16_t* u16, wchar_t* wc, size_t wcsize) noexcept
{
#if PLATFORM_WINDOWS
	size_t length = dseed::minimum(wcsize, std::char_traits<char16_t>::length(u16));
	memcpy(wc, u16, length * sizeof(wchar_t*));
#else
	const char* u16bytes = reinterpret_cast<const char*>(u16);
	size_t length = std::char_traits<char16_t>::length(u16);

	std::wstring_convert<std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>, wchar_t> conv;
	std::wstring ws = conv.from_bytes(u16bytes, u16bytes + length);

	memcpy(wc, ws.c_str(), dseed::minimum(wcsize, ws.size()) * sizeof(wchar_t));
#endif
}

void dseed::wchar_to_utf16(const wchar_t* wc, char16_t* u16, size_t u16size) noexcept
{
#if PLATFORM_WINDOWS
	size_t length = dseed::minimum(u16size, std::char_traits<wchar_t>::length(wc));
	memcpy(u16, wc, length * sizeof(char16_t*));
#else
	const char* u16bytes = reinterpret_cast<const char*>(wc);
	size_t length = std::char_traits<wchar_t>::length(wc);

	std::wstring_convert<std::codecvt_utf16<char16_t, 0x10ffff, std::little_endian>, char16_t> conv;
	std::u16string u16s = conv.from_bytes(u16bytes, u16bytes + length);

	memcpy(u16, u16s.c_str(), dseed::minimum(u16size, u16s.size()) * sizeof(char16_t));
#endif
}

#include <sstream>
void dseed::path_combine(const char* path1, const char* path2, char* ret, size_t retsize) noexcept
{
	std::stringstream ss;

	if (path1 != nullptr)
	{
		size_t path1len = strlen(path1);

		if (path1len > 0)
		{
			ss << path1;

			char lastword = path1[path1len - 1];
			if (!(lastword == '/' || lastword == '\\'))
				ss << PATH_SEPARATOR;
		}
	}

	ss << path2;

	auto retstr = ss.str();
	memcpy(ret, retstr.c_str(), dseed::minimum(retsize, retstr.length() + 1) * sizeof(char));
}
void dseed::path_combine(const char16_t* path1, const char16_t* path2, char16_t* ret, size_t retsize) noexcept
{
#if !COMPILER_CLANG
	std::basic_stringstream<char16_t> ss;

	if (path1 != nullptr)
	{
		size_t path1len = std::char_traits<char16_t>::length(path1);

		if (path1len > 0)
		{
			ss << path1;

			char16_t lastword = path1[path1len - 1];
			if (!(lastword == u'/' || lastword == u'\\'))
				ss << PATH_SEPARATOR;
		}
	}

	ss << path2;

	auto retstr = ss.str();
	memcpy(ret, retstr.c_str(), dseed::minimum(retsize, retstr.length() + 1) * sizeof(char16_t));
#else
	wchar_t path1temp[128], path2temp[128], temp[256];
	dseed::utf16_to_wchar(path1, path1temp, 128);
	dseed::utf16_to_wchar(path2, path2temp, 128);

	dseed::path_combine(path1temp, path2temp, temp, 256);

	dseed::wchar_to_utf16(temp, ret, retsize);
#endif
}

void dseed::path_combine(const wchar_t* path1, const wchar_t* path2, wchar_t* ret, size_t retsize) noexcept
{
	std::wstringstream ss;

	if (path1 != nullptr)
	{
		size_t path1len = std::char_traits<wchar_t>::length(path1);

		if (path1len > 0)
		{
			ss << path1;

			char16_t lastword = path1[path1len - 1];
			if (!(lastword == u'/' || lastword == u'\\'))
				ss << PATH_SEPARATOR;
		}
	}

	ss << path2;

	auto retstr = ss.str();
	memcpy(ret, retstr.c_str(), dseed::minimum(retsize, retstr.length() + 1) * sizeof(wchar_t));
}