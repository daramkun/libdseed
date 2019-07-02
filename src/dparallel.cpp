#include <dseed.h>

#include <mutex>
#include <shared_mutex>

class __threadsyncer_mutex : public dseed::threadsyncer
{
public:
	__threadsyncer_mutex ()
		: _refCount (1), _mutex ()
	{ }

public:
	virtual int32_t retain () override { return ++_refCount; }
	virtual int32_t release () override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual void lock_exclusive () override { _mutex.lock (); }
	virtual void lock_shared () override { _mutex.lock_shared (); }

public:
	virtual bool try_lock_exclusive () override { return _mutex.try_lock (); }
	virtual bool try_lock_shared () override { return _mutex.try_lock_shared (); }

public:
	virtual void unlock_exclusive () override { _mutex.unlock (); }
	virtual void unlock_shared () override { _mutex.unlock_shared (); }

private:
	std::atomic<int32_t> _refCount;
	std::shared_timed_mutex _mutex;
};

dseed::error_t dseed::create_mutex (threadsyncer** ts)
{
	*ts = new __threadsyncer_mutex ();
	if (*ts == nullptr)
		return error_out_of_memory;
	return error_good;
}

class __threadsyncer_spinlock : public dseed::threadsyncer
{
public:
	__threadsyncer_spinlock ()
		: _refCount (1), _sharedCount (0), _exclusiveFlag (false)
	{ }

public:
	virtual int32_t retain () override { return ++_refCount; }
	virtual int32_t release () override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual void lock_exclusive () override
	{
		bool expected = false;
		while (!_exclusiveFlag.compare_exchange_weak (expected, true, std::memory_order_acquire))
			std::this_thread::yield ();

		while (_sharedCount.load (std::memory_order_seq_cst) != 0)
			std::this_thread::yield ();
	}
	virtual void lock_shared () override
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
	virtual bool try_lock_exclusive () override
	{
		bool expected = false;
		if (!_exclusiveFlag.load (std::memory_order_seq_cst) && _sharedCount.load (std::memory_order_seq_cst) == 0)
			return _exclusiveFlag.compare_exchange_weak (expected, true, std::memory_order_acquire);
		return false;
	}

	virtual bool try_lock_shared () override
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
	virtual void unlock_exclusive () override
	{
		_exclusiveFlag.store (false, std::memory_order_release);
	}

	virtual void unlock_shared () override
	{
		_sharedCount.fetch_sub (1, std::memory_order_release);
	}

private:
	std::atomic<int32_t> _refCount;
	std::atomic<uint64_t> _sharedCount;
	std::atomic<bool> _exclusiveFlag;
};

dseed::error_t dseed::create_spinlock (threadsyncer** ts)
{
	*ts = new __threadsyncer_spinlock ();
	if (*ts == nullptr)
		return error_out_of_memory;
	return error_good;
}

class __internal_threadpool : public dseed::threadpool
{
public:
	__internal_threadpool (size_t maximum_threads)
		: _refCount (1), _stop (false)
	{
		_workers.reserve (maximum_threads);
		for (size_t i = 0; i < maximum_threads; ++i)
			_workers.emplace_back ([this] {
			for (;;)
			{
				std::tuple<dseed::threadpool_body, void*> task;
				{
					std::unique_lock<std::mutex> lock (this->_mutex);

					this->_condition.wait (lock, [this] { return this->_stop || !this->_tasks.empty (); });
					if (this->_stop || this->_tasks.empty ())
						break;

					task = std::move (this->_tasks.front ());
					this->_tasks.pop ();
				}

				std::get<dseed::threadpool_body> (task)(std::get<void*> (task));
			}
				});
	}
	~__internal_threadpool ()
	{
		{
			std::unique_lock<std::mutex> lock (_mutex);
			_stop = true;
		}
		_condition.notify_all ();
		for (std::thread& worker : _workers)
			worker.join ();
	}

public:
	virtual int32_t retain () override { return ++_refCount; }
	virtual int32_t release () override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

	virtual void enqueue (dseed::threadpool_body body, void* user_data) override
	{
		std::unique_lock<std::mutex> lock (_mutex);
		if (_stop)
			throw std::runtime_error ("enqueue on stopped ThreadPool");
		_tasks.emplace (std::tuple<dseed::threadpool_body, void*> (body, user_data));
		_condition.notify_one ();
	}

public:
	virtual void cancel_all () override
	{
		std::unique_lock<std::mutex> lock (_mutex);
		while (!_tasks.empty ())
			_tasks.pop ();
	}

private:
	std::atomic<int32_t> _refCount;

	std::vector<std::thread> _workers;
	std::queue<std::tuple<dseed::threadpool_body, void*>> _tasks;

	std::mutex _mutex;
	std::condition_variable _condition;

	bool _stop;
};

dseed::error_t dseed::create_threadpool (threadpool** pool, size_t maximum_threads)
{
	if (maximum_threads == 0)
		maximum_threads = std::thread::hardware_concurrency ();
	*pool = new __internal_threadpool (maximum_threads);
	if (*pool == nullptr)
		return dseed::error_out_of_memory;
	return dseed::error_good;
}