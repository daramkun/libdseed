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

class __wic_encoder : public dseed::bitmaps::bitmap_encoder
{
public:
	__wic_encoder(IWICImagingFactoryP* factory, IWICBitmapEncoder* encoder, const dseed::bitmaps::wic_encoder_options* options)
		: _refCount(1), _factory(factory), _encoder(encoder)
	{
		if (options)
		{
			_haveOptions = true;
			_options = *options;
		}
		else
			_haveOptions = false;
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
		dseed::size3i size = bitmap->size();
		dseed::color::pixelformat format = bitmap->format();
		size_t stride = dseed::color::calc_bitmap_stride(format, size.width);

		if (size.depth > 1)
			return dseed::error_not_support;

		Microsoft::WRL::ComPtr<IWICBitmapFrameEncode> encodeFrame;
		Microsoft::WRL::ComPtr<IPropertyBag2> encoderOptions;
		if (FAILED(_encoder->CreateNewFrame(&encodeFrame, &encoderOptions)))
			return dseed::error_fail;

		if (_haveOptions)
		{
			switch (_options.format)
			{
			case dseed::bitmaps::wic_encoder_format::heif:
			{
				VARIANT varValue;

				PROPBAG2 imageQualityOption = { 0 };
				imageQualityOption.pstrName = L"ImageQuality";
				VariantInit(&varValue);
				varValue.vt = VT_R4;
				varValue.fltVal = _options.quality / 100.0f;
				encoderOptions->Write(1, &imageQualityOption, &varValue);
				break;
			}

			case dseed::bitmaps::wic_encoder_format::jpeg:
			{
				VARIANT varValue;

				PROPBAG2 imageQualityOption = { 0 };
				imageQualityOption.pstrName = L"ImageQuality";
				VariantInit(&varValue);
				varValue.vt = VT_R4;
				varValue.fltVal = _options.quality / 100.0f;
				encoderOptions->Write(1, &imageQualityOption, &varValue);
				break;
			}

			case dseed::bitmaps::wic_encoder_format::tiff:
			{
				VARIANT varValue;

				PROPBAG2 compressionMethodOption = { 0 };
				compressionMethodOption.pstrName = L"TiffCompressionMethod";
				VariantInit(&varValue);
				varValue.vt = VT_UI1;
				varValue.bVal = WICTiffCompressionDontCare;
				encoderOptions->Write(1, &compressionMethodOption, &varValue);

				PROPBAG2 compressionQualityOption = { 0 };
				compressionQualityOption.pstrName = L"CompressionQuality";
				VariantInit(&varValue);
				varValue.vt = VT_R4;
				varValue.fltVal = _options.compressionQuality / 100.0f;
				encoderOptions->Write(2, &compressionQualityOption, &varValue);
				break;
			}

			case dseed::bitmaps::wic_encoder_format::png:
			{
				VARIANT varValue;

				PROPBAG2 filterOption = { 0 };
				filterOption.pstrName = L"FilterOption";
				VariantInit(&varValue);
				varValue.vt = VT_UI1;
				varValue.bVal = WICPngFilterAdaptive;
				encoderOptions->Write(1, &filterOption, &varValue);
				break;
			}

			case dseed::bitmaps::wic_encoder_format::wmp:
			{
				VARIANT varValue;

				PROPBAG2 imageQualityOption = { 0 };
				imageQualityOption.pstrName = L"ImageQuality";
				VariantInit(&varValue);
				varValue.vt = VT_R4;
				varValue.fltVal = _options.quality / 100.0f;
				encoderOptions->Write(1, &imageQualityOption, &varValue);

				PROPBAG2 losslessOption = { 0 };
				losslessOption.pstrName = L"Lossless";
				VariantInit(&varValue);
				varValue.vt = VT_BOOL;
				varValue.boolVal = _options.lossless;
				encoderOptions->Write(2, &losslessOption, &varValue);
				break;
			}

			default:
				break;
			}
		}

		if (FAILED(encodeFrame->Initialize(encoderOptions.Get())))
			return dseed::error_fail;

		if (FAILED(encodeFrame->SetSize(size.width, size.height)))
			return dseed::error_fail;

		GUID pixelFormat;
		switch (format)
		{
		case dseed::color::pixelformat::rgb8: pixelFormat = GUID_WICPixelFormat24bppRGB; break;
		case dseed::color::pixelformat::rgba8: pixelFormat = GUID_WICPixelFormat32bppRGBA; break;
		case dseed::color::pixelformat::rgbaf: pixelFormat = GUID_WICPixelFormat128bppRGBAFloat; break;
		case dseed::color::pixelformat::bgr8: pixelFormat = GUID_WICPixelFormat24bppBGR; break;
		case dseed::color::pixelformat::bgra8: pixelFormat = GUID_WICPixelFormat32bppBGRA; break;
		case dseed::color::pixelformat::bgr565: pixelFormat = GUID_WICPixelFormat16bppBGR565; break;
		case dseed::color::pixelformat::r8: pixelFormat = GUID_WICPixelFormat8bppGray; break;
		case dseed::color::pixelformat::rf: pixelFormat = GUID_WICPixelFormat32bppGrayFloat; break;
		case dseed::color::pixelformat::bgra8_indexed8: pixelFormat = GUID_WICPixelFormat8bppIndexed; break;
		default: return dseed::error_invalid_args;
		}

		if (FAILED(encodeFrame->SetPixelFormat(&pixelFormat)))
			return dseed::error_fail;

		if (bitmap->format() == dseed::color::pixelformat::bgra8_indexed8)
		{
			dseed::autoref<dseed::bitmaps::palette> palette;
			if(dseed::failed(bitmap->palette(&palette)))
				return dseed::error_invalid_args;

			Microsoft::WRL::ComPtr<IWICPalette> wicPalette;
			if (FAILED(_factory->CreatePalette(&wicPalette)))
				return dseed::error_fail;

			dseed::color::bgra8 colors[256];
			palette->lock(reinterpret_cast<void**>(&colors));
			wicPalette->InitializeCustom(reinterpret_cast<WICColor*>(colors), 256);
			palette->unlock();

			if (FAILED(encodeFrame->SetPalette(wicPalette.Get())))
				return dseed::error_fail;
		}

		std::vector<uint8_t> pixels(stride* size.height);
		if (dseed::failed(bitmap->copy_pixels(pixels.data(), 1)))
			return dseed::error_fail;
		
		if (FAILED(encodeFrame->WritePixels(size.height, stride, stride * size.height, pixels.data())))
			return dseed::error_fail;

		if (_options.format == dseed::bitmaps::wic_encoder_format::gif)
		{
			dseed::autoref<dseed::attributes> attr;
			if (dseed::succeeded(bitmap->extra_info(&attr)))
			{
				dseed::timespan duration;
				if (dseed::succeeded(attr->get_int64(dseed::attrkey_duration, reinterpret_cast<int64_t*>(&duration))))
				{
					int delay = static_cast<int>(duration.total_milliseconds() / 10);

					Microsoft::WRL::ComPtr<IWICMetadataQueryWriter> queryWriter;
					if(SUCCEEDED(encodeFrame->GetMetadataQueryWriter(&queryWriter)))
					{
						PROPVARIANT var = {};
						var.vt = VT_UI2;
						var.uiVal = static_cast<uint16_t>(delay);
						queryWriter->SetMetadataByName(L"/grctlext/Delay", &var);
					}
				}
			}
		}

		if (FAILED(encodeFrame->Commit()))
			return dseed::error_fail;

		return dseed::error_good;
	}

public:
	virtual dseed::error_t commit() noexcept override
	{
		if (FAILED(_encoder->Commit()))
			return dseed::error_fail;

		return dseed::error_good;
	}
	virtual dseed::bitmaps::arraytype type() noexcept override { return dseed::bitmaps::arraytype::plain; }

private:
	std::atomic<int32_t> _refCount;
	Microsoft::WRL::ComPtr<IWICImagingFactoryP> _factory;
	Microsoft::WRL::ComPtr<IWICBitmapEncoder> _encoder;

	bool _haveOptions;
	dseed::bitmaps::wic_encoder_options _options;
};

#endif

dseed::error_t dseed::bitmaps::create_wic_bitmap_encoder(dseed::io::stream* stream, const dseed::bitmaps::bitmap_encoder_options* options, dseed::bitmaps::bitmap_encoder** encoder)
{
#if PLATFORM_MICROSOFT
	const dseed::bitmaps::wic_encoder_options* wicOptions;
	if (options != nullptr)
	{
		if (options->option_type != dseed::bitmaps::bitmap_encoder_options_for_wic)
			return dseed::error_invalid_args;
		wicOptions = reinterpret_cast<const dseed::bitmaps::wic_encoder_options*>(options);
	}

	Microsoft::WRL::ComPtr<IWICImagingFactoryP> factory;
	bool coinit = false;
	do
	{
		if (FAILED(CoCreateInstance(CLSID_WICImagingFactoryP, nullptr, CLSCTX_ALL,
			__uuidof (IWICImagingFactoryP), static_cast<void**>(&factory))))
		{
#if PLATFORM_WINDOWS
			if (coinit == false)
			{
				if (FAILED(CoInitialize(nullptr)))
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

	Microsoft::WRL::ComPtr<IStream> istream;
	if (FAILED(ImpledIStream::Create(stream, &istream)))
		return dseed::error_fail;

	GUID containerFormat;
	if (options != nullptr)
	{
		switch (wicOptions->format)
		{
		case wic_encoder_format::bmp:
			containerFormat = CLSID_WICBmpEncoder;
			break;

		case wic_encoder_format::gif:
			containerFormat = CLSID_WICGifEncoder;
			break;

		case wic_encoder_format::heif:
			containerFormat = CLSID_WICHeifEncoder;
			break;

		case wic_encoder_format::jpeg:
			containerFormat = CLSID_WICJpegEncoder;
			break;

		case wic_encoder_format::tiff:
			containerFormat = CLSID_WICTiffEncoder;
			break;

		case wic_encoder_format::png:
			containerFormat = CLSID_WICPngEncoder;
			break;

		case wic_encoder_format::wmp:
			containerFormat = CLSID_WICWmpEncoder;
			break;

		default:
			return dseed::error_invalid_args;
		}
	}
	else
		containerFormat = CLSID_WICPngEncoder;

	Microsoft::WRL::ComPtr<IWICBitmapEncoder> wicEncoder;
	if (FAILED(factory->CreateEncoder(containerFormat, nullptr, &wicEncoder)))
		return dseed::error_not_support;

	if (FAILED(wicEncoder->Initialize(istream.Get(), WICBitmapEncoderNoCache)))
		return dseed::error_fail;

	*encoder = new __wic_encoder(factory.Get(), wicEncoder.Get(), reinterpret_cast<const dseed::bitmaps::wic_encoder_options*>(options));
	if (*encoder == nullptr)
		return dseed::error_out_of_memory;
	return dseed::error_good;
#else
	return dseed::error_platform_not_support;
#endif
}