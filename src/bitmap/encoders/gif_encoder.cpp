#include <dseed.h>

#if defined ( USE_GIF ) && defined ( USE_BITMAP_ENCODERS )
#	include <gif_lib.h>
#	include <vector>

class __gif_encoder : public dseed::bitmaps::bitmap_encoder
{
public:
	__gif_encoder(dseed::io::stream* stream)
		: _refCount(1), _stream(stream), _pgif(nullptr), _isFirst(true), _transparent(-1)
	{
		int err;
		_pgif = EGifOpen(stream, [](GifFileType* file, const GifByteType* buf, int len) -> int
			{
				return (int) reinterpret_cast<dseed::io::stream*>(file->UserData)->write(buf, len);
			}, &err
		);
		if (_pgif == nullptr)
			return;
	}
	~__gif_encoder()
	{
		int err;
		EGifCloseFile(_pgif, &err);
	}

public:
	virtual int32_t retain() override { return ++_refCount; }
	virtual int32_t release() override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual dseed::error_t encode_frame(dseed::bitmaps::bitmap* bitmap) noexcept override
	{
		if (_pgif == nullptr) return dseed::error_fail;
		if (bitmap == nullptr)
			return dseed::error_invalid_args;
		if ((!(bitmap->format() == dseed::color::pixelformat::bgr8_indexed8 || bitmap->format() == dseed::color::pixelformat::bgra8_indexed8))
			|| (bitmap->size().depth > 1))
			return dseed::error_not_support;

		auto size = bitmap->size();
		ColorMapObject* cmo = nullptr;

		if (_isFirst)
		{
			EGifSetGifVersion(_pgif, true);

			if (bitmap->format() == dseed::color::pixelformat::bgr8_indexed8)
			{
				std::vector<dseed::color::bgr8> tempPalette(256);
				dseed::autoref<dseed::bitmaps::palette> paletteObj;
				bitmap->palette(&paletteObj);
				paletteObj->copy_palette(tempPalette.data());
				size_t paletteSize = paletteObj->size();
				_palette.resize(paletteSize);
				for (int i = 0; i < paletteSize; ++i)
				{
					_palette[i].b = tempPalette[i].b;
					_palette[i].g = tempPalette[i].g;
					_palette[i].r = tempPalette[i].r;
				}
			}
			else if (bitmap->format() == dseed::color::pixelformat::bgra8_indexed8)
			{
				std::vector<dseed::color::bgra8> tempPalette(256);
				dseed::autoref<dseed::bitmaps::palette> paletteObj;
				bitmap->palette(&paletteObj);
				paletteObj->copy_palette(tempPalette.data());
				size_t paletteSize = paletteObj->size();
				_palette.resize(paletteSize);
				for (int i = 0; i < paletteSize; ++i)
				{
					_palette[i].b = tempPalette[i].b;
					_palette[i].g = tempPalette[i].g;
					_palette[i].r = tempPalette[i].r;
					if (_transparent == -1 && tempPalette[i].a != 255)
						_transparent = i;
				}
			}
			cmo = GifMakeMapObject((int)_palette.size(), (const GifColorType*)_palette.data());
			EGifPutScreenDesc(_pgif, size.width, size.height, (int)_palette.size(), _transparent, cmo);

			char netscape_extension[] = "NETSCAPE2.0";
			EGifPutExtension(_pgif, APPLICATION_EXT_FUNC_CODE, 11, netscape_extension);
			char animation_extension[] = { 1, 0, 0 };
			EGifPutExtension(_pgif, APPLICATION_EXT_FUNC_CODE, 3, animation_extension);
		}
		else
		{
			std::vector<dseed::color::rgb8> tempPalette(256);
			int transparent = -1;
			if (bitmap->format() == dseed::color::pixelformat::bgr8_indexed8)
			{
				std::vector<dseed::color::bgr8> tempTempPalette(256);
				dseed::autoref<dseed::bitmaps::palette> paletteObj;
				bitmap->palette(&paletteObj);
				paletteObj->copy_palette(tempTempPalette.data());
				size_t paletteSize = paletteObj->size();
				tempPalette.resize(paletteSize);
				for (int i = 0; i < paletteSize; ++i)
				{
					tempPalette[i].b = tempTempPalette[i].b;
					tempPalette[i].g = tempTempPalette[i].g;
					tempPalette[i].r = tempTempPalette[i].r;
				}
			}
			else if (bitmap->format() == dseed::color::pixelformat::bgra8_indexed8)
			{
				std::vector<dseed::color::bgra8> tempTempPalette(256);
				dseed::autoref<dseed::bitmaps::palette> paletteObj;
				bitmap->palette(&paletteObj);
				paletteObj->copy_palette(tempTempPalette.data());
				size_t paletteSize = paletteObj->size();
				tempPalette.resize(paletteSize);
				for (int i = 0; i < paletteSize; ++i)
				{
					tempPalette[i].b = tempTempPalette[i].b;
					tempPalette[i].g = tempTempPalette[i].g;
					tempPalette[i].r = tempTempPalette[i].r;
					if (transparent == -1 && tempTempPalette[i].a != 255)
						transparent = i;
				}
			}
			for (int i = 0; i < tempPalette.size(); ++i)
			{
				if (_palette[i].r != tempPalette[i].r || _palette[i].g != tempPalette[i].g || _palette[i].b != tempPalette[i].b)
				{
					cmo = GifMakeMapObject((int)tempPalette.size(), (const GifColorType*)tempPalette.data());
					_transparent = transparent;
					break;
				}
			}
		}

		dseed::autoref<dseed::attributes> attr;
		bitmap->extra_info(&attr);

		dseed::timespan duration;
		if (dseed::failed(attr->get_int64(dseed::attrkey_duration, (int64_t*)&duration)))
			duration = 0;

		int delay = (int)(duration.total_milliseconds() / 10);
		char extension[] = { (3 << 2) | (_transparent != -1 ? 1 : 0),
			(char)(delay % 256), (char)(delay / 256), (char)_transparent };
		EGifPutExtension(_pgif, GRAPHICS_EXT_FUNC_CODE, 4, extension);

		EGifPutImageDesc(_pgif, 0, 0, size.width, size.height, false, cmo);

		std::vector<uint8_t> pixels(size.width * size.height);
		bitmap->copy_pixels(pixels.data(), 0);

		for (int y = 0; y < size.height; ++y)
			EGifPutLine(_pgif, pixels.data() + (y * size.width), size.width);

		if (cmo)
			GifFreeMapObject(cmo);
		cmo = nullptr;

		_isFirst = false;
		return dseed::error_good;
	}

	virtual dseed::bitmaps::arraytype type() noexcept override { return dseed::bitmaps::arraytype::plain; }

public:
	virtual dseed::error_t commit() noexcept override
	{
		if (_pgif == nullptr) return dseed::error_fail;

		EGifSpew(_pgif);

		return dseed::error_good;
	}

public:
	bool isInitialized() { return _pgif != nullptr; }

private:
	std::atomic<int32_t> _refCount;
	dseed::autoref<dseed::io::stream> _stream;

	GifFileType* _pgif;
	bool _isFirst;

	std::vector<dseed::color::rgb8> _palette;
	int _transparent;
};
#endif

dseed::error_t dseed::bitmaps::create_gif_bitmap_encoder(dseed::io::stream* stream, const dseed::bitmaps::bitmap_encoder_options* options, dseed::bitmaps::bitmap_encoder** encoder)
{
#if defined ( USE_GIF ) && defined ( USE_BITMAP_ENCODERS )
	auto enc = new __gif_encoder(stream);
	if (enc == nullptr)
		return dseed::error_out_of_memory;
	if (!enc->isInitialized())
	{
		enc->release();
		return dseed::error_fail;
	}

	*encoder = enc;

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}