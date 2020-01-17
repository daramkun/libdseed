#include <dseed.h>

#if PLATFORM_MICROSOFT

#	include <Windows.h>
#	include <wincodec.h>
#	include <atlbase.h>

#	include "../../libs/IStreamHelper.hxx"

#if PLATFORM_WINDOWS
using IWICImagingFactoryP = IWICImagingFactory;
#define CLSID_WICImagingFactoryP CLSID_WICImagingFactory
#else
using IWICImagingFactoryP = IWICImagingFactory2;
#define CLSID_WICImagingFactoryP CLSID_WICImagingFactory2
#endif

class __windowsimagingcodec_palette : public dseed::bitmaps::palette
{
public:
	__windowsimagingcodec_palette (IWICPalette* palette)
		: _refCount (1), _palette (palette)
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
	virtual size_t size () noexcept override
	{
		UINT pc;
		if (FAILED (_palette->GetColorCount (&pc)))
			return 0;
		return pc;
	}
	virtual size_t bits_per_pixel () noexcept override { return 32; }

public:
	virtual dseed::error_t copy_palette (void* dest) noexcept override
	{
		UINT ac;
		if (FAILED (_palette->GetColors ((UINT)size (), (WICColor*)dest, &ac)))
			return dseed::error_fail;
		return dseed::error_good;
	}

public:
	virtual dseed::error_t lock (void** ptr) noexcept override { return dseed::error_not_impl; }
	virtual dseed::error_t unlock () noexcept override { return dseed::error_not_impl; }

private:
	std::atomic<int32_t> _refCount;
	CComPtr<IWICPalette> _palette;
};

class __windowsimagingcodec_bitmap : public dseed::bitmaps::bitmap
{
public:
	__windowsimagingcodec_bitmap (IWICBitmapSource* bitmap, IWICPalette* palette)
		:_refCount (1), _bitmap (bitmap)
	{
		_palette = new __windowsimagingcodec_palette (palette);

		dseed::create_attributes (&_extraInfo);
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
	virtual dseed::size3i size () noexcept override
	{
		UINT width, height;
		_bitmap->GetSize (&width, &height);
		return dseed::size3i (width, height, 1);
	}
	virtual dseed::color::pixelformat format () noexcept override
	{
		WICPixelFormatGUID pixelFormat;
		_bitmap->GetPixelFormat (&pixelFormat);

		if (pixelFormat == GUID_WICPixelFormat32bppRGBA)
			return dseed::color::pixelformat::rgba8;
		else if (pixelFormat == GUID_WICPixelFormat24bppRGB)
			return dseed::color::pixelformat::rgb8;
		else if (pixelFormat == GUID_WICPixelFormat32bppBGRA)
			return dseed::color::pixelformat::bgra8;
		else if (pixelFormat == GUID_WICPixelFormat24bppBGR)
			return dseed::color::pixelformat::bgr8;
		else if (pixelFormat == GUID_WICPixelFormat128bppRGBAFloat)
			return dseed::color::pixelformat::rgbaf;
		else if (pixelFormat == GUID_WICPixelFormat8bppGray)
			return dseed::color::pixelformat::r8;
		else if (pixelFormat == GUID_WICPixelFormat32bppGrayFloat)
			return dseed::color::pixelformat::rf;
		else if (pixelFormat == GUID_WICPixelFormat8bppIndexed)
			return dseed::color::pixelformat::bgra8_indexed8;
		else if (pixelFormat == GUID_WICPixelFormat16bppBGR565)
			return dseed::color::pixelformat::bgr565;

		return dseed::color::pixelformat::unknown;
	}
	virtual dseed::bitmaps::bitmaptype type () noexcept override { return dseed::bitmaps::bitmaptype::bitmap2d; }

public:
	virtual dseed::error_t palette (dseed::bitmaps::palette** palette) noexcept override
	{
		if (palette == nullptr) return dseed::error_invalid_args;
		if (_palette == nullptr) return dseed::error_invalid_op;
		*palette = _palette;
		(*palette)->retain ();
		return dseed::error_good;
	}

public:
	virtual dseed::error_t lock (void** ptr) noexcept override { return dseed::error_not_impl; }
	virtual dseed::error_t unlock () noexcept override { return dseed::error_not_impl; }

public:
	virtual dseed::error_t copy_pixels (void* dest, size_t depth) noexcept override
	{
		if (dest == nullptr || depth != 0) return dseed::error_invalid_args;

		dseed::size3i sz = size ();
		size_t stride = dseed::color::calc_bitmap_stride (format (), sz.width);

		if (FAILED (_bitmap->CopyPixels (nullptr, (UINT)stride, (UINT)(stride * sz.height), (BYTE*)dest)))
			return dseed::error_fail;

		return dseed::error_good;
	}

public:
	virtual dseed::error_t read_pixels (const dseed::rect2i& area, void* buffer, size_t depth) noexcept override
	{
		dseed::size3i sz = this->size ();

		if (buffer == nullptr || depth != 0 || (area.y < 0 || area.y >= sz.height))
			return dseed::error_invalid_args;

		size_t stride = dseed::color::calc_bitmap_stride (format (), sz.width);

		WICRect rect = { area.x, area.y, area.width, area.height };
		if (FAILED (_bitmap->CopyPixels (&rect, (UINT)stride, (UINT)stride, (BYTE*)buffer)))
			return dseed::error_fail;

		return dseed::error_good;
	}
	virtual dseed::error_t write_pixels (const dseed::rect2i& area, const void* data, size_t depth = 0) noexcept override
	{
		return dseed::error_not_impl;
	}

public:
	virtual dseed::error_t extra_info (dseed::attributes** attr) noexcept override
	{
		if (attr == nullptr) return dseed::error_invalid_args;
		*attr = _extraInfo;
		(*attr)->retain ();
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	CComPtr<IWICBitmapSource> _bitmap;
	dseed::autoref<__windowsimagingcodec_palette> _palette;
	dseed::autoref<dseed::attributes> _extraInfo;
};

class __windowsimagingcodec_decoder : public dseed::bitmaps::bitmap_array
{
public:
	__windowsimagingcodec_decoder (IWICImagingFactoryP* factory, IWICBitmapDecoder* decoder)
		: _refCount (1), _factory (factory), _decoder (decoder)
	{

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
		UINT frameCount;
		_decoder->GetFrameCount (&frameCount);

		if (i < 0 || i >= frameCount)
			return dseed::error_invalid_args;

		CComPtr<IWICBitmapFrameDecode> decodeFrame;
		if (FAILED (_decoder->GetFrame ((UINT)i, &decodeFrame)))
			return dseed::error_fail;

		WICPixelFormatGUID formatGUID;
		decodeFrame->GetPixelFormat (&formatGUID);

		CComPtr<IWICBitmapSource> source;
		if (!(formatGUID == GUID_WICPixelFormat32bppRGBA || formatGUID == GUID_WICPixelFormat24bppRGB
			|| formatGUID == GUID_WICPixelFormat32bppBGRA || formatGUID == GUID_WICPixelFormat24bppBGR
			|| formatGUID == GUID_WICPixelFormat8bppGray || formatGUID == GUID_WICPixelFormat32bppGrayFloat
			|| formatGUID == GUID_WICPixelFormat8bppIndexed
			|| formatGUID == GUID_WICPixelFormat128bppRGBAFloat
			|| formatGUID == GUID_WICPixelFormat16bppBGR565))
		{
			CComPtr<IWICFormatConverter> fc;
			_factory->CreateFormatConverter (&fc);
			fc->Initialize (decodeFrame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom);
			formatGUID = GUID_WICPixelFormat32bppRGBA;
			source = fc;
		}
		else
			source = decodeFrame;

		CComPtr<IWICPalette> palette;
		if (formatGUID == GUID_WICPixelFormat8bppIndexed)
		{
			_factory->CreatePalette (&palette);
			source->CopyPalette (palette);
		}

		*bitmap = new __windowsimagingcodec_bitmap (source, palette);

		CComPtr<IWICMetadataQueryReader> reader;
		if (SUCCEEDED (decodeFrame->GetMetadataQueryReader (&reader)))
		{
			PROPVARIANT var = {};
			if (FAILED (reader->GetMetadataByName (L"/grctlext/Delay", &var)))
			{
				dseed::timespan duration = dseed::timespan::from_milliseconds (var.uiVal);

				dseed::autoref<dseed::attributes> attrs;
				if (dseed::succeeded ((*bitmap)->extra_info (&attrs)))
					attrs->set_int64 (dseed::attrkey_duration, duration);
			}
		}

		return dseed::error_good;
	}
	virtual size_t size () noexcept override
	{
		UINT c = 1;
		_decoder->GetFrameCount (&c);
		return c;
	}
	virtual dseed::bitmaps::arraytype type () noexcept override { return dseed::bitmaps::arraytype::plain; }

private:
	std::atomic<int32_t> _refCount;
	CComPtr<IWICImagingFactoryP> _factory;
	CComPtr<IWICBitmapDecoder> _decoder;
};

#endif

dseed::error_t dseed::bitmaps::create_windows_imaging_codec_bitmap_decoder (dseed::io::stream* stream, dseed::bitmaps::bitmap_array** decoder) noexcept
{
#if PLATFORM_MICROSOFT
	if (stream == nullptr || decoder == nullptr)
		return dseed::error_invalid_args;

	CComPtr<IWICImagingFactoryP> factory;
	bool coinit = false;
	do
	{
		if (FAILED (CoCreateInstance (CLSID_WICImagingFactoryP, nullptr, CLSCTX_ALL,
			__uuidof (IWICImagingFactoryP), (void**)&factory)))
		{
#if PLATFORM_WINDOWS
			if (coinit == false)
			{
				if (FAILED (CoInitialize (nullptr)))
					return dseed::error_platform_not_support;
				coinit = true;
				continue;
			}
			else
#endif
				return dseed::error_platform_not_support;
		}
		break;
	} while (true);

	CComPtr<IStream> istream;
	if (FAILED (ImpledIStream::Create (stream, &istream)))
		return dseed::error_fail;

	IWICBitmapDecoder* wicDecoder;
	if (FAILED (factory->CreateDecoderFromStream (istream, nullptr, WICDecodeMetadataCacheOnDemand, &wicDecoder)))
		return dseed::error_not_support;

	*decoder = new __windowsimagingcodec_decoder (factory, wicDecoder);
	if (*decoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
#else
	return dseed::error_platform_not_support;
#endif
}