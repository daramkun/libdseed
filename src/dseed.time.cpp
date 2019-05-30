#include <dseed.h>
#include <algorithm>

constexpr int64_t TICKS_PER_MILLISEC = 10000LL;
constexpr int64_t TICKS_PER_SECOND = (TICKS_PER_MILLISEC * 1000LL);
constexpr int64_t TICKS_PER_MINUTE = (TICKS_PER_SECOND * 60LL);
constexpr int64_t TICKS_PER_HOUR = (TICKS_PER_MINUTE * 60LL);
constexpr int64_t TICKS_PER_DAY = (TICKS_PER_HOUR * 24LL);

constexpr double MILLISECS_PER_TICK = (1.0 / TICKS_PER_MILLISEC);
constexpr double SECONDS_PER_TICK = (1.0 / TICKS_PER_SECOND);
constexpr double MINUTES_PER_TICK = (1.0 / TICKS_PER_MINUTE);
constexpr double HOURS_PER_TICK = (1.0 / TICKS_PER_HOUR);
constexpr double DAYS_PER_TICK = (1.0 / TICKS_PER_DAY);

constexpr int64_t MILLISECS_PER_SECOND = 1000LL;
constexpr int64_t MILLISECS_PER_MINUTE = (MILLISECS_PER_SECOND * 60LL);
constexpr int64_t MILLISECS_PER_HOUR = (MILLISECS_PER_MINUTE * 60LL);
constexpr int64_t MILLISECS_PER_DAY = (MILLISECS_PER_HOUR * 24LL);

#if COMPILER_MSVC
#define min min
#define max max
#endif

dseed::timespan_t dseed::timespan_t::current_ticks () noexcept
{
#if PLATFORM_MICROSOFT
	LARGE_INTEGER freq, counter;
	QueryPerformanceFrequency (&freq);
	QueryPerformanceCounter (&counter);
	return timespan_t::from_seconds (counter.QuadPart / (double)freq.QuadPart);
#else
	struct timespec tspec;
	clock_gettime (CLOCK_MONOTONIC, &tspec);
	return timespan_t ((tspec.tv_sec * 1000000000LL) + tspec.tv_nsec);
#endif
}

dseed::timespan_t::timespan_t (int64_t ticks) noexcept : _ticks (ticks) { }
dseed::timespan_t::timespan_t (int32_t hours, int32_t minutes, int32_t seconds, int32_t millisecs) noexcept
{
	int ms = (hours * 3600 + minutes * 60 + seconds) * MILLISECS_PER_SECOND + millisecs;
	_ticks = ms * TICKS_PER_MILLISEC;
}

int64_t dseed::timespan_t::ticks () const noexcept { return _ticks; }
dseed::timespan_t::operator int64_t() const noexcept { return _ticks; }
int32_t dseed::timespan_t::milliseconds () const noexcept { return (int32_t)(_ticks / TICKS_PER_MILLISEC) % 1000; }
int32_t dseed::timespan_t::seconds () const noexcept { return (int32_t)(_ticks / TICKS_PER_SECOND) % 60; }
int32_t dseed::timespan_t::minutes () const noexcept { return (int32_t)(_ticks / TICKS_PER_MINUTE) % 60; }
int32_t dseed::timespan_t::hours () const noexcept { return (int32_t)(_ticks / TICKS_PER_HOUR) % 24; }
int32_t dseed::timespan_t::days () const noexcept { return (int32_t)(_ticks / TICKS_PER_DAY); }

double dseed::timespan_t::total_milliseconds () const noexcept
{
	return std::max (
		std::min (_ticks * MILLISECS_PER_TICK, INT_MAX / ( double ) TICKS_PER_MILLISEC)
		, INT_MIN / ( double ) TICKS_PER_MILLISEC
	);
}
double dseed::timespan_t::total_seconds () const noexcept { return _ticks * SECONDS_PER_TICK; }
double dseed::timespan_t::total_minutes () const noexcept { return _ticks * MINUTES_PER_TICK; }
double dseed::timespan_t::total_hours () const noexcept { return _ticks * HOURS_PER_TICK; }
double dseed::timespan_t::total_days () const noexcept { return _ticks * DAYS_PER_TICK; }

dseed::timespan_t dseed::timespan_t::from_milliseconds (double millisecs) noexcept { return (int64_t)(millisecs * TICKS_PER_MILLISEC); }
dseed::timespan_t dseed::timespan_t::from_seconds (double seconds) noexcept { return (int64_t)(seconds * MILLISECS_PER_SECOND * TICKS_PER_MILLISEC); }
dseed::timespan_t dseed::timespan_t::from_minutes (double minutes) noexcept { return (int64_t)(minutes * MILLISECS_PER_MINUTE * TICKS_PER_MILLISEC); }
dseed::timespan_t dseed::timespan_t::from_hours (double hours) noexcept { return (int64_t)(hours * MILLISECS_PER_HOUR * TICKS_PER_MILLISEC); }
dseed::timespan_t dseed::timespan_t::from_days (double days) noexcept { return (int64_t)(days * MILLISECS_PER_DAY * TICKS_PER_MILLISEC); }

dseed::frame_skipper::frame_skipper (int32_t framerate)
	: _recommend_framerate (framerate), _framerate_checktime (0), _framerate (0), _calculated_framerate (0)
{
	_last_time = _cur_time = timespan_t::current_ticks ();
}
bool dseed::frame_skipper::is_avaliable () noexcept { return ((_cur_time = timespan_t::current_ticks ()) - _last_time) >= (1 / _recommend_framerate); }
void dseed::frame_skipper::reset_skipper_time () noexcept { _last_time = _cur_time; }
dseed::timespan_t dseed::frame_skipper::delta_time () const noexcept { return _cur_time - _last_time; }
void dseed::frame_skipper::calculate () noexcept
{
	++_framerate;
	_framerate_checktime += delta_time ();
	if (_framerate_checktime >= 1)
	{
		_calculated_framerate = _framerate * _framerate_checktime;
		_framerate_checktime -= timespan_t::from_seconds (1);
		_framerate = 0;
	}
}
dseed::timespan_t dseed::frame_skipper::calced_framerate () const noexcept { return _calculated_framerate; }

dseed::stopwatch::stopwatch () : _startedFrom (0), _isStarted (false) { }
void dseed::stopwatch::start () noexcept { if (_isStarted) return; _startedFrom = timespan_t::current_ticks (); _isStarted = true; }
void dseed::stopwatch::stop () noexcept { _isStarted = false; }
bool dseed::stopwatch::is_started () const noexcept { return _isStarted; }
dseed::timespan_t dseed::stopwatch::elapsed () const noexcept { return timespan_t::current_ticks () - _startedFrom; }