#include <dseed.h>

#include <locale>
#include <codecvt>

std::u16string dseed::utf8toutf16 (const std::string& u8)
{
#if COMPILER_MSVC
	int nLen = MultiByteToWideChar (CP_UTF8, 0, u8.c_str (), (int)strlen (u8.c_str ()), nullptr, 0);
	std::shared_ptr<char16_t[]> ret (new char16_t[nLen]);
	MultiByteToWideChar (CP_UTF8, 0, u8.c_str (), (int)strlen (u8.c_str ()), (LPWSTR)&ret[0], nLen);
	return &ret[0];
#else
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	return converter.from_bytes (u8);
#endif
}

std::string dseed::utf16toutf8 (const std::u16string& u16)
{
#if COMPILER_MSVC
	int nLen = WideCharToMultiByte (CP_UTF8, 0, (LPCWSTR) u16.c_str (), (int)lstrlenW ((LPCWSTR) u16.c_str ()), nullptr, 0, nullptr, nullptr);
	std::shared_ptr<char[]> ret (new char[nLen + 1]);
	WideCharToMultiByte (CP_UTF8, 0, (LPCWSTR)u16.c_str (), (int)lstrlenW ((LPCWSTR)u16.c_str ()), &ret[0], nLen, nullptr, nullptr);
	return &ret[0];
#else
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	return converter.to_bytes (u16);
#endif
}