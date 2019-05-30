#ifndef __DMATH_H__
#define __DMATH_H__

#include <dseed.h>

#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
#	include <mmintrin.h>
#	include <emmintrin.h>
#	include <immintrin.h>
#	include <pmmintrin.h>
#	include <ammintrin.h>
#	include <smmintrin.h>
#	include <xmmintrin.h>
#elif ARCH_ARM && !defined ( NO_INTRINSIC )
#	include <arm_neon.h>
#elif ARCH_ARM64 && !defined ( NO_INTRINSIC )
#	include <arm64_neon.h>
#endif

#include <cmath>

namespace dmath
{
	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Type Pre-definitions
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct float2;
	struct float3;
	struct float4;
	struct quaternion;

	struct float4x4;

	struct vectorf;
	struct matrixf;

	struct plane;
	struct ray;

	struct bounding_box;
	struct bounding_sphere;
	struct bounding_frustum;

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
		inline float2 (float x, float y) noexcept
			: x (x), y (y)
		{ }
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
		inline float3 (float x, float y, float z) noexcept
			: x (x), y (y), z (z)
		{ }
		inline float3 (const float2& v, float z) noexcept
			: x (v.x), y (v.y), z (z)
		{ }
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
		inline float4 (float x, float y, float z, float w) noexcept
			: x (x), y (y), z (z), w (w)
		{ }
		inline float4 (const float2& v, float z, float w) noexcept
			: x (v.x), y (v.y), z (z), w (w)
		{ }
		inline float4 (const float3& v, float w) noexcept
			: x (v.x), y (v.y), z (v.z), w (w)
		{ }
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
	// Quaternion, Plane Type definitions
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct quaternion : public float4
	{
	public:
		quaternion () = default;
		inline quaternion (float x, float y, float z, float w) noexcept
			: float4 (x, y, z, w)
		{ }
		inline quaternion (const float4& v) noexcept
			: float4 (v)
		{ }
		inline quaternion (float yaw, float pitch, float roll);
		inline quaternion (const float4x4& m);

	public:
		static inline quaternion zero () noexcept
		{
			return quaternion (0, 0, 0, 0);
		}
		static inline quaternion identity () noexcept
		{
			return quaternion (0, 0, 0, 1);
		}
	};

	struct plane
	{
	public:
		float x, y, z, w;

	public:
		plane () = default;
		inline plane (float x, float y, float z, float w)
			: x (x), y (y), z (z), w (w)
		{ }
		inline plane (const float3& p1, const float3& p2, const float3& p3);
		inline plane (const float3& p, const float3& n);
		inline plane (const float4& v)
			: plane (v.x, v.y, v.z, v.w)
		{ }

	public:
		inline operator float4 () const noexcept { return float4 (x, y, z, w); }

	public:
		inline vectorf normalize ();

	public:
		inline bool intersect (const plane& p);
		inline void intersect (const plane& p, float3* p1, float3* p2);
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
			float arr[16];
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
			: float4x4 ( c1.x, c1.y, c1.z, c1.w, c2.x, c2.y, c2.z, c2.w, c3.x, c3.y, c3.z, c3.w, c4.x, c4.y, c4.z, c4.w )
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
		inline float4& operator [] (int index) { return reinterpret_cast<float4*>(arr)[index]; }
		inline const float4& operator [] (int index) const { return reinterpret_cast<const float4*>(arr)[index]; }

	public:
		inline float determinant () const noexcept;
		inline matrixf transpose () const noexcept;
		inline matrixf invert () const noexcept;

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
	// SIMD-operation Type definitions
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct vectorf
	{
		vectorf () = default;
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		__m128 _vector;
		inline vectorf (__m128 vector) : _vector (vector) { }
		inline operator __m128 () const { return _vector; }
		inline vectorf splat_x () const { return _mm_permute_ps (_vector, _MM_SHUFFLE (0, 0, 0, 0)); }
		inline vectorf splat_y () const { return _mm_permute_ps (_vector, _MM_SHUFFLE (1, 1, 1, 1)); }
		inline vectorf splat_z () const { return _mm_permute_ps (_vector, _MM_SHUFFLE (2, 2, 2, 2)); }
		inline vectorf splat_w () const { return _mm_permute_ps (_vector, _MM_SHUFFLE (3, 3, 3, 3)); }
		inline float x () const { return _mm_cvtss_f32 (splat_x ()); }
		inline float y () const { return _mm_cvtss_f32 (splat_y ()); }
		inline float z () const { return _mm_cvtss_f32 (splat_z ()); }
		inline float w () const { return _mm_cvtss_f32 (splat_w ()); }
#else
		float4 _vector;
		inline vectorf splat_x () const { return float4 (_vector.x).operator dmath::vectorf (); }
		inline vectorf splat_y () const { return float4 (_vector.y).operator dmath::vectorf (); }
		inline vectorf splat_z () const { return float4 (_vector.z).operator dmath::vectorf (); }
		inline vectorf splat_w () const { return float4 (_vector.w).operator dmath::vectorf (); }
		inline float x () const { return _vector.x; }
		inline float y () const { return _vector.y; }
		inline float z () const { return _vector.z; }
		inline float w () const { return _vector.w; }
#endif
	};

	struct matrixf
	{
		vectorf column1, column2, column3, column4;

		matrixf () = default;
		inline matrixf (const vectorf& c1, const vectorf& c2, const vectorf& c3, const vectorf& c4)
			: column1 (c1), column2 (c2), column3 (c3), column4 (c4)
		{ }
	};

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Vector Type operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline vectorf operator+ (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_add_ps (v1, v2);
#else
		return float4 { v1._vector.x + v2._vector.x, v1._vector.y + v2._vector.y, v1._vector.z + v2._vector.z, v1._vector.w + v2._vector.w };
#endif
	}
	inline vectorf operator- (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_sub_ps (v1, v2);
#else
		return float4 { v1._vector.x - v2._vector.x, v1._vector.y - v2._vector.y, v1._vector.z - v2._vector.z, v1._vector.w - v2._vector.w };
#endif
	}
	inline vectorf operator- (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_sub_ps (_mm_load1_ps (0), v);
#else
		return float4 { -v._vector.x, -v._vector.y, -v._vector.z, -v._vector.w };
#endif
	}
	inline vectorf operator* (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_mul_ps (v1, v2);
#else
		return float4 { v1._vector.x * v2._vector.x, v1._vector.y * v2._vector.y, v1._vector.z * v2._vector.z, v1._vector.w * v2._vector.w };
#endif
	}
	inline vectorf operator* (const vectorf& v, float s) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_mul_ps (v, _mm_set1_ps (s));
#else
		return float4 { v._vector.x * s, v._vector.y * s, v._vector.z * s, v._vector.w * s };
#endif
	}
	inline vectorf operator* (float s, const vectorf& v) noexcept
	{
		return v * s;
	}
	inline vectorf operator/ (const vectorf& v1, const vectorf& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_div_ps (v1, v2);
#else
		return float4 { v1._vector.x / v2._vector.x, v1._vector.y / v2._vector.y, v1._vector.z / v2._vector.z, v1._vector.w / v2._vector.w };
#endif
	}
	inline vectorf operator/ (const vectorf& v, float s)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_div_ps (v, _mm_set1_ps (s));
#else
		return float4{ v._vector.x / s, v._vector.y / s, v._vector.z / s, v._vector.w / s };
#endif
	}
	inline bool operator== (const vectorf& v1, const vectorf& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_movemask_ps (_mm_cmpeq_ps (v1, v2)) == 0xF;
#else
		return v1._vector.x == v2._vector.x && v1._vector.y == v2._vector.y
			&& v1._vector.z == v2._vector.z && v1._vector.w == v2._vector.w;
#endif
	}
	inline bool operator!= (const vectorf& v1, const vectorf& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_movemask_ps (_mm_cmpneq_ps (v1, v2)) != 0;
#else
		return v1._vector.x != v2._vector.x || v1._vector.y != v2._vector.y
			|| v1._vector.z != v2._vector.z || v1._vector.w != v2._vector.w;
#endif
	}
	inline float dot2 (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtss_f32 (_mm_dp_ps (v1, v2, 0x3f));
#else
		return (v1._vector.x * v2._vector.x) + (v1._vector.y * v2._vector.y);
#endif
	}
	inline float dot2 (const vectorf& v, float s) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtss_f32 (_mm_dp_ps (v, _mm_set1_ps (s), 0x3f));
#else
		return (v._vector.x * s) + (v._vector.y * s);
#endif
	}
	inline float dot3 (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtss_f32 (_mm_dp_ps (v1, v2, 0x7f));
#else
		return (v1._vector.x * v2._vector.x) + (v1._vector.y * v2._vector.y) + (v1._vector.z * v2._vector.z);
#endif
	}
	inline float dot3 (const vectorf& v, float s) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtss_f32 (_mm_dp_ps (v, _mm_set1_ps (s), 0x7f));
#else
		return (v._vector.x * s) + (v._vector.y * s) + (v._vector.z * s);
#endif
	}
	inline float dot4 (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtss_f32 (_mm_dp_ps (v1, v2, 0xff));
#else
		return (v1._vector.x * v2._vector.x) + (v1._vector.y * v2._vector.y) + (v1._vector.z * v2._vector.z) + (v1._vector.w * v2._vector.w);
#endif
	}

	inline vectorf cross2 (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		vectorf ret = _mm_permute_ps (v2, _MM_SHUFFLE (0, 1, 0, 1));
		ret = _mm_mul_ps (ret, v1);
		vectorf vTemp = _mm_permute_ps (ret, _MM_SHUFFLE (1, 1, 1, 1));
		ret = _mm_sub_ss (ret, vTemp);
		ret = _mm_permute_ps (ret, _MM_SHUFFLE (0, 0, 0, 0));
		return ret;
#else
		return float2 (
			v1._vector.x * v2._vector.y - v1._vector.y * v2._vector.x,
			v1._vector.x * v2._vector.y - v1._vector.y * v2._vector.x
		);
#endif
	}
	inline vectorf cross3 (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		static union { uint32_t u[4]; __m128 v; } mask3 = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x000000000 };

		vectorf temp1 = _mm_permute_ps (v1, _MM_SHUFFLE (3, 0, 2, 1));
		vectorf temp2 = _mm_permute_ps (v2, _MM_SHUFFLE (3, 1, 0, 2));
		vectorf ret = _mm_mul_ps (temp1, temp2);
		temp1 = _mm_permute_ps (temp1, _MM_SHUFFLE (3, 0, 2, 1));
		temp2 = _mm_permute_ps (temp2, _MM_SHUFFLE (3, 1, 0, 2));
		temp1 = _mm_mul_ps (temp1, temp2);
		ret = _mm_sub_ps (ret, temp1);
		return _mm_and_ps (ret, mask3.v);
#else
		return float3 (
			(v1._vector.y * v2._vector.z) - (v1._vector.z * v2._vector.y),
			(v1._vector.z * v2._vector.x) - (v1._vector.x * v2._vector.z),
			(v1._vector.x * v2._vector.y) - (v1._vector.y * v2._vector.x)
		);
#endif
	}
	inline vectorf cross4 (const vectorf& v1, const vectorf& v2, const vectorf& v3) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		vectorf ret = _mm_permute_ps (v2, _MM_SHUFFLE (2, 1, 3, 2));
		vectorf temp3 = _mm_permute_ps (v3, _MM_SHUFFLE (1, 3, 2, 3));
		ret = _mm_mul_ps (ret, temp3);

		vectorf temp2 = _mm_permute_ps (v2, _MM_SHUFFLE (1, 3, 2, 3));
		temp3 = _mm_permute_ps (temp3, _MM_SHUFFLE (1, 3, 0, 1));
		temp2 = _mm_mul_ps (temp2, temp3);
		ret = _mm_sub_ps (ret, temp2);

		vectorf temp1 = _mm_permute_ps (v1, _MM_SHUFFLE (0, 0, 0, 1));
		ret = _mm_mul_ps (ret, temp1);

		temp2 = _mm_permute_ps (v2, _MM_SHUFFLE (2, 0, 3, 1));
		temp3 = _mm_permute_ps (v3, _MM_SHUFFLE (0, 3, 0, 3));
		temp3 = _mm_mul_ps (temp3, temp2);

		temp2 = _mm_permute_ps (temp2, _MM_SHUFFLE (2, 1, 2, 1));
		temp1 = _mm_permute_ps (v3, _MM_SHUFFLE (2, 0, 3, 1));
		temp2 = _mm_mul_ps (temp2, temp1);
		temp3 = _mm_sub_ps (temp3, temp2);

		temp1 = _mm_permute_ps (v1, _MM_SHUFFLE (1, 1, 2, 2));
		temp1 = _mm_mul_ps (temp1, temp3);
		ret = _mm_sub_ps (ret, temp1);

		temp2 = _mm_permute_ps (v2, _MM_SHUFFLE (1, 0, 2, 1));
		temp3 = _mm_permute_ps (v3, _MM_SHUFFLE (0, 1, 0, 2));
		temp3 = _mm_mul_ps (temp3, temp2);

		temp2 = _mm_permute_ps (temp2, _MM_SHUFFLE (2, 0, 2, 1));
		temp1 = _mm_permute_ps (v3, _MM_SHUFFLE (1, 0, 2, 1));
		temp1 = _mm_mul_ps (temp1, temp2);
		temp3 = _mm_sub_ps (temp3, temp1);

		temp1 = _mm_permute_ps (v1, _MM_SHUFFLE (2, 3, 3, 3));
		temp3 = _mm_mul_ps (temp3, temp1);
		ret = _mm_add_ps (ret, temp3);

		return ret;
#else
		return float4 (
			((v2._vector.z * v3._vector.w - v2._vector.w * v3._vector.z) * v1._vector.y) - ((v2._vector.y * v3._vector.w - v2._vector.w * v3._vector.y) * v1._vector.z) + ((v2._vector.y * v3._vector.z - v2._vector.z * v3._vector.y) * v1._vector.w),
			((v2._vector.w * v3._vector.z - v2._vector.z * v3._vector.w) * v1._vector.x) - ((v2._vector.w * v3._vector.x - v2._vector.x * v3._vector.w) * v1._vector.z) + ((v2._vector.z * v3._vector.x - v2._vector.x * v3._vector.z) * v1._vector.w),
			((v2._vector.y * v3._vector.w - v2._vector.w * v3._vector.y) * v1._vector.x) - ((v2._vector.x * v3._vector.w - v2._vector.w * v3._vector.x) * v1._vector.y) + ((v2._vector.x * v3._vector.y - v2._vector.y * v3._vector.x) * v1._vector.w),
			((v2._vector.z * v3._vector.y - v2._vector.y * v3._vector.z) * v1._vector.x) - ((v2._vector.z * v3._vector.x - v2._vector.x * v3._vector.z) * v1._vector.y) + ((v2._vector.y * v3._vector.x - v2._vector.x * v3._vector.y) * v1._vector.z)
		);
#endif
	}

	inline float length_quared2 (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtss_f32 (_mm_dp_ps (v, v, 0x3f));
#else
		return v._vector.x * v._vector.x + v._vector.y * v._vector.y;
#endif
	}
	inline float length_squared3 (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtss_f32 (_mm_dp_ps (v, v, 0x7f));
#else
		return v._vector.x * v._vector.x + v._vector.y * v._vector.y + v._vector.z * v._vector.z;
#endif
	}
	inline float length_squared4 (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtss_f32 (_mm_dp_ps (v, v, 0xff));
#else
		return v._vector.x * v._vector.x + v._vector.y * v._vector.y + v._vector.z * v._vector.z + v._vector.w * v._vector.w;
#endif
	}
	inline float length2 (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtss_f32 (_mm_sqrt_ps (_mm_dp_ps (v, v, 0x3f)));
#else
		return sqrt (length_quared2 (v));
#endif
	}
	inline float length3 (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtss_f32 (_mm_sqrt_ps (_mm_dp_ps (v, v, 0x7f)));
#else
		return sqrt (length_squared3 (v));
#endif
	}
	inline float length4 (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtss_f32 (_mm_sqrt_ps (_mm_dp_ps (v, v, 0xff)));
#else
		return sqrt (length_squared4 (v));
#endif
	}

	inline vectorf normalize2 (const vectorf& v) noexcept
	{
		return v / length2 (v);
	}
	inline vectorf normalize3 (const vectorf& v) noexcept
	{
		return v / length3 (v);
	}
	inline vectorf normalize4 (const vectorf& v) noexcept
	{
		return v / length4 (v);
	}

	inline vectorf fma (const vectorf& mv1, const vectorf& mv2, const vectorf& av) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC ) && !defined ( NO_FMA3 )
		return _mm_fmadd_ps (mv1, mv2, av);
#else
		return (mv1 * mv2) + av;
#endif
	}
	inline vectorf fms (const vectorf& mv1, const vectorf& mv2, const vectorf& sv) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC ) && !defined ( NO_FMA3 )
		return _mm_fmsub_ps (mv1, mv2, sv);
#else
		return (mv1 * mv2) - sv;
#endif
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Quaternion, Plane Type operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline float dot_plane_normal (const vectorf& p, const vectorf& n)
	{
		return dot3 (p, n);
	}
	inline float dot_plane_coord (const vectorf& p, const vectorf& v)
	{
		return dot_plane_normal (p, v) + p.w ();
	}
	inline vectorf normalize_plane (const vectorf& p)
	{
		return p / length3 (p);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Matrix Type operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline matrixf operator+ (const matrixf& m1, const matrixf& m2) noexcept
	{
		matrixf ret (m1.column1 + m2.column1, m1.column2 + m2.column2, m1.column3 + m2.column3, m1.column4 + m2.column4);
		return ret;
	}
	inline matrixf operator- (const matrixf& m1, const matrixf& m2) noexcept
	{
		matrixf ret (m1.column1 + m2.column1, m1.column2 + m2.column2, m1.column3 + m2.column3, m1.column4 + m2.column4);
		return ret;
	}
	inline matrixf operator- (const matrixf& m) noexcept
	{
		matrixf ret (-m.column1, -m.column2, -m.column3, -m.column4);
		return ret;
	}
	inline matrixf operator* (const matrixf& m1, const matrixf& m2) noexcept
	{
		matrixf ret;
		ret.column1 = fma (m1.column1.splat_x (), m2.column1,
			fma (m1.column1.splat_y (), m2.column2,
				fma (m1.column1.splat_z (), m2.column3,
					m1.column1.splat_w () * m2.column4)));
		ret.column2 = fma (m1.column2.splat_x (), m2.column1,
			fma (m1.column2.splat_y (), m2.column2,
				fma (m1.column2.splat_z (), m2.column3,
					m1.column2.splat_w () * m2.column4)));
		ret.column3 = fma (m1.column3.splat_x (), m2.column1,
			fma (m1.column3.splat_y (), m2.column2,
				fma (m1.column3.splat_z (), m2.column3,
					m1.column3.splat_w () * m2.column4)));
		ret.column4 = fma (m1.column4.splat_x (), m2.column1,
			fma (m1.column4.splat_y (), m2.column2,
				fma (m1.column4.splat_z (), m2.column3,
					m1.column4.splat_w () * m2.column4)));
		return ret;
	}
	inline matrixf operator* (const matrixf& m, float s) noexcept
	{
		matrixf ret (m.column1 * s, m.column2 * s, m.column3 * s, m.column4 * s);
		return ret;
	}
	inline matrixf operator* (float s, const matrixf& m) noexcept
	{
		return m * s;
	}
	inline matrixf operator/ (const matrixf& m1, const matrixf& m2) noexcept
	{
		matrixf ret (m1.column1 / m2.column1, m1.column2 / m2.column2, m1.column3 / m2.column3, m1.column4 / m2.column4);
		return ret;
	}
	inline matrixf operator/ (const matrixf& m, float s) noexcept
	{
		matrixf ret (m.column1 / s, m.column2 / s, m.column3 / s, m.column4 / s);
		return ret;
	}
	inline matrixf transpose (const matrixf& m) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		matrixf ret;
		__m128 temp3, temp2, temp1, temp0;
		temp0 = _mm_shuffle_ps ((m.column1), (m.column2), 0x44);
		temp2 = _mm_shuffle_ps ((m.column1), (m.column2), 0xee);
		temp1 = _mm_shuffle_ps ((m.column3), (m.column4), 0x44);
		temp3 = _mm_shuffle_ps ((m.column3), (m.column4), 0xee);

		ret.column1 = _mm_shuffle_ps (temp0, temp1, 0x88);
		ret.column2 = _mm_shuffle_ps (temp0, temp1, 0xDD);
		ret.column3 = _mm_shuffle_ps (temp2, temp3, 0x88);
		ret.column4 = _mm_shuffle_ps (temp2, temp3, 0xDD);

		return ret;
#else
		return float4x4 (
			m.column1.x (), m.column2.x (), m.column3.x (), m.column4.x (),
			m.column1.y (), m.column2.y (), m.column3.y (), m.column4.y (),
			m.column1.z (), m.column2.z (), m.column3.z (), m.column4.z (),
			m.column1.w (), m.column2.w (), m.column3.w (), m.column4.w ()
		);
#endif
	}
	inline float determinant (const matrixf& m) noexcept
	{
		float m11 = m.column1.x (), m12 = m.column1.y (), m13 = m.column1.z (), m14 = m.column1.w ()
			, m21 = m.column2.x (), m22 = m.column2.y (), m23 = m.column2.z (), m24 = m.column2.w ();
		float m31 = m.column3.x (), m32 = m.column3.y (), m33 = m.column3.z (), m34 = m.column3.w ()
			, m41 = m.column4.x (), m42 = m.column4.y (), m43 = m.column4.z (), m44 = m.column4.w ();

		float n18 = (m33 * m44) - (m34 * m43), n17 = (m32 * m44) - (m34 * m42);
		float n16 = (m32 * m43) - (m33 * m42), n15 = (m31 * m44) - (m34 * m41);
		float n14 = (m31 * m43) - (m33 * m41), n13 = (m31 * m42) - (m32 * m41);

		return (((m11 * (((m22 * n18) - (m23 * n17)) + (m24 * n16))) -
				(m12 * (((m21 * n18) - (m23 * n15)) + (m24 * n14)))) +
				(m13 * (((m21 * n17) - (m22 * n15)) + (m24 * n13)))) -
				(m14 * (((m21 * n16) - (m22 * n14)) + (m23 * n13)));
	}
	inline matrixf invert (const matrixf& m) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		static __m128 one = _mm_set_ps (1, 1, 1, 1);

		matrixf transposed = transpose (m);
		vectorf v00 = _mm_permute_ps (transposed.column3, _MM_SHUFFLE (1, 1, 0, 0));
		vectorf v10 = _mm_permute_ps (transposed.column4, _MM_SHUFFLE (3, 2, 3, 2));
		vectorf v01 = _mm_permute_ps (transposed.column1, _MM_SHUFFLE (1, 1, 0, 0));
		vectorf v11 = _mm_permute_ps (transposed.column2, _MM_SHUFFLE (3, 2, 3, 2));
		vectorf v02 = _mm_shuffle_ps (transposed.column3, transposed.column1, _MM_SHUFFLE (2, 0, 2, 0));
		vectorf v12 = _mm_shuffle_ps (transposed.column4, transposed.column2, _MM_SHUFFLE (3, 1, 3, 1));

		vectorf d0 = _mm_mul_ps (v00, v10);
		vectorf d1 = _mm_mul_ps (v01, v11);
		vectorf d2 = _mm_mul_ps (v02, v12);

		v00 = _mm_permute_ps (transposed.column3, _MM_SHUFFLE (3, 2, 3, 2));
		v10 = _mm_permute_ps (transposed.column4, _MM_SHUFFLE (1, 1, 0, 0));
		v01 = _mm_permute_ps (transposed.column1, _MM_SHUFFLE (3, 2, 3, 2));
		v11 = _mm_permute_ps (transposed.column2, _MM_SHUFFLE (1, 1, 0, 0));
		v02 = _mm_shuffle_ps (transposed.column3, transposed.column1, _MM_SHUFFLE (3, 1, 3, 1));
		v12 = _mm_shuffle_ps (transposed.column4, transposed.column2, _MM_SHUFFLE (2, 0, 2, 0));

		v00 = _mm_mul_ps (v00, v10);
		v01 = _mm_mul_ps (v01, v11);
		v02 = _mm_mul_ps (v02, v12);

		d0 = _mm_sub_ps (d0, v00);
		d1 = _mm_sub_ps (d1, v01);
		d2 = _mm_sub_ps (d2, v02);

		v11 = _mm_shuffle_ps (d0, d2, _MM_SHUFFLE (1, 1, 3, 1));
		v00 = _mm_permute_ps (transposed.column2, _MM_SHUFFLE (1, 0, 2, 1));
		v10 = _mm_shuffle_ps (v11, d0, _MM_SHUFFLE (0, 3, 0, 2));
		v01 = _mm_permute_ps (transposed.column1, _MM_SHUFFLE (0, 1, 0, 2));
		v11 = _mm_shuffle_ps (v11, d0, _MM_SHUFFLE (2, 1, 2, 1));

		vectorf v13 = _mm_shuffle_ps (d1, d2, _MM_SHUFFLE (3, 3, 3, 1));
		v02 = _mm_permute_ps (transposed.column4, _MM_SHUFFLE (1, 0, 2, 1));
		v12 = _mm_shuffle_ps (v13, d1, _MM_SHUFFLE (0, 3, 0, 2));
		vectorf v03 = _mm_permute_ps (transposed.column3, _MM_SHUFFLE (0, 1, 0, 2));
		v13 = _mm_shuffle_ps (v13, d1, _MM_SHUFFLE (2, 1, 2, 1));

		vectorf c0 = _mm_mul_ps (v00, v10);
		vectorf c2 = _mm_mul_ps (v01, v11);
		vectorf c4 = _mm_mul_ps (v02, v12);
		vectorf c6 = _mm_mul_ps (v03, v13);

		v11 = _mm_shuffle_ps (d0, d2, _MM_SHUFFLE (0, 0, 1, 0));
		v00 = _mm_permute_ps (transposed.column2, _MM_SHUFFLE (2, 1, 3, 2));
		v10 = _mm_shuffle_ps (d0, v11, _MM_SHUFFLE (2, 1, 0, 3));
		v01 = _mm_permute_ps (transposed.column1, _MM_SHUFFLE (1, 3, 2, 3));
		v11 = _mm_shuffle_ps (d0, v11, _MM_SHUFFLE (0, 2, 1, 2));

		v13 = _mm_shuffle_ps (d1, d2, _MM_SHUFFLE (2, 2, 1, 0));
		v02 = _mm_permute_ps (transposed.column4, _MM_SHUFFLE (2, 1, 3, 2));
		v12 = _mm_shuffle_ps (d1, v13, _MM_SHUFFLE (2, 1, 0, 3));
		v03 = _mm_permute_ps (transposed.column3, _MM_SHUFFLE (1, 3, 2, 3));
		v13 = _mm_shuffle_ps (d1, v13, _MM_SHUFFLE (0, 2, 1, 2));

		v00 = _mm_mul_ps (v00, v10);
		v01 = _mm_mul_ps (v01, v11);
		v02 = _mm_mul_ps (v02, v12);
		v03 = _mm_mul_ps (v03, v13);
		c0 = _mm_sub_ps (c0, v00);
		c2 = _mm_sub_ps (c2, v01);
		c4 = _mm_sub_ps (c4, v02);
		c6 = _mm_sub_ps (c6, v03);

		v00 = _mm_permute_ps (transposed.column2, _MM_SHUFFLE (0, 3, 0, 3));
		v10 = _mm_shuffle_ps (d0, d2, _MM_SHUFFLE (1, 0, 2, 2));
		v10 = _mm_permute_ps (v10, _MM_SHUFFLE (0, 2, 3, 0));
		v01 = _mm_permute_ps (transposed.column1, _MM_SHUFFLE (2, 0, 3, 1));
		v11 = _mm_shuffle_ps (d0, d2, _MM_SHUFFLE (1, 0, 3, 0));
		v11 = _mm_permute_ps (v11, _MM_SHUFFLE (2, 1, 0, 3));
		v02 = _mm_permute_ps (transposed.column4, _MM_SHUFFLE (0, 3, 0, 3));
		v12 = _mm_shuffle_ps (d1, d2, _MM_SHUFFLE (3, 2, 2, 2));
		v12 = _mm_permute_ps (v12, _MM_SHUFFLE (0, 2, 3, 0));
		v03 = _mm_permute_ps (transposed.column3, _MM_SHUFFLE (2, 0, 3, 1));
		v13 = _mm_shuffle_ps (d1, d2, _MM_SHUFFLE (3, 2, 3, 0));
		v13 = _mm_permute_ps (v13, _MM_SHUFFLE (2, 1, 0, 3));

		v00 = _mm_mul_ps (v00, v10);
		v01 = _mm_mul_ps (v01, v11);
		v02 = _mm_mul_ps (v02, v12);
		v03 = _mm_mul_ps (v03, v13);
		vectorf c1 = _mm_sub_ps (c0, v00);
		c0 = _mm_add_ps (c0, v00);
		vectorf c3 = _mm_add_ps (c2, v01);
		c2 = _mm_sub_ps (c2, v01);
		vectorf c5 = _mm_sub_ps (c4, v02);
		c4 = _mm_add_ps (c4, v02);
		vectorf c7 = _mm_add_ps (c6, v03);
		c6 = _mm_sub_ps (c6, v03);

		c0 = _mm_shuffle_ps (c0, c1, _MM_SHUFFLE (3, 1, 2, 0));
		c2 = _mm_shuffle_ps (c2, c3, _MM_SHUFFLE (3, 1, 2, 0));
		c4 = _mm_shuffle_ps (c4, c5, _MM_SHUFFLE (3, 1, 2, 0));
		c6 = _mm_shuffle_ps (c6, c7, _MM_SHUFFLE (3, 1, 2, 0));
		c0 = _mm_permute_ps (c0, _MM_SHUFFLE (3, 1, 2, 0));
		c2 = _mm_permute_ps (c2, _MM_SHUFFLE (3, 1, 2, 0));
		c4 = _mm_permute_ps (c4, _MM_SHUFFLE (3, 1, 2, 0));
		c6 = _mm_permute_ps (c6, _MM_SHUFFLE (3, 1, 2, 0));

		vectorf temp = float4 (dot4 (c0, transposed.column1));
		temp = _mm_div_ps (one, temp);

		matrixf ret;
		ret.column1 = _mm_mul_ps (c0, temp);
		ret.column2 = _mm_mul_ps (c2, temp);
		ret.column3 = _mm_mul_ps (c4, temp);
		ret.column4 = _mm_mul_ps (c6, temp);

		return ret;
#else
		float n1 = m.column1.x (), n2 = m.column1.y (), n3 = m.column1.z (), n4 = m.column1.w ()
			, n5 = m.column2.x (), n6 = m.column2.y (), n7 = m.column2.z (), n8 = m.column2.w ();
		float n9 = m.column3.x (), n10 = m.column3.y (), n11 = m.column3.z (), n12 = m.column3.w ()
			, n13 = m.column4.x (), n14 = m.column4.y (), n15 = m.column4.z (), n16 = m.column4.w ();

		float n17 = n11 * n16 - n12 * n15, n18 = n10 * n16 - n12 * n14, n19 = n10 * n15 - n11 * n14;
		float n20 = n9 * n16 - n12 * n13, n21 = n9 * n15 - n11 * n13, n22 = n9 * n14 - n10 * n13;
		float n23 = n6 * n17 - n7 * n18 + n8 * n19, n24 = -(n5 * n17 - n7 * n20 + n8 * n21);
		float n25 = n5 * n18 - n6 * n20 + n8 * n22, n26 = -(n5 * n19 - n6 * n21 + n7 * n22);
		float n27 = 1.0f / (n1 * n23 + n2 * n24 + n3 * n25 + n4 * n26);
		float n28 = n7 * n16 - n8 * n15, n29 = n6 * n16 - n8 * n14, n30 = n6 * n15 - n7 * n14;
		float n31 = n5 * n16 - n8 * n13, n32 = n5 * n15 - n7 * n13, n33 = n5 * n14 - n6 * n13;
		float n34 = n7 * n12 - n8 * n11, n35 = n6 * n12 - n8 * n10, n36 = n6 * n11 - n7 * n10;
		float n37 = n5 * n12 - n8 * n9, n38 = n5 * n11 - n7 * n9, n39 = n5 * n10 - n6 * n9;

		matrixf ret;
		ret.column1 = float4 (n23 * n27, -(n2 * n17 - n3 * n18 + n4 * n19) * n27, +(n2 * n28 - n3 * n29 + n4 * n30) * n27, -(n2 * n34 - n3 * n35 + n4 * n36) * n27);
		ret.column2 = float4 (n24 * n27, +(n1 * n17 - n3 * n20 + n4 * n21) * n27, -(n1 * n28 - n3 * n31 + n4 * n32) * n27, +(n1 * n34 - n3 * n37 + n4 * n38) * n27);
		ret.column3 = float4 (n25 * n27, -(n1 * n18 - n2 * n20 + n4 * n22) * n27, +(n1 * n29 - n2 * n31 + n4 * n33) * n27, -(n1 * n35 - n2 * n37 + n4 * n39) * n27);
		ret.column4 = float4 (n26 * n27, +(n1 * n19 - n2 * n21 + n3 * n22) * n27, -(n1 * n30 - n2 * n32 + n3 * n33) * n27, +(n1 * n36 - n2 * n38 + n3 * n39) * n27);
		return ret;
#endif
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Vector * Matrix Operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline vectorf transform2 (const vectorf& v, const matrixf& m) noexcept
	{
		return fma (v.splat_x (), m.column1, fma (v.splat_y (), m.column2, m.column4));
	}
	inline vectorf transform_normal2 (const vectorf& v, const matrixf& m) noexcept
	{
		return fma (v.splat_x (), m.column1, v.splat_y () * m.column2);
	}
	inline vectorf transform3 (const vectorf& v, const matrixf& m) noexcept
	{
		return fma (v.splat_x (), m.column1, fma (v.splat_y (), m.column2, fma (v.splat_z (), m.column3, m.column4)));
	}
	inline vectorf transform_normal3 (const vectorf& v, const matrixf& m) noexcept
	{
		return fma (v.splat_x (), m.column1, fma (v.splat_y (), m.column2, v.splat_z () * m.column3));
	}
	inline vectorf transform4 (const vectorf& v, const matrixf& m) noexcept
	{
		return fma (v.splat_x (), m.column1, fma (v.splat_y (), m.column2, fma (v.splat_z (), m.column3, v.splat_w () * m.column4)));
	}
	inline vectorf transform_plane (const vectorf& p, const matrixf& m) noexcept
	{
		return transform4 (p, m);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Vector Type Implements
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline dmath::float2::float2 (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		float temp[4];
		_mm_store_ps (temp, v._vector);
		x = temp[0];
		y = temp[1];
#else
		x = v._vector.x;
		y = v._vector.y;
#endif
	}

	inline float2::operator vectorf () const noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_set_ps (0, 0, y, x);
#else
		vectorf ret;
		ret._vector.x = x;
		ret._vector.y = y;
		ret._vector.z = ret._vector.w = 0;
		return ret;
#endif
	}

	inline vectorf float2::normalize () const noexcept
	{
		return normalize2 (*this);
	}

	inline float float2::length_squared () const noexcept
	{
		return length_quared2 (*this);
	}

	inline float float2::length () const noexcept
	{
		return length2 (*this);
	}

	inline dmath::float3::float3 (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		float temp[4];
		_mm_store_ps (temp, v);
		x = temp[0];
		y = temp[1];
		z = temp[2];
#else
		x = v._vector.x;
		y = v._vector.y;
		z = v._vector.z;
#endif
	}

	inline float3::operator vectorf () const noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_set_ps (0, z, y, x);
#else
		vectorf ret;
		ret._vector.x = x;
		ret._vector.y = y;
		ret._vector.z = z;
		ret._vector.w = 0;
		return ret;
#endif
	}

	inline vectorf float3::normalize () const noexcept
	{
		return normalize3 (*this);
	}

	inline float float3::length_squared () const noexcept
	{
		return length_squared3 (*this);
	}

	inline float float3::length () const noexcept
	{
		return length3 (*this);
	}

	inline dmath::float4::float4 (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		_mm_store_ps ((float*)arr, v);
#else
		* this = v._vector;
#endif
	}

	inline float4::operator vectorf () const noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_load_ps ((float*)this);
#else
		return *this;
#endif
	}
	inline vectorf float4::normalize () const noexcept
	{
		return normalize4 (*this);
	}
	inline float float4::length_squared () const noexcept
	{
		return length_squared4 (*this);
	}
	inline float float4::length () const noexcept
	{
		return length4 (*this);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Quaternion, Plane Type Implements
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline quaternion::quaternion (float yaw, float pitch, float roll)
	{
		float halfRoll = roll * 0.5f, halfPitch = pitch * 0.5f, halfYaw = yaw * 0.5f;

		float sinRoll = (float)sin (halfRoll), cosRoll = (float)cos (halfRoll);
		float sinPitch = (float)sin (halfPitch), cosPitch = (float)cos (halfPitch);
		float sinYaw = (float)sin (halfYaw), cosYaw = (float)cos (halfYaw);

		x = (cosYaw * sinPitch * cosRoll) + (sinYaw * cosPitch * sinRoll);
		y = (sinYaw * cosPitch * cosRoll) - (cosYaw * sinPitch * sinRoll);
		z = (cosYaw * cosPitch * sinRoll) - (sinYaw * sinPitch * cosRoll);
		w = (cosYaw * cosPitch * cosRoll) + (sinYaw * sinPitch * sinRoll);
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

	inline plane::plane (const float3& p1, const float3& p2, const float3& p3)
		: plane (p1, normalize3 (cross3 ((vectorf) p2 - p1, (vectorf) p3 - p1)))
	{ }
	inline plane::plane (const float3 & p, const float3 & n)
		: plane (n.x, n.y, n.z, -dot3 (p, n))
	{ }
	inline vectorf plane::normalize () { return normalize_plane ((float4)* this); }

	inline bool plane::intersect (const plane& p)
	{
		float3 p1, p2;
		intersect (p, &p1, &p2);
		return !( p1 == p2 && p1 == float3 (dseed::single_nan));
	}

	inline void plane::intersect (const plane& p, float3* p1, float3* p2)
	{
		if (p1 == nullptr || p2 == nullptr) return;

		vectorf v1 = cross3 ((float4)p, (float4)*this);
		float lengthsq = length_squared3 (v1);

		vectorf v2 = cross3 ((float4)p, v1);
		vectorf point = v1 * float4 (w, w, w, w);

		vectorf v3 = cross3 (v1, (float4)* this);
		point = fma (v3, float4 (p.w, p.w, p.w, p.w), point);

		if (lengthsq <= dseed::single_epsilon)
		{
			*p1 = point / lengthsq;
			*p2 = *p1 + v1;
		}
		else
		{
			*p1 = *p2 = float3 (dseed::single_nan);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Matrix Type Implement
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline float4x4::float4x4 (const matrixf& m) noexcept
	{
		this->operator[](0) = m.column1;
		this->operator[](1) = m.column2;
		this->operator[](2) = m.column3;
		this->operator[](3) = m.column4;
	}

	inline float4x4::operator matrixf () const noexcept
	{
		matrixf ret;
		ret.column1 = this->operator[](0);
		ret.column2 = this->operator[](1);
		ret.column3 = this->operator[](2);
		ret.column4 = this->operator[](3);
		return ret;
	}

	inline float float4x4::determinant () const noexcept
	{
		return dmath::determinant (*this);
	}

	inline matrixf float4x4::transpose () const noexcept
	{
		return dmath::transpose (*this);
	}

	inline matrixf float4x4::invert () const noexcept
	{
		return dmath::invert (*this);
	}

	inline float4x4 float4x4::identity () noexcept
	{
		return float4x4 (1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	}

	inline float4x4 float4x4::translate (float x, float y, float z) noexcept
	{
		return float4x4 (1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, z, 1);
	}

	inline float4x4 float4x4::scale (float sx, float sy, float sz) noexcept
	{
		return float4x4 (sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, sz, 0, 0, 0, 0, 1);
	}

	inline float4x4 float4x4::rotation_x (float r) noexcept
	{
		float v1 = cosf (r), v2 = sinf (r);
		return float4x4 (
			1, 0, 0, 0,
			0, v1, v2, 0,
			0, -v2, v1, 0,
			0, 0, 0, 1
		);
	}

	inline float4x4 float4x4::rotation_y (float r) noexcept
	{
		float v1 = cosf (r), v2 = sinf (r);
		return float4x4 (
			v1, 0, -v2, 0,
			0, 0, 0, 0,
			v2, 0, v1, 0,
			0, 0, 0, 1
		);
	}

	inline float4x4 float4x4::rotation_z (float r) noexcept
	{
		float v1 = cosf (r), v2 = sinf (r);
		return float4x4 (
			v1, v2, 0, 0,
			-v2, v1, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 1
		);
	}

	inline float4x4 float4x4::look_at (const float3& pos, const float3& target, const float3& up) noexcept
	{
		float3 zaxis, xaxis, yaxis;
		zaxis = pos * target;
		zaxis = zaxis.normalize ();
		xaxis = cross3 (up, zaxis);
		xaxis = xaxis.normalize ();
		yaxis = cross3 (zaxis, xaxis);

		float xd, yd, zd;
		xd = dot3 (xaxis, pos);
		yd = dot3 (yaxis, pos);
		zd = dot3 (zaxis, pos);
		return float4x4
		(
			xaxis.x, yaxis.x, zaxis.x, 0,
			xaxis.y, yaxis.y, zaxis.y, 0,
			xaxis.z, yaxis.z, zaxis.z, 0,
			-xd, -yd, -zd, 1
		);
	}
	inline float4x4 float4x4::orthographic (float w, float h, float zn, float zf)
	{
		float fRange = 1.0f / (zn - zf);
		return float4x4 (
			2 / w, 0, 0, 0,
			0, 2 / h, 0, 0,
			0, 0, fRange, 0,
			0, 0, zn * fRange, 1
		);
	}
	inline float4x4 float4x4::orthographic_offcenter (float l, float r, float b, float t, float zn, float zf)
	{
		float reciprocalWidth = 1.f / (r - l);
		float reciprocalHeight = 1.f / (t - b);
		float fRange = 1.0f / (zn - zf);
		return float4x4 (
			reciprocalWidth + reciprocalWidth, 0, 0, 0,
			0, reciprocalHeight + reciprocalHeight, 0, 0,
			0, 0, fRange, 0,
			-(l + r) * reciprocalWidth, -(t + b) * reciprocalHeight, zn * fRange, 1
		);
	}
	inline float4x4 float4x4::perspective (float w, float h, float zn, float zf)
	{
		float twoNearZ = zn + zn;
		float fRange = zf / (zn - zf);
		return float4x4 (
			twoNearZ / w, 0, 0, 0,
			0, twoNearZ / h, 0, 0,
			0, 0, fRange, -1,
			0, 0, zn * fRange, 0
		);
	}
	inline float4x4 float4x4::perspective_offcenter (float l, float r, float b, float t, float zn, float zf)
	{
		float twoNearZ = zn + zn;
		float reciprocalWidth = 1.f / (r - l);
		float reciprocalHeight = 1.f / (t - b);
		float fRange = zf / (zn - zf);
		return float4x4 (
			twoNearZ * reciprocalWidth, 0, 0, 0,
			0, twoNearZ * reciprocalHeight, 0, 0,
			-(l + r) * reciprocalWidth, -(t + b) * reciprocalHeight, fRange, -1,
			0, 0, zn * fRange, 0
		);
	}
	inline float4x4 float4x4::perspective_fov (float fov, float aspectRatio, float zn, float zf)
	{
		float ys = 1 / tanf (fov * 0.5f), xs = ys / aspectRatio;
		float fRange = zf / (zn - zf);
		return float4x4 (
			xs, 0, 0, 0,
			0, ys, 0, 0,
			0, 0, fRange, -1,
			0, 0, zn * fRange, 0
		);
	}
	inline float4x4 float4x4::billboard (const float3& pos, const float3& camPos, const float3& camUp, const float3& camForward) noexcept
	{
		float3 vector, vector2, vector3;
		vector = pos - camPos;
		float num = vector.length_squared ();
		if (num < 0.0001f) vector = -camForward;
		else vector = vector * (1.0f / sqrtf (num));

		vector3 = cross3 (camUp, vector);
		vector3 = vector3.normalize ();
		vector2 = cross3 (vector, vector3);
		return float4x4 (
			vector3.x, vector3.y, vector3.z, 0,
			vector2.x, vector2.y, vector2.z, 0,
			vector.x, vector.y, vector.z, 0,
			pos.x, pos.y, pos.z, 1
		);
	}

}

#endif