#include <dseed.h>

#include "common.hxx"

#include "../../libs/DDSHelper.hxx"

dseed::error_t dseed::bitmaps::create_dds_bitmap_decoder (dseed::io::stream* stream, dseed::bitmaps::bitmap_array** decoder) noexcept
{
	uint32_t magic;
	if (sizeof (uint32_t) != stream->read (&magic, sizeof (uint32_t)) || magic != DDS_MAGIC)
		return dseed::error_not_support;

	DDS_HEADER header;
	if (sizeof (DDS_HEADER) != stream->read (&header, sizeof (DDS_HEADER)))
		return dseed::error_fail;

	if (header.size != sizeof (DDS_HEADER) || header.ddspf.size != sizeof (DDS_PIXELFORMAT))
		return dseed::error_not_support;

	uint32_t dimension = 0;

	bool bc7HeaderLoaded = false;
	DDS_HEADER_DXT10 bc7Header;
	if (header.ddspf.flags & DDS_FOURCC && make_fourcc ('D', 'X', '1', '0') == header.ddspf.fourCC)
	{
		if (sizeof (DDS_HEADER_DXT10) != stream->read (&bc7Header, sizeof (DDS_HEADER_DXT10)))
			return dseed::error_fail;
		bc7HeaderLoaded = true;

		if (bc7Header.arraySize == 0) return dseed::error_io;


		dimension = bc7Header.resourceDimension - 1;
	}
	else
	{
		if (header.flags & DDS_HEADER_FLAGS_VOLUME)
			dimension = 3;
		else
			dimension = 2;
	}

	dseed::color::pixelformat format = dseed::color::pixelformat::unknown;
	if (header.ddspf.flags & DDS_RGB)
	{
		switch (header.ddspf.RGBBitCount)
		{
		case 32:
			if (header.ddspf.RBitMask == 0x000000ff && header.ddspf.GBitMask == 0x0000ff00 && header.ddspf.BBitMask == 0x00ff0000
				&& (header.ddspf.ABitMask == 0xff000000 || header.ddspf.ABitMask == 0))
				format = dseed::color::pixelformat::rgba8;
			else if (header.ddspf.RBitMask == 0x00ff0000 && header.ddspf.GBitMask == 0x0000ff00
				&& header.ddspf.BBitMask == 0x000000ff && header.ddspf.ABitMask == 0xff000000)
				format = dseed::color::pixelformat::bgra8;
			else if (header.ddspf.RBitMask == 0xffffffff && header.ddspf.GBitMask == header.ddspf.BBitMask == header.ddspf.ABitMask == 0)
				format = dseed::color::pixelformat::rf;
			else
				return dseed::error_not_support;
			break;

		case 24:
			if (header.ddspf.RBitMask == 0x000000ff && header.ddspf.GBitMask == 0x0000ff00
				&& header.ddspf.BBitMask == 0x00ff0000)
				format = dseed::color::pixelformat::rgb8;
			else if (header.ddspf.RBitMask == 0x00ff0000 && header.ddspf.GBitMask == 0x0000ff00
				&& header.ddspf.BBitMask == 0x000000ff)
				format = dseed::color::pixelformat::bgr8;
			else
				return dseed::error_not_support;
			break;

		case 16:
			if (header.ddspf.RBitMask == 0x0f00 && header.ddspf.GBitMask == 0x00f0
				&& header.ddspf.BBitMask == 0x000f && header.ddspf.ABitMask == 0xf000)
				format = dseed::color::pixelformat::bgra4;
			else if (header.ddspf.RBitMask == 0xf800 && header.ddspf.GBitMask == 0x07e0
				&& header.ddspf.BBitMask == 0x001f)
				format = dseed::color::pixelformat::bgr565;
			else
				return dseed::error_not_support;
			break;

		case 8:
			if (header.ddspf.RBitMask == 0x000000ff && header.ddspf.GBitMask == header.ddspf.BBitMask == header.ddspf.ABitMask == 0)
				format = dseed::color::pixelformat::r8;
			else
				return dseed::error_not_support;
			break;
		}
	}
	else if (header.ddspf.flags & DDS_YUV)
	{
		switch (header.ddspf.RGBBitCount)
		{
		case 32:
			if (header.ddspf.RBitMask == 0x000000ff && header.ddspf.GBitMask == 0x0000ff00 && header.ddspf.BBitMask == 0x00ff0000
				&& (header.ddspf.ABitMask == 0xff000000 || header.ddspf.ABitMask == 0))
				format = dseed::color::pixelformat::yuva8;
			else if (header.ddspf.RBitMask == 0xffffffff && header.ddspf.GBitMask == header.ddspf.BBitMask == header.ddspf.ABitMask == 0)
				format = dseed::color::pixelformat::rf;
			else
				return dseed::error_not_support;
			break;

		case 24:
			if (header.ddspf.RBitMask == 0x000000ff && header.ddspf.GBitMask == 0x0000ff00 && header.ddspf.BBitMask == 0x00ff0000)
				format = dseed::color::pixelformat::yuv8;
			else
				return dseed::error_not_support;
			break;

		case 8:
			if (header.ddspf.RBitMask == 0x000000ff && header.ddspf.GBitMask == header.ddspf.BBitMask == header.ddspf.ABitMask == 0)
				format = dseed::color::pixelformat::r8;
			else
				return dseed::error_not_support;
			break;
		}
	}
	else if (header.ddspf.flags & DDS_FOURCC)
	{
		if (!bc7HeaderLoaded)
		{
			switch (header.ddspf.fourCC)
			{
			case make_fourcc ('D', 'X', 'T', '1'): format = dseed::color::pixelformat::bc1; break;
			case make_fourcc ('D', 'X', 'T', '2'):
			case make_fourcc ('D', 'X', 'T', '3'): format = dseed::color::pixelformat::bc2; break;
			case make_fourcc ('D', 'X', 'T', '4'):
			case make_fourcc ('D', 'X', 'T', '5'): format = dseed::color::pixelformat::bc3; break;
			case make_fourcc ('B', 'C', '4', 'U'):
			case make_fourcc ('B', 'C', '4', 'S'):
			case make_fourcc ('A', 'T', 'I', '1'): format = dseed::color::pixelformat::bc4; break;
			case make_fourcc ('B', 'C', '5', 'U'):
			case make_fourcc ('B', 'C', '5', 'S'):
			case make_fourcc ('A', 'T', 'I', '2'): format = dseed::color::pixelformat::bc5; break;
			case make_fourcc ('P', 'T', 'C', '2'): format = dseed::color::pixelformat::pvrtc_2bpp; break;
			case make_fourcc ('P', 'T', 'C', '4'): format = dseed::color::pixelformat::pvrtc_4bpp; break;
			case make_fourcc ('E', 'T', 'C', ' '):
			case make_fourcc ('E', 'T', 'C', '1'): format = dseed::color::pixelformat::etc1; break;
			case 116: format = dseed::color::pixelformat::rgbaf; break;
			default:
				return dseed::error_not_support;
			}
		}
		else
		{
			switch (bc7Header.dxgiFormat)
			{
			case DXGI_FORMAT_BC6H_SF16: format = dseed::color::pixelformat::bc6; break;
			case DXGI_FORMAT_BC7_UNORM: format = dseed::color::pixelformat::bc7; break;

			case DXGI_FORMAT_BC1_UNORM: format = dseed::color::pixelformat::bc1; break;
			case DXGI_FORMAT_BC2_UNORM: format = dseed::color::pixelformat::bc2; break;
			case DXGI_FORMAT_BC3_UNORM: format = dseed::color::pixelformat::bc3; break;
			case DXGI_FORMAT_BC4_UNORM: format = dseed::color::pixelformat::bc4; break;
			case DXGI_FORMAT_BC5_UNORM: format = dseed::color::pixelformat::bc5; break;

			case DXGI_FORMAT_R8G8B8A8_UNORM: format = dseed::color::pixelformat::rgba8; break;
			case DXGI_FORMAT_B8G8R8A8_UNORM: format = dseed::color::pixelformat::bgra8; break;
			case DXGI_FORMAT_B4G4R4A4_UNORM: format = dseed::color::pixelformat::bgra4; break;

			case DXGI_FORMAT_AYUV: format = dseed::color::pixelformat::yuva8; break;
			case DXGI_FORMAT_YUY2: format = dseed::color::pixelformat::yuyv8; break;
			case DXGI_FORMAT_NV12: format = dseed::color::pixelformat::nv12; break;

			case DXGI_FORMAT_R8_UNORM: format = dseed::color::pixelformat::r8; break;
			case DXGI_FORMAT_R32_FLOAT: format = dseed::color::pixelformat::rf; break;

			case DXGI_FORMAT_ASTC_4X4_UNORM: format = dseed::color::pixelformat::astc4x4; break;
			case DXGI_FORMAT_ASTC_5X4_UNORM: format = dseed::color::pixelformat::astc5x4; break;
			case DXGI_FORMAT_ASTC_5X5_UNORM: format = dseed::color::pixelformat::astc5x5; break;
			case DXGI_FORMAT_ASTC_6X5_UNORM: format = dseed::color::pixelformat::astc6x5; break;
			case DXGI_FORMAT_ASTC_6X6_UNORM: format = dseed::color::pixelformat::astc6x6; break;
			case DXGI_FORMAT_ASTC_8X5_UNORM: format = dseed::color::pixelformat::astc8x5; break;
			case DXGI_FORMAT_ASTC_8X6_UNORM: format = dseed::color::pixelformat::astc8x6; break;
			case DXGI_FORMAT_ASTC_8X8_UNORM: format = dseed::color::pixelformat::astc8x8; break;
			case DXGI_FORMAT_ASTC_10X5_UNORM: format = dseed::color::pixelformat::astc10x5; break;
			case DXGI_FORMAT_ASTC_10X6_UNORM: format = dseed::color::pixelformat::astc10x6; break;
			case DXGI_FORMAT_ASTC_10X8_UNORM: format = dseed::color::pixelformat::astc10x8; break;
			case DXGI_FORMAT_ASTC_10X10_UNORM: format = dseed::color::pixelformat::astc10x10; break;
			case DXGI_FORMAT_ASTC_12X10_UNORM: format = dseed::color::pixelformat::astc12x10; break;
			case DXGI_FORMAT_ASTC_12X12_UNORM: format = dseed::color::pixelformat::astc12x12; break;

			default:
				return dseed::error_not_support;
			}
		}
	}

	dseed::bitmaps::bitmaptype type = dseed::bitmaps::bitmaptype::bitmap2d;
	if (dimension == 3)
		type = dseed::bitmaps::bitmaptype::bitmap2d;
	else
	{
		if (header.caps2 & DDS_CUBEMAP)
		{
			type = dseed::bitmaps::bitmaptype::bitmap2dcube;
			header.depth = 6;
		}
		else header.depth = 1;
	}

	std::vector<dseed::bitmaps::bitmap*> bitmaps;
	bitmaps.resize (1 + header.mipMapCount);

	for (uint32_t mip = 0; mip <= header.mipMapCount; ++mip)
	{
		int halfmaker = (int)powf (2, mip);
		dseed::size3i currentSize (header.width / halfmaker, header.height / halfmaker, header.depth / halfmaker);
		if (currentSize.width <= 0) currentSize.width = 1;
		if (currentSize.height <= 0) currentSize.height = 1;
		if (currentSize.depth <= 0) currentSize.depth = 1;

		std::vector<uint8_t> buf;
		buf.resize (dseed::color::calc_bitmap_plane_size (format, size2i (currentSize.width, currentSize.height)));
		stream->read (buf.data (), buf.size ());

		if (dseed::failed (dseed::bitmaps::create_bitmap (type, currentSize, format, nullptr, &bitmaps[mip])))
			return dseed::error_fail;

		void* ptr;
		bitmaps[mip]->lock (&ptr);
		memcpy (ptr, buf.data (), buf.size ());
		bitmaps[mip]->unlock ();
	}

	*decoder = new dseed::__common_bitmap_array (bitmaps, dseed::bitmaps::arraytype::mipmap);
	if (decoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}