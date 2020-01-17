#ifndef __DSEED_MATH_VECTORS_SIMD_DEF_H__
#define __DSEED_MATH_VECTORS_SIMD_DEF_H__

namespace dseed
{
	////////////////////////////////////////////////////////////////////////////////////////
	//
	// 32-bit Floating Point 4D Vector
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct f32x4_arith
	{
	public:
		union
		{
			struct { float _x, _y, _z, _w; };
			float _arr[4];
		};

	public:
		inline f32x4_arith () noexcept = default;
		inline f32x4_arith (float v) noexcept : _x (v), _y (v), _z (v), _w (v) { }
		inline f32x4_arith (float x, float y, float z, float w) noexcept : _x (x), _y (y), _z (z), _w (w) { }
		inline f32x4_arith (READS (4) const float* arr) noexcept : _x (arr[0]), _y (arr[1]), _z (arr[2]), _w (arr[3]) { }

	public:
		inline f32x4_arith splat_x () const noexcept { return f32x4_arith (_x, _x, _x, _x); }
		inline f32x4_arith splat_y () const noexcept { return f32x4_arith (_y, _y, _y, _y); }
		inline f32x4_arith splat_z () const noexcept { return f32x4_arith (_z, _z, _z, _z); }
		inline f32x4_arith splat_w () const noexcept { return f32x4_arith (_w, _w, _w, _w); }

	public:
		inline float x () const noexcept { return _x; }
		inline float y () const noexcept { return _y; }
		inline float z () const noexcept { return _z; }
		inline float w () const noexcept { return _w; }

	public:
		inline void store (WRITES (4) float* arr) const noexcept { arr[0] = _x; arr[1] = _y; arr[2] = _z; arr[3] = _w; }
	};

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// 32-bit Signed Integer 4D Vector
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct i32x4_arith
	{
	public:
		union
		{
			struct { int _x, _y, _z, _w; };
			int _arr[4];
			unsigned char _arr8[16];
		};

	public:
		inline i32x4_arith () noexcept = default;
		inline i32x4_arith (int v) noexcept : _x (v), _y (v), _z (v), _w (v) { }
		inline i32x4_arith (int x, int y, int z, int w) noexcept : _x (x), _y (y), _z (z), _w (w) { }
		inline i32x4_arith (READS (4) const int* arr) noexcept : _x (arr[0]), _y (arr[1]), _z (arr[2]), _w (arr[3]) { }

	public:
		inline i32x4_arith splat_x () const noexcept { return i32x4_arith (_x, _x, _x, _x); }
		inline i32x4_arith splat_y () const noexcept { return i32x4_arith (_y, _y, _y, _y); }
		inline i32x4_arith splat_z () const noexcept { return i32x4_arith (_z, _z, _z, _z); }
		inline i32x4_arith splat_w () const noexcept { return i32x4_arith (_w, _w, _w, _w); }

	public:
		inline int x () const noexcept { return _x; }
		inline int y () const noexcept { return _y; }
		inline int z () const noexcept { return _z; }
		inline int w () const noexcept { return _w; }

	public:
		inline void store (WRITES (4) int* arr) const noexcept { arr[0] = _x; arr[1] = _y; arr[2] = _z; arr[3] = _w; }
	};

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// 32-bit Floating Point 4*4 Matrix
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct f32x4x4_arith
	{
	public:
		union
		{
			struct
			{
				float _11, _12, _13, _14
					, _21, _22, _23, _24
					, _31, _32, _33, _34
					, _41, _42, _43, _44;
			};
			float _arr[16];
			struct
			{
				f32x4_arith _column1, _column2, _column3, _column4;
			};
			f32x4_arith _columns[4];
		};

		inline f32x4x4_arith () noexcept = default;
		inline f32x4x4_arith (float v) noexcept
			: _11 (v), _12 (v), _13 (v), _14 (v)
			, _21 (v), _22 (v), _23 (v), _24 (v)
			, _31 (v), _32 (v), _33 (v), _34 (v)
			, _41 (v), _42 (v), _43 (v), _44 (v)
		{ }
		inline f32x4x4_arith (READS (16) const float* arr) noexcept
			: _11 (arr[0]), _12 (arr[1]), _13 (arr[2]), _14 (arr[3])
			, _21 (arr[4]), _22 (arr[5]), _23 (arr[6]), _24 (arr[7])
			, _31 (arr[8]), _32 (arr[9]), _33 (arr[10]), _34 (arr[11])
			, _41 (arr[12]), _42 (arr[13]), _43 (arr[14]), _44 (arr[15])
		{ }
		inline f32x4x4_arith (const f32x4_arith& c1, const f32x4_arith& c2, const f32x4_arith& c3, const f32x4_arith& c4) noexcept
			: _column1 (c1), _column2 (c2), _column3 (c3), _column4 (c4)
		{ }
		inline f32x4x4_arith (
			float m11, float m12, float m13, float m14,
			float m21, float m22, float m23, float m24,
			float m31, float m32, float m33, float m34,
			float m41, float m42, float m43, float m44) noexcept
			: _11 (m11), _12 (m12), _13 (m13), _14 (m14)
			, _21 (m21), _22 (m22), _23 (m23), _24 (m24)
			, _31 (m31), _32 (m32), _33 (m33), _34 (m34)
			, _41 (m41), _42 (m42), _43 (m43), _44 (m44)
		{ }

	public:
		inline f32x4_arith column1 () const noexcept { return _column1; }
		inline f32x4_arith column2 () const noexcept { return _column2; }
		inline f32x4_arith column3 () const noexcept { return _column3; }
		inline f32x4_arith column4 () const noexcept { return _column4; }

	public:
		inline float m11 () const noexcept { return _11; }
		inline float m12 () const noexcept { return _12; }
		inline float m13 () const noexcept { return _13; }
		inline float m14 () const noexcept { return _14; }

		inline float m21 () const noexcept { return _21; }
		inline float m22 () const noexcept { return _22; }
		inline float m23 () const noexcept { return _23; }
		inline float m24 () const noexcept { return _24; }

		inline float m31 () const noexcept { return _31; }
		inline float m32 () const noexcept { return _32; }
		inline float m33 () const noexcept { return _33; }
		inline float m34 () const noexcept { return _34; }

		inline float m41 () const noexcept { return _41; }
		inline float m42 () const noexcept { return _42; }
		inline float m43 () const noexcept { return _43; }
		inline float m44 () const noexcept { return _44; }

	public:
		inline void store (WRITES (16) float* arr) const noexcept
		{
			arr[0] = _11; arr[1] = _12; arr[2] = _13; arr[3] = _14;
			arr[4] = _21; arr[5] = _22; arr[6] = _23; arr[7] = _24;
			arr[8] = _31; arr[9] = _32; arr[10] = _33; arr[11] = _34;
			arr[12] = _41; arr[13] = _42; arr[14] = _43; arr[15] = _44;
		}
	};

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Vector/Matrix Casting and Reinterpreting Operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline f32x4_arith cast_if (const i32x4_arith& v) noexcept { return f32x4_arith ((float)v.x (), (float)v.y (), (float)v.z (), (float)v.w ()); }
	inline i32x4_arith cast_fi (const f32x4_arith& v) noexcept { return i32x4_arith ((int)v.x (), (int)v.y (), (int)v.z (), (int)v.w ()); }
	inline f32x4_arith reinterpret_if (const i32x4_arith& v) noexcept { return *reinterpret_cast<const f32x4_arith*>(&v); }
	inline i32x4_arith reinterpret_fi (const f32x4_arith& v) noexcept { return *reinterpret_cast<const i32x4_arith*>(&v); }

	inline int movemask (const f32x4_arith& v) noexcept
	{
		i32x4_arith iv = reinterpret_fi (v);
		return (iv.x () & 1) | (iv.y () & 2) | (iv.z () & 4) | (iv.w () & 8);
	}
	inline int movemask (const i32x4_arith& v) noexcept
	{
		return (v.x () & 0x0000000ff) | (v.y () & 0x0000ff00) | (v.z () & 0x00ff0000) | (v.w () & 0xff0000000);
	}
	inline int movemask (const f32x4x4_arith& m) noexcept
	{
		return ((movemask (m.column1 ()) & 0xf) | ((movemask (m.column2 ()) << 4) & 0xf0)
			| ((movemask (m.column3 ()) << 8) & 0xf00) | ((movemask (m.column4 ()) << 12) & 0xf000));
	}

	template<int x, int y, int z, int w>
	inline f32x4_arith permute (const f32x4_arith& v) noexcept
	{
		return f32x4_arith (v._arr[(int)x], v._arr[(int)y], v._arr[(int)z], v._arr[(int)w]);
	}
	template<int x, int y, int z, int w>
	inline f32x4_arith shuffle (const f32x4_arith& v1, const f32x4_arith& v2) noexcept
	{
		return f32x4_arith (
			(x >= 0 && x <= 3) ? v1._arr[x] : v2._arr[4 - x],
			(y >= 0 && y <= 3) ? v1._arr[y] : v2._arr[4 - y],
			(z >= 0 && z <= 3) ? v2._arr[z] : v1._arr[4 - z],
			(w >= 0 && w <= 3) ? v2._arr[w] : v1._arr[4 - w]
		);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// 32-bit Floating Point 4D Vector Operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline f32x4_arith add (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return f32x4_arith (v1.x () + v2.x (), v1.y () + v2.y (), v1.z () + v2.z (), v1.w () + v2.w ()); }
	inline f32x4_arith subtract (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return f32x4_arith (v1.x () - v2.x (), v1.y () - v2.y (), v1.z () - v2.z (), v1.w () - v2.w ()); }
	inline f32x4_arith negate (const f32x4_arith& v) noexcept { return f32x4_arith (-v.x (), -v.y (), -v.z (), -v.w ()); }
	inline f32x4_arith multiply (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return f32x4_arith (v1.x () * v2.x (), v1.y () * v2.y (), v1.z () * v2.z (), v1.w () * v2.w ()); }
	inline f32x4_arith multiply (const f32x4_arith& v, float s) noexcept { return f32x4_arith (v.x () * s, v.y () * s, v.z () * s, v.w () * s); }
	inline f32x4_arith multiply (float s, const f32x4_arith& v) noexcept { return multiply (v, s); }
	inline f32x4_arith divide (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return f32x4_arith (v1.x () / v2.x (), v1.y () / v2.y (), v1.z () / v2.z (), v1.w () / v2.w ()); }
	inline f32x4_arith divide (const f32x4_arith& v, float s) noexcept { return f32x4_arith (v.x () / s, v.y () / s, v.z () / s, v.w () / s); }

	inline f32x4_arith fma (const f32x4_arith& mv1, const f32x4_arith& mv2, const f32x4_arith& av) noexcept { return add (multiply (mv1, mv2), av); }
	inline f32x4_arith fms (const f32x4_arith& mv1, const f32x4_arith& mv2, const f32x4_arith& sv) noexcept { return subtract (multiply (mv1, mv2), sv); }
	inline f32x4_arith fnms (const f32x4_arith& mv1, const f32x4_arith& mv2, const f32x4_arith& sv) noexcept { return subtract (sv, multiply (mv1, mv2)); }

	inline f32x4_arith and (const f32x4_arith& v1, const f32x4_arith& v2) noexcept
	{
		i32x4_arith iv1 = reinterpret_fi (v1), iv2 = reinterpret_fi (v2);
		iv1 = i32x4_arith (iv1.x () & iv2.x (), iv1.y () & iv2.y (), iv1.z () & iv2.z (), iv1.w () & iv2.w ());
		return reinterpret_if (iv1);
	}
	inline f32x4_arith or (const f32x4_arith & v1, const f32x4_arith & v2) noexcept
	{
		i32x4_arith iv1 = reinterpret_fi (v1), iv2 = reinterpret_fi (v2);
		iv1 = i32x4_arith (iv1.x () | iv2.x (), iv1.y () | iv2.y (), iv1.z () | iv2.z (), iv1.w () | iv2.w ());
		return reinterpret_if (iv1);
	}
	inline f32x4_arith xor (const f32x4_arith& v1, const f32x4_arith& v2) noexcept
	{
		i32x4_arith iv1 = reinterpret_fi (v1), iv2 = reinterpret_fi (v2);
		iv1 = i32x4_arith (iv1.x () ^ iv2.x (), iv1.y () ^ iv2.y (), iv1.z () ^ iv2.z (), iv1.w () ^ iv2.w ());
		return reinterpret_if (iv1);
	}
	inline f32x4_arith not (const f32x4_arith& v) noexcept
	{
		i32x4_arith iv = reinterpret_fi (v);
		iv = i32x4_arith (~iv.x (), ~iv.y (), ~iv.z (), ~iv.w ());
		return reinterpret_if (iv);
	}

	inline f32x4_arith operator+ (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return add (v1, v2); }
	inline f32x4_arith operator- (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return subtract (v1, v2); }
	inline f32x4_arith operator- (const f32x4_arith& v) noexcept { return negate (v); }
	inline f32x4_arith operator* (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return multiply (v1, v2); }
	inline f32x4_arith operator* (const f32x4_arith& v, float s) noexcept { return multiply (v, s); }
	inline f32x4_arith operator* (float s, const f32x4_arith& v) noexcept { return multiply (s, v); }
	inline f32x4_arith operator/ (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return divide (v1, v2); }
	inline f32x4_arith operator/ (const f32x4_arith& v, float s) noexcept { return divide (v, s); }
	inline f32x4_arith operator& (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return and (v1, v2); }
	inline f32x4_arith operator| (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return or (v1, v2); }
	inline f32x4_arith operator^ (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return xor (v1, v2); }
	inline f32x4_arith operator~ (const f32x4_arith& v) noexcept { return not (v); }

	inline f32x4_arith equals (const f32x4_arith& v1, const f32x4_arith& v2) noexcept
	{
		return reinterpret_if (
			i32x4_arith (
				v1.x () == v2.x () ? 0xffffffff : 0,
				v1.y () == v2.y () ? 0xffffffff : 0,
				v1.z () == v2.z () ? 0xffffffff : 0,
				v1.w () == v2.w () ? 0xffffffff : 0
			)
		);
	}
	inline f32x4_arith not_equals (const f32x4_arith& v1, const f32x4_arith& v2) noexcept
	{
		return reinterpret_if (
			i32x4_arith (
				v1.x () != v2.x () ? 0xffffffff : 0,
				v1.y () != v2.y () ? 0xffffffff : 0,
				v1.z () != v2.z () ? 0xffffffff : 0,
				v1.w () != v2.w () ? 0xffffffff : 0
			)
		);
	}
	inline f32x4_arith lesser (const f32x4_arith& v1, const f32x4_arith& v2) noexcept
	{
		return reinterpret_if (
			i32x4_arith (
				v1.x () < v2.x () ? 0xffffffff : 0,
				v1.y () < v2.y () ? 0xffffffff : 0,
				v1.z () < v2.z () ? 0xffffffff : 0,
				v1.w () < v2.w () ? 0xffffffff : 0
			)
		);
	}
	inline f32x4_arith lesser_equals (const f32x4_arith& v1, const f32x4_arith& v2) noexcept
	{
		return reinterpret_if (
			i32x4_arith (
				v1.x () <= v2.x () ? 0xffffffff : 0,
				v1.y () <= v2.y () ? 0xffffffff : 0,
				v1.z () <= v2.z () ? 0xffffffff : 0,
				v1.w () <= v2.w () ? 0xffffffff : 0
			)
		);
	}
	inline f32x4_arith greater (const f32x4_arith& v1, const f32x4_arith& v2) noexcept
	{
		return reinterpret_if (
			i32x4_arith (
				v1.x () > v2.x () ? 0xffffffff : 0,
				v1.y () > v2.y () ? 0xffffffff : 0,
				v1.z () > v2.z () ? 0xffffffff : 0,
				v1.w () > v2.w () ? 0xffffffff : 0
			)
		);
	}
	inline f32x4_arith greater_equals (const f32x4_arith& v1, const f32x4_arith& v2) noexcept
	{
		return reinterpret_if (
			i32x4_arith (
				v1.x () >= v2.x () ? 0xffffffff : 0,
				v1.y () >= v2.y () ? 0xffffffff : 0,
				v1.z () >= v2.z () ? 0xffffffff : 0,
				v1.w () >= v2.w () ? 0xffffffff : 0
			)
		);
	}

	inline bool operator== (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return 0xf == movemask (equals (v1, v2)); }
	inline bool operator!= (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return 0xf == movemask (not_equals (v1, v2)); }
	inline bool operator< (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return 0xf == movemask (lesser (v1, v2)); }
	inline bool operator<= (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return 0xf == movemask (lesser_equals (v1, v2)); }
	inline bool operator> (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return 0xf == movemask (greater (v1, v2)); }
	inline bool operator>= (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return 0xf == movemask (greater_equals (v1, v2)); }

	inline float dot2df (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return v1.x () * v2.x () + v1.y () * v2.y (); }
	inline float dot3df (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return v1.x () * v2.x () + v1.y () * v2.y () + v1.z () * v2.z (); }
	inline float dot4df (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return v1.x () * v2.x () + v1.y () * v2.y () + v1.z () * v2.z () + v1.w () * v2.w (); }

	inline f32x4_arith dot2d (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return f32x4_arith (dot2df (v1, v2)); }
	inline f32x4_arith dot3d (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return f32x4_arith (dot3df (v1, v2)); }
	inline f32x4_arith dot4d (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return f32x4_arith (dot4df (v1, v2)); }

	inline f32x4_arith cross2d (const f32x4_arith& v1, const f32x4_arith& v2) noexcept { return f32x4_arith (v1.x () * v2.y (), v1.y () * v2.x (), 0, 0); }
	inline f32x4_arith cross3d (const f32x4_arith& v1, const f32x4_arith& v2) noexcept
	{
		return f32x4_arith (v1.y () * v2.z () - v1.z () * v2.y (), v1.z () * v2.x () - v1.x () * v2.z (), v1.x () * v2.y () - v1.y () * v2.x (), 0);
	}
	inline f32x4_arith cross4d (const f32x4_arith& v1, const f32x4_arith& v2, const f32x4_arith& v3) noexcept
	{
		return f32x4_arith (
			+(v1.y () * ((v2.z () * v3.w ()) - (v2.w () * v3.z ()))) - (v1.z () * ((v2.y () * v3.w ()) - (v2.w () * v3.y ()))) + (v1.w () * ((v2.y () * v3.z ()) - (v2.z () * v3.y ()))),
			-(v1.x () * ((v2.z () * v3.w ()) - (v2.w () * v3.z ()))) + (v1.z () * ((v2.x () * v3.w ()) - (v2.w () * v3.x ()))) - (v1.w () * ((v2.x () * v3.z ()) - (v2.z () * v3.x ()))),
			+(v1.x () * ((v2.y () * v3.w ()) - (v2.w () * v3.y ()))) - (v1.y () * ((v2.x () * v3.w ()) - (v2.w () * v3.x ()))) + (v1.w () * ((v2.x () * v3.y ()) - (v2.y () * v3.x ()))),
			-(v1.x () * ((v2.y () * v3.z ()) - (v2.z () * v3.y ()))) + (v1.y () * ((v2.x () * v3.z ()) - (v2.z () * v3.x ()))) - (v1.z () * ((v2.x () * v3.y ()) - (v2.y () * v3.x ())))
		);
	}

	inline f32x4_arith transform2d (const f32x4_arith& v, const f32x4x4_arith& m) noexcept
	{
		return fma (v.splat_x (), m.column1 (), fma (v.splat_y (), m.column2 (), m.column4 ()));
	}
	inline f32x4_arith transform3d (const f32x4_arith& v, const f32x4x4_arith& m) noexcept
	{
		return fma (v.splat_x (), m.column1 (), fma (v.splat_y (), m.column2 (), fma (v.splat_z (), m.column3 (), m.column4 ())));
	}
	inline f32x4_arith transform4d (const f32x4_arith& v, const f32x4x4_arith& m) noexcept
	{
		return fma (v.splat_x (), m.column1 (), fma (v.splat_y (), m.column2 (), fma (v.splat_z (), m.column3 (), multiply (v.splat_w (), m.column4 ()))));
	}

	inline f32x4_arith transform2dnorm (const f32x4_arith& v, const f32x4x4_arith& m) noexcept
	{
		return fma (v.splat_x (), m.column1 (), multiply (v.splat_y (), m.column2 ()));
	}
	inline f32x4_arith transform3dnorm (const f32x4_arith& v, const f32x4x4_arith& m) noexcept
	{
		return fma (v.splat_x (), m.column1 (), fma (v.splat_y (), m.column2 (), multiply (v.splat_z (), m.column3 ())));
	}

	inline float lengthsq2df (const f32x4_arith& v) noexcept { return dot2df (v, v); }
	inline float lengthsq3df (const f32x4_arith& v) noexcept { return dot3df (v, v); }
	inline float lengthsq4df (const f32x4_arith& v) noexcept { return dot4df (v, v); }

	inline f32x4_arith lengthsq2d (const f32x4_arith& v) noexcept { return dot2d (v, v); }
	inline f32x4_arith lengthsq3d (const f32x4_arith& v) noexcept { return dot3d (v, v); }
	inline f32x4_arith lengthsq4d (const f32x4_arith& v) noexcept { return dot4d (v, v); }

	inline float length2df (const f32x4_arith& v) noexcept { return sqrtf (v.x () * v.x () + v.y () * v.y ()); }
	inline float length3df (const f32x4_arith& v) noexcept { return sqrtf (v.x () * v.x () + v.y () * v.y () + v.z () * v.z ()); }
	inline float length4df (const f32x4_arith& v) noexcept { return sqrtf (v.x () * v.x () + v.y () * v.y () + v.z () * v.z () + v.w () * v.w ()); }

	inline f32x4_arith length2d (const f32x4_arith& v) noexcept { return f32x4_arith (length2df (v)); }
	inline f32x4_arith length3d (const f32x4_arith& v) noexcept { return f32x4_arith (length3df (v)); }
	inline f32x4_arith length4d (const f32x4_arith& v) noexcept { return f32x4_arith (length4df (v)); }

	inline f32x4_arith normalize2d (const f32x4_arith& v) noexcept { divide (v, length2d (v)); }
	inline f32x4_arith normalize3d (const f32x4_arith& v) noexcept { divide (v, length3d (v)); }
	inline f32x4_arith normalize4d (const f32x4_arith& v) noexcept { divide (v, length4d (v)); }

	inline f32x4_arith rcp (const f32x4_arith& v) noexcept { return f32x4_arith (1 / v.x (), 1 / v.y (), 1 / v.z (), 1 / v.w ()); }
	inline f32x4_arith pow (const f32x4_arith& v1, const f32x4_arith& v2) noexcept
	{
		return f32x4_arith (powf (v1.x (), v2.x ()), powf (v1.y (), v2.y ()), powf (v1.z (), v2.z ()), powf (v1.w (), v2.w ()));
	}
	inline f32x4_arith sqrt (const f32x4_arith& v) noexcept { return f32x4_arith (sqrtf (v.x ()), sqrtf (v.y ()), sqrtf (v.z ()), sqrtf (v.w ())); }
	inline f32x4_arith rsqrt (const f32x4_arith& v) noexcept { return rcp (sqrt (v)); }
	inline f32x4_arith round (const f32x4_arith& v) noexcept { return f32x4_arith (roundf (v.x ()), roundf (v.y ()), roundf (v.z ()), roundf (v.w ())); }
	inline f32x4_arith floor (const f32x4_arith& v) noexcept { return f32x4_arith (floorf (v.x ()), floorf (v.y ()), floorf (v.z ()), floorf (v.w ())); }
	inline f32x4_arith ceil (const f32x4_arith& v) noexcept { return f32x4_arith (ceilf (v.x ()), ceilf (v.y ()), ceilf (v.z ()), ceilf (v.w ())); }
	inline f32x4_arith abs (const f32x4_arith& v) noexcept { return f32x4_arith (fabs (v.x ()), fabs (v.y ()), fabs (v.z ()), fabs (v.w ())); }
	inline f32x4_arith sin (const f32x4_arith& v) noexcept { return f32x4_arith (sinf (v.x ()), sinf (v.y ()), sinf (v.z ()), sinf (v.w ())); }
	inline f32x4_arith cos (const f32x4_arith& v) noexcept { return f32x4_arith (cosf (v.x ()), cosf (v.y ()), cosf (v.z ()), cosf (v.w ())); }
	inline void sincos (const f32x4_arith& v, f32x4_arith* osin, f32x4_arith* ocos) noexcept { *osin = sin (v); *ocos = cos (v); }
	inline f32x4_arith tan (const f32x4_arith& v) noexcept { return f32x4_arith (tanf (v.x ()), tanf (v.y ()), tanf (v.z ()), tanf (v.w ())); }
	inline f32x4_arith asin (const f32x4_arith& v) noexcept { return f32x4_arith (asinf (v.x ()), asinf (v.y ()), asinf (v.z ()), asinf (v.w ())); }
	inline f32x4_arith acos (const f32x4_arith& v) noexcept { return f32x4_arith (acosf (v.x ()), acosf (v.y ()), acosf (v.z ()), acosf (v.w ())); }
	inline f32x4_arith atan (const f32x4_arith& v) noexcept { return f32x4_arith (atanf (v.x ()), atanf (v.y ()), atanf (v.z ()), atanf (v.w ())); }
	inline f32x4_arith atan2 (const f32x4_arith& vy, const f32x4_arith& vx) noexcept
	{
		return f32x4_arith (atan2f (vy.x (), vx.x ()), atan2f (vy.y (), vx.y ()), atan2f (vy.z (), vx.z ()), atan2f (vy.w (), vx.w ()));
	}

	inline f32x4_arith maximumf (const f32x4_arith& v1, const f32x4_arith& v2) noexcept
	{
		return f32x4_arith (dseed::maximum (v1.x (), v2.x ()), dseed::maximum (v1.y (), v2.y ()), dseed::maximum (v1.z (), v2.z ()), dseed::maximum (v1.w (), v2.w ()));
	}
	inline f32x4_arith minimumf (const f32x4_arith& v1, const f32x4_arith& v2) noexcept
	{
		return f32x4_arith (dseed::minimum (v1.x (), v2.x ()), dseed::minimum (v1.y (), v2.y ()), dseed::minimum (v1.z (), v2.z ()), dseed::minimum (v1.w (), v2.w ()));
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// 32-bit Floating Point 4*4 Matrix Operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline f32x4x4_arith add (const f32x4x4_arith& m1, const f32x4x4_arith& m2) noexcept
	{
		return f32x4x4_arith (
			add (m1.column1 (), m2.column1 ()),
			add (m1.column2 (), m2.column2 ()),
			add (m1.column3 (), m2.column3 ()),
			add (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_arith subtract (const f32x4x4_arith& m1, const f32x4x4_arith& m2) noexcept
	{
		return f32x4x4_arith (
			subtract (m1.column1 (), m2.column1 ()),
			subtract (m1.column2 (), m2.column2 ()),
			subtract (m1.column3 (), m2.column3 ()),
			subtract (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_arith negate (const f32x4x4_arith& m) noexcept
	{
		return f32x4x4_arith (
			negate (m.column1 ()),
			negate (m.column2 ()),
			negate (m.column3 ()),
			negate (m.column4 ())
		);
	}
	inline f32x4x4_arith multiply (const f32x4x4_arith& m1, const f32x4x4_arith& m2) noexcept
	{
		return f32x4x4_arith (
			fma (m1.column1 ().splat_x (), m2.column1 (),
				fma (m1.column1 ().splat_y (), m2.column2 (),
					fma (m1.column1 ().splat_z (), m2.column3 (),
						multiply (m1.column1 ().splat_w (), m2.column4 ())))),
			fma (m1.column2 ().splat_x (), m2.column1 (),
				fma (m1.column2 ().splat_y (), m2.column2 (),
					fma (m1.column2 ().splat_z (), m2.column3 (),
						multiply (m1.column2 ().splat_w (), m2.column4 ())))),
			fma (m1.column3 ().splat_x (), m2.column1 (),
				fma (m1.column3 ().splat_y (), m2.column2 (),
					fma (m1.column3 ().splat_z (), m2.column3 (),
						multiply (m1.column3 ().splat_w (), m2.column4 ())))),
			fma (m1.column4 ().splat_x (), m2.column1 (),
				fma (m1.column4 ().splat_y (), m2.column2 (),
					fma (m1.column4 ().splat_z (), m2.column3 (),
						multiply (m1.column4 ().splat_w (), m2.column4 ()))))
		);
	}
	inline f32x4x4_arith multiply (const f32x4x4_arith& m, float s) noexcept
	{
		return f32x4x4_arith (
			multiply (m.column1 (), s),
			multiply (m.column2 (), s),
			multiply (m.column3 (), s),
			multiply (m.column4 (), s)
		);
	}
	inline f32x4x4_arith multiply (float s, const f32x4x4_arith& m) noexcept { return multiply (m, s); }
	inline f32x4x4_arith divide (const f32x4x4_arith& m1, const f32x4x4_arith& m2) noexcept
	{
		return f32x4x4_arith (
			divide (m1.column1 (), m2.column1 ()),
			divide (m1.column2 (), m2.column2 ()),
			divide (m1.column3 (), m2.column3 ()),
			divide (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_arith divide (const f32x4x4_arith& m, float s) noexcept
	{
		return f32x4x4_arith (
			divide (m.column1 (), s),
			divide (m.column2 (), s),
			divide (m.column3 (), s),
			divide (m.column4 (), s)
		);
	}

	inline f32x4x4_arith fma (const f32x4x4_arith& mm1, const f32x4x4_arith& mm2, const f32x4x4_arith& am) noexcept { return add (multiply (mm1, mm2), am); }
	inline f32x4x4_arith fms (const f32x4x4_arith& mm1, const f32x4x4_arith& mm2, const f32x4x4_arith& sm) noexcept { return subtract (multiply (mm1, mm2), sm); }
	inline f32x4x4_arith fnms (const f32x4x4_arith& mm1, const f32x4x4_arith& mm2, const f32x4x4_arith& sm) noexcept { return subtract (sm, multiply (mm1, mm2)); }

	inline f32x4x4_arith and (const f32x4x4_arith& m1, const f32x4x4_arith& m2) noexcept
	{
		return f32x4x4_arith (
			and (m1.column1 (), m2.column1 ()),
			and (m1.column2 (), m2.column2 ()),
			and (m1.column3 (), m2.column3 ()),
			and (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_arith or (const f32x4x4_arith & m1, const f32x4x4_arith & m2) noexcept
	{
		return f32x4x4_arith (
			or (m1.column1 (), m2.column1 ()),
			or (m1.column2 (), m2.column2 ()),
			or (m1.column3 (), m2.column3 ()),
			or (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_arith xor (const f32x4x4_arith& m1, const f32x4x4_arith& m2) noexcept
	{
		return f32x4x4_arith (
			xor (m1.column1 (), m2.column1 ()),
			xor (m1.column2 (), m2.column2 ()),
			xor (m1.column3 (), m2.column3 ()),
			xor (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_arith not (const f32x4x4_arith& m) noexcept
	{
		return f32x4x4_arith (
			not (m.column1 ()),
			not (m.column2 ()),
			not (m.column3 ()),
			not (m.column4 ())
		);
	}

	inline f32x4x4_arith operator+ (const f32x4x4_arith& v1, const f32x4x4_arith& v2) noexcept { return add (v1, v2); }
	inline f32x4x4_arith operator- (const f32x4x4_arith& v1, const f32x4x4_arith& v2) noexcept { return subtract (v1, v2); }
	inline f32x4x4_arith operator- (const f32x4x4_arith& v) noexcept { return negate (v); }
	inline f32x4x4_arith operator* (const f32x4x4_arith& v1, const f32x4x4_arith& v2) noexcept { return multiply (v1, v2); }
	inline f32x4x4_arith operator* (const f32x4x4_arith& v, float s) noexcept { return multiply (v, s); }
	inline f32x4x4_arith operator* (float s, const f32x4x4_arith& v) noexcept { return multiply (s, v); }
	inline f32x4x4_arith operator/ (const f32x4x4_arith& v1, const f32x4x4_arith& v2) noexcept { return divide (v1, v2); }
	inline f32x4x4_arith operator/ (const f32x4x4_arith& v, float s) noexcept { return divide (v, s); }
	inline f32x4x4_arith operator& (const f32x4x4_arith& v1, const f32x4x4_arith& v2) noexcept { return and (v1, v2); }
	inline f32x4x4_arith operator| (const f32x4x4_arith& v1, const f32x4x4_arith& v2) noexcept { return or (v1, v2); }
	inline f32x4x4_arith operator^ (const f32x4x4_arith& v1, const f32x4x4_arith& v2) noexcept { return xor (v1, v2); }
	inline f32x4x4_arith operator~ (const f32x4x4_arith& v) noexcept { return not (v); }

	inline f32x4x4_arith transpose (const f32x4x4_arith& m) noexcept
	{
		return f32x4x4_arith (m._11, m._21, m._31, m._41, m._12, m._22, m._32, m._42, m._13, m._23, m._33, m._43, m._14, m._24, m._34, m._44);
	}
	inline float determinant (const f32x4x4_arith& m) noexcept
	{
		float m11 = m.column1 ().x (), m12 = m.column1 ().y (), m13 = m.column1 ().z (), m14 = m.column1 ().w ()
			, m21 = m.column2 ().x (), m22 = m.column2 ().y (), m23 = m.column2 ().z (), m24 = m.column2 ().w ();
		float m31 = m.column3 ().x (), m32 = m.column3 ().y (), m33 = m.column3 ().z (), m34 = m.column3 ().w ()
			, m41 = m.column4 ().x (), m42 = m.column4 ().y (), m43 = m.column4 ().z (), m44 = m.column4 ().w ();

		float n18 = (m33 * m44) - (m34 * m43), n17 = (m32 * m44) - (m34 * m42);
		float n16 = (m32 * m43) - (m33 * m42), n15 = (m31 * m44) - (m34 * m41);
		float n14 = (m31 * m43) - (m33 * m41), n13 = (m31 * m42) - (m32 * m41);

		return (((m11 * (((m22 * n18) - (m23 * n17)) + (m24 * n16))) -
			(m12 * (((m21 * n18) - (m23 * n15)) + (m24 * n14)))) +
			(m13 * (((m21 * n17) - (m22 * n15)) + (m24 * n13)))) -
			(m14 * (((m21 * n16) - (m22 * n14)) + (m23 * n13)));
	}
	inline f32x4x4_arith invert (const f32x4x4_arith& m, float& det) noexcept
	{
		static f32x4_arith one = f32x4_arith (1);

		f32x4x4_arith transposed = transpose (m);
		f32x4_arith v00 = permute<0, 0, 1, 1> (transposed.column3 ());
		f32x4_arith v10 = permute<2, 3, 2, 3> (transposed.column4 ());
		f32x4_arith v01 = permute<0, 0, 1, 1> (transposed.column1 ());
		f32x4_arith v11 = permute<2, 3, 2, 3> (transposed.column2 ());
		f32x4_arith v02 = shuffle<0, 2, 0, 2> (transposed.column3 (), transposed.column1 ());
		f32x4_arith v12 = shuffle<1, 3, 1, 3> (transposed.column4 (), transposed.column2 ());

		f32x4_arith d0 = v00 * v10;
		f32x4_arith d1 = v01 * v11;
		f32x4_arith d2 = v02 * v12;

		v00 = permute<2, 3, 2, 3> (transposed.column3 ());
		v10 = permute<0, 0, 1, 1> (transposed.column4 ());
		v01 = permute<2, 3, 2, 3> (transposed.column1 ());
		v11 = permute<0, 0, 1, 1> (transposed.column2 ());
		v02 = shuffle<1, 3, 1, 3> (transposed.column3 (), transposed.column1 ());
		v12 = shuffle<0, 2, 0, 2> (transposed.column4 (), transposed.column2 ());

		v00 = v00 * v10;
		v01 = v01 * v11;
		v02 = v02 * v12;

		d0 = d0 - v00;
		d1 = d1 - v01;
		d2 = d2 - v02;

		v11 = shuffle<1, 3, 1, 1> (d0, d2);
		v00 = permute<1, 2, 0, 1> (transposed.column2 ());
		v10 = shuffle<2, 0, 3, 0> (v11, d0);
		v01 = permute<2, 0, 1, 0> (transposed.column1 ());
		v11 = shuffle<1, 2, 1, 2> (v11, d0);

		f32x4_arith v13 = shuffle<1, 3, 3, 3> (d1, d2);
		v02 = permute<1, 2, 0, 1> (transposed.column4 ());
		v12 = shuffle<2, 0, 3, 0> (v13, d1);
		f32x4_arith v03 = permute<2, 0, 1, 0> (transposed.column3 ());
		v13 = shuffle<1, 2, 1, 2> (v13, d1);

		f32x4_arith c0 = v00 * v10;
		f32x4_arith c2 = v01 * v11;
		f32x4_arith c4 = v02 * v12;
		f32x4_arith c6 = v03 * v13;

		v11 = shuffle<0, 1, 0, 0> (d0, d2);
		v00 = permute<2, 3, 1, 2> (transposed.column2 ());
		v10 = shuffle<3, 0, 1, 2> (d0, v11);
		v01 = permute<3, 2, 3, 1> (transposed.column1 ());
		v11 = shuffle<2, 1, 2, 0> (d0, v11);

		v13 = shuffle<0, 1, 2, 2> (d1, d2);
		v02 = permute<2, 3, 1, 2> (transposed.column4 ());
		v12 = shuffle<3, 0, 1, 2> (d1, v13);
		v03 = permute<3, 2, 3, 1> (transposed.column3 ());
		v13 = shuffle<2, 1, 2, 0> (d1, v13);

		v00 = v00 * v10;
		v01 = v01 * v11;
		v02 = v02 * v12;
		v03 = v03 * v13;
		c0 = c0 - v00;
		c2 = c2 - v01;
		c4 = c4 - v02;
		c6 = c6 - v03;

		v00 = permute<3, 0, 3, 0> (transposed.column2 ());
		v10 = shuffle<2, 2, 0, 1> (d0, d2);
		v10 = permute<0, 3, 2, 0> (v10);
		v01 = permute<1, 3, 0, 2> (transposed.column1 ());
		v11 = shuffle<0, 3, 0, 1> (d0, d2);
		v11 = permute<3, 0, 1, 2> (v11);
		v02 = permute<3, 0, 3, 0> (transposed.column4 ());
		v12 = shuffle<2, 2, 2, 3> (d1, d2);
		v12 = permute<0, 3, 2, 0> (v12);
		v03 = permute<1, 3, 0, 2> (transposed.column3 ());
		v13 = shuffle<0, 3, 2, 3> (d1, d2);
		v13 = permute<3, 0, 1, 2> (v13);

		v00 = v00 * v10;
		v01 = v01 * v11;
		v02 = v02 * v12;
		v03 = v03 * v13;
		f32x4_arith c1 = c0 - v00;
		c0 = c0 + v00;
		f32x4_arith c3 = c2 + v01;
		c2 = c2 - v01;
		f32x4_arith c5 = c4 - v02;
		c4 = c4 + v02;
		f32x4_arith c7 = c6 + v03;
		c6 = c6 - v03;

		c0 = shuffle<0, 2, 1, 3> (c0, c1);
		c2 = shuffle<0, 2, 1, 3> (c2, c3);
		c4 = shuffle<0, 2, 1, 3> (c4, c5);
		c6 = shuffle<0, 2, 1, 3> (c6, c7);
		c0 = permute<0, 2, 1, 3> (c0);
		c2 = permute<0, 2, 1, 3> (c2);
		c4 = permute<0, 2, 1, 3> (c4);
		c6 = permute<0, 2, 1, 3> (c6);

		f32x4_arith temp = f32x4_arith (dot4d (c0, transposed.column1 ()));
		det = temp.x ();
		temp = one / temp;

		f32x4x4_arith ret;
		ret._column1 = c0 * temp;
		ret._column2 = c2 * temp;
		ret._column3 = c4 * temp;
		ret._column4 = c6 * temp;

		return ret;
	}

	inline f32x4x4_arith equals (const f32x4x4_arith& m1, const f32x4x4_arith& m2) noexcept
	{
		return f32x4x4_arith (
			equals (m1.column1 (), m2.column1 ()),
			equals (m1.column2 (), m2.column2 ()),
			equals (m1.column3 (), m2.column3 ()),
			equals (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_arith not_equals (const f32x4x4_arith& m1, const f32x4x4_arith& m2) noexcept
	{
		return f32x4x4_arith (
			not_equals (m1.column1 (), m2.column1 ()),
			not_equals (m1.column2 (), m2.column2 ()),
			not_equals (m1.column3 (), m2.column3 ()),
			not_equals (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_arith lesser (const f32x4x4_arith& m1, const f32x4x4_arith& m2) noexcept
	{
		return f32x4x4_arith (
			lesser (m1.column1 (), m2.column1 ()),
			lesser (m1.column2 (), m2.column2 ()),
			lesser (m1.column3 (), m2.column3 ()),
			lesser (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_arith lesser_equals (const f32x4x4_arith& m1, const f32x4x4_arith& m2) noexcept
	{
		return f32x4x4_arith (
			lesser_equals (m1.column1 (), m2.column1 ()),
			lesser_equals (m1.column2 (), m2.column2 ()),
			lesser_equals (m1.column3 (), m2.column3 ()),
			lesser_equals (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_arith greater (const f32x4x4_arith& m1, const f32x4x4_arith& m2) noexcept
	{
		return f32x4x4_arith (
			greater (m1.column1 (), m2.column1 ()),
			greater (m1.column2 (), m2.column2 ()),
			greater (m1.column3 (), m2.column3 ()),
			greater (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_arith greater_equals (const f32x4x4_arith& m1, const f32x4x4_arith& m2) noexcept
	{
		return f32x4x4_arith (
			greater_equals (m1.column1 (), m2.column1 ()),
			greater_equals (m1.column2 (), m2.column2 ()),
			greater_equals (m1.column3 (), m2.column3 ()),
			greater_equals (m1.column4 (), m2.column4 ())
		);
	}

	inline bool operator== (const f32x4x4_arith& v1, const f32x4x4_arith& v2) noexcept { return 0xffff == movemask (equals (v1, v2)); }
	inline bool operator!= (const f32x4x4_arith& v1, const f32x4x4_arith& v2) noexcept { return 0xffff == movemask (not_equals (v1, v2)); }
	inline bool operator< (const f32x4x4_arith& v1, const f32x4x4_arith& v2) noexcept { return 0xffff == movemask (lesser (v1, v2)); }
	inline bool operator<= (const f32x4x4_arith& v1, const f32x4x4_arith& v2) noexcept { return 0xffff == movemask (lesser_equals (v1, v2)); }
	inline bool operator> (const f32x4x4_arith& v1, const f32x4x4_arith& v2) noexcept { return 0xffff == movemask (greater (v1, v2)); }
	inline bool operator>= (const f32x4x4_arith& v1, const f32x4x4_arith& v2) noexcept { return 0xffff == movemask (greater_equals (v1, v2)); }
}

#endif