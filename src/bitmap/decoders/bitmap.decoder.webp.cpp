#include <dseed.h>

#if defined ( USE_WEBP )
#	include <webp/decode.h>
#	include <webp/demux.h>
#	include "bitmap.decoder.hxx"
#endif

dseed::error_t dseed::create_webp_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder)
{
#if defined ( USE_WEBP )
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
	dseed::pixelformat_t format;
	if (iter.has_alpha)
	{
		config.output.colorspace = MODE_RGBA;
		format = dseed::pixelformat_rgba8888;
	}
	else
	{
		config.output.colorspace = MODE_RGB;
		format = dseed::pixelformat_rgb888;
	}
	size_t stride = dseed::get_bitmap_stride (format, width);

	int index = 0;
	std::vector<std::tuple<dseed::bitmap*, dseed::timespan_t>> bitmaps;
	bitmaps.resize (frameCount);

	do
	{
		if (WebPDecode (iter.fragment.bytes, iter.fragment.size, &config) == VP8_STATUS_OK)
		{
			timespan_t duration = dseed::timespan_t::from_milliseconds (iter.duration);
			
			dseed::auto_object<dseed::bitmap> bitmap;
			dseed::create_bitmap (dseed::bitmaptype_2d, size, format, nullptr, &bitmap);
			uint8_t* ptr;
			if (dseed::failed (bitmap->pixels_pointer ((void**)& ptr)))
				continue;

			if (format == pixelformat_rgba8888)
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

			bitmaps[index] = std::tuple<dseed::bitmap*, dseed::timespan_t> (bitmap.detach (), duration);
		}
		++index;
	} while (WebPDemuxNextFrame (&iter));

	WebPDemuxReleaseIterator (&iter);
	WebPDemuxDelete (demuxer);

	*decoder = new __common_bitmap_decoder (bitmaps);
	if (*decoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}
