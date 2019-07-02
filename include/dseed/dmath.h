#ifndef __DSEED_DMATH_H__
#define __DSEED_DMATH_H__

#include <dseed/dcommon.h>

namespace dseed
{
	constexpr int8_t int8_max = CHAR_MAX;
	constexpr int8_t int8_min = CHAR_MIN;
	constexpr uint8_t uint8_max = UCHAR_MAX;
	constexpr uint8_t uint8_min = 0;

	constexpr int16_t int16_max = SHRT_MAX;
	constexpr int16_t int16_min = SHRT_MIN;
	constexpr uint16_t uint16_max = USHRT_MAX;
	constexpr uint16_t uint16_min = 0;

	constexpr int32_t int32_max = INT_MAX;
	constexpr int32_t int32_min = INT_MIN;
	constexpr uint32_t uint32_max = UINT_MAX;
	constexpr uint32_t uint32_min = 0;

	constexpr int64_t int64_max = LLONG_MAX;
	constexpr int64_t int64_min = LLONG_MIN;
	constexpr uint64_t uint64_max = ULLONG_MAX;
	constexpr uint64_t uint64_min = 0;

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
	struct DSEEDEXP uint24_t
	{
		uint8_t value[3];
		uint24_t () = default;
		uint24_t (uint32_t v) noexcept;
		operator uint32_t () const noexcept;
	};
#if COMPILER_MSVC
#	pragma pack (pop)
#else
#	pragma pack ()
#endif

	constexpr int32_t int24_max = 8388607;
	constexpr int32_t int24_min = -8388608;
	constexpr uint32_t uint24_max = 16777215;
	constexpr uint32_t uint24_min = 0;

	using single = float;

	constexpr single single_positive_max = FLT_MAX;
	constexpr single single_positive_min = FLT_MIN;
	constexpr single single_negative_max = -FLT_MAX;
	constexpr single single_negative_min = -FLT_MIN;
	constexpr single single_positive_infinity = INFINITY;
	constexpr single single_negative_infinity = -INFINITY;
	constexpr single single_epsilon = FLT_EPSILON;
	constexpr single single_nan = NAN;

	constexpr double double_positive_max = DBL_MAX;
	constexpr double double_positive_min = DBL_MIN;
	constexpr double double_negative_max = -DBL_MAX;
	constexpr double double_negative_min = -DBL_MIN;
	constexpr double double_positive_infinity = HUGE_VAL;
	constexpr double double_negative_infinity = -HUGE_VAL;
	constexpr double double_epsilon = DBL_EPSILON;
	constexpr double double_nan = HUGE_VAL * 0.0;

#if ARCH_ARMSET && (COMPILER_GCC || COMPILER_CLANG)
	using half = __fp16;
#else
#	if COMPILER_MSVC
#		pragma pack (push, 1)
#	else
#		pragma pack (1)
#	endif
	struct DSEEDEXP half
	{
		uint16_t word;
		half () = default;
		half (float v);
		operator float () const;
	};
#	if COMPILER_MSVC
#		pragma pack (pop)
#	else
#		pragma pack ()
#	endif

	constexpr uint16_t half_positive_max = 0x7bff;
	constexpr uint16_t half_positive_min = 0x0001;
	constexpr uint16_t half_negative_max = 0xfbff;
	constexpr uint16_t half_negative_min = 0x8001;
	constexpr uint16_t half_positive_infinity = 0x7c00;
	constexpr uint16_t half_negative_infinity = 0xfc00;
	constexpr uint16_t half_epsilon = 0x1400;
	constexpr uint16_t half_nan = 0x7e00;
#endif

	constexpr single pi = 3.14159265358979323846f;
	constexpr single pi2 = pi * 2;
	constexpr single piover2 = pi / 2;
	constexpr single piover4 = pi / 4;
	constexpr single e = 2.7182818285f;
	constexpr single log10e = 0.4342944819f;
	constexpr single log2e = 1.4426950409f;

	constexpr bool equals (single s1, single s2) noexcept { return (s1 - s2) < single_epsilon; }
	constexpr bool equals (double d1, double d2) noexcept { return (d1 - d2) < double_epsilon; }
	template<typename T>
	inline T minimum (const T& v1, const T& v2) noexcept { return (v1 < v2) ? v1 : v2; }
	template<typename T>
	inline T maximum (const T& v1, const T& v2) noexcept { return (v1 < v2) ? v2 : v1; }

	constexpr int gcd (int a, int b) { return (b == 0) ? a : gcd (b, a % b); }

	struct DSEEDEXP fraction
	{
		int32_t numerator, denominator;

		fraction () = default;
		fraction (int32_t numerator) noexcept;
		fraction (int32_t numerator, int32_t denominator) noexcept;
		fraction (double fp) noexcept;

		operator double () noexcept;
	};
}

#include <dseed/dmath.simd.inl>
#include <dseed/dmath.vector.inl>
#include <dseed/dmath.collision.inl>

#endif