#include <dseed.h>

void __set_coefficient (dseed::quadfilter& f, double aa0, double aa1, double aa2, double b0, double b1, double b2)
{
	double invaa0 = 1 / aa0;
	f.a0 = b0 * invaa0;
	f.a1 = b1 * invaa0;
	f.a2 = b2 * invaa0;
	f.a3 = aa1 * invaa0;
	f.a4 = aa2 * invaa0;
}

float dseed::quadfilter::process (float input, int ch)
{
	double result = a0 * input + a1 * x1[ch] + a2 * x2[ch] - a3 * y1[ch] - a4 * y2[ch];
	x2[ch] = x1[ch];
	x1[ch] = input;

	y2[ch] = y1[ch];
	y1[ch] = result;

	return (float)result;
}

dseed::quadfilter dseed::quadfilter::high_pass_filter (int samplerate, double freq, double q)
{
	dseed::quadfilter f = {};

	double w0 = dseed::pi2 * freq / samplerate;
	double cosw0 = cos (w0);
	double alpha = sin (w0) / (2 * q);

	double b0 = (1 + cosw0) / 2;
	double b1 = -(1 + cosw0);
	double b2 = (1 + cosw0) / 2;
	double aa0 = 1 + alpha;
	double aa1 = -2 * cosw0;
	double aa2 = 1 - alpha;

	__set_coefficient (f, aa0, aa1, aa2, b0, b1, b2);

	return f;
}

dseed::quadfilter dseed::quadfilter::high_shelf_filter (int samplerate, double freq, float shelfSlope, double gaindb)
{
	dseed::quadfilter f = {};

	double w0 = dseed::pi2 * freq / samplerate;
	double cosw0 = cos (w0);
	double sinw0 = sin (w0);
	double a = pow (10, gaindb / 40);
	double alpha = sinw0 / 2 * sqrt ((a + 1 / a) * (1 / shelfSlope - 1) + 2);
	double temp = 2 * sqrt (a) * alpha;

	double b0 = a * ((a + 1) + (a - 1) * cosw0 + temp);
	double b1 = -2 * a * ((a - 1) + (a + 1) * cosw0);
	double b2 = a * ((a + 1) + (a - 1) * cosw0 - temp);
	double aa0 = (a + 1) - (a - 1) * cosw0 + temp;
	double aa1 = 2 * ((a - 1) - (a + 1) * cosw0);
	double aa2 = (a + 1) - (a - 1) * cosw0 - temp;

	__set_coefficient (f, aa0, aa1, aa2, b0, b1, b2);

	return f;
}

dseed::quadfilter dseed::quadfilter::low_pass_filter (int samplerate, double freq, double q)
{
	dseed::quadfilter f = {};

	double w0 = dseed::pi2 * freq / samplerate;
	double cosw0 = cos (w0);
	double alpha = sin (w0) / (2 * q);

	double b0 = (1 - cosw0) / 2;
	double b1 = 1 - cosw0;
	double b2 = (1 - cosw0) / 2;
	double aa0 = 1 + alpha;
	double aa1 = -2 * cosw0;
	double aa2 = 1 - alpha;

	__set_coefficient (f, aa0, aa1, aa2, b0, b1, b2);

	return f;
}

dseed::quadfilter dseed::quadfilter::low_shelf_filter (int samplerate, double freq, float shelfSlope, double gaindb)
{
	dseed::quadfilter f = {};

	double w0 = dseed::pi2 * freq / samplerate;
	double cosw0 = cos (w0);
	double sinw0 = sin (w0);
	double a = pow (10, gaindb / 40);
	double alpha = sinw0 / 2 * sqrt ((a + 1 / a) * (1 / shelfSlope - 1) + 2);
	double temp = 2 * sqrt (a) * alpha;

	double b0 = a * ((a + 1) - (a - 1) * cosw0 + temp);
	double b1 = 2 * a * ((a - 1) - (a + 1) * cosw0);
	double b2 = a * ((a + 1) - (a - 1) * cosw0 - temp);
	double aa0 = (a + 1) + (a - 1) * cosw0 + temp;
	double aa1 = -2 * ((a - 1) + (a + 1) * cosw0);
	double aa2 = (a + 1) + (a - 1) * cosw0 - temp;

	__set_coefficient (f, aa0, aa1, aa2, b0, b1, b2);

	return f;
}

dseed::quadfilter dseed::quadfilter::all_pass_filter (int samplerate, double freq, double q)
{
	dseed::quadfilter f = {};

	double w0 = dseed::pi2 * freq / samplerate;
	double cosw0 = cos (w0);
	double sinw0 = sin (w0);
	double alpha = sinw0 / (2 * q);

	double b0 = 1 - alpha;
	double b1 = -2 * cosw0;
	double b2 = 1 + alpha;
	double aa0 = 1 + alpha;
	double aa1 = -2 * cosw0;
	double aa2 = 1 - alpha;

	__set_coefficient (f, aa0, aa1, aa2, b0, b1, b2);

	return f;
}

dseed::quadfilter dseed::quadfilter::notch_filter (int samplerate, double freq, double q)
{
	dseed::quadfilter f = {};

	double w0 = dseed::pi2 * freq / samplerate;
	double cosw0 = cos (w0);
	double sinw0 = sin (w0);
	double alpha = sinw0 / (2 * q);

	double b0 = 1;
	double b1 = -2 * cosw0;
	double b2 = 1;
	double aa0 = 1 + alpha;
	double aa1 = -2 * cosw0;
	double aa2 = 1 - alpha;

	__set_coefficient (f, aa0, aa1, aa2, b0, b1, b2);

	return f;
}

dseed::quadfilter dseed::quadfilter::peak_eq_filter (int samplerate, double freq, double bandwidth, double peakGainDB)
{
	dseed::quadfilter f = {};

	double w0 = dseed::pi2 * freq / samplerate;
	double cosw0 = cos (w0);
	double sinw0 = sin (w0);
	double alpha = sinw0 / (2 * bandwidth);
	double a = pow (10, peakGainDB / 40);

	double b0 = 1 + alpha * a;
	double b1 = -2 * cosw0;
	double b2 = 1 - alpha * a;
	double aa0 = 1 + alpha / a;
	double aa1 = -2 * cosw0;
	double aa2 = 1 - alpha / a;

	__set_coefficient (f, aa0, aa1, aa2, b0, b1, b2);

	return f;
}

dseed::quadfilter dseed::quadfilter::band_pass_filter_csg (int samplerate, double freq, double q)
{
	dseed::quadfilter f = {};

	double w0 = dseed::pi2 * freq / samplerate;
	double cosw0 = cos (w0);
	double sinw0 = sin (w0);
	double alpha = sinw0 / (2 * q);

	double b0 = sinw0 / 2;
	double b1 = 0;
	double b2 = -sinw0 / 2;
	double aa0 = 1 + alpha;
	double aa1 = -2 * cosw0;
	double aa2 = 1 - alpha;

	__set_coefficient (f, aa0, aa1, aa2, b0, b1, b2);

	return f;
}

dseed::quadfilter dseed::quadfilter::band_pass_filter_cpg (int samplerate, double freq, double q)
{
	dseed::quadfilter f = {};

	double w0 = dseed::pi2 * freq / samplerate;
	double cosw0 = cos (w0);
	double sinw0 = sin (w0);
	double alpha = sinw0 / (2 * q);

	double b0 = 1 - alpha;
	double b1 = -2 * cosw0;
	double b2 = 1 + alpha;
	double aa0 = 1 + alpha;
	double aa1 = -2 * cosw0;
	double aa2 = 1 - alpha;

	__set_coefficient (f, aa0, aa1, aa2, b0, b1, b2);

	return f;
}

int __equalizerFrequencies[10] = { 32, 64, 125, 250, 500, 1000, 2000, 4000, 8000, 16000 };
double __equalizerPresetGainsDatabase[][10] = {
	{ +0, +0, +0, +0, +0, +0, +0, +0, +0, +0 },
	{ +5, +5, +4, +1, +2, +2, +4, +5, +4, +3 },
	{ +4, +7, +5, +1, +2, +3, +5, +4, +3, +1 },
	{ +4, +3, +2, +3, -2, -2, +0, +2, +3, +4 },
	{ +4, +3, +2, +0, -2, +3, +2, +2, +4, +5 },
	{ -2, -1, +0, +2, +3, +3, +2, +1, -1, -2 },
	{ +4, +3, +2, +1, +0, +0, +1, +2, +3, +4 },
	{ +4, +3, +1, +2, +0, +0, +2, +0, +2, +3 },
	{ +5, +4, +3, +3, -1, -1, +0, +2, +3, +4 },
	{ +3, +2, +0, +3, +3, +2, +3, +4, +3, +4 },
};

dseed::error_t dseed::initialize_equalizer_filter (uint32_t samplerate, double bandwidth, eqpreset_t preset, quadfilter* filters, size_t filtersCount)
{
	if (filters == nullptr || filtersCount != 10) return dseed::error_invalid_args;

	for (int i = 0; i < filtersCount; ++i)
		filters[i] = dseed::quadfilter::peak_eq_filter (samplerate, __equalizerFrequencies[i], bandwidth, __equalizerPresetGainsDatabase[preset][i]);

	return dseed::error_good;
}

dseed::error_t dseed::initialize_equalizer_filter_with_gaindbs (uint32_t samplerate, double bandwidth,
	quadfilter* filters, double* gainDbs, size_t filtersAndGainDbsCount)
{
	if (filters == nullptr || filtersAndGainDbsCount != 10) return dseed::error_invalid_args;

	for (int i = 0; i < filtersAndGainDbsCount; ++i)
		filters[i] = dseed::quadfilter::peak_eq_filter (samplerate, __equalizerFrequencies[i], bandwidth, gainDbs[i]);

	return dseed::error_good;
}