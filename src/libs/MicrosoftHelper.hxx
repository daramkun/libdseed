#pragma once

static dseed::media::audioformat waveFormatFromWaveFormatEX (const WAVEFORMATEX* pwfx)
{
	dseed::media::audioformat wf = {};
	wf.channels = (uint8_t)pwfx->nChannels;
	wf.bits_per_sample = (uint8_t)pwfx->wBitsPerSample;
	wf.samples_per_sec = pwfx->nSamplesPerSec;
	wf.block_align = pwfx->nBlockAlign;
	wf.bytes_per_sec = pwfx->nAvgBytesPerSec;
	if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
	{
		WAVEFORMATEXTENSIBLE* extensible = (WAVEFORMATEXTENSIBLE*)pwfx;
		if (extensible->SubFormat == KSDATAFORMAT_SUBTYPE_PCM)
			wf.pulse_format = dseed::media::pulseformat::pcm;
		else if (extensible->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
			wf.pulse_format = dseed::media::pulseformat::ieee_float;
		else
			wf.pulse_format = dseed::media::pulseformat::unknown;
	}
	else
	{
		switch (pwfx->wFormatTag)
		{
			case WAVE_FORMAT_PCM:
				wf.pulse_format = dseed::media::pulseformat::pcm;
				break;
			case WAVE_FORMAT_IEEE_FLOAT:
				wf.pulse_format = dseed::media::pulseformat::ieee_float;
				break;
			default:
				wf.pulse_format = dseed::media::pulseformat::unknown;
				break;
		}
	}
	return wf;
}

static WAVEFORMATEX* waveFormatExFromWaveFormat (const dseed::media::audioformat* waveFormat)
{
	WAVEFORMATEX* pwfx = (WAVEFORMATEX*)CoTaskMemAlloc (sizeof (WAVEFORMATEX));
	pwfx->cbSize = sizeof (WAVEFORMATEX);
	pwfx->nChannels = waveFormat->channels;
	pwfx->wBitsPerSample = waveFormat->bits_per_sample;
	pwfx->nSamplesPerSec = waveFormat->samples_per_sec;
	pwfx->nBlockAlign = waveFormat->block_align;
	pwfx->nAvgBytesPerSec = waveFormat->bytes_per_sec;
	switch (waveFormat->pulse_format)
	{
		case dseed::media::pulseformat::unknown: pwfx->wFormatTag = 0; break;
		case dseed::media::pulseformat::pcm: pwfx->wFormatTag = WAVE_FORMAT_PCM; break;
		case dseed::media::pulseformat::ieee_float: pwfx->wFormatTag = WAVE_FORMAT_IEEE_FLOAT; break;
	}
	return pwfx;
}