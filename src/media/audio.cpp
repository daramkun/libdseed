#include <dseed.h>

dseed::timespan dseed::media::get_audio_duration (const audioformat& wf, size_t bytes)
{
	return dseed::timespan::from_seconds (bytes / (double)wf.bytes_per_sec);
}

size_t dseed::media::get_audio_bytes_count (const audioformat& wf, timespan duration)
{
	return (size_t)(duration.total_seconds () * wf.bytes_per_sec);
}

size_t dseed::media::get_audio_bytes_count (uint32_t bytes_per_sec, timespan duration)
{
	return (size_t)(duration.total_seconds () * bytes_per_sec);
}