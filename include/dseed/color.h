#ifndef __DSEED_COLOR_H__
#define __DSEED_COLOR_H__

namespace dseed
{
	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Pixel Format
	//
	////////////////////////////////////////////////////////////////////////////////////////
	enum pixelformat_t : int32_t
	{
		// Unknown Pixel Format
		pixelformat_unknown = 0x00000000,

		// 8-bit Grayscale Pixel Format
		pixelformat_grayscale8 = 0x00010001,
		// 32-bit Floating-Point Grayscale Pixel Format
		pixelformat_grayscalef = 0x00010004,
		// 32-bit Integer RGBA Pixel Format
		pixelformat_rgba8888 = 0x00010104,
		// 128-bit Floating-Point RGBA Pixel Format
		pixelformat_rgbaf = 0x00010110,
		// 24-bit RGB Pixel Format
		pixelformat_rgb888 = 0x00010103,
		// 32-bit Integer BGRA Pixel Format
		pixelformat_bgra8888 = 0x00010204,
		// 32-bit Integer BGRA Pixel Format
		pixelformat_bgr888 = 0x00010203,
		// 16-bit Integer BGRA Pixel Format
		pixelformat_bgra4444 = 0x00010202,

		// 16-bit Integer BGR Pixel Format(Legacy)
		pixelformat_bgr565 = 0x00010302,

		// 8-bit Indexed BGRA Pixel Format
		pixelformat_bgra8888_indexed8 = 0x00020104,
		// 8-bit Indexed BGR Pixel Format
		pixelformat_bgr888_indexed8 = 0x00020103,

		// 24-bit YCbCr:4:4:4 Pixel Format
		pixelformat_yuv888 = 0x00030003,
		// 32-bit YCbCr:4:4:4 and Alpha Pixel Format
		pixelformat_yuva8888 = 0x00030004,

		// 32-bit YCbCr:4:2:2(YUYV or YUY2) Packed Pixel Format
		pixelformat_yuyv8888 = 0x00040204,
		// YCbCr:4:2:0(NV12) Packed Pixel Format
		pixelformat_yyyyuv888888 = 0x00040406,
		pixelformat_nv12 = pixelformat_yyyyuv888888,

		// BC1(DXT1) Compressed Pixel Format
		pixelformat_bc1 = 0x00100001, pixelformat_dxt1 = pixelformat_bc1,
		// BC2(DXT3) Compressed Pixel Format
		pixelformat_bc2 = 0x00100002, pixelformat_dxt3 = pixelformat_bc2,
		// BC3(DXT5) Compressed Pixel Format
		pixelformat_bc3 = 0x00100003, pixelformat_dxt5 = pixelformat_bc3,
		// BC4(ATI1) Compressed Pixel Format
		pixelformat_bc4 = 0x00100004, pixelformat_ati1 = pixelformat_bc4,
		// BC5(ATI2) Compressed Pixel Format
		pixelformat_bc5 = 0x00100005, pixelformat_ati2 = pixelformat_bc5,
		// BC6 Compressed Pixel Format
		pixelformat_bc6h = 0x00100006,
		// BC7 Compressed Pixel Format
		pixelformat_bc7 = 0x00100007,

		// ETC Compressed Pixel Format
		pixelformat_etc1 = 0x00110001,
		// ETC2 Compressed Pixel Format
		pixelformat_etc2 = 0x00110002,
		// ETC2 with Alpha Compressed Pixel Format
		pixelformat_etc2a = 0x00110102,
		// ETC2 with Dedicated Alpha Compressed Pixel Format
		pixelformat_etc2a8 = 0x00110103,

		// PVRTC Compressed Pixel Formats
		pixelformat_pvrtc_2bpp = 0x00120001,
		pixelformat_pvrtc_2abpp = 0x00120101,
		pixelformat_pvrtc_4bpp = 0x00120002,
		pixelformat_pvrtc_4abpp = 0x00120102,
		// PVRTC2 Compressed Pixel Formats
		pixelformat_pvrtc2_2bpp = 0x00120003,
		pixelformat_pvrtc2_4bpp = 0x00120004,

		// ASTC Compressed Pixel Formats
		pixelformat_astc_4x4 = 0x00130001,
		pixelformat_astc_5x4 = 0x00130002,
		pixelformat_astc_5x5 = 0x00130003,
		pixelformat_astc_6x5 = 0x00130004,
		pixelformat_astc_6x6 = 0x00130005,
		pixelformat_astc_8x5 = 0x00130006,
		pixelformat_astc_8x6 = 0x00130007,
		pixelformat_astc_8x8 = 0x00130008,
		pixelformat_astc_10x5 = 0x00130009,
		pixelformat_astc_10x6 = 0x0013000a,
		pixelformat_astc_10x8 = 0x0013000b,
		pixelformat_astc_10x10 = 0x0013000c,
		pixelformat_astc_12x10 = 0x0013000d,
		pixelformat_astc_12x12 = 0x0013000e,

		// Depth-Stencil Pixel Formats (Not support in dseed Bitmap object, Only for Texture objects)
		pixelformat_depth16 = 0x01000002,
		pixelformat_depth24stencil8 = 0x01000104,
		pixelformat_depth32 = 0x01000004,
	};

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Utility Procedures
	//
	////////////////////////////////////////////////////////////////////////////////////////

	// Get Stride value
	DSEEDEXP size_t get_bitmap_stride (dseed::pixelformat_t format, int32_t width) noexcept;
	// Get Bitmap Plane size
	DSEEDEXP size_t get_bitmap_plane_size (dseed::pixelformat_t format, int32_t width, int32_t height) noexcept;
	// Get Total Bitmap Planes Buffer size
	DSEEDEXP size_t get_bitmap_total_size (dseed::pixelformat_t format, const dseed::size3i& size) noexcept;
	// Get Mipmap Size
	DSEEDEXP dseed::size3i get_mipmap_size (int mipLevel, const dseed::size3i& size, bool cubemap) noexcept;
	// Get Maximum Mip-Levels
	DSEEDEXP size_t get_maximum_mip_levels (const dseed::size3i& size, bool cubemap) noexcept;
	// Saturate Integer color value
	constexpr uint8_t saturate (int32_t v) { return (uint8_t)maximum (0, minimum (255, v)); }
	// Saturate Floating point color value
	constexpr float saturate (float v) { return maximum (0.000f, minimum (1.000f, v)); }

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Red/Green/Blue Color Types
	//
	////////////////////////////////////////////////////////////////////////////////////////

	struct rgba;
	struct rgb;
	struct rgbaf;
	struct bgra;
	struct bgr;
	struct bgra4;
	struct bgr565;

#	if COMPILER_MSVC
#		pragma pack (push, 1)
#	else
#		pragma pack (1)
#	endif

	struct rgba
	{
		union
		{
			struct { uint8_t r, g, b, a; };
			uint32_t color;
		};
		rgba () = default;
		inline rgba (uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
			: r (r), g (g), b (b), a (a)
		{ }
		inline rgba (uint32_t rgba)
			: color (rgba)
		{ }

		inline operator rgb () const noexcept;
		inline operator rgbaf () const noexcept;
		inline operator bgra () const noexcept;
		inline operator bgr () const noexcept;
		inline operator bgra4 () const noexcept;
		inline operator bgr565 () const noexcept;

		inline rgba& operator+= (const rgba& c) { r += c.r; g += c.g; b += c.b; a += c.a; return *this; }
		inline rgba& operator-= (const rgba& c) { r -= c.r; g -= c.g; b -= c.b; a -= c.a; return *this; }
		inline rgba& operator*= (const rgba& c) { r *= c.r; g *= c.g; b *= c.b; a *= c.a; return *this; }
		inline rgba& operator/= (const rgba& c) { r /= c.r; g /= c.g; b /= c.b; a /= c.a; return *this; }
	};
	inline rgba operator+ (const rgba& c1, const rgba& c2) { return rgba (c1.r + c2.r, c1.g + c2.g, c1.b + c2.b, c1.a + c2.a); }
	inline rgba operator- (const rgba& c1, const rgba& c2) { return rgba (c1.r - c2.r, c1.g - c2.g, c1.b - c2.b, c1.a - c2.a); }
	inline rgba operator* (const rgba& c1, const rgba& c2) { return rgba (c1.r * c2.r, c1.g * c2.g, c1.b * c2.b, c1.a * c2.a); }
	inline rgba operator/ (const rgba& c1, const rgba& c2) { return rgba (c1.r / c2.r, c1.g / c2.g, c1.b / c2.b, c1.a / c2.a); }

	struct rgb
	{
		union
		{
			struct { uint8_t r, g, b; };
			uint24_t color;
		};
		rgb () = default;
		inline rgb (uint8_t r, uint8_t g, uint8_t b)
			: r (r), g (g), b (b)
		{ }
		inline rgb (uint24_t rgb)
			: color (rgb)
		{ }

		inline operator rgba () const noexcept;
		inline operator rgbaf () const noexcept;
		inline operator bgra () const noexcept;
		inline operator bgr () const noexcept;
		inline operator bgra4 () const noexcept;
		inline operator bgr565 () const noexcept;

		inline rgb& operator+= (const rgb& c) { r += c.r; g += c.g; b += c.b; return *this; }
		inline rgb& operator-= (const rgb& c) { r -= c.r; g -= c.g; b -= c.b; return *this; }
		inline rgb& operator*= (const rgb& c) { r *= c.r; g *= c.g; b *= c.b; return *this; }
		inline rgb& operator/= (const rgb& c) { r /= c.r; g /= c.g; b /= c.b; return *this; }
	};
	inline rgb operator+ (const rgb& c1, const rgb& c2) { return rgb (c1.r + c2.r, c1.g + c2.g, c1.b + c2.b); }
	inline rgb operator- (const rgb& c1, const rgb& c2) { return rgb (c1.r - c2.r, c1.g - c2.g, c1.b - c2.b); }
	inline rgb operator* (const rgb& c1, const rgb& c2) { return rgb (c1.r * c2.r, c1.g * c2.g, c1.b * c2.b); }
	inline rgb operator/ (const rgb& c1, const rgb& c2) { return rgb (c1.r / c2.r, c1.g / c2.g, c1.b / c2.b); }

	struct rgbaf
	{
		union
		{
			struct { float r, g, b, a; };
			float4 color;
		};
		rgbaf () = default;
		inline rgbaf (float r, float g, float b, float a = 1.00000000f)
			: r (r), g (g), b (b), a (a)
		{ }
		inline rgbaf (const float4& rgba)
			: color (rgba)
		{ }

		inline operator rgba () const noexcept;
		inline operator rgb () const noexcept;
		inline operator bgra () const noexcept;
		inline operator bgr () const noexcept;
		inline operator bgra4 () const noexcept;
		inline operator bgr565 () const noexcept;
		inline operator float4 () const noexcept { return color; }

		inline rgbaf& operator+= (const rgbaf& c) { r += c.r; g += c.g; b += c.b; a += c.a; return *this; }
		inline rgbaf& operator-= (const rgbaf& c) { r -= c.r; g -= c.g; b -= c.b; a -= c.a; return *this; }
		inline rgbaf& operator*= (const rgbaf& c) { r *= c.r; g *= c.g; b *= c.b; a *= c.a; return *this; }
		inline rgbaf& operator/= (const rgbaf& c) { r /= c.r; g /= c.g; b /= c.b; a /= c.a; return *this; }
	};
	inline rgbaf operator+ (const rgbaf& c1, const rgbaf& c2) { return rgbaf (c1.r + c2.r, c1.g + c2.g, c1.b + c2.b, c1.a + c2.a); }
	inline rgbaf operator- (const rgbaf& c1, const rgbaf& c2) { return rgbaf (c1.r - c2.r, c1.g - c2.g, c1.b - c2.b, c1.a - c2.a); }
	inline rgbaf operator* (const rgbaf& c1, const rgbaf& c2) { return rgbaf (c1.r * c2.r, c1.g * c2.g, c1.b * c2.b, c1.a * c2.a); }
	inline rgbaf operator/ (const rgbaf& c1, const rgbaf& c2) { return rgbaf (c1.r / c2.r, c1.g / c2.g, c1.b / c2.b, c1.a / c2.a); }

	struct bgra
	{
		union
		{
			struct { uint8_t b, g, r, a; };
			uint32_t color;
		};
		bgra () = default;
		inline bgra (uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
			: r (r), g (g), b (b), a (a)
		{ }
		inline bgra (uint32_t bgra)
			: color (bgra)
		{ }

		inline operator rgba () const noexcept;
		inline operator rgb () const noexcept;
		inline operator rgbaf () const noexcept;
		inline operator bgr () const noexcept;
		inline operator bgra4 () const noexcept;
		inline operator bgr565 () const noexcept;

		inline bgra& operator+= (const bgra& c) { r += c.r; g += c.g; b += c.b; a += c.a; return *this; }
		inline bgra& operator-= (const bgra& c) { r -= c.r; g -= c.g; b -= c.b; a -= c.a; return *this; }
		inline bgra& operator*= (const bgra& c) { r *= c.r; g *= c.g; b *= c.b; a *= c.a; return *this; }
		inline bgra& operator/= (const bgra& c) { r /= c.r; g /= c.g; b /= c.b; a /= c.a; return *this; }
	};
	inline bgra operator+ (const bgra& c1, const bgra& c2) { return bgra (c1.r + c2.r, c1.g + c2.g, c1.b + c2.b, c1.a + c2.a); }
	inline bgra operator- (const bgra& c1, const bgra& c2) { return bgra (c1.r - c2.r, c1.g - c2.g, c1.b - c2.b, c1.a - c2.a); }
	inline bgra operator* (const bgra& c1, const bgra& c2) { return bgra (c1.r * c2.r, c1.g * c2.g, c1.b * c2.b, c1.a * c2.a); }
	inline bgra operator/ (const bgra& c1, const bgra& c2) { return bgra (c1.r / c2.r, c1.g / c2.g, c1.b / c2.b, c1.a / c2.a); }

	struct bgr
	{
		union
		{
			struct { uint8_t b, g, r; };
			uint24_t color;
		};
		bgr () = default;
		inline bgr (uint8_t r, uint8_t g, uint8_t b)
			: r (r), g (g), b (b)
		{ }
		inline bgr (uint24_t bgr)
			: color (bgr)
		{ }

		inline operator rgba () const noexcept;
		inline operator rgb () const noexcept;
		inline operator rgbaf () const noexcept;
		inline operator bgra () const noexcept;
		inline operator bgra4 () const noexcept;
		inline operator bgr565 () const noexcept;

		inline bgr& operator+= (const bgr& c) { r += c.r; g += c.g; b += c.b; return *this; }
		inline bgr& operator-= (const bgr& c) { r -= c.r; g -= c.g; b -= c.b; return *this; }
		inline bgr& operator*= (const bgr& c) { r *= c.r; g *= c.g; b *= c.b; return *this; }
		inline bgr& operator/= (const bgr& c) { r /= c.r; g /= c.g; b /= c.b; return *this; }
	};
	inline bgr operator+ (const bgr& c1, const bgr& c2) { return bgr (c1.r + c2.r, c1.g + c2.g, c1.b + c2.b); }
	inline bgr operator- (const bgr& c1, const bgr& c2) { return bgr (c1.r - c2.r, c1.g - c2.g, c1.b - c2.b); }
	inline bgr operator* (const bgr& c1, const bgr& c2) { return bgr (c1.r * c2.r, c1.g * c2.g, c1.b * c2.b); }
	inline bgr operator/ (const bgr& c1, const bgr& c2) { return bgr (c1.r / c2.r, c1.g / c2.g, c1.b / c2.b); }

	struct bgra4
	{
		union
		{
			struct { uint8_t b : 4, g : 4, r : 4, a : 4; };
			uint16_t color;
		};
		bgra4 () = default;
		inline bgra4 (uint8_t r, uint8_t g, uint8_t b, uint8_t a = 15)
			: r (r), g (g), b (b), a (a)
		{ }
		inline bgra4 (uint16_t bgra)
			: color (bgra)
		{ }

		inline operator rgba () const noexcept;
		inline operator rgb () const noexcept;
		inline operator rgbaf () const noexcept;
		inline operator bgra () const noexcept;
		inline operator bgr () const noexcept;
		inline operator bgr565 () const noexcept;

		inline bgra4& operator+= (const bgra4& c) { r += c.r; g += c.g; b += c.b; a += c.a; return *this; }
		inline bgra4& operator-= (const bgra4& c) { r -= c.r; g -= c.g; b -= c.b; a -= c.a; return *this; }
		inline bgra4& operator*= (const bgra4& c) { r *= c.r; g *= c.g; b *= c.b; a *= c.a; return *this; }
		inline bgra4& operator/= (const bgra4& c) { r /= c.r; g /= c.g; b /= c.b; a /= c.a; return *this; }
	};
	inline bgra4 operator+ (const bgra4& c1, const bgra4& c2) { return bgra4 (c1.r + c2.r, c1.g + c2.g, c1.b + c2.b, c1.a + c2.a); }
	inline bgra4 operator- (const bgra4& c1, const bgra4& c2) { return bgra4 (c1.r - c2.r, c1.g - c2.g, c1.b - c2.b, c1.a - c2.a); }
	inline bgra4 operator* (const bgra4& c1, const bgra4& c2) { return bgra4 (c1.r * c2.r, c1.g * c2.g, c1.b * c2.b, c1.a * c2.a); }
	inline bgra4 operator/ (const bgra4& c1, const bgra4& c2) { return bgra4 (c1.r / c2.r, c1.g / c2.g, c1.b / c2.b, c1.a / c2.a); }

	struct bgr565
	{
		union
		{
			struct { uint8_t b : 5, g : 6, r : 5; };
			uint16_t color;
		};
		bgr565 () = default;
		inline bgr565 (uint8_t r, uint8_t g, uint8_t b)
			: r (r), g (g), b (b)
		{ }
		inline bgr565 (uint16_t bgr)
			: color (bgr)
		{ }

		inline operator rgba () const noexcept;
		inline operator rgb () const noexcept;
		inline operator rgbaf () const noexcept;
		inline operator bgra () const noexcept;
		inline operator bgr () const noexcept;
		inline operator bgra4 () const noexcept;

		inline bgr565& operator+= (const bgr565& c) { r += c.r; g += c.g; b += c.b; return *this; }
		inline bgr565& operator-= (const bgr565& c) { r -= c.r; g -= c.g; b -= c.b; return *this; }
		inline bgr565& operator*= (const bgr565& c) { r *= c.r; g *= c.g; b *= c.b; return *this; }
		inline bgr565& operator/= (const bgr565& c) { r /= c.r; g /= c.g; b /= c.b; return *this; }
	};
	inline bgr565 operator+ (const bgr565& c1, const bgr565& c2) { return bgr565 (c1.r + c2.r, c1.g + c2.g, c1.b + c2.b); }
	inline bgr565 operator- (const bgr565& c1, const bgr565& c2) { return bgr565 (c1.r - c2.r, c1.g - c2.g, c1.b - c2.b); }
	inline bgr565 operator* (const bgr565& c1, const bgr565& c2) { return bgr565 (c1.r * c2.r, c1.g * c2.g, c1.b * c2.b); }
	inline bgr565 operator/ (const bgr565& c1, const bgr565& c2) { return bgr565 (c1.r / c2.r, c1.g / c2.g, c1.b / c2.b); }

#	if COMPILER_MSVC
#		pragma pack (pop)
#	else
#		pragma pack ()
#	endif

	inline rgba::operator rgb() const noexcept { return rgb (r, g, b); }
	inline rgba::operator rgbaf() const noexcept { return rgbaf (r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f); }
	inline rgba::operator bgra() const noexcept { return bgra (r, g, b, a); }
	inline rgba::operator bgr() const noexcept { return bgr (r, g, b); }
	inline rgba::operator bgra4() const noexcept { return bgra4 (r / 17, g / 17, b / 17, a / 17); }
	inline rgba::operator bgr565() const noexcept { return bgr565 (r / 8, g / 4, b / 8); }

	inline rgb::operator rgba() const noexcept { return rgba (r, g, b, 255); }
	inline rgb::operator rgbaf() const noexcept { return rgbaf (r / 255.0f, g / 255.0f, b / 255.0f, 1); }
	inline rgb::operator bgra() const noexcept { return bgra (r, g, b, 255); }
	inline rgb::operator bgr() const noexcept { return bgr (r, g, b); }
	inline rgb::operator bgra4() const noexcept { return bgra4 (r / 17, g / 17, b / 17, 15); }
	inline rgb::operator bgr565() const noexcept { return bgr565 (r / 8, g / 4, b / 8); }
	
	inline rgbaf::operator rgba() const noexcept { return rgba ((uint8_t)r * 255, (uint8_t)g * 255, (uint8_t)b * 255, (uint8_t)a * 255); }
	inline rgbaf::operator rgb() const noexcept { return rgb ((uint8_t)r * 255, (uint8_t)g * 255, (uint8_t)b * 255); }
	inline rgbaf::operator bgra() const noexcept { return bgra ((uint8_t)r * 255, (uint8_t)g * 255, (uint8_t)b * 255, (uint8_t)a * 255); }
	inline rgbaf::operator bgr() const noexcept { return bgr ((uint8_t)r * 255, (uint8_t)g * 255, (uint8_t)b * 255); }
	inline rgbaf::operator bgra4() const noexcept { return bgra4 ((uint8_t)r * 15, (uint8_t)g * 15, (uint8_t)b * 15, (uint8_t)a * 15); }
	inline rgbaf::operator bgr565() const noexcept { return bgr565 ((uint8_t)r * 31, (uint8_t)g * 63, (uint8_t)b * 31); }
	
	inline bgra::operator rgba() const noexcept { return rgba (r, g, b, a); }
	inline bgra::operator rgb() const noexcept { return rgb (r, g, b); }
	inline bgra::operator rgbaf() const noexcept { return rgbaf (r / 255.0f, g / 255.0f, b / 255.0f); }
	inline bgra::operator bgr() const noexcept { return bgr (r, g, b); }
	inline bgra::operator bgra4() const noexcept { return bgra4 (r / 17, g / 17, b / 17, a / 17); }
	inline bgra::operator bgr565() const noexcept { return bgr565 (r / 8, g / 4, b / 8); }
	
	inline bgr::operator rgba() const noexcept { return rgba (r, g, b, 255); }
	inline bgr::operator rgb() const noexcept { return rgb (r, g, b); }
	inline bgr::operator rgbaf() const noexcept { return rgbaf (r / 255.0f, g / 255.0f, b / 255.0f, 1); }
	inline bgr::operator bgra() const noexcept { return bgra (r, g, b, 255); }
	inline bgr::operator bgra4() const noexcept { return bgra4 (r / 17, g / 17, b / 17, 15); }
	inline bgr::operator bgr565() const noexcept { return bgr565 (r / 8, g / 4, b / 8); }
	
	inline bgra4::operator rgba() const noexcept { return rgba (r * 17, g * 17, b * 17, a * 17); }
	inline bgra4::operator rgb() const noexcept { return rgb (r * 17, g * 17, b * 17); }
	inline bgra4::operator rgbaf() const noexcept { return rgbaf (r / 15.0f, g / 15.0f, b / 15.0f, a / 15.0f); }
	inline bgra4::operator bgra() const noexcept { return bgra (r * 17, g * 17, b * 17, a * 17); }
	inline bgra4::operator bgr() const noexcept { return bgr (r * 17, g * 17, b * 17); }
	inline bgra4::operator bgr565() const noexcept { return bgr565 (r * 2, g * 4, b * 2); }
	
	inline bgr565::operator rgba() const noexcept { return rgba (r * 8, g * 4, b * 8, 255); }
	inline bgr565::operator rgb() const noexcept { return rgb (r * 8, g * 4, b * 8); }
	inline bgr565::operator rgbaf() const noexcept { return rgbaf (r / 31.0f, g / 63.0f, b / 31.0f, 1); }
	inline bgr565::operator bgra() const noexcept { return bgra (r * 8, g * 4, b * 8, 255); }
	inline bgr565::operator bgr() const noexcept { return bgr (r * 8, g * 4, b * 8); }
	inline bgr565::operator bgra4() const noexcept { return bgra4 ( r / 2, g / 4, b / 2, 15 ); }
	
	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Y/Cb(U)/Cr(V) Color Types
	//
	////////////////////////////////////////////////////////////////////////////////////////
	
	// https://stackoverflow.com/questions/1737726/how-to-perform-rgb-yuv-conversion-in-c-c , Modified
// RGB -> YUV
#define RGB2Y(R, G, B) dseed::saturate(( (  66 * (R) + 129 * (G) +  25 * (B) + 128) >> 8) +  16)
#define RGB2U(R, G, B) dseed::saturate(( ( -38 * (R) -  74 * (G) + 112 * (B) + 128) >> 8) + 128)
#define RGB2V(R, G, B) dseed::saturate(( ( 112 * (R) -  94 * (G) -  18 * (B) + 128) >> 8) + 128)
// YUV -> RGB
#define C(Y) ( (Y) - 16  )
#define D(U) ( (U) - 128 )
#define E(V) ( (V) - 128 )
#define YUV2R(Y, U, V) dseed::saturate(( 298 * C(Y)              + 409 * E(V) + 128) >> 8)
#define YUV2G(Y, U, V) dseed::saturate(( 298 * C(Y) - 100 * D(U) - 208 * E(V) + 128) >> 8)
#define YUV2B(Y, U, V) dseed::saturate(( 298 * C(Y) + 516 * D(U)              + 128) >> 8)

	struct yuva;
	struct yuv;

#	if COMPILER_MSVC
#		pragma pack (push, 1)
#	else
#		pragma pack (1)
#	endif

	struct yuva
	{
		union
		{
			struct { uint8_t v, u, y, a; };
			uint32_t color;
		};
		yuva () = default;
		inline yuva (uint8_t y, uint8_t u, uint8_t v, uint8_t a = 255)
			: y (y), u (v), v (v), a (a)
		{ }
		inline yuva (uint32_t yuva)
			: color (yuva)
		{ }
		inline yuva (const bgra& bgra)
		{
			y = RGB2Y (bgra.r, bgra.g, bgra.b);
			u = RGB2U (bgra.r, bgra.g, bgra.b);
			v = RGB2V (bgra.r, bgra.g, bgra.b);
			a = bgra.a;
		}

		inline operator bgra () const noexcept
		{
			return bgra (
				YUV2R (y, u, v),
				YUV2G (y, u, v),
				YUV2B (y, u, v),
				a
			);
		}
	};

	struct yuv
	{
		union
		{
			struct { uint8_t v, u, y; };
			uint24_t color;
		};
		yuv () = default;
		inline yuv (uint8_t y, uint8_t u, uint8_t v)
			: y (y), u (v), v (v)
		{ }
		inline yuv (uint24_t yuv)
			: color (yuv)
		{ }
		inline yuv (const bgr& bgr)
		{
			y = RGB2Y (bgr.r, bgr.g, bgr.b);
			u = RGB2U (bgr.r, bgr.g, bgr.b);
			v = RGB2V (bgr.r, bgr.g, bgr.b);
		}

		inline operator bgr () const noexcept
		{
			return bgr (
				YUV2R (y, u, v),
				YUV2G (y, u, v),
				YUV2B (y, u, v)
			);
		}
	};

	struct yuyv
	{
		union
		{
			struct { uint8_t y1, u, y2, v; };
			uint32_t color;
		};
		yuyv () = default;
		inline yuyv (uint8_t y1, uint8_t u, uint8_t y2, uint8_t v)
			: y1 (y1), u (u), y2 (y2), v (v)
		{ }
		inline yuyv (uint32_t yuyv)
			: color (color)
		{ }
		inline yuyv (const bgr & bgr1, const bgr & bgr2)
			: yuyv ((yuv)bgr1, (yuv)bgr2)
		{ }
		inline yuyv (const yuv& yuv1, const yuv& yuv2)
		{
			y1 = yuv1.y;
			y2 = yuv2.y;
			u = yuv1.u;//(uint8_t)((yuv1.u + yuv2.u) / 2);
			v = yuv1.v;//(uint8_t)((yuv1.v + yuv2.v) / 2);
		}

		inline error_t to_bgr (bgr* bgr1, bgr* bgr2) const
		{
			if (bgr1 == nullptr || bgr2 == nullptr)
				return dseed::error_invalid_args;
			yuv yuv1 (y1, u, v), yuv2 (y2, u, v);
			*bgr1 = (bgr)yuv1;
			*bgr2 = (bgr)yuv2;
			return dseed::error_good;
		}
		inline error_t to_yuv444 (yuv* yuv1, yuv* yuv2) const
		{
			if (yuv1 == nullptr || yuv2 == nullptr)
				return dseed::error_invalid_args;
			*yuv1 = yuv (y1, u, v);
			*yuv2 = yuv (y2, u, v);
			return dseed::error_good;
		}
	};

#	if COMPILER_MSVC
#		pragma pack (pop)
#	else
#		pragma pack ()
#	endif

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Grayscale Color Types
	//
	////////////////////////////////////////////////////////////////////////////////////////

	struct grayscale;
	struct grayscalef;

#	if COMPILER_MSVC
#		pragma pack (push, 1)
#	else
#		pragma pack (1)
#	endif

	struct grayscale
	{
		uint8_t color;
		grayscale () = default;
		inline grayscale (uint8_t grayscale) : color (grayscale) { }
		inline grayscale (const rgb& rgb)
			: color ((uint8_t)maximum (0.0, minimum (255.0, +0.2627 * rgb.r + +0.678 * rgb.g + +0.0593 * rgb.b)))
		{ }

		inline operator rgb () const noexcept { return rgb (color, color, color); }
	};

	struct grayscalef
	{
		float color;
		grayscalef () = default;
		inline grayscalef (float grayscale) : color (grayscale) { }
		inline grayscalef (const rgbaf& rgbaf)
			: color ((float)maximum (0.0, minimum (1.0, (+0.2627 * rgbaf.r + +0.678 * rgbaf.g + +0.0593 * rgbaf.b))))
		{ }

		inline operator rgbaf () const noexcept { return rgbaf (color, color, color, 1); }
	};

#	if COMPILER_MSVC
#		pragma pack (pop)
#	else
#		pragma pack ()
#	endif
}

#endif