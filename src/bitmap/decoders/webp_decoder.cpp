#include <dseed.h>

#if defined(USE_WEBP)
#	include <webp/decode.h>
#	include <webp/demux.h>
#endif

dseed::error_t dseed::bitmaps::create_webp_bitmap_decoder (dseed::io::stream* stream, dseed::bitmaps::bitmap_array** decoder) noexcept
{
#if defined(USE_WEBP)
	if (stream == nullptr || decoder == nullptr)
		return dseed::error_invalid_args;

	std::vector<uint8_t> bytes;
	bytes.resize (stream->length ());

	size_t totalRead = 0;
	while (totalRead != stream->length ())
	{
		size_t read = stream->read (bytes.data () + totalRead, stream->length () - totalRead);
		totalRead += read;
	}

	WebPData webpData = { bytes.data (), bytes.size () };
	WebPDemuxer* demuxer = WebPDemux (&webpData);
	if (demuxer == nullptr)
		return dseed::error_fail;

	auto width = WebPDemuxGetI (demuxer, WEBP_FF_CANVAS_WIDTH),
		height = WebPDemuxGetI (demuxer, WEBP_FF_CANVAS_HEIGHT),
		flags = WebPDemuxGetI (demuxer, WEBP_FF_FORMAT_FLAGS),
		frameCount = WebPDemuxGetI (demuxer, WEBP_FF_FRAME_COUNT);

	WebPDecoderConfig config;
	config.options.use_threads = 1;

	WebPInitDecoderConfig (&config);

	WebPIterator iter;
	if (!WebPDemuxGetFrame (demuxer, frameCount, &iter))
		return dseed::error_fail;

	dseed::size3i size (width, height, 1);
	dseed::color::pixelformat format;
	if (iter.has_alpha)
	{
		config.output.colorspace = MODE_RGBA;
		format = dseed::color::pixelformat::rgba8;
	}
	else
	{
		config.output.colorspace = MODE_RGB;
		format = dseed::color::pixelformat::rgb8;
	}
	size_t stride = dseed::color::calc_bitmap_stride (format, width);

	int index = 0;
	std::vector<dseed::bitmaps::bitmap*> bitmaps;
	bitmaps.resize (frameCount);

	do
	{
		if (WebPDecode (iter.fragment.bytes, iter.fragment.size, &config) == VP8_STATUS_OK)
		{
			timespan duration = dseed::timespan::from_milliseconds (iter.duration);

			dseed::autoref<dseed::bitmaps::bitmap> bitmap;
			dseed::bitmaps::create_bitmap (dseed::bitmaps::bitmaptype::bitmap2d, size, format, nullptr, &bitmap);
			
			uint8_t* ptr;
			if (dseed::failed (bitmap->lock ((void**)&ptr)))
				continue;

			if (format == color::pixelformat::rgba8)
			{
				memcpy (ptr, config.output.u.RGBA.rgba, stride * height);
			}
			else
			{
				for (size_t y = 0; y < height; ++y)
				{
					memcpy (ptr + (y * stride),
						config.output.u.RGBA.rgba + (y * config.output.u.RGBA.stride),
						config.output.u.RGBA.stride);
				}
			}

			bitmap->unlock ();

			dseed::autoref<dseed::attributes> attr;
			bitmap->extra_info(&attr);
			attr->set_int64(dseed::attrkey_duration, duration.ticks());

			bitmaps[index] = bitmap.detach();
		}
		++index;
	} while (WebPDemuxNextFrame (&iter));

	WebPDemuxReleaseIterator (&iter);
	WebPDemuxDelete (demuxer);

	return create_bitmap_array(arraytype::plain, bitmaps, decoder);
#else
	return dseed::error_not_support;
#endif
}