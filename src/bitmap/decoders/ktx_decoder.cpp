#include <dseed.h>

struct KTXHeader
{
	uint8_t identifier[12];
	uint32_t endianness;
	uint32_t glType;
	uint32_t glTypeSize;
	uint32_t glFormat;
	uint32_t glInternalFormat;
	uint32_t glBaseInternalFormat;
	uint32_t pixelWidth;
	uint32_t pixelHeight;
	uint32_t pixelDepth;
	uint32_t numberOfArrayElements;
	uint32_t numberOfFaces;
	uint32_t numberOfMipmapLevels;
	uint32_t bytesOfKeyValueData;
};

const uint8_t KTXFileIdentifier[12] = {
   0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
};

#define KTX_ENDIAN_REF								0x04030201
#define KTX_ENDIAN_REF_REV							0x01020304

#define GL_COMPRESSED_RGB_S3TC_DXT1					0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1				0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3				0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5				0x83F3

#define GL_COMPRESSED_RGB_PVRTC_4BPPV1				0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1				0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1				0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1				0x8C03

#define GL_ETC1_RGB8_OES							0x8D64

#define GL_COMPRESSED_RGB8_ETC2						0x9274
#define GL_COMPRESSED_SRGB8_ETC2					0x9275
#define GL_COMPRESSED_RGBA8_ETC2_EAC				0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC			0x9279

#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR				0x93B0
#define GL_COMPRESSED_RGBA_ASTC_5x4_KHR				0x93B1
#define GL_COMPRESSED_RGBA_ASTC_5x5_KHR				0x93B2
#define GL_COMPRESSED_RGBA_ASTC_6x5_KHR				0x93B3
#define GL_COMPRESSED_RGBA_ASTC_6x6_KHR				0x93B4
#define GL_COMPRESSED_RGBA_ASTC_8x5_KHR				0x93B5
#define GL_COMPRESSED_RGBA_ASTC_8x6_KHR				0x93B6
#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR				0x93B7
#define GL_COMPRESSED_RGBA_ASTC_10x5_KHR			0x93B8
#define GL_COMPRESSED_RGBA_ASTC_10x6_KHR			0x93B9
#define GL_COMPRESSED_RGBA_ASTC_10x8_KHR			0x93BA
#define GL_COMPRESSED_RGBA_ASTC_10x10_KHR			0x93BB
#define GL_COMPRESSED_RGBA_ASTC_12x10_KHR			0x93BC
#define GL_COMPRESSED_RGBA_ASTC_12x12_KHR			0x93BD

#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR		0x93D0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR		0x93D1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR		0x93D2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR		0x93D3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR		0x93D4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR		0x93D5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR		0x93D6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR		0x93D7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR	0x93D8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR	0x93D9
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR	0x93DA
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR	0x93DB
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR	0x93DC
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR	0x93DD

#define GL_RED										0x1903
#define GL_RGB										0x1907
#define GL_RGBA										0x1908

#define GL_UNSIGNED_BYTE							0x1401
#define GL_UNSIGNED_INT								0x1405
#define GL_FLOAT									0x1406

dseed::error_t dseed::bitmaps::create_ktx_bitmap_decoder (dseed::io::stream* stream, dseed::bitmaps::bitmap_array** decoder) noexcept
{
	KTXHeader header;
	stream->read (&header, sizeof (header));

	if (memcmp (header.identifier, KTXFileIdentifier, 12) != 0)
		return dseed::error_fail;

	dseed::color::pixelformat format;
	if (header.glType == 0)	//< Compressed Texture
	{
		switch (header.glInternalFormat)
		{
		case GL_COMPRESSED_RGB_S3TC_DXT1:
		case GL_COMPRESSED_RGBA_S3TC_DXT1: format = dseed::color::pixelformat::bc1; break;
		case GL_COMPRESSED_RGBA_S3TC_DXT3: format = dseed::color::pixelformat::bc2; break;
		case GL_COMPRESSED_RGBA_S3TC_DXT5: format = dseed::color::pixelformat::bc3; break;

		case GL_COMPRESSED_RGB_PVRTC_2BPPV1: format = dseed::color::pixelformat::pvrtc_2bpp; break;
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV1: format = dseed::color::pixelformat::pvrtc_2abpp; break;
		case GL_COMPRESSED_RGB_PVRTC_4BPPV1: format = dseed::color::pixelformat::pvrtc_4bpp; break;
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV1: format = dseed::color::pixelformat::pvrtc_4abpp; break;

		case GL_ETC1_RGB8_OES: format = dseed::color::pixelformat::etc1; break;

		case GL_COMPRESSED_RGB8_ETC2:
		case GL_COMPRESSED_SRGB8_ETC2: format = dseed::color::pixelformat::etc2; break;
		case GL_COMPRESSED_RGBA8_ETC2_EAC:  format = dseed::color::pixelformat::etc2a; break;
		//case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC: format = dseed::color::pixelformat::etc2a; break;

		case GL_COMPRESSED_RGBA_ASTC_4x4_KHR: format = dseed::color::pixelformat::astc4x4; break;
		case GL_COMPRESSED_RGBA_ASTC_5x4_KHR: format = dseed::color::pixelformat::astc5x4; break;
		case GL_COMPRESSED_RGBA_ASTC_5x5_KHR: format = dseed::color::pixelformat::astc5x5; break;
		case GL_COMPRESSED_RGBA_ASTC_6x5_KHR: format = dseed::color::pixelformat::astc6x5; break;
		case GL_COMPRESSED_RGBA_ASTC_6x6_KHR: format = dseed::color::pixelformat::astc6x6; break;
		case GL_COMPRESSED_RGBA_ASTC_8x5_KHR: format = dseed::color::pixelformat::astc8x5; break;
		case GL_COMPRESSED_RGBA_ASTC_8x6_KHR: format = dseed::color::pixelformat::astc8x6; break;
		case GL_COMPRESSED_RGBA_ASTC_8x8_KHR: format = dseed::color::pixelformat::astc8x8; break;
		case GL_COMPRESSED_RGBA_ASTC_10x5_KHR: format = dseed::color::pixelformat::astc10x5; break;
		case GL_COMPRESSED_RGBA_ASTC_10x6_KHR: format = dseed::color::pixelformat::astc10x6; break;
		case GL_COMPRESSED_RGBA_ASTC_10x8_KHR: format = dseed::color::pixelformat::astc10x8; break;
		case GL_COMPRESSED_RGBA_ASTC_10x10_KHR: format = dseed::color::pixelformat::astc10x10; break;
		case GL_COMPRESSED_RGBA_ASTC_12x10_KHR: format = dseed::color::pixelformat::astc12x10; break;
		case GL_COMPRESSED_RGBA_ASTC_12x12_KHR: format = dseed::color::pixelformat::astc12x12; break;

		default: return dseed::error_not_support;
		}
	}
	else
	{
		switch (header.glFormat)
		{
		case GL_RED:
			if (header.glType == GL_UNSIGNED_BYTE)
				format = dseed::color::pixelformat::r8;
			else return dseed::error_not_support;
			break;

		case GL_RGB:
			if (header.glTypeSize == GL_UNSIGNED_BYTE)
				format = dseed::color::pixelformat::rgb8;
			else return dseed::error_not_support;
			break;

		case GL_RGBA:
			if (header.glTypeSize == GL_UNSIGNED_BYTE)
				format = dseed::color::pixelformat::rgba8;
			else if (header.glTypeSize == GL_FLOAT)
				format = dseed::color::pixelformat::rgbaf;
			else return dseed::error_not_support;
			break;
		}
	}

	if (header.numberOfMipmapLevels == 0)
	{
		header.numberOfMipmapLevels = 1;
	}
	if (header.numberOfArrayElements == 0)
		header.numberOfArrayElements = 1;
	if (header.numberOfFaces == 0)
		header.numberOfFaces = 1;

	dseed::bitmaps::bitmaptype type = dseed::bitmaps::bitmaptype::bitmap2d;
	if (header.numberOfArrayElements == 1 && header.numberOfFaces == 6)
		type = dseed::bitmaps::bitmaptype::bitmap2dcube;
	else if (header.numberOfArrayElements > 1 && header.numberOfFaces == 1)
		type = dseed::bitmaps::bitmaptype::bitmap3d;

	dseed::size3i size (header.pixelWidth, header.pixelHeight, header.numberOfArrayElements * header.numberOfFaces);

	size_t imageOffset = sizeof (header) + header.bytesOfKeyValueData + 4;
	stream->seek (dseed::io::seekorigin::begin, imageOffset);

	std::vector<dseed::bitmaps::bitmap*> bitmaps (header.numberOfMipmapLevels);
	for (uint32_t mipLevel = 0; mipLevel < header.numberOfMipmapLevels; ++mipLevel)
	{
		dseed::size3i currentSize = dseed::color::calc_mipmap_size (mipLevel, size, false);
		size_t imageSize = dseed::color::calc_bitmap_total_size (format, currentSize);
		std::vector<uint8_t> pixels (imageSize);
		size_t read = stream->read (pixels.data (), imageSize);

		if (header.endianness == KTX_ENDIAN_REF_REV)
		{
			for (int i = 0; i < pixels.size (); i += 4)
			{
				uint8_t a = pixels[i + 0], b = pixels[i + 1], c = pixels[i + 2], d = pixels[i + 3];
				pixels[i + 0] = d; pixels[i + 1] = c; pixels[i + 2] = b; pixels[i + 3] = a;
			}
		}

		if (dseed::failed (dseed::bitmaps::create_bitmap (type, currentSize, format, nullptr, &bitmaps[mipLevel])))
			return dseed::error_fail;

		void* ptr;
		bitmaps[mipLevel]->lock (&ptr);
		memcpy (ptr, pixels.data (), pixels.size ());
		bitmaps[mipLevel]->unlock ();
	}

	return create_bitmap_array(arraytype::mipmap, bitmaps, decoder);
}