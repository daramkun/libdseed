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
#undef min
#undef max
#define min min
#define max max
#endif

dseed::timespan dseed::timespan::current_ticks () noexcept
{
#if PLATFORM_MICROSOFT
	LARGE_INTEGER freq, counter;
	QueryPerformanceFrequency (&freq);
	QueryPerformanceCounter (&counter);
	return timespan::from_seconds (counter.QuadPart / (double)freq.QuadPart);
#else
	struct timespec tspec;
	clock_gettime (CLOCK_MONOTONIC, &tspec);
	return timespan ((tspec.tv_sec * 1000000000LL) + tspec.tv_nsec);
#endif
}

dseed::timespan::timespan (int64_t ticks) noexcept : _ticks (ticks) { }
dseed::timespan::timespan (int32_t hours, int32_t minutes, int32_t seconds, int32_t millisecs) noexcept
{
	int ms = (hours * 3600 + minutes * 60 + seconds) * MILLISECS_PER_SECOND + millisecs;
	_ticks = ms * TICKS_PER_MILLISEC;
}

int64_t dseed::timespan::ticks () const noexcept { return _ticks; }
dseed::timespan::operator int64_t() const noexcept { return _ticks; }
int32_t dseed::timespan::milliseconds () const noexcept { return (int32_t)(_ticks / TICKS_PER_MILLISEC) % 1000; }
int32_t dseed::timespan::seconds () const noexcept { return (int32_t)(_ticks / TICKS_PER_SECOND) % 60; }
int32_t dseed::timespan::minutes () const noexcept { return (int32_t)(_ticks / TICKS_PER_MINUTE) % 60; }
int32_t dseed::timespan::hours () const noexcept { return (int32_t)(_ticks / TICKS_PER_HOUR) % 24; }
int32_t dseed::timespan::days () const noexcept { return (int32_t)(_ticks / TICKS_PER_DAY); }

double dseed::timespan::total_milliseconds () const noexcept
{
	return std::max (
		std::min (_ticks * MILLISECS_PER_TICK, INT_MAX / (double)TICKS_PER_MILLISEC)
		, INT_MIN / (double)TICKS_PER_MILLISEC
	);
}
double dseed::timespan::total_seconds () const noexcept { return _ticks * SECONDS_PER_TICK; }
double dseed::timespan::total_minutes () const noexcept { return _ticks * MINUTES_PER_TICK; }
double dseed::timespan::total_hours () const noexcept { return _ticks * HOURS_PER_TICK; }
double dseed::timespan::total_days () const noexcept { return _ticks * DAYS_PER_TICK; }

dseed::timespan dseed::timespan::from_milliseconds (double millisecs) noexcept { return (int64_t)(millisecs * TICKS_PER_MILLISEC); }
dseed::timespan dseed::timespan::from_seconds (double seconds) noexcept { return (int64_t)(seconds * MILLISECS_PER_SECOND * TICKS_PER_MILLISEC); }
dseed::timespan dseed::timespan::from_minutes (double minutes) noexcept { return (int64_t)(minutes * MILLISECS_PER_MINUTE * TICKS_PER_MILLISEC); }
dseed::timespan dseed::timespan::from_hours (double hours) noexcept { return (int64_t)(hours * MILLISECS_PER_HOUR * TICKS_PER_MILLISEC); }
dseed::timespan dseed::timespan::from_days (double days) noexcept { return (int64_t)(days * MILLISECS_PER_DAY * TICKS_PER_MILLISEC); }

dseed::frameskipper::frameskipper (int32_t framerate)
	: _recommend_framerate (framerate), _framerate_checktime (0), _framerate (0), _calculated_framerate (0)
{
	_last_time = _cur_time = timespan::current_ticks ();
}
bool dseed::frameskipper::is_avaliable () noexcept { return ((_cur_time = timespan::current_ticks ()) - _last_time) >= (1 / _recommend_framerate); }
void dseed::frameskipper::reset_skipper_time () noexcept { _last_time = _cur_time; }
dseed::timespan dseed::frameskipper::delta_time () const noexcept { return _cur_time - _last_time; }
void dseed::frameskipper::calculate () noexcept
{
	++_framerate;
	_framerate_checktime += delta_time ();
	if (_framerate_checktime >= 1)
	{
		_calculated_framerate = _framerate * _framerate_checktime;
		_framerate_checktime -= timespan::from_seconds (1);
		_framerate = 0;
	}
}
dseed::timespan dseed::frameskipper::calced_framerate () const noexcept { return _calculated_framerate; }

dseed::stopwatch::stopwatch () : _startedFrom (0), _isStarted (false) { }
void dseed::stopwatch::start () noexcept { if (_isStarted) return; _startedFrom = timespan::current_ticks (); _isStarted = true; }
void dseed::stopwatch::stop () noexcept { _isStarted = false; }
bool dseed::stopwatch::is_started () const noexcept { return _isStarted; }
dseed::timespan dseed::stopwatch::elapsed () const noexcept { return timespan::current_ticks () - _startedFrom; }

dseed::framemeasurer::framemeasurer () : _elapsed (0), _count (0), _fps (0) { }
void dseed::framemeasurer::update (timespan delta) noexcept
{
	static timespan onesec = timespan::from_seconds (1);

	_elapsed += delta;
	++_count;

	if (_elapsed >= onesec)
	{
		_elapsed -= onesec;
		_fps = _count -(_count * _elapsed.total_seconds ());
		_count = 0;
	}
}
double dseed::framemeasurer::fps () const noexcept { return _fps; }