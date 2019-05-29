#include <dseed.h>

#include <locale>
#include <codecvt>

std::u16string dseed::utf8toutf16 (const std::string& u8)
{
#if COMPILER_MSVC
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	auto temp = converter.from_bytes (u8);
	return std::u16string (temp.begin (), temp.end ());
#else
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	return converter.from_bytes (u8);
#endif
}

std::string dseed::utf16toutf8 (const std::u16string& u16)
{
#if COMPILER_MSVC
	std::wstring temp (u16.begin (), u16.end ());
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes (temp);
#else
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	return converter.to_bytes (u16);
#endif
}