#include <dseed.h>

dseed::fraction::fraction (int32_t numerator)
	: numerator (numerator), denominator (1)
{ }
dseed::fraction::fraction (int32_t numerator, int32_t denominator)
	: numerator (numerator), denominator (denominator)
{ }
dseed::fraction::fraction (double fp)
{
	int num = (int)(fp * 1000),
		denom = 1000,
		fpgcd = gcd (num, denom);
	num /= fpgcd;
	denom /= fpgcd;

	numerator = num;
	denominator = denom;
}