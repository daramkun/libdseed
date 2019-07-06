#include <dseed.h>

dseed::timespan_t dseed::get_audio_duration (const dseed::audioformat& wf, size_t bytes)
{
	return dseed::timespan_t::from_seconds (bytes / (double)wf.bytes_per_sec);
}

size_t dseed::get_audio_bytes_count (const dseed::audioformat& wf, dseed::timespan_t duration)
{
	return (size_t)(duration.total_seconds () * wf.bytes_per_sec);
}