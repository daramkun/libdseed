#ifndef __DSEED_INTERNAL_BITMAP_DECODER_HXX__
#define __DSEED_INTERNAL_BITMAP_DECODER_HXX__

class __common_bitmap_decoder : public dseed::bitmap_decoder
{
public:
	__common_bitmap_decoder (const std::vector<std::tuple<dseed::bitmap*, dseed::timespan_t>>& bitmaps)
		: _refCount (0)
	{
		for (auto i : bitmaps)
			_bitmaps.push_back (std::tuple<dseed::auto_object<dseed::bitmap>, dseed::timespan_t> (
				std::get<dseed::bitmap*> (i),
				std::get<dseed::timespan_t> (i)
				));
	}
	__common_bitmap_decoder (const std::vector<dseed::bitmap*>& bitmaps)
		: _refCount (0)
	{
		for (auto i : bitmaps)
			_bitmaps.push_back (std::tuple<dseed::auto_object<dseed::bitmap>, dseed::timespan_t> (i, 0));
	}
	__common_bitmap_decoder (dseed::bitmap* bitmap)
		: _refCount (0)
	{
		_bitmaps.push_back (std::tuple<dseed::auto_object<dseed::bitmap>, dseed::timespan_t> (bitmap, 0));
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
	virtual dseed::error_t decode_frame (size_t i, dseed::bitmap** bitmap, dseed::timespan_t* duration) override
	{
		if (!(i >= 0 && i < _bitmaps.size ())) return dseed::error_invalid_args;
		if (bitmap == nullptr) return dseed::error_invalid_args;

		*bitmap = std::get<dseed::auto_object<dseed::bitmap>> (_bitmaps[i]);
		(*bitmap)->retain ();

		if (duration)
			* duration = std::get<dseed::timespan_t> (_bitmaps[i]);

		return dseed::error_good;
	}
	virtual size_t frame_count () override { return _bitmaps.size (); }

private:
	std::atomic<int32_t> _refCount;
	std::vector<std::tuple<dseed::auto_object<dseed::bitmap>, dseed::timespan_t>> _bitmaps;
};

#endif