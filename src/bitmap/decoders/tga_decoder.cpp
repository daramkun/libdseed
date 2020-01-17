#include <dseed.h>

#include "common.hxx"

#if COMPILER_MSVC
#	pragma pack (push, 1)
#else
#	pragma pack (1)
#endif

struct TGAHeader
{
	uint8_t id_len;
	uint8_t map_t;
	uint8_t img_t;
	uint16_t map_first;
	uint16_t map_len;
	uint8_t map_entry;
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	uint8_t depth;
	uint8_t alpha;
	uint8_t horz;
	uint8_t vert;
};

#if COMPILER_MSVC
#	pragma pack (pop)
#else
#	pragma pack ()
#endif

#define TGA_BOTTOM	0x0
#define TGA_TOP		0x1
#define TGA_LEFT	0x0
#define TGA_RIGHT	0x1

dseed::error_t dseed::bitmaps::create_tga_bitmap_decoder (dseed::io::stream* stream, dseed::bitmaps::bitmap_array** decoder) noexcept
{
	uint8_t headerBytes[18];
	stream->read (headerBytes, 18);

	TGAHeader header;
	header.id_len = headerBytes[0];
	header.map_t = headerBytes[1];
	header.img_t = headerBytes[2];
	header.map_first = headerBytes[3] + ((uint16_t)headerBytes[4]) * 256;
	header.map_len = headerBytes[5] + ((uint16_t)headerBytes[6]) * 256;
	header.map_entry = headerBytes[7];
	header.x = headerBytes[8] + ((uint16_t)headerBytes[9]) * 256;
	header.y = headerBytes[10] + ((uint16_t)headerBytes[11]) * 256;
	header.width = headerBytes[12] + ((uint16_t)headerBytes[13]) * 256;
	header.height = headerBytes[14] + ((uint16_t)headerBytes[15]) * 256;
	header.depth = headerBytes[16];
	header.alpha = headerBytes[17] & 0x0f;
	header.horz = (headerBytes[17] & 0x10) ? TGA_RIGHT : TGA_LEFT;
	header.vert = (headerBytes[17] & 0x20) ? TGA_TOP : TGA_BOTTOM;

	if (header.map_t == 1 && header.depth != 8)
		return dseed::error_fail;

	if (header.depth != 8 && header.depth != 15 && header.depth != 16 &&
		header.depth != 24 && header.depth != 32)
		return dseed::error_fail;

	dseed::color::pixelformat format = dseed::color::pixelformat::unknown;

	std::vector<uint8_t> map;
	size_t mapSize = header.map_len * header.map_entry / 8;
	if (mapSize != 0)
	{
		size_t mapOffset = 18 + header.id_len;
		stream->seek (dseed::io::seekorigin::begin, mapOffset);

		map.resize (mapSize);
		stream->read (map.data (), mapSize);

		if (header.map_entry != 24 && header.map_entry != 32)
		{
			std::vector<uint8_t> remap (map);
			map.resize (header.map_len * 3);

			// TODO
			return dseed::error_not_support;
		}

		switch (header.map_entry)
		{
		case 24: format = dseed::color::pixelformat::bgr8_indexed8; break;
		case 32: format = dseed::color::pixelformat::bgra8_indexed8; break;
		}
	}
	else
	{
		switch (header.depth)
		{
		case 15:
		case 16:
		case 24: format = dseed::color::pixelformat::bgr8; break;
		case 32: format = dseed::color::pixelformat::bgra8; break;
		}
	}

	size_t scanlineSize = header.width * header.depth / 8;
	std::vector<uint8_t> pixels (scanlineSize * header.height);
	size_t scanlineOffset = 18 + header.id_len + mapSize;
	stream->seek (dseed::io::seekorigin::begin, scanlineOffset);
	if ((header.img_t & 0x8) == 0x8) //< RLE Encoded
	{
		for (int y = 0; y < header.height; ++y)
		{
			uint8_t packet_head;
			const uint8_t sample_bytes = header.depth / 8;
			std::vector<uint8_t> sample (sample_bytes);
			uint8_t repetition = 0;
			uint8_t raw = 0;

			uint8_t* buf = pixels.data () + ((header.height - y - 1) * scanlineSize);
			for (int x = 0; x < header.width; ++x)
			{
				if (repetition == 0 && raw == 0) {
					stream->read (&packet_head, 1);
					if (packet_head & 0x80) {
						repetition = 1 + (packet_head & 0x7f);
						stream->read (sample.data (), sample_bytes);
					}
					else
						raw = packet_head + 1;
				}
				if (repetition > 0) {
					for (uint8_t k = 0; k < sample_bytes; ++k)
						buf[k] = sample[k];
					--repetition;
				}
				else {
					stream->read (sample.data (), sample_bytes);
					for (uint8_t k = 0; k < sample_bytes; ++k)
						buf[k] = sample[k];
					--raw;
				}
				buf += sample_bytes;
			}

			if (repetition || raw)
				return dseed::error_fail;
		}
	}
	else
	{
		stream->read (pixels.data (), pixels.size ());
	}

	if (header.depth == 24)
	{
		std::vector<uint8_t> repixels (pixels);
		size_t stride = dseed::color::calc_bitmap_stride (format, header.width);
		pixels.resize (stride * header.height);
		for (int y = 0; y < header.height; ++y)
			memcpy (pixels.data () + (y * stride), repixels.data () + (y * scanlineSize), dseed::minimum (stride, scanlineSize));
	}

	dseed::autoref<dseed::bitmaps::palette> palette;
	if (mapSize > 0)
	{
		int bpp = (format == dseed::color::pixelformat::bgr8_indexed8 ? 24 : 32);
		if (dseed::failed (dseed::bitmaps::create_palette (map.data (), bpp, mapSize / (bpp / 8), &palette)))
			return dseed::error_fail;
	}

	dseed::autoref<dseed::bitmaps::bitmap> bitmap;
	if (dseed::failed (dseed::bitmaps::create_bitmap (bitmaptype::bitmap2d, dseed::size3i (header.width, header.height, 1),
		format, palette, &bitmap)))
		return dseed::error_fail;

	void* ptr;
	if (bitmap->lock (&ptr))
		return dseed::error_fail;

	memcpy (ptr, pixels.data (), pixels.size ());

	bitmap->unlock ();

	*decoder = new dseed::__common_bitmap_array (bitmap);
	if (*decoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}