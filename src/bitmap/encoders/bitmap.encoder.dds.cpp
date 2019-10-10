#include <dseed.h>

#include "../../libs/Microsoft/DDSCommon.hxx"

class __dds_encoder : public dseed::bitmap_encoder
{
public:
	__dds_encoder (dseed::stream* stream)
		: _refCount (1), _stream (stream)
	{ }

public:
	virtual int32_t retain () override { return ++_refCount; }
	virtual int32_t release () override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual dseed::error_t encode_frame (dseed::bitmap* bitmap, dseed::timespan_t duration) override
	{
		using namespace dseed;
		static dseed::pixelformat support_formats[] = {
			pixelformat::bc1, pixelformat::bc2, pixelformat::bc3, pixelformat::bc4,
			pixelformat::bc5, pixelformat::bc6h, pixelformat::bc7,
			pixelformat::rgba8888, pixelformat::bgra8888, pixelformat::bgra4444,
			pixelformat::yuv888, pixelformat::yuva8888,
			pixelformat::yuyv8888, pixelformat::nv12,
			pixelformat::grayscale8, pixelformat::grayscalef,
			pixelformat::astc_4x4, pixelformat::astc_5x4, pixelformat::astc_5x5,
			pixelformat::astc_6x5, pixelformat::astc_6x6, pixelformat::astc_8x5,
			pixelformat::astc_8x6, pixelformat::astc_8x8, pixelformat::astc_10x5,
			pixelformat::astc_10x6, pixelformat::astc_10x8, pixelformat::astc_10x10,
			pixelformat::astc_12x10, pixelformat::astc_12x12,
			pixelformat::pvrtc_2bpp, pixelformat::pvrtc_2abpp, pixelformat::pvrtc_4bpp,
			pixelformat::pvrtc_4abpp, pixelformat::pvrtc2_2bpp, pixelformat::pvrtc2_4bpp,
		};

		if (bitmap == nullptr) return dseed::error_invalid_args;
		auto format = bitmap->format ();

		bool found = false;
		for (auto sf : support_formats)
			if (sf == format)
			{
				found = true;
				break;
			}
		if (!found)
			return dseed::error_not_support;

		if (_bitmaps.size () == 0)
			_bitmaps.push_back (bitmap);
		else
		{
			if (_bitmaps[0]->format () != bitmap->format () || _bitmaps[0]->type () != bitmap->type ())
				return dseed::error_invalid_args;

			auto mainSize = _bitmaps[0]->size ();
			int halfmaker = (int)pow (2, _bitmaps.size ());
			dseed::size3i currentSize (mainSize.width / halfmaker, mainSize.height / halfmaker, mainSize.depth / halfmaker);
			if (currentSize.width <= 0) currentSize.width = 1;
			if (currentSize.height <= 0) currentSize.height = 1;
			if (currentSize.depth <= 0) currentSize.depth = 1;

			auto bitmapSize = bitmap->size ();
			if (currentSize.width != bitmapSize.width || currentSize.height != bitmapSize.height || currentSize.depth != bitmapSize.height)
				return dseed::error_invalid_args;

			_bitmaps.push_back (bitmap);
		}

		return dseed::error_good;
	}

	virtual dseed::error_t commit () override
	{
		if (_bitmaps.size () <= 0)
			return dseed::error_invalid_op;

		_stream->write (&DDS_MAGIC, 4);

		auto format = _bitmaps[0]->format ();
		dseed::size3i mainSize = _bitmaps[0]->size ();
		auto type = _bitmaps[0]->type ();

		DDS_HEADER header = {};
		set_header (header, format, type, mainSize);
		_stream->write (&header, sizeof (header));

		if (format == dseed::pixelformat::bc6h || format == dseed::pixelformat::bc7
			|| format == dseed::pixelformat::yuyv8888
			|| format == dseed::pixelformat::nv12
			|| format == dseed::pixelformat::astc_4x4
			|| format == dseed::pixelformat::astc_5x4
			|| format == dseed::pixelformat::astc_5x5
			|| format == dseed::pixelformat::astc_6x5
			|| format == dseed::pixelformat::astc_6x6
			|| format == dseed::pixelformat::astc_8x5
			|| format == dseed::pixelformat::astc_8x6
			|| format == dseed::pixelformat::astc_8x8
			|| format == dseed::pixelformat::astc_10x5
			|| format == dseed::pixelformat::astc_10x6
			|| format == dseed::pixelformat::astc_10x8
			|| format == dseed::pixelformat::astc_10x10
			|| format == dseed::pixelformat::astc_12x10
			|| format == dseed::pixelformat::astc_12x12)
		{
			DDS_HEADER_DXT10 bc7header = {};
			if (type == dseed::bitmaptype_2d || type == dseed::bitmaptype_2d_cubemap)
				bc7header.resourceDimension = 3;
			else if (type == dseed::bitmaptype_3d)
				bc7header.resourceDimension = 4;

			switch (format)
			{
			case dseed::pixelformat::bc6h: bc7header.dxgiFormat = DXGI_FORMAT_BC6H_SF16; break;
			case dseed::pixelformat::bc7: bc7header.dxgiFormat = DXGI_FORMAT_BC7_UNORM; break;

			case dseed::pixelformat::yuva8888: bc7header.dxgiFormat = DXGI_FORMAT_YUY2; break;
			case dseed::pixelformat::nv12: bc7header.dxgiFormat = DXGI_FORMAT_NV12; break;

			case dseed::pixelformat::astc_4x4: bc7header.dxgiFormat = DXGI_FORMAT_ASTC_4X4_UNORM; break;
			case dseed::pixelformat::astc_5x4: bc7header.dxgiFormat = DXGI_FORMAT_ASTC_5X4_UNORM; break;
			case dseed::pixelformat::astc_5x5: bc7header.dxgiFormat = DXGI_FORMAT_ASTC_5X5_UNORM; break;
			case dseed::pixelformat::astc_6x5: bc7header.dxgiFormat = DXGI_FORMAT_ASTC_6X5_UNORM; break;
			case dseed::pixelformat::astc_6x6: bc7header.dxgiFormat = DXGI_FORMAT_ASTC_6X6_UNORM; break;
			case dseed::pixelformat::astc_8x5: bc7header.dxgiFormat = DXGI_FORMAT_ASTC_8X5_UNORM; break;
			case dseed::pixelformat::astc_8x6: bc7header.dxgiFormat = DXGI_FORMAT_ASTC_8X6_UNORM; break;
			case dseed::pixelformat::astc_8x8: bc7header.dxgiFormat = DXGI_FORMAT_ASTC_8X8_UNORM; break;
			case dseed::pixelformat::astc_10x5: bc7header.dxgiFormat = DXGI_FORMAT_ASTC_10X5_UNORM; break;
			case dseed::pixelformat::astc_10x6: bc7header.dxgiFormat = DXGI_FORMAT_ASTC_10X6_UNORM; break;
			case dseed::pixelformat::astc_10x8: bc7header.dxgiFormat = DXGI_FORMAT_ASTC_10X8_UNORM; break;
			case dseed::pixelformat::astc_10x10: bc7header.dxgiFormat = DXGI_FORMAT_ASTC_10X10_UNORM; break;
			case dseed::pixelformat::astc_12x10: bc7header.dxgiFormat = DXGI_FORMAT_ASTC_12X10_UNORM; break;
			case dseed::pixelformat::astc_12x12: bc7header.dxgiFormat = DXGI_FORMAT_ASTC_12X12_UNORM; break;

			default: return dseed::error_fail;
			}

			_stream->write (&bc7header, sizeof (bc7header));
		}

		size_t currentMip = 0;
		for (auto& bitmap : _bitmaps)
		{
			auto mainSize = bitmap->size ();
			int halfmaker = (int)pow (2, currentMip++);
			dseed::size3i currentSize (mainSize.width / halfmaker, mainSize.height / halfmaker, mainSize.depth / halfmaker);
			size_t stride = dseed::get_bitmap_stride (format, currentSize.width);

			std::vector<uint8_t> pixels;
			pixels.resize (stride * currentSize.height * currentSize.depth);
			for (int d = 0; d < currentSize.depth; ++d)
				bitmap->copy_pixels (pixels.data () + (stride * currentSize.height * d), d);

			_stream->write (pixels.data (), pixels.size ());
		}

		_stream->flush ();
		return dseed::error_good;
	}

private:
	void set_header (DDS_HEADER& header, dseed::pixelformat format, dseed::bitmaptype_t type, const dseed::size3i& size)
	{
		using namespace dseed;
		header.size = sizeof (DDS_HEADER);
		header.width = size.width;
		header.height = size.height;
		header.caps = 0x1000;
		if (_bitmaps.size () > 1)
			header.mipMapCount = (uint32_t)(_bitmaps.size () - 1);
		header.ddspf.size = sizeof (DDS_PIXELFORMAT);
		switch (format)
		{
		case pixelformat::bc1:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_LINEARSIZE;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_plane_size (format, size.width, size.height);
			header.ddspf.flags = DDS_FOURCC;
			header.ddspf.fourCC = make_fourcc ('D', 'X', 'T', '1');
			break;
		case pixelformat::bc2:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_LINEARSIZE;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_plane_size (format, size.width, size.height);
			header.ddspf.flags = DDS_FOURCC;
			header.ddspf.fourCC = make_fourcc ('D', 'X', 'T', '3');
			break;
		case pixelformat::bc3:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_LINEARSIZE;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_plane_size (format, size.width, size.height);
			header.ddspf.flags = DDS_FOURCC;
			header.ddspf.fourCC = make_fourcc ('D', 'X', 'T', '5');
			break;
		case pixelformat::bc4:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_LINEARSIZE;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_plane_size (format, size.width, size.height);
			header.ddspf.flags = DDS_FOURCC;
			header.ddspf.fourCC = make_fourcc ('A', 'T', 'I', '1');
			break;
		case pixelformat::bc5:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_LINEARSIZE;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_plane_size (format, size.width, size.height);
			header.ddspf.flags = DDS_FOURCC;
			header.ddspf.fourCC = make_fourcc ('A', 'T', 'I', '2');
			break;
		case pixelformat::bc6h:
		case pixelformat::bc7:
		case pixelformat::astc_4x4:
		case pixelformat::astc_5x4:
		case pixelformat::astc_5x5:
		case pixelformat::astc_6x5:
		case pixelformat::astc_6x6:
		case pixelformat::astc_8x5:
		case pixelformat::astc_8x6:
		case pixelformat::astc_8x8:
		case pixelformat::astc_10x5:
		case pixelformat::astc_10x6:
		case pixelformat::astc_10x8:
		case pixelformat::astc_10x10:
		case pixelformat::astc_12x10:
		case pixelformat::astc_12x12:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_LINEARSIZE;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_plane_size (format, size.width, size.height);
			header.ddspf.flags = DDS_FOURCC;
			header.ddspf.fourCC = make_fourcc ('D', 'X', '1', '0');
			break;

		case pixelformat::pvrtc_2bpp:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_LINEARSIZE;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_plane_size (format, size.width, size.height);
			header.ddspf.flags = DDS_FOURCC;
			header.ddspf.fourCC = make_fourcc ('P', 'V', 'R', '2');
			break;
		case pixelformat::pvrtc_4bpp:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_LINEARSIZE;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_plane_size (format, size.width, size.height);
			header.ddspf.flags = DDS_FOURCC;
			header.ddspf.fourCC = make_fourcc ('P', 'V', 'R', '4');
			break;
		case pixelformat::etc1:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_LINEARSIZE;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_plane_size (format, size.width, size.height);
			header.ddspf.flags = DDS_FOURCC;
			header.ddspf.fourCC = make_fourcc ('E', 'T', 'C', '1');
			break;

		case pixelformat::rgba8888:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_PITCH;
			header.ddspf.flags = DDS_RGB | DDS_ALPHA;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_stride (format, size.width);
			header.ddspf.RGBBitCount = 32;
			header.ddspf.RBitMask = 0x000000ff;
			header.ddspf.GBitMask = 0x0000ff00;
			header.ddspf.BBitMask = 0x00ff0000;
			header.ddspf.ABitMask = 0xff000000;
			break;
		case pixelformat::rgb888:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_PITCH;
			header.ddspf.flags = DDS_RGB;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_stride (format, size.width);
			header.ddspf.RGBBitCount = 24;
			header.ddspf.RBitMask = 0x000000ff;
			header.ddspf.GBitMask = 0x0000ff00;
			header.ddspf.BBitMask = 0x00ff0000;
			header.ddspf.ABitMask = 0x00000000;
			break;
		case pixelformat::bgra8888:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_PITCH;
			header.ddspf.flags = DDS_RGB | DDS_ALPHA;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_stride (format, size.width);
			header.ddspf.RGBBitCount = 32;
			header.ddspf.BBitMask = 0x000000ff;
			header.ddspf.GBitMask = 0x0000ff00;
			header.ddspf.RBitMask = 0x00ff0000;
			header.ddspf.ABitMask = 0xff000000;
			break;
		case pixelformat::bgr888:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_PITCH;
			header.ddspf.flags = DDS_RGB;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_stride (format, size.width);
			header.ddspf.RGBBitCount = 24;
			header.ddspf.BBitMask = 0x000000ff;
			header.ddspf.GBitMask = 0x0000ff00;
			header.ddspf.RBitMask = 0x00ff0000;
			header.ddspf.ABitMask = 0x00000000;
			break;
		case pixelformat::bgr565:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_PITCH;
			header.ddspf.flags = DDS_RGB;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_stride (format, size.width);
			header.ddspf.RGBBitCount = 16;
			header.ddspf.RBitMask = 0xf800;
			header.ddspf.GBitMask = 0x07e0;
			header.ddspf.BBitMask = 0x001f;
			header.ddspf.ABitMask = 0x00000000;
			break;
		case pixelformat::bgra4444:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_PITCH;
			header.ddspf.flags = DDS_RGB | DDS_ALPHA;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_stride (format, size.width);
			header.ddspf.RGBBitCount = 16;
			header.ddspf.BBitMask = 0x000f;
			header.ddspf.GBitMask = 0x00f0;
			header.ddspf.RBitMask = 0x0f00;
			header.ddspf.ABitMask = 0xf000;
			break;

		case pixelformat::yuv888:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_PITCH;
			header.ddspf.flags = DDS_YUV;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_stride (format, size.width);
			header.ddspf.RGBBitCount = 24;
			header.ddspf.BBitMask = 0x000000ff;
			header.ddspf.GBitMask = 0x0000ff00;
			header.ddspf.RBitMask = 0x00ff0000;
			header.ddspf.ABitMask = 0x00000000;
			break;
		case pixelformat::yuva8888:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_PITCH;
			header.ddspf.flags = DDS_YUV | DDS_ALPHA;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_stride (format, size.width);
			header.ddspf.RGBBitCount = 32;
			header.ddspf.BBitMask = 0x000000ff;
			header.ddspf.GBitMask = 0x0000ff00;
			header.ddspf.RBitMask = 0x00ff0000;
			header.ddspf.ABitMask = 0xff000000;
			break;

		case pixelformat::grayscale8:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_PITCH;
			header.ddspf.flags = DDS_LUMINANCE;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_stride (format, size.width);
			header.ddspf.RGBBitCount = 8;
			header.ddspf.BBitMask = 0x000000ff;
			header.ddspf.GBitMask = 0x00000000;
			header.ddspf.RBitMask = 0x00000000;
			header.ddspf.ABitMask = 0x00000000;
			break;
		case pixelformat::grayscalef:
			header.flags = DDS_HEADER_FLAGS_REQUIRED | DDS_HEADER_FLAGS_PITCH;
			header.ddspf.flags = DDS_LUMINANCE;
			header.pitchOrLinearSize = (uint32_t)dseed::get_bitmap_stride (format, size.width);
			header.ddspf.RGBBitCount = 32;
			header.ddspf.BBitMask = 0xffffffff;
			header.ddspf.GBitMask = 0x00000000;
			header.ddspf.RBitMask = 0x00000000;
			header.ddspf.ABitMask = 0x00000000;
			break;
		}
		switch (type)
		{
		case dseed::bitmaptype_2d:
			break;

		case dseed::bitmaptype_2d_cubemap:
			header.caps2 = DDS_CUBEMAP_ALLFACES;
			break;

		case dseed::bitmaptype_3d:
			header.flags |= DDS_HEADER_FLAGS_VOLUME;
			break;
		}
	}
	virtual dseed::frametype_t frame_type () override { return dseed::frametype_mipmap; }

private:
	std::atomic<int32_t> _refCount;
	dseed::auto_object<dseed::stream> _stream;

	std::vector<dseed::auto_object<dseed::bitmap>> _bitmaps;
};

dseed::error_t dseed::create_dds_bitmap_encoder (dseed::stream* stream, const dseed::bitmap_encoder_options* options, dseed::bitmap_encoder** encoder)
{
	if (stream == nullptr || encoder == nullptr)
		return dseed::error_invalid_args;

	*encoder = new __dds_encoder (stream);
	if (*encoder == nullptr)
		return dseed::error_out_of_memory;
	return dseed::error_good;

	return dseed::error_good;
}