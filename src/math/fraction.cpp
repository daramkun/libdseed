#include <dseed.h>

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