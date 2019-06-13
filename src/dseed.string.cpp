#include <dseed.h>

#include <locale>
#include <codecvt>

void dseed::utf8toutf16 (const char* u8, char16_t* u16, size_t u16size)
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

void dseed::utf16toutf8 (const char16_t* u16, char* u8, size_t u8size)
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