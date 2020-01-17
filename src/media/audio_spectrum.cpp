#include <dseed.h>

#include <vector>
#include <complex>
#include <cmath>

void __swap (std::complex<double>& a, std::complex<double>& b)
{
	std::complex<double> temp = a;
	a = b;
	b = temp;
}
void __fft (std::complex<double>* complexes, size_t count, bool isInv)
{
	int n = (int)count;
	for (int i = 1, j = 0; i < n; ++i)
	{
		int bit = n >> 1;
		while (!((j ^= bit) & bit))
			bit >>= 1;
		if (i < j)
			__swap (complexes[i], complexes[j]);
	}

	for (int i = 1; i < n; i <<= 1)
	{
		double x = isInv ? dseed::pi / i : -dseed::pi / i;
		std::complex<double> w (cos (x), sin (x));
		for (int j = 0; j < n; j += i << 1)
		{
			std::complex<double> th (1, 0);
			for (int k = 0; k < i; ++k)
			{
				std::complex<double> tmp = complexes[i + j + k] * th;
				complexes[i + j + k] = complexes[j + k] - tmp;
				complexes[j + k] += tmp;
				th *= w;
			}
		}
	}

	if (isInv)
		for (int i = 0; i < n; ++i)
			complexes[i] /= n;
}

int __chConv[] = {
	dseed::media::spectrumchannels_ch1,
	dseed::media::spectrumchannels_ch2,
	dseed::media::spectrumchannels_ch3,
	dseed::media::spectrumchannels_ch4,
	dseed::media::spectrumchannels_ch5,
	dseed::media::spectrumchannels_ch6,
	dseed::media::spectrumchannels_ch7,
	dseed::media::spectrumchannels_ch8,
	dseed::media::spectrumchannels_ch9,
	dseed::media::spectrumchannels_ch10
};

dseed::error_t dseed::media::convert_pcm_to_frequency (const float* pcm, size_t pcmSize, int16_t channels, spectrumchannels ch, float* out)
{
	if (ch == dseed::media::spectrumchannels_none || pcmSize % channels != 0)
		return dseed::error_invalid_args;

	std::vector<std::complex<double>> complexBuffer (pcmSize / channels);
	for (int i = 0; i < pcmSize; i += channels)
	{
		float value = 0;
		int avg = 0;
		for (int ch = 0; ch < channels; ++ch)
		{
			if (ch & __chConv[ch])
			{
				value += pcm[i + ch];
				++avg;
			}
		}
		complexBuffer[i / channels] = std::complex<double> (value / avg, 0);
	}
	__fft (complexBuffer.data (), pcmSize, false);

	for (int i = 0; i < complexBuffer.size (); ++i)
		out[i] = (float)sqrtf (powf (complexBuffer[i].real (), 2) + powf (complexBuffer[i].imag (), 2));

	return dseed::error_good;
}

dseed::error_t dseed::media::convert_pcm_to_decibel (const float* pcm, size_t pcmSize, int16_t channels, spectrumchannels ch, float* out)
{
	if (ch == dseed::media::spectrumchannels_none || pcmSize % channels != 0)
		return dseed::error_invalid_args;

	for (int i = 0; i < pcmSize; i += channels)
	{
		float value = 0;
		int avg = 0;
		for (int ch = 0; ch < channels; ++ch)
		{
			if (ch & __chConv[ch])
			{
				value += pcm[i + ch];
				++avg;
			}
		}
		out[i / channels] = 20 * log10 (fabs (value / avg));
	}

	return dseed::error_good;
}