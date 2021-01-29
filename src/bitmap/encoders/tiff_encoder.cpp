#include <dseed.h>

#if defined(USE_TIFF) && defined(USE_BITMAP_ENCODERS)
#	include <tiff.h>
#	include <tiffio.h>

class __tiff_encoder : public dseed::bitmaps::bitmap_encoder
{
public:
	__tiff_encoder(dseed::io::stream* stream)
		: _refCount(1), _stream(stream), _tiff(nullptr), _writtenBitmap(false)
	{
		_tiff = TIFFClientOpen("Stream", "w", stream,
			// Read
			[](thandle_t handle, tdata_t data, tsize_t sz) -> tsize_t
			{
				return reinterpret_cast<dseed::io::stream*>(handle)->read(data, sz);
			},
			// Write
				[](thandle_t handle, tdata_t data, tsize_t sz) -> tsize_t
			{
				return reinterpret_cast<dseed::io::stream*>(handle)->write(data, sz);
			},
				// Seek
				[](thandle_t handle, toff_t offset, int origin) -> toff_t
			{
				auto stream = reinterpret_cast<dseed::io::stream*>(handle);
				if (!stream->seek((dseed::io::seekorigin)origin, offset))
					return -1;
				return stream->position();
			},
				// Close
				[](thandle_t handle) -> int { return 1; },
				// Size
				[](thandle_t handle) -> toff_t { return reinterpret_cast<dseed::io::stream*>(handle)->length(); },
				// Map
				[](thandle_t, tdata_t*, toff_t*) -> int { return 0; },
				// Unmap
				[](thandle_t, tdata_t, toff_t) {}
			);

	}

	~__tiff_encoder() override
	{
		if (_tiff)
			TIFFClose(_tiff);
		_tiff = nullptr;
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
		if (_writtenBitmap)
			return dseed::error_invalid_op;
		
		const auto format = bitmap->format();
		
		int photometric, samplesPerPixel, bitsPerSample, sampleFormat;
		switch (format)
		{
		case dseed::color::pixelformat::rgb8:
			photometric = PHOTOMETRIC_RGB;
			samplesPerPixel = 3;
			bitsPerSample = 8;
			sampleFormat = SAMPLEFORMAT_UINT;
			break;

		case dseed::color::pixelformat::rgba8:
			photometric = PHOTOMETRIC_RGB;
			samplesPerPixel = 4;
			bitsPerSample = 8;
			sampleFormat = SAMPLEFORMAT_UINT;
			break;

		case dseed::color::pixelformat::rgbaf:
			photometric = PHOTOMETRIC_RGB;
			samplesPerPixel = 4;
			bitsPerSample = 32;
			sampleFormat = SAMPLEFORMAT_IEEEFP;
			break;

		case dseed::color::pixelformat::r8:
			photometric = PHOTOMETRIC_MINISBLACK;
			samplesPerPixel = 1;
			bitsPerSample = 8;
			sampleFormat = SAMPLEFORMAT_UINT;
			break;

		case dseed::color::pixelformat::rf:
			photometric = PHOTOMETRIC_MINISBLACK;
			samplesPerPixel = 1;
			bitsPerSample = 32;
			sampleFormat = SAMPLEFORMAT_IEEEFP;
			break;

		case dseed::color::pixelformat::yuv8:
			photometric = PHOTOMETRIC_YCBCR;
			samplesPerPixel = 3;
			bitsPerSample = 8;
			sampleFormat = SAMPLEFORMAT_UINT;
			break;

		default:
			return dseed::error_not_support;
		}

		const auto size = bitmap->size();
		const auto stride = dseed::color::calc_bitmap_stride(format, size.width);
		TIFFSetField(_tiff, TIFFTAG_IMAGEWIDTH, size.width);
		TIFFSetField(_tiff, TIFFTAG_IMAGELENGTH, size.height);
		TIFFSetField(_tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(_tiff, TIFFTAG_PHOTOMETRIC, photometric);
		TIFFSetField(_tiff, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
		TIFFSetField(_tiff, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel);
		TIFFSetField(_tiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(_tiff, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
		TIFFSetField(_tiff, TIFFTAG_SAMPLEFORMAT, sampleFormat);
		TIFFSetField(_tiff, TIFFTAG_THRESHHOLDING, 1);
		TIFFSetField(_tiff, TIFFTAG_XRESOLUTION, 96);
		TIFFSetField(_tiff, TIFFTAG_YRESOLUTION, 96);
		TIFFSetField(_tiff, TIFFTAG_RESOLUTIONUNIT, 2);

		uint32 rowsPerStrip = size.height;
		rowsPerStrip = TIFFDefaultStripSize(_tiff, rowsPerStrip);
		TIFFSetField(_tiff, TIFFTAG_ROWSPERSTRIP, rowsPerStrip);
		TIFFSetupStrips(_tiff);

		bool error = false;
		uint8_t* ptr;
		bitmap->lock(reinterpret_cast<void**>(&ptr));
		for (auto y = 0; y < size.height; ++y)
		{
			const auto offset = y * stride;
			if (TIFFWriteScanline(_tiff, ptr + offset, y, 0) != 1)
			{
				error = true;
				break;
			}
		}
		bitmap->unlock();

		return !error ? dseed::error_good : dseed::error_fail;
	}
	virtual dseed::error_t commit() noexcept override
	{
		if (!_writtenBitmap)
			return dseed::error_invalid_op;
		
		_stream->flush();
		return dseed::error_good;
	}
	virtual dseed::bitmaps::arraytype type() noexcept override { return dseed::bitmaps::arraytype::plain; }

private:
	std::atomic<int32_t> _refCount;
	dseed::autoref<dseed::io::stream> _stream;

	TIFF* _tiff;
	bool _writtenBitmap;
};

#endif

dseed::error_t dseed::bitmaps::create_tiff_bitmap_encoder(dseed::io::stream* stream, const dseed::bitmaps::bitmap_encoder_options* options, dseed::bitmaps::bitmap_encoder** encoder)
{
#if defined(USE_TIFF) && defined(USE_BITMAP_ENCODERS)
	*encoder = new __tiff_encoder(stream);
	if (*encoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}