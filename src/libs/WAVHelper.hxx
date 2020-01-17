#ifndef __WAVHELPER_HXX__
#define __WAVHELPER_HXX__

#include <cstdint>

struct RIFF_HEADER
{
	uint8_t riff[4];
	uint32_t total_file_size;
	RIFF_HEADER () { riff[0] = 'R'; riff[1] = 'I'; riff[2] = 'F'; riff[3] = 'F'; total_file_size = 0; }
};

struct WAVE_HEADER
{
	uint8_t wave[4];
	WAVE_HEADER () { wave[0] = 'W'; wave[1] = 'A'; wave[2] = 'V'; wave[3] = 'E'; }
};

struct fmt_HEADER
{
	uint8_t fmt[4];
	uint32_t fmtSize;
	uint16_t audio_type;
	uint16_t audio_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
	fmt_HEADER ()
	{
		fmt[0] = 'f'; fmt[1] = 'm'; fmt[2] = 't'; fmt[3] = ' ';
		fmtSize = 16;
		audio_type = audio_channels = block_align = bits_per_sample = 0;
		sample_rate = byte_rate = 0;
	}
};

struct data_HEADER
{
	uint8_t data[4];
	uint32_t dataSize;
	data_HEADER () { data[0] = 'd'; data[1] = 'a'; data[2] = 't'; data[3] = 'a'; dataSize = 0; }
};

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM										1
#endif

#ifndef WAVE_FORMAT_IEEE_FLOAT
#define WAVE_FORMAT_IEEE_FLOAT								3
#endif

#endif