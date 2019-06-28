#include <dseed.h>

void dseed::threadsyncer::do_exclusive (std::function<void (void*)> func, void* user_data)
{
	lock_exclusive ();
	func (user_data);
	unlock_exclusive ();
}

void dseed::threadsyncer::do_shared (std::function<void (void*)> func, void* user_data)
{
	lock_shared ();
	func (user_data);
	unlock_shared ();
}

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