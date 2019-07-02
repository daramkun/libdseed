#include <dseed.h>

#include <locale>
#include <codecvt>

void dseed::utf8_to_utf16 (const char* u8, char16_t* u16, size_t u16size)
{
#if COMPILER_MSVC
	int nLen = MultiByteToWideChar (CP_UTF8, 0, u8, (int)strlen (u8), nullptr, 0);
	std::shared_ptr<char16_t[]> ret (new char16_t[nLen + 1]);
	MultiByteToWideChar (CP_UTF8, 0, u8, (int)strlen (u8), (LPWSTR)&ret[0], nLen);
	ret[nLen] = u'\0';
	memcpy (u16, &ret[0], min (u16size, nLen + 1) * sizeof (char16_t));
#else
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	auto ret = converter.from_bytes (u8);
	memcpy (u16, ret.c_str (), min (u16size, ret.length () + 1) * sizeof(char16_t));
#endif
}

void dseed::utf16_to_utf8 (const char16_t* u16, char* u8, size_t u8size)
{
#if COMPILER_MSVC
	int nLen = WideCharToMultiByte (CP_UTF8, 0, (LPCWSTR) u16, (int)lstrlenW ((LPCWSTR) u16), nullptr, 0, nullptr, nullptr);
	std::shared_ptr<char[]> ret (new char[nLen + 1]);
	WideCharToMultiByte (CP_UTF8, 0, (LPCWSTR)u16, (int)lstrlenW ((LPCWSTR)u16), &ret[0], nLen, nullptr, nullptr);
	ret[nLen] = '\0';
	memcpy (u8, &ret[0], min (u8size, nLen + 1) * sizeof (char));
#else
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	auto ret = converter.to_bytes (u16);
	memcpy (u8, ret.c_str (), min (u8size, ret.length () + 1) * sizeof (char));
#endif
}

#include <sstream>
void dseed::path_combine (const char* path1, const char* path2, char* ret, size_t retsize)
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
void dseed::path_combine (const char16_t* path1, const char16_t* path2, char16_t* ret, size_t retsize)
{
	std::basic_stringstream<char16_t> ss;
	ss << path1;

	size_t path1len = std::char_traits<char16_t>::length (path1);

	char lastword = path1[path1len - 1];
	if (!(lastword == '/' || lastword == '\\'))
		ss << PATH_SEPARATOR;

	ss << path2;

	auto retstr = ss.str ();
	memcpy (ret, retstr.c_str (), min (retsize, retstr.length () + 1) * sizeof (char));
}