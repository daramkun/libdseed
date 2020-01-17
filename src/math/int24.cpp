#include <dseed.h>

#include <cstring>

inline dseed::int24_t::int24_t (int32_t v) noexcept { memcpy (value, &v, 3); }
inline dseed::int24_t::operator int32_t() const noexcept
{
	return ((value[2] & 0x80) << 24) >> 7
		| (value[2] << 16) | (value[1] << 8) | value[0];
}

inline dseed::uint24_t::uint24_t (uint32_t v) noexcept { memcpy (value, &v, 3); }
inline dseed::uint24_t::operator uint32_t() const noexcept
{
	return (value[2] << 16) | (value[1] << 8) | value[0];
}