#include <dseed.h>

#include <string.h>

dseed::object::object () { memcpy (_signature, "DSEEDOBJ", 8); }
dseed::object::~object () { }

dseed::wrapped::~wrapped () { }

class __internal_blob : public dseed::blob
{
public:
	__internal_blob (const void* data, size_t length) noexcept
		: _refCount (1)
	{
		_data.resize (length);
		if (data != nullptr)
			memcpy (_data.data (), data, length);
	}

public:
	virtual int32_t retain () { return ++_refCount; }
	virtual int32_t release ()
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual const void* ptr () noexcept { return _data.data (); }
	virtual size_t size () noexcept { return _data.size (); }

private:
	std::atomic<int32_t> _refCount;
	std::vector<uint8_t> _data;
};

dseed::error_t dseed::create_buffered_blob (const void* data, size_t length, dseed::blob** blob) noexcept
{
	*blob = new __internal_blob (data, length);
	if (*blob == nullptr)
		return dseed::error_out_of_memory;
	return dseed::error_good;
}

dseed::error_t dseed::create_empty_blob (size_t length, dseed::blob** blob) noexcept
{
	return create_buffered_blob (nullptr, length, blob);
}

class __internal_wrapped_blob : public dseed::blob
{
public:
	__internal_wrapped_blob (void* data, size_t length) noexcept
		: _refCount (1), _ptr (data), _size (length)
	{ }

public:
	virtual int32_t retain () { return ++_refCount; }
	virtual int32_t release ()
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual const void* ptr () noexcept { return _ptr; }
	virtual size_t size () noexcept { return _size; }

private:
	std::atomic<int32_t> _refCount;
	void* _ptr;
	size_t _size;
};

dseed::error_t dseed::create_wrapped_blob (void* data, size_t length, dseed::blob** blob) noexcept
{
	*blob = new __internal_wrapped_blob (data, length);
	if (*blob == nullptr)
		return dseed::error_out_of_memory;
	return dseed::error_good;
}

dseed::error_t dseed::attributes::get_size (dseed::attrkey_t key, uint32_t* width, uint32_t* height)
{
	uint64_t temp;
	error_t err;

	if (dseed::failed (err = get_uint64 (key, &temp)))
		return err;

	*width = (temp >> 32) & 0xffffffff;
	*height = temp & 0xffffffff;

	return error_good;
}

dseed::error_t dseed::attributes::set_size (dseed::attrkey_t key, uint32_t width, uint32_t height)
{
	return set_uint64 (key, ((((uint64_t)width) << 32) & 0xffffffff00000000) | height);
}

dseed::error_t dseed::attributes::get_fraction (dseed::attrkey_t key, int32_t* numerator, int32_t* denominator)
{
	int64_t temp;
	error_t err;

	if (dseed::failed (err = get_int64 (key, &temp)))
		return err;

	*numerator = (temp >> 32) & 0xffffffff;
	*denominator = temp & 0xffffffff;

	return error_good;
}

dseed::error_t dseed::attributes::set_fraction (dseed::attrkey_t key, int32_t numerator, int32_t denominator)
{
	return set_int64 (key, ((((int64_t)(numerator)) << 32) & 0xffffffff00000000) | denominator);
}

dseed::error_t dseed::attributes::get_string (dseed::attrkey_t key, char* strBuffer, size_t strBufferMaxSize)
{
	dseed::error_t err;

	dseed::autoref<dseed::object> value;
	if (dseed::failed (err = get_object (key, &value)))
		return err;

	blob* ptrobj = dynamic_cast<blob*>(value.get ());
	if (ptrobj == nullptr)
		return dseed::error_type_incorrect;

	memcpy (strBuffer, ptrobj->ptr (), dseed::minimum (ptrobj->size (), strBufferMaxSize));

	return dseed::error_good;
}

dseed::error_t dseed::attributes::set_string (dseed::attrkey_t key, const char* str)
{
	dseed::autoref<blob> value;
	create_buffered_blob (str, std::char_traits<char>::length (str), &value);

	return set_object (key, value);
}

dseed::error_t dseed::attributes::get_u16string (dseed::attrkey_t key, char16_t* strBuffer, size_t strBufferMaxSize)
{
	dseed::error_t err;

	dseed::autoref<dseed::object> value;
	if (dseed::failed (err = get_object (key, &value)))
		return err;

	blob* ptrobj = dynamic_cast<blob*>(value.get ());
	if (ptrobj == nullptr)
		return dseed::error_type_incorrect;

	memcpy (strBuffer, ptrobj->ptr (), dseed::minimum (ptrobj->size (), strBufferMaxSize * sizeof (char16_t)));

	return dseed::error_good;
}

dseed::error_t dseed::attributes::set_u16string (dseed::attrkey_t key, const char16_t* str)
{
	dseed::autoref<blob> value;
	create_buffered_blob (str, std::char_traits<char16_t>::length (str), &value);

	return set_object (key, value);
}

dseed::error_t dseed::attributes::get_struct (attrkey_t key, void* buffer, size_t bufferSize)
{
	dseed::error_t err;

	dseed::autoref<dseed::object> value;
	if (dseed::failed (err = get_object (key, &value)))
		return err;

	blob* ptrobj = dynamic_cast<blob*>(value.get ());
	if (ptrobj == nullptr)
		return dseed::error_type_incorrect;
	if (ptrobj->size () != bufferSize)
		return dseed::error_type_incorrect;

	memcpy (buffer, ptrobj->ptr (), bufferSize);

	return dseed::error_good;
}

dseed::error_t dseed::attributes::set_struct (attrkey_t key, const void* data, size_t dataSize)
{
	dseed::autoref<blob> value;
	create_buffered_blob (data, dataSize, &value);

	return set_object (key, value);
}

enum class __ait { i32, u32, i64, u64, f32, f64, obj };
union __aiv
{
	int32_t i32;
	uint32_t u32;
	int64_t i64;
	uint64_t u64;
	float f32;
	double f64;
	dseed::object* obj;
};
struct __attributes_item
{
	__ait type;
	__aiv value;
};

class __internal_attributes : public dseed::attributes
{
public:
	__internal_attributes ()
		: _refCount (1)
	{

	}
	~__internal_attributes ()
	{
		for (auto i : _items)
		{
			if (i.second.type == __ait::obj)
				i.second.value.obj->release ();
		}
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

public:
	virtual dseed::error_t get_int32 (dseed::attrkey_t key, int32_t* value) override
	{
		auto found = _items.find (key);
		if (found == _items.end ())
			return dseed::error_not_contained;
		if (found->second.type != __ait::i32)
			return dseed::error_type_incorrect;

		*value = found->second.value.i32;

		return dseed::error_good;
	}

	virtual dseed::error_t get_uint32 (dseed::attrkey_t key, uint32_t* value) override
	{
		auto found = _items.find (key);
		if (found == _items.end ())
			return dseed::error_not_contained;
		if (found->second.type != __ait::u32)
			return dseed::error_type_incorrect;

		*value = found->second.value.u32;

		return dseed::error_good;
	}

	virtual dseed::error_t get_int64 (dseed::attrkey_t key, int64_t* value) override
	{
		auto found = _items.find (key);
		if (found == _items.end ())
			return dseed::error_not_contained;
		if (found->second.type != __ait::i64)
			return dseed::error_type_incorrect;

		*value = found->second.value.i64;

		return dseed::error_good;
	}

	virtual dseed::error_t get_uint64 (dseed::attrkey_t key, uint64_t* value) override
	{
		auto found = _items.find (key);
		if (found == _items.end ())
			return dseed::error_not_contained;
		if (found->second.type != __ait::u64)
			return dseed::error_type_incorrect;

		*value = found->second.value.u64;

		return dseed::error_good;
	}

	virtual dseed::error_t get_single (dseed::attrkey_t key, float* value) override
	{
		auto found = _items.find (key);
		if (found == _items.end ())
			return dseed::error_not_contained;
		if (found->second.type != __ait::f32)
			return dseed::error_type_incorrect;

		*value = found->second.value.f32;

		return dseed::error_good;
	}

	virtual dseed::error_t get_double (dseed::attrkey_t key, double* value) override
	{
		auto found = _items.find (key);
		if (found == _items.end ())
			return dseed::error_not_contained;
		if (found->second.type != __ait::f64)
			return dseed::error_type_incorrect;

		*value = found->second.value.f64;

		return dseed::error_good;
	}

	virtual dseed::error_t get_object (dseed::attrkey_t key, dseed::object** value) override
	{
		auto found = _items.find (key);
		if (found == _items.end ())
			return dseed::error_not_contained;
		if (found->second.type != __ait::obj)
			return dseed::error_type_incorrect;

		*value = found->second.value.obj;
		if (*value != nullptr)
			(*value)->retain ();

		return dseed::error_good;
	}

public:
	virtual dseed::error_t set_int32 (dseed::attrkey_t key, int32_t value) override
	{
		auto found = _items.find (key);
		if (found != _items.end ())
		{
			if (found->second.type == __ait::obj && found->second.value.obj != nullptr)
				found->second.value.obj->release ();
		}

		__attributes_item item;
		item.type = __ait::i32;
		item.value.i32 = value;

		_items.insert_or_assign (key, item);

		return dseed::error_good;
	}

	virtual dseed::error_t set_uint32 (dseed::attrkey_t key, uint32_t value) override
	{
		auto found = _items.find (key);
		if (found != _items.end ())
		{
			if (found->second.type == __ait::obj && found->second.value.obj != nullptr)
				found->second.value.obj->release ();
		}

		__attributes_item item;
		item.type = __ait::u32;
		item.value.u32 = value;

		_items.insert_or_assign (key, item);

		return dseed::error_good;
	}

	virtual dseed::error_t set_int64 (dseed::attrkey_t key, int64_t value) override
	{
		auto found = _items.find (key);
		if (found != _items.end ())
		{
			if (found->second.type == __ait::obj && found->second.value.obj != nullptr)
				found->second.value.obj->release ();
		}

		__attributes_item item;
		item.type = __ait::i64;
		item.value.i64 = value;

		_items.insert_or_assign (key, item);

		return dseed::error_good;
	}

	virtual dseed::error_t set_uint64 (dseed::attrkey_t key, uint64_t value) override
	{
		auto found = _items.find (key);
		if (found != _items.end ())
		{
			if (found->second.type == __ait::obj && found->second.value.obj != nullptr)
				found->second.value.obj->release ();
		}

		__attributes_item item;
		item.type = __ait::u64;
		item.value.u64 = value;

		_items.insert_or_assign (key, item);

		return dseed::error_good;
	}

	virtual dseed::error_t set_single (dseed::attrkey_t key, float value) override
	{
		auto found = _items.find (key);
		if (found != _items.end ())
		{
			if (found->second.type == __ait::obj && found->second.value.obj != nullptr)
				found->second.value.obj->release ();
		}

		__attributes_item item;
		item.type = __ait::f32;
		item.value.f32 = value;

		_items.insert_or_assign (key, item);

		return dseed::error_good;
	}

	virtual dseed::error_t set_double (dseed::attrkey_t key, double value) override
	{
		auto found = _items.find (key);
		if (found != _items.end ())
		{
			if (found->second.type == __ait::obj && found->second.value.obj != nullptr)
				found->second.value.obj->release ();
		}

		__attributes_item item;
		item.type = __ait::f64;
		item.value.f64 = value;

		_items.insert_or_assign (key, item);

		return dseed::error_good;
	}

	virtual dseed::error_t set_object (dseed::attrkey_t key, dseed::object* value) override
	{
		auto found = _items.find (key);
		if (found != _items.end ())
		{
			if (found->second.type == __ait::obj && found->second.value.obj != nullptr)
				found->second.value.obj->release ();
		}

		__attributes_item item;
		item.type = __ait::obj;
		item.value.obj = value;
		item.value.obj->retain ();

		_items.insert_or_assign (key, item);

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;

	std::unordered_map<uint64_t, __attributes_item> _items;
};

dseed::error_t dseed::create_attributes (attributes** attr) noexcept
{
	if (attr == nullptr)
		return dseed::error_invalid_args;

	*attr = new __internal_attributes ();
	if (*attr == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}