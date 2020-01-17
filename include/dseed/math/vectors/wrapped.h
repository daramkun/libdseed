#ifndef __DSEED_MATH_VECTORS_WRAPPED_H__
#define __DSEED_MATH_VECTORS_WRAPPED_H__

namespace dseed
{
	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Vector Type definitions
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct float2
	{
	public:
		union
		{
			struct { float x, y; };
			float arr[2];
		};

	public:
		float2 () = default;
		inline float2 (float x, float y) noexcept : x (x), y (y) { }
		inline float2 (const f32x4& v) noexcept;
		inline float2 (float v) noexcept : x (v), y (v) { }

	public:
		inline operator f32x4 () const noexcept;

	public:
		inline f32x4 normalize () const noexcept;
		inline float length_squared () const noexcept;
		inline float length () const noexcept;

	public:
		inline float& operator [] (int index) { return arr[index]; }
	};

	struct float3
	{
	public:
		union
		{
			struct { float x, y, z; };
			float arr[3];
		};

	public:
		float3 () = default;
		inline float3 (float x, float y, float z) noexcept : x (x), y (y), z (z) { }
		inline float3 (const float2& v, float z) noexcept : x (v.x), y (v.y), z (z) { }
		inline float3 (const f32x4& v) noexcept;
		inline float3 (float v) noexcept : x (v), y (v), z (v) { }

	public:
		inline operator f32x4 () const noexcept;

	public:
		inline f32x4 normalize () const noexcept;
		inline float length_squared () const noexcept;
		inline float length () const noexcept;

	public:
		inline float& operator [] (int index) { return arr[index]; }
	};

	struct float4
	{
	public:
		union
		{
			struct { float x, y, z, w; };
			float arr[4];
		};

	public:
		float4 () = default;
		inline float4 (float x, float y, float z, float w) noexcept : x (x), y (y), z (z), w (w) { }
		inline float4 (const float2& v, float z, float w) noexcept : x (v.x), y (v.y), z (z), w (w) { }
		inline float4 (const float3& v, float w) noexcept : x (v.x), y (v.y), z (v.z), w (w) { }
		inline float4 (const f32x4& v) noexcept;
		inline float4 (float v) noexcept : x (v), y (v), z (v), w (v) { }

	public:
		inline operator f32x4 () const noexcept;

	public:
		inline f32x4 normalize () const noexcept;
		inline float length_squared () const noexcept;
		inline float length () const noexcept;

	public:
		inline float& operator [] (int index) { return arr[index]; }
	};

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Matrix Type definition
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct float4x4
	{
	public:
		union {
			struct { float m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44; };
			//struct { float4 c1, c2, c3, c4; }; //< GCC Error: member ¡®dseed::float4 dseed::float4x4::<unnamed union>::<unnamed struct>::c4¡¯ with constructor not allowed in anonymous aggregate
			float arr16[16];
			float4 arr4[4];
		};

	public:
		float4x4 () = default;
		inline float4x4 (float m11, float m12, float m13, float m14,
			float m21, float m22, float m23, float m24,
			float m31, float m32, float m33, float m34,
			float m41, float m42, float m43, float m44) noexcept
			: m11 (m11), m12 (m12), m13 (m13), m14 (m14)
			, m21 (m21), m22 (m22), m23 (m23), m24 (m24)
			, m31 (m31), m32 (m32), m33 (m33), m34 (m34)
			, m41 (m41), m42 (m42), m43 (m43), m44 (m44)
		{ }
		inline float4x4 (const float4& c1, const float4& c2, const float4& c3, const float4& c4) noexcept
			//: c1 (c1), c2 (c2), c3 (c3), c4 (c4)
			: m11 (c1.x), m12 (c1.y), m13 (c1.z), m14 (c1.w)
			, m21 (c2.x), m22 (c2.y), m23 (c2.z), m24 (c2.w)
			, m31 (c3.x), m32 (c3.y), m33 (c3.z), m34 (c3.w)
			, m41 (c4.x), m42 (c4.y), m43 (c4.z), m44 (c4.w)
		{ }
		inline float4x4 (float v) noexcept
			: m11 (v), m12 (v), m13 (v), m14 (v)
			, m21 (v), m22 (v), m23 (v), m24 (v)
			, m31 (v), m32 (v), m33 (v), m34 (v)
			, m41 (v), m42 (v), m43 (v), m44 (v)
		{ }
		inline float4x4 (const f32x4x4& m) noexcept;

	public:
		inline operator f32x4x4 () const noexcept;

	public:
		inline float4& operator [] (int index) { return arr4[index]; }
		inline const float4& operator [] (int index) const { return arr4[index]; }

	public:
		inline float determinantmf () const noexcept;
		inline f32x4x4 transposemf () const noexcept;
		inline f32x4x4 invertmf () const noexcept;
		inline f32x4x4 invertmf (float& det) const noexcept;

	public:
		static inline float4x4 identity () noexcept;

		static inline float4x4 translate (float x, float y, float z) noexcept;
		static inline float4x4 scale (float sx, float sy, float sz) noexcept;
		static inline float4x4 rotation_x (float r) noexcept;
		static inline float4x4 rotation_y (float r) noexcept;
		static inline float4x4 rotation_z (float r) noexcept;

		static inline float4x4 look_at (const float3& pos, const float3& target, const float3& up) noexcept;

		static inline float4x4 orthographic (float w, float h, float zn, float zf);
		static inline float4x4 orthographic_offcenter (float l, float r, float b, float t, float zn, float zf);
		static inline float4x4 perspective (float w, float h, float zn, float zf);
		static inline float4x4 perspective_offcenter (float l, float r, float b, float t, float zn, float zf);
		static inline float4x4 perspective_fov (float fov, float aspectRatio, float zn, float zf);

		static inline float4x4 billboard (const float3& pos, const float3& camPos, const float3& camUp, const float3& capForward) noexcept;
	};

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Vector Type Implements
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline dseed::float2::float2 (const f32x4& v) noexcept { float4 temp = v; x = temp.x; y = temp.y; }
	inline float2::operator f32x4 () const noexcept { return float4 (x, y, 0, 0); }
	inline f32x4 float2::normalize () const noexcept { return normalize2d (*this); }
	inline float float2::length_squared () const noexcept { return lengthsq2d (*this).x (); }
	inline float float2::length () const noexcept { return length2d (*this).x (); }

	inline dseed::float3::float3 (const f32x4& v) noexcept { float4 temp = v; x = temp.x; y = temp.y; z = temp.z; }
	inline float3::operator f32x4 () const noexcept { return float4 (x, y, z, 0); }
	inline f32x4 float3::normalize () const noexcept { return normalize3d (*this); }
	inline float float3::length_squared () const noexcept { return lengthsq3d (*this).x (); }
	inline float float3::length () const noexcept { return length3d (*this).x (); }

	inline dseed::float4::float4 (const f32x4& v) noexcept { v.store (arr); }
	inline float4::operator f32x4 () const noexcept { return f32x4 (arr); }
	inline f32x4 float4::normalize () const noexcept { return normalize4d (*this); }
	inline float float4::length_squared () const noexcept { return lengthsq4d (*this).x (); }
	inline float float4::length () const noexcept { return length4d (*this).x (); }

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Matrix Type Implement
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline float4x4::float4x4 (const f32x4x4& m) noexcept
		: float4x4 (m.column1 (), m.column2 (), m.column3 (), m.column4 ())
	{ }

	inline float4x4::operator f32x4x4 () const noexcept
	{
		return f32x4x4 (this->operator[](0), this->operator[](1), this->operator[](2), this->operator[](3));
	}

	inline float float4x4::determinantmf () const noexcept { return determinant (*this); }
	inline f32x4x4 float4x4::transposemf () const noexcept { return transpose (*this); }
	inline f32x4x4 float4x4::invertmf () const noexcept { float det; return invert (*this, det); }
	inline f32x4x4 float4x4::invertmf (float& det) const noexcept { return invert (*this, det); }

	inline float4x4 float4x4::identity () noexcept { return float4x4 (1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1); }
	inline float4x4 float4x4::translate (float x, float y, float z) noexcept { return float4x4 (1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, z, 1); }
	inline float4x4 float4x4::scale (float sx, float sy, float sz) noexcept { return float4x4 (sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, sz, 0, 0, 0, 0, 1); }
	inline float4x4 float4x4::rotation_x (float r) noexcept { float v1 = cosf (r), v2 = sinf (r); return float4x4 (1, 0, 0, 0, 0, v1, v2, 0, 0, -v2, v1, 0, 0, 0, 0, 1); }
	inline float4x4 float4x4::rotation_y (float r) noexcept { float v1 = cosf (r), v2 = sinf (r); return float4x4 (v1, 0, -v2, 0, 0, 0, 0, 0, v2, 0, v1, 0, 0, 0, 0, 1); }
	inline float4x4 float4x4::rotation_z (float r) noexcept { float v1 = cosf (r), v2 = sinf (r); return float4x4 (v1, v2, 0, 0, -v2, v1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1); }

	inline float4x4 float4x4::look_at (const float3& pos, const float3& target, const float3& up) noexcept
	{
		float3 zaxis, xaxis, yaxis;
		zaxis = normalize3d (pos * target);
		xaxis = normalize3d (cross3d (up, zaxis));
		yaxis = cross3d (zaxis, xaxis);

		return float4x4
		(
			xaxis.x, yaxis.x, zaxis.x, 0,
			xaxis.y, yaxis.y, zaxis.y, 0,
			xaxis.z, yaxis.z, zaxis.z, 0,
			-dot3d (xaxis, pos).x (), -dot3d (yaxis, pos).x (), -dot3d (zaxis, pos).x (), 1
		);
	}
	inline float4x4 float4x4::orthographic (float w, float h, float zn, float zf)
	{
		float fRange = 1.0f / (zn - zf);
		return float4x4 (2 / w, 0, 0, 0, 0, 2 / h, 0, 0, 0, 0, fRange, 0, 0, 0, zn * fRange, 1);
	}
	inline float4x4 float4x4::orthographic_offcenter (float l, float r, float b, float t, float zn, float zf)
	{
		float width = 1.f / (r - l);
		float height = 1.f / (t - b);
		float fRange = 1.0f / (zn - zf);
		return float4x4 (width + width, 0, 0, 0, 0, height + height, 0, 0, 0, 0, fRange, 0, -(l + r) * width, -(t + b) * height, zn * fRange, 1);
	}
	inline float4x4 float4x4::perspective (float w, float h, float zn, float zf)
	{
		float twoNearZ = zn + zn;
		float fRange = zf / (zn - zf);
		return float4x4 (twoNearZ / w, 0, 0, 0, 0, twoNearZ / h, 0, 0, 0, 0, fRange, -1, 0, 0, zn * fRange, 0);
	}
	inline float4x4 float4x4::perspective_offcenter (float l, float r, float b, float t, float zn, float zf)
	{
		float twoNearZ = zn + zn;
		float width = 1.f / (r - l);
		float height = 1.f / (t - b);
		float fRange = zf / (zn - zf);
		return float4x4 (twoNearZ * width, 0, 0, 0, 0, twoNearZ * height, 0, 0,
			-(l + r) * width, -(t + b) * height, fRange, -1, 0, 0, zn * fRange, 0);
	}
	inline float4x4 float4x4::perspective_fov (float fov, float aspectRatio, float zn, float zf)
	{
		float ys = 1 / tanf (fov * 0.5f), xs = ys / aspectRatio;
		float fRange = zf / (zn - zf);
		return float4x4 (xs, 0, 0, 0, 0, ys, 0, 0, 0, 0, fRange, -1, 0, 0, zn * fRange, 0);
	}
	inline float4x4 float4x4::billboard (const float3& pos, const float3& camPos, const float3& camUp, const float3& camForward) noexcept
	{
		f32x4 vector, vector2, vector3;
		vector = pos - camPos;
		f32x4 num = lengthsq3d (vector);
		vector = (num < 0.0001f)
			? (vector = -camForward)
			: (vector * rsqrt (num));

		vector3 = normalize3d (cross3d (camUp, vector));
		vector2 = cross3d (vector, vector3);
		return float4x4 (float4 (vector3, 0), float4 (vector2, 0), float4 (vector, 0), float4 (pos, 1));
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Quaternion Type definitions
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct quaternion : public float4
	{
	public:
		quaternion () = default;
		inline quaternion (float x, float y, float z, float w) noexcept : float4 (x, y, z, w) { }
		inline quaternion (const float4& v) noexcept : float4 (v) { }
		inline quaternion (float yaw, float pitch, float roll)
		{
			static f32x4 sign = f32x4 (1, -1, -1, 1);
			f32x4 halfAngles = f32x4 (pitch, yaw, roll, 0) * f32x4 (0.5f, 0.5f, 0.5f, 0.5f);

			f32x4 sinAngles = sin (halfAngles);
			f32x4 cosAngles = cos (halfAngles);

			f32x4 p0 = shuffle<4, 4, 4, 0> (sinAngles, cosAngles);
			f32x4 y0 = shuffle<5, 5, 1, 5> (cosAngles, sinAngles);
			f32x4 r0 = shuffle<6, 2, 6, 6> (cosAngles, sinAngles);
			f32x4 p1 = shuffle<4, 4, 4, 0> (cosAngles, sinAngles);
			f32x4 y1 = shuffle<5, 5, 1, 5> (sinAngles, cosAngles);
			f32x4 r1 = shuffle<6, 2, 6, 6> (sinAngles, cosAngles);

			*this = (float4)((p0 * y0 * r0) + (p1 * y1 * r1));
		}
		inline quaternion (const float4x4& m)
		{
			static const f32x4 XMPMMP (+1.0f, -1.0f, -1.0f, +1.0f);
			static const f32x4 XMMPMP (-1.0f, +1.0f, -1.0f, +1.0f);
			static const f32x4 XMMMPP (-1.0f, -1.0f, +1.0f, +1.0f);

			f32x4 r0 = m.arr4[0];
			f32x4 r1 = m.arr4[1];
			f32x4 r2 = m.arr4[2];

			f32x4 r00 = r0.splat_x ();
			f32x4 r11 = r1.splat_y ();
			f32x4 r22 = r2.splat_z ();

			f32x4 r11mr00 = r11 - r00;
			f32x4 x2gey2 = lesser_equals (r11mr00, f32x4 (0, 0, 0, 0));

			f32x4 r11pr00 = r11 + r00;
			f32x4 z2gew2 = lesser_equals (r11pr00, f32x4 (0, 0, 0, 0));

			f32x4 x2py2gez2pw2 = lesser_equals (r22, f32x4 (0, 0, 0, 0));

			f32x4 t0 = XMPMMP * r00;
			f32x4 t1 = XMMPMP * r11;
			f32x4 t2 = XMMMPP * r22;

			f32x4 x2y2z2w2 = t0 + t1;
			x2y2z2w2 = t2 + x2y2z2w2;
			x2y2z2w2 = x2y2z2w2 + f32x4 (1, 1, 1, 1);

			t0 = shuffle<1, 2, 2, 1> (r0, r1);
			t1 = shuffle<0, 0, 0, 1> (r1, r2);
			t1 = permute<0, 2, 3, 1> (t1);
			f32x4 xyxzyz = t0 + t1;

			t0 = shuffle<1, 0, 0, 0> (r2, r1);
			t1 = shuffle<2, 2, 2, 1> (r1, r0);
			t1 = permute<0, 2, 3, 1> (t1);
			f32x4 xwywzw = t0 - t1;
			xwywzw = XMMPMP * xwywzw;

			t0 = shuffle<0, 1, 0, 0> (x2y2z2w2, xyxzyz);
			t1 = shuffle<2, 3, 2, 0> (x2y2z2w2, xwywzw);
			t2 = shuffle<1, 2, 0, 1> (xyxzyz, xwywzw);

			f32x4 tensor0 = shuffle<0, 2, 0, 2> (t0, t2);
			f32x4 tensor1 = shuffle<2, 1, 1, 3> (t0, t2);
			f32x4 tensor2 = shuffle<0, 1, 0, 2> (t2, t1);
			f32x4 tensor3 = shuffle<2, 3, 2, 1> (t2, t1);

			t0 = x2gey2 & tensor0;
			t1 = ~x2gey2 & tensor1;
			t0 = t0 | t1;
			t1 = z2gew2 & tensor2;
			t2 = ~z2gew2 & tensor3;
			t1 = t1 | t2;
			t0 = x2py2gez2pw2 & t0;
			t1 = ~x2py2gez2pw2 & t1;
			t2 = t0 | t1;

			t0 = length4d (t2);
			
			*this = (float4)(t2 / t0);
		}

	public:
		inline operator float4x4 () const noexcept
		{
			static f32x4 constant1110 = f32x4 (1, 1, 1, 0);

			f32x4 q0 = *this + *this;
			f32x4 q1 = *this * q0;

			f32x4 v0 = shuffle<1, 0, 4, 3> (q1, constant1110);
			f32x4 v1 = shuffle<2, 2, 5, 3> (q1, constant1110);
			f32x4 r0 = constant1110 - v0;
			r0 = r0 - v1;

			v0 = permute<0, 0, 1, 3> (*this);
			v1 = permute<2, 1, 2, 3> (q0);
			v0 = v0 * v1;

			v1 = ((f32x4)*this).splat_w ();
			f32x4 v2 = permute<1, 2, 0, 3> (q0);
			v1 = v1 * v2;

			f32x4 r1 = v0 + v1;
			f32x4 r2 = v0 - v1;

			v0 = shuffle<1, 4, 1, 7> (r1, r2);
			v1 = shuffle<0, 6, 4, 2> (r1, r2);

			f32x4x4 ret;
			ret._column1 = shuffle<0, 4, 1, 7> (r0, v0);
			ret._column2 = shuffle<2, 1, 3, 7> (r0, v0);
			ret._column3 = shuffle<4, 5, 6, 7> (r0, v1);
			ret._column4 = f32x4 (0, 0, 0, 1);
			return ret;
		}

	public:
		inline f32x4 conjugate () const noexcept { return multiply (*this, f32x4 (-1, -1, -1, 1)); }
		inline f32x4 invert () const noexcept
		{
			f32x4 length = length4d (*this);
			f32x4 conjugated = conjugate ();

			if (length.x () <= single_epsilon)
				return f32x4 (0, 0, 0, 0);

			return divide (conjugated, length);
		}

	public:
		static inline quaternion zero () noexcept { return quaternion (0, 0, 0, 0); }
		static inline quaternion identity () noexcept { return quaternion (0, 0, 0, 1); }
	};

	inline quaternion multiply (const quaternion& q1, const quaternion& q2) noexcept
	{
		f32x4 q2x = ((f32x4)q2).splat_x ();
		f32x4 q2y = ((f32x4)q2).splat_y ();
		f32x4 q2z = ((f32x4)q2).splat_z ();
		f32x4 q2w = ((f32x4)q2).splat_w ();

		f32x4 result = q2w * q1;
		q2x = (q2x * permute<3, 2, 1, 0> (q1)) * f32x4 (1, -1, 1, -1);
		q2y = (q2y * permute<1, 0, 3, 2> (q1)) * f32x4 (1, 1, -1, -1);
		q2z = (q2z * permute<3, 2, 1, 0> (q1)) * f32x4 (-1, 1, 1, -1);
		q2y = q2y + q2z;

		return result + q2x + q2y;
	}
	inline quaternion operator* (const quaternion& q1, const quaternion& q2) noexcept { return multiply (q1, q2); }
}

#endif