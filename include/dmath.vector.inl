#ifndef __DMATH_VECTOR_INL__
#define __DMATH_VECTOR_INL__

#include <dmath.simd.inl>

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
		inline float2 (const vectorf& v) noexcept;
		inline float2 (float v) noexcept : x (v), y (v) { }

	public:
		inline operator vectorf () const noexcept;

	public:
		inline vectorf normalize () const noexcept;
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
		inline float3 (const vectorf& v) noexcept;
		inline float3 (float v) noexcept : x (v), y (v), z (v) { }

	public:
		inline operator vectorf () const noexcept;

	public:
		inline vectorf normalize () const noexcept;
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
		inline float4 (const vectorf& v) noexcept;
		inline float4 (float v) noexcept : x (v), y (v), z (v), w (v) { }

	public:
		inline operator vectorf () const noexcept;

	public:
		inline vectorf normalize () const noexcept;
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
			struct { float4 c1, c2, c3, c4; };
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
			: c1 (c1), c2 (c2), c3 (c3), c4 (c4)
		{ }
		inline float4x4 (float v) noexcept
			: m11 (v), m12 (v), m13 (v), m14 (v)
			, m21 (v), m22 (v), m23 (v), m24 (v)
			, m31 (v), m32 (v), m33 (v), m34 (v)
			, m41 (v), m42 (v), m43 (v), m44 (v)
		{ }
		inline float4x4 (const matrixf& m) noexcept;

	public:
		inline operator matrixf () const noexcept;

	public:
		inline float4& operator [] (int index) { return arr4[index]; }
		inline const float4& operator [] (int index) const { return arr4[index]; }

	public:
		inline float determinantmf () const noexcept;
		inline matrixf transposemf () const noexcept;
		inline matrixf invertmf () const noexcept;
		inline matrixf invertmf (float& det) const noexcept;

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
	inline dseed::float2::float2 (const vectorf& v) noexcept { float4 temp = v; x = temp.x; y = temp.y; }
	inline float2::operator vectorf () const noexcept { return float4 (x, y, 0, 0); }
	inline vectorf float2::normalize () const noexcept { return normalize2 (*this); }
	inline float float2::length_squared () const noexcept { return length_squared2 (*this); }
	inline float float2::length () const noexcept { return length2 (*this); }

	inline dseed::float3::float3 (const vectorf& v) noexcept { float4 temp = v; x = temp.x; y = temp.y; z = temp.z; }
	inline float3::operator vectorf () const noexcept { return float4 (x, y, z, 0); }
	inline vectorf float3::normalize () const noexcept { return normalize3 (*this); }
	inline float float3::length_squared () const noexcept { return length_squared3 (*this); }
	inline float float3::length () const noexcept { return length3 (*this); }

	inline dseed::float4::float4 (const vectorf& v) noexcept { v.store (arr); }
	inline float4::operator vectorf () const noexcept { return vectorf (arr); }
	inline vectorf float4::normalize () const noexcept { return normalize4 (*this); }
	inline float float4::length_squared () const noexcept { return length_squared4 (*this); }
	inline float float4::length () const noexcept { return length4 (*this); }

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Matrix Type Implement
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline float4x4::float4x4 (const matrixf& m) noexcept
		: float4x4 (m.column1, m.column2, m.column3, m.column4)
	{ }

	inline float4x4::operator matrixf () const noexcept
	{
		return matrixf (this->operator[](0), this->operator[](1), this->operator[](2), this->operator[](3));
	}

	inline float float4x4::determinantmf () const noexcept { return dseed::determinantmf (*this); }
	inline matrixf float4x4::transposemf () const noexcept { return dseed::transposemf (*this); }
	inline matrixf float4x4::invertmf () const noexcept { float det; return dseed::invertmf (*this, det); }
	inline matrixf float4x4::invertmf (float& det) const noexcept { return dseed::invertmf (*this, det); }

	inline float4x4 float4x4::identity () noexcept { return float4x4 (1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1); }
	inline float4x4 float4x4::translate (float x, float y, float z) noexcept { return float4x4 (1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, z, 1); }
	inline float4x4 float4x4::scale (float sx, float sy, float sz) noexcept { return float4x4 (sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, sz, 0, 0, 0, 0, 1); }
	inline float4x4 float4x4::rotation_x (float r) noexcept { float v1 = cosf (r), v2 = sinf (r); return float4x4 (1, 0, 0, 0, 0, v1, v2, 0, 0, -v2, v1, 0, 0, 0, 0, 1); }
	inline float4x4 float4x4::rotation_y (float r) noexcept { float v1 = cosf (r), v2 = sinf (r); return float4x4 (v1, 0, -v2, 0, 0, 0, 0, 0, v2, 0, v1, 0, 0, 0, 0, 1); }
	inline float4x4 float4x4::rotation_z (float r) noexcept { float v1 = cosf (r), v2 = sinf (r); return float4x4 (v1, v2, 0, 0, -v2, v1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1); }

	inline float4x4 float4x4::look_at (const float3& pos, const float3& target, const float3& up) noexcept
	{
		float3 zaxis, xaxis, yaxis;
		zaxis = normalize3 (pos * target);
		xaxis = normalize3 (cross3 (up, zaxis));
		yaxis = cross3 (zaxis, xaxis);

		return float4x4
		(
			xaxis.x, yaxis.x, zaxis.x, 0,
			xaxis.y, yaxis.y, zaxis.y, 0,
			xaxis.z, yaxis.z, zaxis.z, 0,
			-dot3 (xaxis, pos), -dot3 (yaxis, pos), -dot3 (zaxis, pos), 1
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
		float3 vector, vector2, vector3;
		vector = pos - camPos;
		float num = vector.length_squared ();
		vector = (num < 0.0001f)
			? (vector = -camForward)
			: float3 (vector * (1.0f / sqrtf (num)));

		vector3 = normalize3 (cross3 (camUp, vector));
		vector2 = cross3 (vector, vector3);
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
		inline quaternion (float yaw, float pitch, float roll);
		inline quaternion (const float4x4& m);

	public:
		inline vectorf conjugate () const noexcept { return conjugateq (*this); }
		inline vectorf invert () const noexcept { return invertq (*this); }

	public:
		static inline quaternion zero () noexcept { return quaternion (0, 0, 0, 0); }
		static inline quaternion identity () noexcept { return quaternion (0, 0, 0, 1); }
	};

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Quaternion Type Implements
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline quaternion::quaternion (float yaw, float pitch, float roll)
	{
		static vectorf sign = float4 (1, -1, -1, 1);
		vectorf halfAngles = float4 (pitch, yaw, roll, 0) * (vectorf)float4 (0.5f, 0.5f, 0.5f, 0.5f);

		vectorf sinAngles = sinvf (halfAngles);
		vectorf cosAngles = cosvf (halfAngles);

		vectorf P0 = sinAngles.shuffle<shuffle_ax1, shuffle_bx1, shuffle_bx2, shuffle_bx2> (cosAngles);
		vectorf Y0 = cosAngles.shuffle<shuffle_ax1, shuffle_bx1, shuffle_ax2, shuffle_ax2> (sinAngles);
		vectorf R0 = cosAngles.shuffle<shuffle_ax1, shuffle_ax1, shuffle_bx2, shuffle_ax2> (sinAngles);
		vectorf P1 = cosAngles.shuffle<shuffle_ax1, shuffle_bx1, shuffle_bx2, shuffle_bx2> (sinAngles);
		vectorf Y1 = sinAngles.shuffle<shuffle_ax1, shuffle_bx1, shuffle_ax2, shuffle_ax2> (cosAngles);
		vectorf R1 = sinAngles.shuffle<shuffle_ax1, shuffle_ax1, shuffle_bx2, shuffle_ax2> (cosAngles);

		P0 = P0 * Y0 * R0;
		P1 = P1 * Y1 * R1;

		float4 result = P0 + P1;
		*this = result;
	}

	inline quaternion::quaternion (const float4x4& m)
	{
		float num8 = (m.m11 + m.m22) + m.m33;
		if (num8 > 0.0f)
		{
			float num = sqrtf (num8 + 1.0f);
			num = 0.5f / num;
			x = (m.m23 - m.m32) * num;
			y = (m.m31 - m.m13) * num;
			z = (m.m12 - m.m21) * num;
			w = num * 0.5f;
		}
		else if ((m.m11 >= m.m22) && (m.m11 >= m.m33))
		{
			float num7 = sqrtf (((1.0f + m.m11) - m.m22) - m.m33);
			float num4 = 0.5f / num7;
			x = 0.5f * num7;
			y = (m.m12 + m.m21) * num4;
			z = (m.m13 + m.m31) * num4;
			w = (m.m23 - m.m32) * num4;
		}
		else if (m.m22 > m.m33)
		{
			float num6 = sqrtf (((1.0f + m.m22) - m.m11) - m.m33);
			float num3 = 0.5f / num6;
			x = (m.m21 + m.m12) * num3;
			y = 0.5f * num6;
			z = (m.m32 + m.m23) * num3;
			w = (m.m31 - m.m13) * num3;
		}
		else
		{
			float num5 = sqrtf (((1.0f + m.m33) - m.m11) - m.m22);
			float num2 = 0.5f / num5;
			x = (m.m31 + m.m13) * num2;
			y = (m.m32 + m.m23) * num2;
			z = 0.5f * num5;
			w = (m.m12 - m.m21) * num2;
		}
	}
}

#endif