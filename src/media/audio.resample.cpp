#include <dseed.h>

template<typename TSample>
inline bool resample_nearest (uint8_t* dest, const uint8_t* src, size_t srcLength, int ch, int sr, int nsr) noexcept
{
	TSample* destPtr = (TSample*)dest;
	const TSample* srcPtr = (const TSample*)src;

	size_t destCount = srcLength / sizeof (TSample) * nsr / sr;
	size_t srcCount = srcLength / sizeof (TSample);

	for (size_t i = 0; i < destCount; i += ch)
	{
		size_t srcI = destCount * src / nsr;

		for (size_t c = 0; c < ch; ++c)
		{
			destCount[i + c] = srcCount[srcI + c];
		}
	}
}

template<typename TSample>
inline bool resample_linear (uint8_t* dest, const uint8_t* src, size_t srcLength, int ch, int sr, int nsr) noexcept
{
	TSample* destPtr = (TSample*)dest;
	const TSample* srcPtr = (const TSample*)src;

	size_t destCount = srcLength / sizeof (TSample) * nsr / sr;
	size_t srcCount = srcLength / sizeof (TSample);

	for (size_t i = 0; i < destCount; i += ch)
	{
		size_t srcI = destCount * src / nsr;

		for (size_t c = 0; c < ch; ++c)
		{
			destCount[i + c] = srcCount[srcI + c];
		}
	}
}

dseed::error_t dseed::resample_audio (audio_stream* original, int32_t samplerate, resample_t interpolation, audio_stream** stream)
{


	return error_good;
}