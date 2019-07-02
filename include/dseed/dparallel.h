#ifndef __DSEED_DPARALLEL_H__
#define __DSEED_DPARALLEL_H__

#include <dseed/dcommon.h>

#include <algorithm>
#include <execution>

#include <thread>
#include <mutex>
#include <shared_mutex>

namespace dseed
{
	// Exclusive Mutex
	using mutex = std::mutex;
	// Exclusive/Shared Mutex
	using rwmutex = std::shared_timed_mutex;

	// Exclusive/Shared Spin-Lock
	struct spinlock
	{
	public:
		spinlock () : _sharedCount (0), _exclusiveFlag (false) { }

	public:
		void lock ()
		{
			bool expected = false;
			while (!_exclusiveFlag.compare_exchange_weak (expected, true, std::memory_order_acquire))
				std::this_thread::yield ();

			while (_sharedCount.load (std::memory_order_seq_cst) != 0)
				std::this_thread::yield ();
		}
		void lock_shared ()
		{
			while (true) {
				while (_exclusiveFlag.load (std::memory_order_seq_cst))
					std::this_thread::yield ();

				_sharedCount.fetch_add (1, std::memory_order_acquire);

				if (_exclusiveFlag.load (std::memory_order_seq_cst))
					_sharedCount.fetch_sub (1, std::memory_order_release);
				else
					break;
			}
		}

	public:
		bool try_lock ()
		{
			bool expected = false;
			if (!_exclusiveFlag.load (std::memory_order_seq_cst) && _sharedCount.load (std::memory_order_seq_cst) == 0)
				return _exclusiveFlag.compare_exchange_weak (expected, true, std::memory_order_acquire);
			return false;
		}

		bool try_lock_shared ()
		{
			if (!_exclusiveFlag.load (std::memory_order_seq_cst))
			{
				_sharedCount.fetch_add (1, std::memory_order_acquire);
				if (_exclusiveFlag.load (std::memory_order_seq_cst))
				{
					_sharedCount.fetch_sub (1, std::memory_order_release);
					return false;
				}
				return true;
			}
			return false;
		}

	public:
		void unlock () { _exclusiveFlag.store (false, std::memory_order_release); }
		void unlock_shared () { _sharedCount.fetch_sub (1, std::memory_order_release); }

	private:
		std::atomic<uint64_t> _sharedCount;
		std::atomic<bool> _exclusiveFlag;
	};

	// Thread-Pool
	template<class TLock = mutex>
	struct threadpool
	{
	public:
		threadpool (size_t maximum_threads = 0)
			: _stop (false)
		{
			_workers.reserve (maximum_threads);
			for (size_t i = 0; i < maximum_threads; ++i)
			{
				_workers.emplace_back ([this] {
					while (true)
					{
						std::function<void ()> task;
						{
							std::unique_lock<TLock> lock (this->_mutex);

							this->_condition.wait (lock, [this] { return this->_stop || !this->_tasks.empty (); });
							if (this->_stop || this->_tasks.empty ())
								break;

							task = std::move (this->_tasks.front ());
							this->_tasks.pop ();
						}
						task ();
					}
					});
			}
		}
		~threadpool ()
		{
			{
				std::unique_lock<TLock> lock (_mutex);
				_stop = true;
			}
			_condition.notify_all ();
			for (std::thread& worker : _workers)
				worker.join ();
		}

	public:
		template<class F, class... Args>
		auto enqueue (F&& f, Args&& ... args)
		{
			using return_type = typename std::result_of<F (Args...)>::type;

			auto task = std::make_shared< std::packaged_task<return_type ()> > (
				std::bind (std::forward<F> (f), std::forward<Args> (args)...)
				);

			std::future<return_type> res = task->get_future ();
			{
				std::unique_lock<TLock> lock (queue_mutex);

				if (_stop)
					throw std::runtime_error ("enqueue on stopped Thread-Pool");

				_tasks.emplace ([task]() { (*task)(); });
			}
			_condition.notify_one ();
			return res;
		}

	public:
		void cancel_all ()
		{
			std::unique_lock<TLock> lock (_mutex);
			while (!_tasks.empty ())
				_tasks.pop ();
		}

	private:
		std::vector<std::thread> _workers;
		std::queue<std::function<void ()>> _tasks;

		TLock _mutex;
		std::condition_variable _condition;

		bool _stop;
	};

	template<class TLock = mutex, size_t maximum_threads = 0>
	class parallel
	{
	public:
		parallel () : _threadpool (maximum_threads) { }

	public:
		template<class T>
		void for_each (T start, T end, const std::function<void (T)>& body) noexcept
		{
			for (T i = start; i != end; ++i)
				_threadpool.enqueue (body, i);
		}

	private:
		threadpool<TLock> _threadpool;
	};
}

#endif