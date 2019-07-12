#include <dseed.h>

int main (int argc, char* argv[])
{
	dseed::auto_object<dseed::stream> inputStream;
	if (dseed::failed (dseed::create_native_filestream (
		u8"..\\..\\..\\sample\\bitmaps\\sample1.etc1.ktx", false, &inputStream)))
		return -1;

	dseed::auto_object<dseed::bitmap_decoder> decoder;
	if (dseed::failed (dseed::create_ktx_bitmap_decoder (inputStream, &decoder)))
		return -2;

	{
		dseed::auto_object<dseed::stream> outputStream;
		if (dseed::failed (dseed::create_native_filestream (u8"output.webp", true, &outputStream)))
			return -3;
		dseed::auto_object<dseed::bitmap_encoder> encoder;
		if (dseed::failed (dseed::create_webp_bitmap_encoder (outputStream, nullptr, &encoder)))
			return -4;

		for (int i = 0; i < /*decoder->frame_count ()*/1; ++i)
		{
			dseed::auto_object<dseed::bitmap> bitmap;
			dseed::timespan_t duration;
			if (dseed::failed (decoder->decode_frame (i, &bitmap, &duration)))
				return -5;

			dseed::timespan_t start = dseed::timespan_t::current_ticks ();
			dseed::auto_object<dseed::bitmap> reformated;
			if (dseed::failed (dseed::reformat_bitmap (bitmap, dseed::pixelformat_rgba8888, &reformated)))
				return -6;
			//dseed::auto_object<morisot::bitmap> reformated2;
			//if (dseed::failed (morisot::create_bitmap_reformat (reformated, morisot::pixelformat_bgr888_indexed8, true, &reformated2)))
			//	return -6;
			dseed::timespan_t end = dseed::timespan_t::current_ticks ();
			printf ("Reformat time: %lf\n", (end - start).total_seconds ());

			{
				if (dseed::failed (encoder->encode_frame (/**/reformated/**//*bitmap/**/, duration)))
					return -7;
			}
		}

		if (dseed::failed (encoder->commit ()))
			return -8;
	}

	return 0;
}