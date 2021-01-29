#include <dseed.h>

int main(int argc, char* argv[])
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