#ifndef __DSEED_DOBJECT_H__
#define __DSEED_DOBJECT_H__

#include <dseed/dcommon.h>

namespace dseed
{
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

#endif