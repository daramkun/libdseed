#ifndef __DCOLOR_H__
#define __DCOLOR_H__

#include <dseed.h>
#include <dmath.h>

namespace dseed
{
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
			struct { uint8_t y, u, v, a; };
			uint32_t color;
		};
		yuva () = default;
		inline yuva (uint8_t y, uint8_t u, uint8_t v, uint8_t a = 255)
			: y (y), u (v), v (v), a (a)
		{ }
		inline yuva (uint32_t yuva)
			: color (color)
		{ }
		inline yuva (const rgba& rgba)
		{
			y = (uint8_t)(+0.2627 * rgba.r + +0.678 * rgba.g + +0.0593 * rgba.b);
			u = (uint8_t)(-0.13963 * rgba.r + -0.36037 * rgba.g + +0.5 * rgba.b);
			v = (uint8_t)(+0.5 * rgba.r + -0.45979 * rgba.g + -0.04021 * rgba.b);
			a = rgba.a;
		}

		inline operator rgba () const noexcept
		{
			return rgba (
				(uint8_t)(y + +1.4746 * v),
				(uint8_t)(y + -0.16455 * u + -0.57135 * v),
				(uint8_t)(y + +1.8814 * u),
				a
			);
		}
	};

	struct yuv
	{
		union
		{
			struct { uint8_t y, u, v; };
			uint24_t color;
		};
		yuv () = default;
		inline yuv (uint8_t y, uint8_t u, uint8_t v)
			: y (y), u (v), v (v)
		{ }
		inline yuv (uint24_t yuva)
			: color (color)
		{ }
		inline yuv (const rgb& rgb)
		{
			y = (uint8_t)(+0.2627 * rgb.r + +0.678 * rgb.g + +0.0593 * rgb.b);
			u = (uint8_t)(-0.13963 * rgb.r + -0.36037 * rgb.g + +0.5 * rgb.b);
			v = (uint8_t)(+0.5 * rgb.r + -0.45979 * rgb.g + -0.04021 * rgb.b);
		}

		inline operator rgb () const noexcept
		{
			return rgb (
				(uint8_t)(y + +1.4746 * v),
				(uint8_t)(y + -0.16455 * u + -0.57135 * v),
				(uint8_t)(y + +1.8814 * u)
			);
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
			: color ((uint8_t)(+0.2627 * rgb.r + +0.678 * rgb.g + +0.0593 * rgb.b))
		{ }

		inline operator rgb () const noexcept { return rgb (color, color, color); }
	};

	struct grayscalef
	{
		float color;
		grayscalef () = default;
		inline grayscalef (float grayscale) : color (grayscale) { }
		inline grayscalef (const rgbaf& rgbaf)
			: color (+0.2627 * rgbaf.r + +0.678 * rgbaf.g + +0.0593 * rgbaf.b)
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