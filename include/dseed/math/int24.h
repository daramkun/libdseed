#ifndef __DSEED_MATH_INT24_H__
#define __DSEED_MATH_INT24_H__

namespace dseed
{
#if COMPILER_MSVC
#	pragma pack (push, 1)
#else
#	pragma pack (1)
#endif
	struct DSEEDEXP int24_t
	{
		uint8_t value[3];
		int24_t () = default;
		int24_t (int32_t v) noexcept;
		operator int32_t () const noexcept;
	};
	inline bool operator== (const int24_t& a, const int24_t& b) noexcept { return memcmp (&a, &b, sizeof (int24_t)) == 0; }
	inline bool operator!= (const int24_t& a, const int24_t& b) noexcept { return memcmp (&a, &b, sizeof (int24_t)) != 0; }

	struct DSEEDEXP uint24_t
	{
		uint8_t value[3];
		uint24_t () = default;
		uint24_t (uint32_t v) noexcept;
		operator uint32_t () const noexcept;
	};
	inline bool operator== (const uint24_t& a, const uint24_t& b) noexcept { return memcmp (&a, &b, sizeof (uint24_t)) == 0; }
	inline bool operator!= (const uint24_t& a, const uint24_t& b) noexcept { return memcmp (&a, &b, sizeof (uint24_t)) != 0; }
#if COMPILER_MSVC
#	pragma pack (pop)
#else
#	pragma pack ()
#endif

	constexpr int32_t int24_max = 8388607;
	constexpr int32_t int24_min = -8388608;
	constexpr uint32_t uint24_max = 16777215;
	constexpr uint32_t uint24_min = 0;
}

#endif