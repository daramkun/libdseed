#ifndef __DSEED_BITMAP_DECODERS_COMMON_HXX__
#define __DSEED_BITMAP_DECODERS_COMMON_HXX__

namespace dseed
{
	class __common_bitmap_array : public dseed::bitmaps::bitmap_array
	{
	public:
		__common_bitmap_array (const std::vector<std::tuple<dseed::bitmaps::bitmap*, dseed::timespan>>& bitmaps, dseed::bitmaps::arraytype frametype = dseed::bitmaps::arraytype::plain)
			: _refCount (1), _frametype (frametype)
		{
			for (auto i : bitmaps)
				_bitmaps.push_back (std::tuple<dseed::autoref<dseed::bitmaps::bitmap>, dseed::timespan> (
					std::get<dseed::bitmaps::bitmap*> (i),
					std::get<dseed::timespan> (i)
					));
		}
		__common_bitmap_array (const std::vector<dseed::bitmaps::bitmap*>& bitmaps, dseed::bitmaps::arraytype frametype = dseed::bitmaps::arraytype::plain)
			: _refCount (1), _frametype (frametype)
		{
			for (auto i : bitmaps)
				_bitmaps.push_back (std::tuple<dseed::autoref<dseed::bitmaps::bitmap>, dseed::timespan> (i, 0));
		}
		__common_bitmap_array (const std::vector<dseed::bitmaps::bitmap*>& bitmaps, const std::vector<dseed::timespan>& timespans, dseed::bitmaps::arraytype frametype = dseed::bitmaps::arraytype::plain)
			: _refCount (1), _frametype (frametype)
		{
			for (int i = 0; i < bitmaps.size (); ++i)
				_bitmaps.push_back (std::tuple<dseed::autoref<dseed::bitmaps::bitmap>, dseed::timespan> (bitmaps[i], timespans[i]));
		}
		__common_bitmap_array (dseed::bitmaps::bitmap* bitmap, dseed::bitmaps::arraytype frametype = dseed::bitmaps::arraytype::plain)
			: _refCount (1), _frametype (frametype)
		{
			_bitmaps.push_back (std::tuple<dseed::autoref<dseed::bitmaps::bitmap>, dseed::timespan> (bitmap, 0));
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
		virtual dseed::error_t at (size_t i, dseed::bitmaps::bitmap** bitmap) noexcept override
		{
			if (!(i >= 0 && i < _bitmaps.size ())) return dseed::error_invalid_args;
			if (bitmap == nullptr) return dseed::error_invalid_args;

			*bitmap = std::get<dseed::autoref<dseed::bitmaps::bitmap>> (_bitmaps[i]);
			(*bitmap)->retain ();

			return dseed::error_good;
		}
		virtual size_t size () noexcept override { return _bitmaps.size (); }
		virtual dseed::bitmaps::arraytype type () noexcept override { return _frametype; }

	private:
		std::atomic<int32_t> _refCount;
		std::vector<std::tuple<dseed::autoref<dseed::bitmaps::bitmap>, dseed::timespan>> _bitmaps;
		dseed::bitmaps::arraytype _frametype;
	};

}

#endif