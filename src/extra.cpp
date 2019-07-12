#include <dseed.h>

class __memblob : public dseed::blob
{
public:
	__memblob (size_t length)
		: _refCount (1), _memLength (length), _mem (length)
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
	virtual void* blob_pointer () override { return _mem.data (); }
	virtual size_t blob_length () override { return _memLength; }

private:
	std::atomic<int32_t> _refCount;
	std::vector<uint8_t> _mem;
	size_t _memLength;
};

dseed::error_t dseed::create_memoryblob (size_t length, dseed::blob** blob)
{
	*blob = new __memblob (length);
	if (*blob == nullptr)
		return error_out_of_memory;
	return error_good;
}

class __bufferblob : public dseed::blob
{
public:
	__bufferblob (void* buffer, size_t length)
		: _refCount (1), _length (length), _buf (buffer)
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
	virtual void* blob_pointer () override { return _buf; }
	virtual size_t blob_length () override { return _length; }

private:
	std::atomic<int32_t> _refCount;
	void* _buf;
	size_t _length;
};

dseed::error_t dseed::create_bufferblob (void* buffer, size_t length, bool copy, dseed::blob** blob)
{
	if (!copy)
	{
		*blob = new __bufferblob (buffer, length);
		if (*blob == nullptr)
			return error_out_of_memory;
	}
	else
	{
		*blob = new __memblob (length);
		if (*blob == nullptr)
			return error_out_of_memory;
		memcpy ((*blob)->blob_pointer (), buffer, length);
	}
	return error_good;
}

dseed::error_t dseed::create_streamblob (dseed::stream* stream, dseed::blob** blob)
{
	dseed::auto_object<dseed::blob> tempBlob;
	if (dseed::failed (dseed::create_memoryblob (stream->length (), &tempBlob)))
		return dseed::error_fail;

	if (stream->read (tempBlob->blob_pointer (), tempBlob->blob_length ()) != tempBlob->blob_length ())
		return dseed::error_io;

	*blob = tempBlob.detach ();

	return error_good;
}

enum __attributes_item_type { ait_i32, ait_u32, ait_i64, ait_u64, ait_f32, ait_f64, ait_obj };
union __attributes_item_value
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
	__attributes_item_type type;
	__attributes_item_value value;
};

class __attributes : public dseed::attributes
{
public:
	__attributes ()
		: _refCount (1)
	{

	}
	~__attributes ()
	{
		for (auto i : _items)
		{
			if (i.second.type == ait_obj)
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
		if (found->second.type != ait_i32)
			return dseed::error_type_incorrect;

		*value = found->second.value.i32;

		return dseed::error_good;
	}

	virtual dseed::error_t get_uint32 (dseed::attrkey_t key, uint32_t* value) override
	{
		auto found = _items.find (key);
		if (found == _items.end ())
			return dseed::error_not_contained;
		if (found->second.type != ait_u32)
			return dseed::error_type_incorrect;

		*value = found->second.value.u32;

		return dseed::error_good;
	}

	virtual dseed::error_t get_int64 (dseed::attrkey_t key, int64_t* value) override
	{
		auto found = _items.find (key);
		if (found == _items.end ())
			return dseed::error_not_contained;
		if (found->second.type != ait_i64)
			return dseed::error_type_incorrect;

		*value = found->second.value.i64;

		return dseed::error_good;
	}

	virtual dseed::error_t get_uint64 (dseed::attrkey_t key, uint64_t* value) override
	{
		auto found = _items.find (key);
		if (found == _items.end ())
			return dseed::error_not_contained;
		if (found->second.type != ait_u64)
			return dseed::error_type_incorrect;

		*value = found->second.value.u64;

		return dseed::error_good;
	}

	virtual dseed::error_t get_single (dseed::attrkey_t key, float* value) override
	{
		auto found = _items.find (key);
		if (found == _items.end ())
			return dseed::error_not_contained;
		if (found->second.type != ait_f32)
			return dseed::error_type_incorrect;

		*value = found->second.value.f32;

		return dseed::error_good;
	}

	virtual dseed::error_t get_double (dseed::attrkey_t key, double* value) override
	{
		auto found = _items.find (key);
		if (found == _items.end ())
			return dseed::error_not_contained;
		if (found->second.type != ait_f64)
			return dseed::error_type_incorrect;

		*value = found->second.value.f64;

		return dseed::error_good;
	}

	virtual dseed::error_t get_object (dseed::attrkey_t key, dseed::object** value) override
	{
		auto found = _items.find (key);
		if (found == _items.end ())
			return dseed::error_not_contained;
		if (found->second.type != ait_obj)
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
			if (found->second.type == ait_obj && found->second.value.obj != nullptr)
				found->second.value.obj->release ();
		}

		__attributes_item item;
		item.type = ait_i32;
		item.value.i32 = value;

		_items.insert_or_assign (key, item);

		return dseed::error_good;
	}

	virtual dseed::error_t set_uint32 (dseed::attrkey_t key, uint32_t value) override
	{
		auto found = _items.find (key);
		if (found != _items.end ())
		{
			if (found->second.type == ait_obj && found->second.value.obj != nullptr)
				found->second.value.obj->release ();
		}

		__attributes_item item;
		item.type = ait_u32;
		item.value.u32 = value;

		_items.insert_or_assign (key, item);

		return dseed::error_good;
	}

	virtual dseed::error_t set_int64 (dseed::attrkey_t key, int64_t value) override
	{
		auto found = _items.find (key);
		if (found != _items.end ())
		{
			if (found->second.type == ait_obj && found->second.value.obj != nullptr)
				found->second.value.obj->release ();
		}

		__attributes_item item;
		item.type = ait_i64;
		item.value.i64 = value;

		_items.insert_or_assign (key, item);

		return dseed::error_good;
	}

	virtual dseed::error_t set_uint64 (dseed::attrkey_t key, uint64_t value) override
	{
		auto found = _items.find (key);
		if (found != _items.end ())
		{
			if (found->second.type == ait_obj && found->second.value.obj != nullptr)
				found->second.value.obj->release ();
		}

		__attributes_item item;
		item.type = ait_u64;
		item.value.u64 = value;

		_items.insert_or_assign (key, item);

		return dseed::error_good;
	}

	virtual dseed::error_t set_single (dseed::attrkey_t key, float value) override
	{
		auto found = _items.find (key);
		if (found != _items.end ())
		{
			if (found->second.type == ait_obj && found->second.value.obj != nullptr)
				found->second.value.obj->release ();
		}

		__attributes_item item;
		item.type = ait_f32;
		item.value.f32 = value;

		_items.insert_or_assign (key, item);

		return dseed::error_good;
	}

	virtual dseed::error_t set_double (dseed::attrkey_t key, double value) override
	{
		auto found = _items.find (key);
		if (found != _items.end ())
		{
			if (found->second.type == ait_obj && found->second.value.obj != nullptr)
				found->second.value.obj->release ();
		}

		__attributes_item item;
		item.type = ait_f64;
		item.value.f64 = value;

		_items.insert_or_assign (key, item);

		return dseed::error_good;
	}

	virtual dseed::error_t set_object (dseed::attrkey_t key, dseed::object* value) override
	{
		auto found = _items.find (key);
		if (found != _items.end ())
		{
			if (found->second.type == ait_obj && found->second.value.obj != nullptr)
				found->second.value.obj->release ();
		}

		__attributes_item item;
		item.type = ait_obj;
		item.value.obj = value;
		item.value.obj->retain ();

		_items.insert_or_assign (key, item);

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;

	std::unordered_map<uint64_t, __attributes_item> _items;
};

dseed::error_t dseed::create_attributes (attributes** attr)
{
	if (attr == nullptr) return dseed::error_invalid_args;

	*attr = new __attributes ();
	if (*attr == nullptr)
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
	return set_uint64 (key, ((width << 32) & 0xffffffff00000000) | height);
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
	return set_int64 (key, ((numerator << 32) & 0xffffffff00000000) | denominator);
}

class __ptr_obj : public dseed::object
{
public:
	__ptr_obj (const void* ptr, size_t ptrSize)
		: _refCount (1)
	{
		_ptr.resize (ptrSize);
		memcpy (_ptr.data (), ptr, ptrSize);
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
	void* get_ptr () { return _ptr.data (); }
	size_t get_ptr_size () { return _ptr.size (); }

private:
	std::atomic<int32_t> _refCount;
	std::vector<uint8_t> _ptr;
};

dseed::error_t dseed::attributes::get_string (dseed::attrkey_t key, char* strBuffer, size_t strBufferMaxSize)
{
	dseed::error_t err;

	dseed::auto_object<dseed::object> value;
	if (dseed::failed (err = get_object (key, &value)))
		return err;

	__ptr_obj* ptrobj = dynamic_cast<__ptr_obj*>(value.get ());
	if (ptrobj == nullptr)
		return dseed::error_type_incorrect;

	memcpy (strBuffer, ptrobj->get_ptr (), dseed::minimum (ptrobj->get_ptr_size (), strBufferMaxSize));

	return dseed::error_good;
}

dseed::error_t dseed::attributes::set_string (dseed::attrkey_t key, const char* str)
{
	dseed::auto_object<__ptr_obj> value;
	*&value = new __ptr_obj (str, std::char_traits<char>::length (str));

	return set_object (key, value);
}

dseed::error_t dseed::attributes::get_u16string (dseed::attrkey_t key, char16_t* strBuffer, size_t strBufferMaxSize)
{
	dseed::error_t err;

	dseed::auto_object<dseed::object> value;
	if (dseed::failed (err = get_object (key, &value)))
		return err;

	__ptr_obj* ptrobj = dynamic_cast<__ptr_obj*>(value.get ());
	if (ptrobj == nullptr)
		return dseed::error_type_incorrect;

	memcpy (strBuffer, ptrobj->get_ptr (), dseed::minimum (ptrobj->get_ptr_size (), strBufferMaxSize * sizeof (char16_t)));

	return dseed::error_good;
}

dseed::error_t dseed::attributes::set_u16string (dseed::attrkey_t key, const char16_t* str)
{
	dseed::auto_object<__ptr_obj> value;
	*&value = new __ptr_obj (str, std::char_traits<char16_t>::length (str));

	return set_object (key, value);
}

dseed::error_t dseed::attributes::get_struct (attrkey_t key, void* buffer, size_t bufferSize)
{
	dseed::error_t err;

	dseed::auto_object<dseed::object> value;
	if (dseed::failed (err = get_object (key, &value)))
		return err;

	__ptr_obj* ptrobj = dynamic_cast<__ptr_obj*>(value.get ());
	if (ptrobj == nullptr)
		return dseed::error_type_incorrect;
	if (ptrobj->get_ptr_size () != bufferSize)
		return dseed::error_type_incorrect;

	memcpy (buffer, ptrobj->get_ptr (), bufferSize);

	return dseed::error_good;
}

dseed::error_t dseed::attributes::set_struct (attrkey_t key, const void* data, size_t dataSize)
{
	dseed::auto_object<__ptr_obj> value;
	*&value = new __ptr_obj (data, dataSize);

	return set_object (key, value);
}