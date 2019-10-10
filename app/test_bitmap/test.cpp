#include <dseed.h>

int main (int argc, char* argv[])
{
	dseed::auto_object<dseed::stream> inputStream;
	if (dseed::failed (dseed::create_native_filestream (
		u8"..\\..\\..\\sample\\bitmaps\\sample4.gif", false, &inputStream)))
		return -1;

	dseed::auto_object<dseed::bitmap_decoder> decoder;
	//if (dseed::failed (dseed::create_png_bitmap_decoder (inputStream, &decoder)))
	//	return -2;
	if (dseed::failed (dseed::detect_bitmap_decoder (inputStream, &decoder)))
		return -2;

	{
		dseed::auto_object<dseed::stream> outputStream;
		if (dseed::failed (dseed::create_native_filestream (u8"output.jpg", true, &outputStream)))
			return -3;
		dseed::auto_object<dseed::bitmap_encoder> encoder;
		if (dseed::failed (dseed::create_jpeg_bitmap_encoder (outputStream, nullptr, &encoder)))
			return -4;

		for (int i = 0; i < /*decoder->frame_count ()*/1; ++i)
		{
			dseed::auto_object<dseed::bitmap> bitmap;
			dseed::timespan_t duration;
			if (dseed::failed (decoder->decode_frame (i, &bitmap, &duration)))
				return -5;

			dseed::bitmap_filter_mask mask;
			dseed::bitmap_filter_mask::unsharpmask_5 (&mask);

			dseed::timespan_t start = dseed::timespan_t::current_ticks ();

			dseed::colorcount_t colours;
			if (dseed::succeeded (dseed::bitmap_get_total_colours (bitmap, &colours)))
			{
				printf ("Colours: %d\n", colours);
			}
			
			dseed::auto_object<dseed::bitmap> reformated;
			if (dseed::failed (dseed::reformat_bitmap (bitmap, dseed::pixelformat::bgra8888, &reformated)))
				return -6;
			
			dseed::auto_object<dseed::bitmap> reformated2;
			if (dseed::failed (dseed::resize_bitmap (reformated, dseed::resize_lanczos, dseed::size3i (600, 600, 1), &reformated2)))
				return -7;
			
			dseed::auto_object<dseed::bitmap> reformated3;
			if (dseed::failed (dseed::filter_bitmap (reformated2, mask, &reformated3)))
				return -8;
			
			dseed::auto_object<dseed::bitmap> reformated4;
			if (dseed::failed (dseed::reformat_bitmap (reformated3, dseed::pixelformat::hsva8888, &reformated4)))
				return -9;

			dseed::auto_object<dseed::bitmap> reformated5;
			if (dseed::failed (dseed::bitmap_auto_histogram_equalization (reformated4, dseed::histogram_color_third, 0, &reformated5)))
				return -10;

			dseed::auto_object<dseed::bitmap> reformated6;
			if (dseed::failed (dseed::reformat_bitmap (reformated5, dseed::pixelformat::rgba8888, &reformated6)))
				return -11;

			dseed::auto_object<dseed::bitmap> reformated7;
			bool grayscale = false;
			if (dseed::succeeded (dseed::bitmap_detect_grayscale_bitmap (reformated6, &grayscale)) && grayscale)
			{
				if (dseed::failed (dseed::reformat_bitmap (reformated6, dseed::pixelformat::grayscale8, &reformated7)))
					return -12;
			}
			else
			{
				reformated7.store (reformated6);
			}
			
			dseed::timespan_t end = dseed::timespan_t::current_ticks ();
			printf ("Reformat time: %lf\n", (end - start).total_seconds ());

			{
				if (dseed::failed (encoder->encode_frame (/**/reformated7/**//*bitmap/**/, duration)))
					return -12;
			}
		}

		if (dseed::failed (encoder->commit ()))
			return -8;
	}

	return 0;
}