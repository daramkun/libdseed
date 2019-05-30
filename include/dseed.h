#ifndef __DSEED_H__
#define __DSEED_H__

#include <atomic>
#include <string>
#include <cstdint>
#include <climits>
#include <cfloat>
#include <cmath>
using namespace std::string_literals;

#define COMPILER_MSVC										defined ( _MSC_VER )
#define COMPILER_GCC										defined ( __GNUC__ ) && !defined ( __clang__ )
#define COMPILER_CLANG										defined ( __clang__ )

#define PLATFORM_ANDROID									( defined ( __ANDROID__ ) )
#define PLATFORM_FUCHSIA									( defined ( __Fuchsia__ ) )
#define PLATFORM_GOOGLE										( PLATFORM_ANDROID || PLATFORM_FUCHSIA )
#define PLATFORM_UNIX										( defined ( __unix__ ) || defined ( __linux__ ) ) && !defined ( __ANDROID__ )
#define PLATFORM_WEBASSEMBLY								( defined ( __EMSCRIPTEN__ ) || defined ( __asmjs__ ) )
#if defined ( __APPLE__ )
#	include <TargetConditionals.h>
#	define PLATFORM_IOS										( TARGET_OS_IOS || TARGET_OS_SIMULATOR )
#	define PLATFORM_MACOS									( TARGET_OS_MAC && !PLATFORM_IOS )
#else
#	define PLATFORM_IOS										0
#	define PLATFORM_MACOS									0
#endif
#define PLATFORM_APPLE										( PLATFORM_IOS || PLATFORM_MACOS )
#if ( defined ( _WINDOWS ) || defined ( _WIN32 ) || defined ( _WIN64 ) ) \
	&& !( PLATFORM_APPLE || PLATFORM_GOOGLE || PLATFORM_UNIX || PLATFORM_WEBASSEMBLY )
#	include <Windows.h>
#	define PLATFORM_WINDOWS									( WINAPI_FAMILY_PARTITION ( WINAPI_PARTITION_DESKTOP ) || !defined ( WINAPI_FAMILY_DESKTOP_APP ) )
#	define PLATFORM_UWP										( WINAPI_FAMILY_PARTITION ( WINAPI_FAMILY_PC_APP ) && !PLATFORM_WINDOWS )
#	if PLATFORM_UWP
#		include <wrl.h>
#	endif
#else
#	define PLATFORM_WINDOWS									0
#	define PLATFORM_UWP										0
#endif
#define PLATFORM_MICROSOFT									( PLATFORM_WINDOWS || PLATFORM_UWP )

#define PLATFORM_DESKTOP									( PLATFORM_WINDOWS || PLATFORM_MACOS || PLATFORM_UNIX )
#define PLATFORM_HANDHELD									( PLATFORM_IOS || PLATFORM_ANDROID )
#define PLATFORM_UNIVERSAL									( PLATFORM_UWP || PLATFORM_FUCHSIA )

#if PLATFORM_MICROSOFT
constexpr char PATH_SEPARATOR = '\\';
#else
constexpr char PATH_SEPARATOR = '/';
#endif

#define ARCH_IA32											( defined ( _M_IX86 ) || defined ( __i386__ ) )
#define ARCH_AMD64											( defined ( _M_AMD64 ) || defined ( __amd64__ ) )
#define ARCH_X86SET											( ARCH_IA32 || ARCH_AMD64 )
#define ARCH_ARM											( defined ( _M_ARM ) || defined ( __arm__ ) )
#define ARCH_ARM64											( defined ( _M_ARM64 ) || defined ( __aarch64__ ) )
#define ARCH_ARMSET											( ARCH_ARM || ARCH_ARM64 )

#if COMPILER_MSVC && defined ( DSEED_WINDOWS_DLL_EXPORT )
#   define DSEEDEXP											__declspec ( dllexport )
#elif COMPILER_MSVC && !defined ( DSEED_WINDOWS_STATIC )
#   define DSEEDEXP											__declspec ( dllimport )
#else
#   define DSEEDEXP
#endif

namespace dseed
{
    using error_t = int32_t;
	constexpr bool succeeded (error_t err) { return err >= 0; }
	constexpr bool failed (error_t err) { return err < 0; }

	constexpr error_t error_good = 0;

	constexpr error_t error_fail = 0x80000000;
	constexpr error_t error_version_mismatch = 0x80000001;

	constexpr error_t error_invalid_args = 0x80010001;
	constexpr error_t error_invalid_op = 0x80010002;
	constexpr error_t error_out_of_range = 0x80010003;
	constexpr error_t error_out_of_memory = 0x80010004;

	constexpr error_t error_io = 0x80020001;
	constexpr error_t error_file_not_found = 0x80020002;
	constexpr error_t error_end_of_file = 0x80020003;
	constexpr error_t error_hash_not_correct = 0x80020004;
	constexpr error_t error_access_denied = 0x80020005;

	constexpr error_t error_not_impl = 0x80030001;
	constexpr error_t error_not_support = 0x80030002;

	constexpr error_t error_device_not_conn = 0x80040001;
	constexpr error_t error_device_disconn = 0x80040002;
	constexpr error_t error_device_reset = 0x80040003;

	constexpr error_t error_network_not_conn = 0x80050001;
	constexpr error_t error_network_disconn = 0x80060002;
	constexpr error_t error_network_sync_fail = 0x80060003;

    class DSEEDEXP object
    {
    public:
        virtual ~object ();

    public:
        virtual int32_t retain () = 0;
        virtual int32_t release () = 0;
    };

	class DSEEDEXP wrapped_object : public object
	{
	public:
		virtual error_t native_object (void** nativeObject) = 0;
	};

	template<typename T>
	struct auto_object
	{
	public:
		inline auto_object () : auto_object (nullptr) { }
		inline auto_object (T* obj)
			: _ptr (obj)
		{
			static_assert (std::is_base_of<object, T>::value, "Not supported type.");

			if (_ptr)
				_ptr->retain ();
		}
		inline auto_object (const auto_object<T>& obj)
			: auto_object (obj.get ())
		{

		}
		inline ~auto_object () { release (); }

	public:
		inline void store (T* obj)
		{
			release ();
			if (_ptr = obj)
				_ptr->retain ();
		}
		inline void release ()
		{
			if (_ptr)
			{
				_ptr->release ();
				_ptr = nullptr;
			}
		}

		inline void attach (T* obj)
		{
			release ();
			_ptr = obj;
		}
		inline T* detach ()
		{
			T* ret = dynamic_cast<T*> (_ptr);
			_ptr = nullptr;
			return ret;
		}
		inline T* get () const { return dynamic_cast<T*> (_ptr); }

	public:
		inline T** operator & () { return reinterpret_cast<T * *> (&_ptr); }
		inline T* operator -> () const { return dynamic_cast<T*> (_ptr); }
		inline operator T* () const { return dynamic_cast<T*> (_ptr); }
		inline explicit operator object* () const { return _ptr; }
		inline auto_object<T>& operator = (T* obj)
		{
			store (obj);
			return *this;
		}

	private:
		object* _ptr;
	};
}

namespace dseed
{
	constexpr int8_t int8_max = CHAR_MAX;
	constexpr int8_t int8_min = CHAR_MIN;
	constexpr uint8_t uint8_max = UCHAR_MAX;
	constexpr uint8_t uint8_min = 0;

	constexpr int16_t int16_max = SHRT_MAX;
	constexpr int16_t int16_min = SHRT_MIN;
	constexpr uint16_t uint16_max = USHRT_MAX;
	constexpr uint16_t uint16_min = 0;

	constexpr int32_t int32_max = INT_MAX;
	constexpr int32_t int32_min = INT_MIN;
	constexpr uint32_t uint32_max = UINT_MAX;
	constexpr uint32_t uint32_min = 0;

	constexpr int64_t int64_max = LLONG_MAX;
	constexpr int64_t int64_min = LLONG_MIN;
	constexpr uint64_t uint64_max = ULLONG_MAX;
	constexpr uint64_t uint64_min = 0;

	using single = float;

	constexpr single single_max = FLT_MAX;
	constexpr single single_min = FLT_MIN;
	constexpr single single_positive_infinite = INFINITY;
	constexpr single single_negative_infinite = -INFINITY;
	constexpr single single_epsilon = FLT_EPSILON;
	constexpr single single_nan = NAN;

	constexpr double double_max = DBL_MAX;
	constexpr double double_min = DBL_MIN;
	constexpr double double_positive_infinite = HUGE_VAL;
	constexpr double double_negative_infinite = -HUGE_VAL;
	constexpr double double_epsilon = DBL_EPSILON;
	constexpr double double_nan = HUGE_VAL * 0.0;

	constexpr single pi = 3.14159265358979323846f;
	constexpr single pi2 = pi * 2;
	constexpr single piover2 = pi / 2;
	constexpr single piover4 = pi / 4;
	constexpr single e = 2.7182818285f;
	constexpr single log10e = 0.4342944819f;
	constexpr single log2e = 1.4426950409f;

	constexpr bool equals (single s1, single s2) noexcept { return (s1 - s2) < single_epsilon; }
	constexpr bool equals (double d1, double d2) noexcept { return (d1 - d2) < double_epsilon; }

	constexpr int gcd (int a, int b)
	{
		if (b == 0)
			return a;
		return gcd (b, a % b);
	}

	struct DSEEDEXP fraction
	{
		int32_t numerator, denominator;

		fraction () = default;
		fraction (int32_t numerator);
		fraction (int32_t numerator, int32_t denominator);
		fraction (double fp);

		operator double () { return numerator / (double)denominator; }
	};
}

namespace dseed
{
	struct DSEEDEXP point2i
	{
	public:
		int32_t x, y;

	public:
		point2i () = default;
		point2i (int32_t x, int32_t y);
	};

	struct DSEEDEXP point2f
	{
	public:
		float x, y;

	public:
		point2f () = default;
		point2f (float x, float y);
	};

	struct DSEEDEXP size2i
	{
	public:
		int32_t width, height;

	public:
		size2i () = default;
		size2i (int32_t width, int32_t height);
	};

	struct DSEEDEXP size2f
	{
	public:
		float width, height;

	public:
		size2f () = default;
		size2f (float width, float height);
	};

	struct DSEEDEXP point3i
	{
	public:
		int32_t x, y, z;

	public:
		point3i () = default;
		point3i (int32_t x, int32_t y, int32_t z);
	};

	struct DSEEDEXP point3f
	{
	public:
		float x, y, z;

	public:
		point3f () = default;
		point3f (float x, float y, float z);
	};

	struct DSEEDEXP size3i
	{
	public:
		int32_t width, height, depth;

	public:
		size3i () = default;
		size3i (int32_t width, int32_t height, int32_t depth);
	};

	struct DSEEDEXP size3f
	{
	public:
		float width, height, depth;

	public:
		size3f () = default;
		size3f (float width, float height, float depth);
	};

	struct DSEEDEXP rectangle
	{
	public:
		int32_t x, y, width, height;

	public:
		rectangle () = default;
		rectangle (int32_t x, int32_t y, int32_t width, int32_t height);
		rectangle (const point2i& p, const size2i& s);

	public:
		bool intersect (const rectangle& rect);
	};

	struct DSEEDEXP rectanglef
	{
	public:
		float x, y, width, height;

	public:
		rectanglef () = default;
		rectanglef (float x, float y, float width, float height);
		rectanglef (const point2f& p, const size2f& s);

	public:
		bool intersect (const rectanglef& rect);
	};

	struct DSEEDEXP circle
	{
	public:
		int32_t x, y, radius;

	public:
		circle () = default;
		circle (int32_t x, int32_t y, int32_t radius);

	public:
		bool intersect (const circle& circle);
	};

	struct DSEEDEXP circlef
	{
	public:
		float x, y, radius;

	public:
		circlef () = default;
		circlef (float x, float y, float radius);

	public:
		bool intersect (const circlef& circle);
	};
}

namespace dseed
{
	struct DSEEDEXP timespan_t
	{
	public:
		static timespan_t current_ticks () noexcept;

	public:
		timespan_t (int64_t ticks = 0) noexcept;
		timespan_t (int32_t hours, int32_t minutes, int32_t seconds, int32_t millisecs) noexcept;

	public:
		int64_t ticks () const noexcept;
		operator int64_t () const noexcept;
		int32_t milliseconds () const noexcept;
		int32_t seconds () const noexcept;
		int32_t minutes () const noexcept;
		int32_t hours () const noexcept;
		int32_t days () const noexcept;

	public:
		double total_milliseconds () const noexcept;
		double total_seconds () const noexcept;
		double total_minutes () const noexcept;
		double total_hours () const noexcept;
		double total_days () const noexcept;

	public:
		static timespan_t from_milliseconds (double millisecs) noexcept;
		static timespan_t from_seconds (double seconds) noexcept;
		static timespan_t from_minutes (double minutes) noexcept;
		static timespan_t from_hours (double hours) noexcept;
		static timespan_t from_days (double days) noexcept;

	public:
		timespan_t operator+ (const timespan_t& t) const { return _ticks + t._ticks; }
		timespan_t operator- (const timespan_t& t) const { return _ticks - t._ticks; }
		timespan_t operator+= (const timespan_t& t) { return _ticks += t._ticks; }
		timespan_t operator-= (const timespan_t& t) { return _ticks -= t._ticks; }

	private:
		int64_t _ticks;
	};

	struct DSEEDEXP frame_skipper
	{
	public:
		frame_skipper (int32_t framerate = 60);

	public:
		bool is_avaliable () noexcept;
		void reset_skipper_time () noexcept;
		timespan_t delta_time () const noexcept;

	public:
		void calculate () noexcept;
		timespan_t calced_framerate () const noexcept;

	public:
		timespan_t _last_time, _cur_time, _recommend_framerate, _framerate_checktime, _calculated_framerate;
		int32_t _framerate;
	};

	struct DSEEDEXP stopwatch
	{
	public:
		stopwatch ();

	public:
		void start () noexcept;
		void stop () noexcept;

	public:
		bool is_started () const noexcept;
		timespan_t elapsed () const noexcept;

	public:
		timespan_t _startedFrom;
		bool _isStarted;
	};
}

namespace dseed
{
	std::u16string utf8toutf16 (const std::string& u8);
	std::string utf16toutf8 (const std::u16string& u16);
}

namespace dseed
{
	enum DSEEDEXP seekorigin_t : int32_t
	{
		seekorigin_begin,
		seekorigin_current,
		seekorigin_end,
	};

	class DSEEDEXP stream : public object
	{
	public:
		virtual size_t read (void* buffer, size_t length) = 0;
		virtual size_t write (const void* data, size_t length) = 0;
		virtual bool seek (seekorigin_t origin, size_t offset) = 0;
		virtual void flush () = 0;
		virtual error_t set_length (size_t length) = 0;

	public:
		virtual size_t position () = 0;
		virtual size_t length () = 0;

	public:
		virtual bool readable () = 0;
		virtual bool writable () = 0;
		virtual bool seekable () = 0;
	};

	DSEEDEXP error_t create_memorystream (void* buffer, size_t length, stream** stream);
	DSEEDEXP error_t create_native_filestream (const std::string& path, bool create, stream** stream);

	std::string path_combine (const std::string& path1, const std::string& path2);
}

#endif