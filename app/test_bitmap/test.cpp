#include <dseed.h>

int test_split_and_join_bitmap_pixel_element()
{
	dseed::autoref<dseed::io::stream> inputStream;
	if (dseed::failed(dseed::io::create_native_filestream(u8"../../../sample/bitmaps/sample1.jpg", false, &inputStream)))
	{
		return -1;
	}

	dseed::autoref<dseed::bitmaps::bitmap_array> decoder;
	if (dseed::failed(dseed::bitmaps::detect_bitmap_decoder(inputStream, &decoder)))
	{
		return -2;
	}

	dseed::autoref<dseed::bitmaps::bitmap> bitmap;
	if (dseed::failed(decoder->at(0, &bitmap)))
	{
		return -3;
	}

	dseed::autoref<dseed::bitmaps::bitmap> r, g, b;
	if (dseed::failed(dseed::bitmaps::bitmap_split_rgb_elements(bitmap, &r, &g, &b, nullptr)))
	{
		return -4;
	}

	dseed::autoref<dseed::bitmaps::bitmap> rgb;
	if (dseed::failed(dseed::bitmaps::bitmap_join_rgb_elements(b, g, r, nullptr, &rgb)))
	{
		return -5;
	}

	dseed::autoref<dseed::io::stream> outputStream;
	if (dseed::failed(dseed::io::create_native_filestream(u8"output.tif", true, &outputStream)))
	{
		return -6;
	}

	dseed::autoref<dseed::bitmaps::bitmap_encoder> encoder;
	if (dseed::failed(dseed::bitmaps::create_tiff_bitmap_encoder(outputStream, nullptr, &encoder)))
	{
		return -7;
	}

	encoder->encode_frame(rgb);

	encoder->commit();

	return 0;
}

int test_histogram_euqualization()
{
	dseed::autoref<dseed::io::stream> inputStream;
	if (dseed::failed(dseed::io::create_native_filestream(u8"../../../sample/bitmaps/sample9.green.png", false, &inputStream)))
	{
		return -1;
	}

	dseed::autoref<dseed::bitmaps::bitmap_array> decoder;
	if (dseed::failed(dseed::bitmaps::detect_bitmap_decoder(inputStream, &decoder)))
	{
		return -2;
	}

	dseed::autoref<dseed::bitmaps::bitmap> bitmap;
	if (dseed::failed(decoder->at(0, &bitmap)))
	{
		return -3;
	}

	dseed::autoref<dseed::bitmaps::bitmap> hsv;
	if (dseed::failed(dseed::bitmaps::reformat_bitmap(bitmap, dseed::color::pixelformat::hsv8, &hsv)))
	{
		return -4;
	}

	dseed::autoref<dseed::bitmaps::bitmap> eq;
	if (dseed::failed(dseed::bitmaps::bitmap_auto_histogram_equalization(hsv, dseed::bitmaps::histogram_color::third, 0, &eq)))
	{
		return -6;
	}

	dseed::autoref<dseed::bitmaps::bitmap> rgb;
	if (dseed::failed(dseed::bitmaps::reformat_bitmap(eq, dseed::color::pixelformat::rgb8, &rgb)))
	{
		return -6;
	}

	dseed::autoref<dseed::io::stream> outputStream;
	if (dseed::failed(dseed::io::create_native_filestream(u8"output.tif", true, &outputStream)))
	{
		return -5;
	}

	dseed::autoref<dseed::bitmaps::bitmap_encoder> encoder;
	if (dseed::failed(dseed::bitmaps::create_tiff_bitmap_encoder(outputStream, nullptr, &encoder)))
	{
		return -6;
	}

	encoder->encode_frame(rgb);

	encoder->commit();

	return 0;
}

int main(int argc, char* argv[])
{
#if COMPILER_MSVC
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	
	//return test_split_and_join_bitmap_pixel_element();
	return test_histogram_euqualization();
}
