#ifndef __DSEED_DPARALLEL_H__
#define __DSEED_DPARALLEL_H__

#include <dseed/dcommon.h>
#include <algorithm>
#include <execution>

namespace dseed
{
	class DSEEDEXP threadsyncer : public object
	{
	public:
		virtual void lock_exclusive () = 0;
		virtual void lock_shared () = 0;

	public:
		virtual bool try_lock_exclusive () = 0;
		virtual bool try_lock_shared () = 0;

	public:
		virtual void unlock_exclusive () = 0;
		virtual void unlock_shared () = 0;

	public:
		inline void do_exclusive (std::function<void (void*)> func, void* user_data) noexcept
		{
			lock_exclusive ();
			func (user_data);
			unlock_exclusive ();
		}
		inline void do_shared (std::function<void (void*)> func, void* user_data) noexcept
		{
			lock_shared ();
			func (user_data);
			unlock_shared ();
		}
	};

	// Create Mutex
	DSEEDEXP error_t create_mutex (threadsyncer** ts);
	// Create Spin-Lock
	DSEEDEXP error_t create_spinlock (threadsyncer** ts);

	// Thread-Pool Task Body
	using threadpool_body = void (*)(void*);

	// Thread-Pool
	class DSEEDEXP threadpool : public object
	{
	public:
		virtual void enqueue (threadpool_body body, void* user_data) = 0;

	public:
		virtual void cancel_all () = 0;
	};

	// Create Thread-Pool
	DSEEDEXP error_t create_threadpool (threadpool** pool, size_t maximum_threads = 0);
}

#endif