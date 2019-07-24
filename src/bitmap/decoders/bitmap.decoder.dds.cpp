#include <dseed.h>

#include <string.h>

#include "../../Microsoft/DDSCommon.hxx"
#include "bitmap.decoder.hxx"

dseed::error_t dseed::create_dds_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder)
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

	dseed::pixelformat_t format = dseed::pixelformat_unknown;
	if (header.ddspf.flags & DDS_RGB)
	{
		switch (header.ddspf.RGBBitCount)
		{
		case 32:
			if (header.ddspf.RBitMask == 0x000000ff && header.ddspf.GBitMask == 0x0000ff00 && header.ddspf.BBitMask == 0x00ff0000
				&& (header.ddspf.ABitMask == 0xff000000 || header.ddspf.ABitMask == 0))
				format = dseed::pixelformat_rgba8888;
			else if (header.ddspf.RBitMask == 0x00ff0000 && header.ddspf.GBitMask == 0x0000ff00
				&& header.ddspf.BBitMask == 0x000000ff && header.ddspf.ABitMask == 0xff000000)
				format = dseed::pixelformat_bgra8888;
			else if (header.ddspf.RBitMask == 0xffffffff && header.ddspf.GBitMask == header.ddspf.BBitMask == header.ddspf.ABitMask == 0)
				format = dseed::pixelformat_grayscalef;
			else
				return dseed::error_not_support;
			break;

		case 24:
			if (header.ddspf.RBitMask == 0x000000ff && header.ddspf.GBitMask == 0x0000ff00
				&& header.ddspf.BBitMask == 0x00ff0000)
				format = dseed::pixelformat_rgb888;
			else if (header.ddspf.RBitMask == 0x00ff0000 && header.ddspf.GBitMask == 0x0000ff00
				&& header.ddspf.BBitMask == 0x000000ff)
				format = dseed::pixelformat_bgr888;
			else
				return dseed::error_not_support;
			break;

		case 16:
			if (header.ddspf.RBitMask == 0x0f00 && header.ddspf.GBitMask == 0x00f0
				&& header.ddspf.BBitMask == 0x000f && header.ddspf.ABitMask == 0xf000)
				format = dseed::pixelformat_bgra4444;
			else if (header.ddspf.RBitMask == 0xf800 && header.ddspf.GBitMask == 0x07e0
				&& header.ddspf.BBitMask == 0x001f)
				format = dseed::pixelformat_bgr565;
			else
				return dseed::error_not_support;
			break;

		case 8:
			if (header.ddspf.RBitMask == 0x000000ff && header.ddspf.GBitMask == header.ddspf.BBitMask == header.ddspf.ABitMask == 0)
				format = dseed::pixelformat_grayscale8;
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
				format = dseed::pixelformat_yuva8888;
			else if (header.ddspf.RBitMask == 0xffffffff && header.ddspf.GBitMask == header.ddspf.BBitMask == header.ddspf.ABitMask == 0)
				format = dseed::pixelformat_grayscalef;
			else
				return dseed::error_not_support;
			break;

		case 24:
			if (header.ddspf.RBitMask == 0x000000ff && header.ddspf.GBitMask == 0x0000ff00 && header.ddspf.BBitMask == 0x00ff0000)
				format = dseed::pixelformat_yuv888;
			else
				return dseed::error_not_support;
			break;

		case 8:
			if (header.ddspf.RBitMask == 0x000000ff && header.ddspf.GBitMask == header.ddspf.BBitMask == header.ddspf.ABitMask == 0)
				format = dseed::pixelformat_grayscale8;
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
			case make_fourcc ('D', 'X', 'T', '1'): format = dseed::pixelformat_bc1; break;
			case make_fourcc ('D', 'X', 'T', '2'):
			case make_fourcc ('D', 'X', 'T', '3'): format = dseed::pixelformat_bc2; break;
			case make_fourcc ('D', 'X', 'T', '4'):
			case make_fourcc ('D', 'X', 'T', '5'): format = dseed::pixelformat_bc3; break;
			case make_fourcc ('B', 'C', '4', 'U'):
			case make_fourcc ('B', 'C', '4', 'S'):
			case make_fourcc ('A', 'T', 'I', '1'): format = dseed::pixelformat_bc4; break;
			case make_fourcc ('B', 'C', '5', 'U'):
			case make_fourcc ('B', 'C', '5', 'S'):
			case make_fourcc ('A', 'T', 'I', '2'): format = dseed::pixelformat_bc5; break;
			case make_fourcc ('P', 'T', 'C', '2'): format = dseed::pixelformat_pvrtc_2bpp; break;
			case make_fourcc ('P', 'T', 'C', '4'): format = dseed::pixelformat_pvrtc_4bpp; break;
			case make_fourcc ('E', 'T', 'C', ' '):
			case make_fourcc ('E', 'T', 'C', '1'): format = dseed::pixelformat_etc1; break;
			case 116: format = dseed::pixelformat_rgbaf; break;
			default:
				return dseed::error_not_support;
			}
		}
		else
		{
			switch (bc7Header.dxgiFormat)
			{
			case DXGI_FORMAT_BC6H_SF16: format = dseed::pixelformat_bc6h; break;
			case DXGI_FORMAT_BC7_UNORM: format = dseed::pixelformat_bc7; break;

			case DXGI_FORMAT_BC1_UNORM: format = dseed::pixelformat_bc1; break;
			case DXGI_FORMAT_BC2_UNORM: format = dseed::pixelformat_bc2; break;
			case DXGI_FORMAT_BC3_UNORM: format = dseed::pixelformat_bc3; break;
			case DXGI_FORMAT_BC4_UNORM: format = dseed::pixelformat_bc4; break;
			case DXGI_FORMAT_BC5_UNORM: format = dseed::pixelformat_bc5; break;

			case DXGI_FORMAT_R8G8B8A8_UNORM: format = dseed::pixelformat_rgba8888; break;
			case DXGI_FORMAT_B8G8R8A8_UNORM: format = dseed::pixelformat_bgra8888; break;
			case DXGI_FORMAT_B4G4R4A4_UNORM: format = dseed::pixelformat_bgra4444; break;

			case DXGI_FORMAT_AYUV: format = dseed::pixelformat_yuva8888; break;
			case DXGI_FORMAT_YUY2: format = dseed::pixelformat_yuyv8888; break;
			case DXGI_FORMAT_NV12: format = dseed::pixelformat_nv12; break;

			case DXGI_FORMAT_R8_UNORM: format = dseed::pixelformat_grayscale8; break;
			case DXGI_FORMAT_R32_FLOAT: format = dseed::pixelformat_grayscalef; break;

			case DXGI_FORMAT_ASTC_4X4_UNORM: format = dseed::pixelformat_astc_4x4; break;
			case DXGI_FORMAT_ASTC_5X4_UNORM: format = dseed::pixelformat_astc_5x4; break;
			case DXGI_FORMAT_ASTC_5X5_UNORM: format = dseed::pixelformat_astc_5x5; break;
			case DXGI_FORMAT_ASTC_6X5_UNORM: format = dseed::pixelformat_astc_6x5; break;
			case DXGI_FORMAT_ASTC_6X6_UNORM: format = dseed::pixelformat_astc_6x6; break;
			case DXGI_FORMAT_ASTC_8X5_UNORM: format = dseed::pixelformat_astc_8x5; break;
			case DXGI_FORMAT_ASTC_8X6_UNORM: format = dseed::pixelformat_astc_8x6; break;
			case DXGI_FORMAT_ASTC_8X8_UNORM: format = dseed::pixelformat_astc_8x8; break;
			case DXGI_FORMAT_ASTC_10X5_UNORM: format = dseed::pixelformat_astc_10x5; break;
			case DXGI_FORMAT_ASTC_10X6_UNORM: format = dseed::pixelformat_astc_10x6; break;
			case DXGI_FORMAT_ASTC_10X8_UNORM: format = dseed::pixelformat_astc_10x8; break;
			case DXGI_FORMAT_ASTC_10X10_UNORM: format = dseed::pixelformat_astc_10x10; break;
			case DXGI_FORMAT_ASTC_12X10_UNORM: format = dseed::pixelformat_astc_12x10; break;
			case DXGI_FORMAT_ASTC_12X12_UNORM: format = dseed::pixelformat_astc_12x12; break;

			default:
				return dseed::error_not_support;
			}
		}
	}

	dseed::bitmaptype_t type = dseed::bitmaptype_2d;
	if (dimension == 3)
		type = dseed::bitmaptype_2d;
	else
	{
		if (header.caps2 & DDS_CUBEMAP)
		{
			type = dseed::bitmaptype_2d_cubemap;
			header.depth = 6;
		}
		else header.depth = 1;
	}

	std::vector<dseed::bitmap*> bitmaps;
	bitmaps.resize (1 + header.mipMapCount);

	for (uint32_t mip = 0; mip <= header.mipMapCount; ++mip)
	{
		int halfmaker = (int)pow (2, mip);
		dseed::size3i currentSize (header.width / halfmaker, header.height / halfmaker, header.depth / halfmaker);
		if (currentSize.width <= 0) currentSize.width = 1;
		if (currentSize.height <= 0) currentSize.height = 1;
		if (currentSize.depth <= 0) currentSize.depth = 1;

		std::vector<uint8_t> buf;
		buf.resize (dseed::get_bitmap_plane_size (format, currentSize.width, currentSize.height));
		stream->read (buf.data (), buf.size ());

		if (dseed::failed (dseed::create_bitmap (type, currentSize, format, nullptr, &bitmaps[mip])))
			return dseed::error_fail;

		void* ptr;
		bitmaps[mip]->pixels_pointer (&ptr);

		memcpy (ptr, buf.data (), buf.size ());
	}

	*decoder = new __common_bitmap_decoder (bitmaps, dseed::frametype_mipmap);
	if (decoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}