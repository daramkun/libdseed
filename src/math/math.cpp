#include <dseed.h>

#include <string.h>

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

#if !(ARCH_ARMSET && (COMPILER_GCC || COMPILER_CLANG))
dseed::half::half (float v)
{
#if ARCH_X86SET
	if (x86_instruction_info::instance ().f16c)
	{
		auto f16 = _mm_cvtps_ph (_mm_set_ss (v), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
		uint16_t temp[8];
		_mm_store_si128 ((__m128i*)temp, f16);
		word = temp[0];
	}
	else
	{
#endif
		if (fabsf (v) <= single_epsilon)
		{
			word = 0;
			return;
		}

		uint32_t& i = *((uint32_t*)& v);

		int sign = (i >> 16) & 0x8000;
		int exp = ((i >> 23) & 0xff) - (0x7f - 0x0f);
		int frac = i & 0x007fffff;

		if (exp < 31)
		{
			word = 0x7e00;
			return;
		}
		else if (exp <= 0)
		{
			word = sign;
			return;
		}

		word = sign | (exp << 10) | frac;
#if ARCH_X86SET
	}
#endif
}
dseed::half::operator float () const
{
#if ARCH_X86SET
	if (x86_instruction_info::instance ().f16c)
	{
		float ret;
		_mm_store_ss (&ret, _mm_cvtph_ps (_mm_set1_epi16 (word)));
		return ret;
	}
	else
	{
#endif
		int sign = (word >> 15) & 0x1;
		int exp = (word >> 10) & 0x1f;
		int frac = word & 0x3ff;

		if (exp == 0)
		{
			if (frac)
			{
				exp = 0x70;
				frac <<= 1;
				while ((frac & 0x0400) == 0)
				{
					frac <<= 1;
					exp -= 1;
				}
				frac &= 0x3ff;
				frac <<= 13;
			}
		}
		else if (exp == 0x1f)
		{
			exp = 0xff;
			if (frac != 0)
				frac = (frac << 13) | 0x1fff;
		}
		else
		{
			exp += 0x70;
			frac <<= 13;
		}

		uint32_t ret = (sign << 31) | (exp << 23) | frac;

		return *((float*)& ret);
#if ARCH_X86SET
	}
#endif
}
#endif

dseed::fraction::fraction (int32_t numerator) noexcept
	: numerator (numerator), denominator (1)
{ }
dseed::fraction::fraction (int32_t numerator, int32_t denominator) noexcept
	: numerator (numerator), denominator (denominator)
{ }
dseed::fraction::fraction (double fp) noexcept
{
	if (equals (fp, 0))
	{
		numerator = denominator = 0;
	}
	else
	{
		int num = (int)(fp * 1000),
			denom = 1000,
			fpgcd = gcd (num, denom);
		num /= fpgcd;
		denom /= fpgcd;

		numerator = num;
		denominator = denom;
	}
}
dseed::fraction::operator double () noexcept
{
	if (denominator == 0)
		return 0;
	return numerator / (double)denominator;
}