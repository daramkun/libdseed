#include <dseed.h>

#if defined(USE_WEBP) && defined(USE_BITMAP_ENCODERS)
#	include <webp/encode.h>
#	include <webp/mux.h>
#	include <functional>

class __webp_encoder : public dseed::bitmaps::bitmap_encoder
{
public:
	__webp_encoder(dseed::io::stream* stream, int quality, bool lossless)
		: _refCount(1), _stream(stream), _config({}), _anim(nullptr)
	{
		memset(&_config, 0, sizeof(_config));

		WebPConfigInit(&_config);

		_config.quality = (float)quality;
		_config.lossless = lossless;
		_config.method = 6;

		_config.thread_level = 1;
	}
	~__webp_encoder()
	{
		if (_anim)
			WebPAnimEncoderDelete(_anim);
		_anim = nullptr;
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
		auto format = bitmap->format();
		if (!(format == dseed::color::pixelformat::rgba8 || format == dseed::color::pixelformat::rgb8
			|| format == dseed::color::pixelformat::bgra8 || format == dseed::color::pixelformat::bgr8))
			return dseed::error_not_support;

		dseed::size3i size = bitmap->size();
		if (size.depth > 1)
			return dseed::error_not_support;

		if (_anim == nullptr)
		{
			WebPAnimEncoderOptions options = {};
			WebPAnimEncoderOptionsInit(&options);
			_anim = WebPAnimEncoderNew(size.width, size.height, &options);
			if (_anim == nullptr)
				return dseed::error_fail;
		}

		WebPPicture picture;
		memset(&picture, 0, sizeof(picture));

		picture.width = size.width;
		picture.height = size.height;
		picture.colorspace = WEBP_YUV420;

		WebPPictureAlloc(&picture);
		//setting_picture (picture);

		size_t stride = dseed::color::calc_bitmap_stride(format, size.width);
		std::function<int(WebPPicture*, const uint8_t*, int)> importPixels;
		if (format == dseed::color::pixelformat::rgba8)
		{
			importPixels = WebPPictureImportRGBA;
			picture.use_argb = true;
		}
		else if (format == dseed::color::pixelformat::bgra8)
		{
			importPixels = WebPPictureImportBGRA;
			picture.use_argb = true;
		}
		else if (format == dseed::color::pixelformat::rgb8)
			importPixels = WebPPictureImportRGB;
		else if (format == dseed::color::pixelformat::bgr8)
			importPixels = WebPPictureImportBGR;

		void* ptr;
		bitmap->lock(&ptr);
		importPixels(&picture, (uint8_t*)ptr, (int)stride);
		bitmap->unlock();

		dseed::autoref<dseed::attributes> attr;
		bitmap->extra_info(&attr);

		dseed::timespan duration;
		if (dseed::failed(attr->get_int64(dseed::attrkey_duration, (int64_t*)&duration)))
			duration = 0;

		int ms = (int)duration.total_milliseconds();
		if (!WebPAnimEncoderAdd(_anim, &picture, ms, &_config))
			return dseed::error_fail;

		WebPPictureFree(&picture);

		return dseed::error_good;
	}
	virtual dseed::error_t commit() noexcept override
	{
		WebPAnimEncoderAdd(_anim, nullptr, 0, &_config);
		WebPData webpData = {};
		if (WebPAnimEncoderAssemble(_anim, &webpData))
		{
			size_t totalWritten = 0;
			while (totalWritten != webpData.size)
			{
				auto written = _stream->write(webpData.bytes + totalWritten, webpData.size - totalWritten);
				totalWritten += written;
			}
		}
		else return dseed::error_fail;

		_stream->flush();
		return dseed::error_good;
	}
	virtual dseed::bitmaps::arraytype type() noexcept override { return dseed::bitmaps::arraytype::plain; }

private:
	std::atomic<int32_t> _refCount;
	dseed::autoref<dseed::io::stream> _stream;
	WebPConfig _config;
	WebPAnimEncoder* _anim;
};
#endif

dseed::error_t dseed::bitmaps::create_webp_bitmap_encoder(dseed::io::stream* stream, const dseed::bitmaps::bitmap_encoder_options* options, dseed::bitmaps::bitmap_encoder** encoder)
{
#if defined(USE_WEBP) && defined(USE_BITMAP_ENCODERS)
	webp_encoder_options webpOptions;
	if (options != nullptr)
	{
		if (options->options_size != sizeof(webp_encoder_options))
			return dseed::error_invalid_args;
		webpOptions = *reinterpret_cast<const webp_encoder_options*>(options);
	}
	*encoder = new __webp_encoder(stream, webpOptions.quality, webpOptions.lossless);
	if (*encoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}