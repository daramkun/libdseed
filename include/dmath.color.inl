#ifndef __DMATH_COLOR_INL__
#define __DMATH_COLOR_INL__

namespace dseed
{
	struct rgbaf;
	struct rgbai32;
	struct bgrai32;
	struct bgrai16;

	struct rgbaf : public float4
	{
	public:
		rgbaf () = default;
		inline rgbaf (float r, float g, float b, float a = 1.0f) noexcept
			: float4 (r, g, b, a)
		{ }
		inline rgbaf (const rgbai32& c) noexcept;
		inline rgbaf (const bgrai32& c) noexcept;
		inline rgbaf (const bgrai16& c) noexcept;
	};

	struct rgbai32
	{
	public:
		union
		{
			struct { unsigned char r, g, b, a; };
			int rgba;
		};

	public:
		rgbai32 () = default;
		rgbai32 (unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255)
			: r (r), g (g), b (b), a (a)
		{ }
		rgbai32 (int rgba)
			: rgba (rgba)
		{ }
		inline rgbai32 (const rgbaf& c) noexcept;
		inline rgbai32 (const bgrai32& c) noexcept;
		inline rgbai32 (const bgrai16& c) noexcept;

	public:
		operator int () { return rgba; }
	};

	struct bgrai32
	{
	public:
		union
		{
			struct { unsigned char b, g, r, a; };
			int bgra;
		};

	public:
		bgrai32 () = default;
		bgrai32 (unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255)
			: r (r), g (g), b (b), a (a)
		{ }
		bgrai32 (int bgra)
			: bgra (bgra)
		{ }
		inline bgrai32 (const rgbaf& c) noexcept;
		inline bgrai32 (const rgbai32& c) noexcept;
		inline bgrai32 (const bgrai16& c) noexcept;

	public:
		operator int () { return bgra; }
	};

	struct bgrai16
	{
	public:
		union
		{
			struct
			{
				unsigned char b : 4;
				unsigned char g : 4;
				unsigned char r : 4;
				unsigned char a : 4;
			};
			short bgra;
		};

	public:
		bgrai16 () = default;
		bgrai16 (unsigned char r, unsigned char g, unsigned char b, unsigned char a = 15)
			: r (r), g (g), b (b), a (a)
		{ }
		bgrai16 (short bgra)
			: bgra (bgra)
		{ }
		inline bgrai16 (const rgbaf& c) noexcept;
		inline bgrai16 (const rgbai32& c) noexcept;
		inline bgrai16 (const bgrai32& c) noexcept;

	public:
		operator short () { return bgra; }
	};
	using dxt3 = bgrai16;

	inline rgbaf::rgbaf (const rgbai32& c) noexcept
	{
		x = c.r / 255.0f;
		y = c.g / 255.0f;
		z = c.b / 255.0f;
		w = c.a / 255.0f;
	}
	inline rgbaf::rgbaf (const bgrai32& c) noexcept
	{
		x = c.r / 255.0f;
		y = c.g / 255.0f;
		z = c.b / 255.0f;
		w = c.a / 255.0f;
	}
	inline rgbaf::rgbaf (const bgrai16& c) noexcept
	{
		x = c.r / 15.0f;
		y = c.g / 15.0f;
		z = c.b / 15.0f;
		w = c.a / 15.0f;
	}

	inline rgbai32::rgbai32 (const rgbaf& c) noexcept
	{
		r = (int)(c.x * 255);
		g = (int)(c.y * 255);
		b = (int)(c.z * 255);
		a = (int)(c.w * 255);
	}
	inline rgbai32::rgbai32 (const bgrai32& c) noexcept
	{
		r = c.r;
		g = c.g;
		b = c.b;
		a = c.a;
	}
	inline rgbai32::rgbai32 (const bgrai16& c) noexcept
	{
		r = c.r * 17;
		g = c.g * 17;
		b = c.b * 17;
		a = c.a * 17;
	}

	inline bgrai32::bgrai32 (const rgbaf& c) noexcept
	{
		r = (int)(c.x * 255);
		g = (int)(c.y * 255);
		b = (int)(c.z * 255);
		a = (int)(c.w * 255);
	}
	inline bgrai32::bgrai32 (const rgbai32& c) noexcept
	{
		r = c.r;
		g = c.g;
		b = c.b;
		a = c.a;
	}
	inline bgrai32::bgrai32 (const bgrai16& c) noexcept
	{
		r = c.r * 17;
		g = c.g * 17;
		b = c.b * 17;
		a = c.a * 17;
	}

	inline bgrai16::bgrai16 (const rgbaf& c) noexcept
	{
		r = (int)(c.x * 15);
		g = (int)(c.y * 15);
		b = (int)(c.z * 15);
		a = (int)(c.w * 15);
	}
	inline bgrai16::bgrai16 (const rgbai32& c) noexcept
	{
		r = c.r / 17;
		g = c.g / 17;
		b = c.b / 17;
		a = c.a / 17;
	}
	inline bgrai16::bgrai16 (const bgrai32& c) noexcept
	{
		r = c.r / 17;
		g = c.g / 17;
		b = c.b / 17;
		a = c.a / 17;
	}
}

#endif