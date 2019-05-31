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

namespace dseed
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
	struct plane;
	struct ray;

	struct float4x4;

	struct vectorf;
	struct matrixf;

	struct bounding_box;
	struct bounding_sphere;
	struct bounding_frustum;

	enum intersect_t { intersect_disjoint = 0, intersect_intersects, intersect_contains };
	enum plane_intersect_t { plane_intersect_front, plane_intersect_end, plane_intersect_intersecting };

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
	// Quaternion, Plane Type definitions
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
		inline quaternion conjugate () const noexcept;
		inline quaternion invert () const noexcept;

	public:
		static inline quaternion zero () noexcept { return quaternion (0, 0, 0, 0); }
		static inline quaternion identity () noexcept { return quaternion (0, 0, 0, 1); }
	};

	struct plane
	{
	public:
		float x, y, z, w;

	public:
		plane () = default;
		inline plane (const float3& p1, const float3& p2, const float3& p3);
		inline plane (const float3& p, const float3& n);
		inline plane (float x, float y, float z, float w) : x (x), y (y), z (z), w (w) { }
		inline plane (const float4& v) : plane (v.x, v.y, v.z, v.w) { }

	public:
		inline operator float4 () const noexcept { return float4 (x, y, z, w); }

	public:
		inline vectorf normalize ();

	public:
		inline bool intersects (const plane& p);
		inline void intersects (const plane& p, float3* ip1, float3* ip2);

		inline bool intersects (const float3& lp1, const float3& lp2);
		inline void intersects (const float3& lp1, const float3& lp2, float3* ip);
	};

	struct ray
	{
	public:
		float3 position, direction;

	public:
		ray () = default;
		inline ray (const float3& position, const float3& direction)
			: position (position), direction (direction)
		{ }
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
	enum vectorf_index_t : int { vectorf_index_x, vectorf_index_y, vectorf_index_z, vectorf_index_w };
	
	struct vectorf
	{
	public:
		vectorf () = default;
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
	public:
		__m128 _vector;

	public:
		inline vectorf (__m128 vector) : _vector (vector) { }
		inline operator __m128 () const { return _vector; }

	public:
		inline vectorf splat_x () const { return _mm_permute_ps (_vector, _MM_SHUFFLE (0, 0, 0, 0)); }
		inline vectorf splat_y () const { return _mm_permute_ps (_vector, _MM_SHUFFLE (1, 1, 1, 1)); }
		inline vectorf splat_z () const { return _mm_permute_ps (_vector, _MM_SHUFFLE (2, 2, 2, 2)); }
		inline vectorf splat_w () const { return _mm_permute_ps (_vector, _MM_SHUFFLE (3, 3, 3, 3)); }
		inline vectorf permute (int x, int y, int z, int w) const { return _mm_permute_ps (_vector, _MM_SHUFFLE (w, z, y, x)); }
		inline vectorf shuffle (const vectorf& v, int x, int y, int z, int w) const { return _mm_shuffle_ps (_vector, v._vector, _MM_SHUFFLE (w, z, y, x)); }
		inline float x () const { return _mm_cvtss_f32 (splat_x ()); }
		inline float y () const { return _mm_cvtss_f32 (splat_y ()); }
		inline float z () const { return _mm_cvtss_f32 (splat_z ()); }
		inline float w () const { return _mm_cvtss_f32 (splat_w ()); }
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
	private:
		constexpr int __SHUFFLE (int x, int y, int z, int w) { return (w << 6) | (z << 4) | (y << 2) | x; }

	public:
		float32x4_t _vector;

	public:
		inline vectorf (float32x4_t vector) : _vector (vector) { }
		inline operator float32x4_t () const { return _vector; }

	public:
		inline vectorf splat_x () const { vdupq_lane_f32 (vget_low_f32 (_vector), 0); }
		inline vectorf splat_y () const { vdupq_lane_f32 (vget_low_f32 (_vector), 1); }
		inline vectorf splat_z () const { vdupq_lane_f32 (vget_high_f32 (_vector), 0); }
		inline vectorf splat_w () const { vdupq_lane_f32 (vget_high_f32 (_vector), 1); }
		inline vectorf permute (int x, int y, int z, int w) const { return shuffle (*this, x, y, z, w); }
		inline vectorf shuffle (const vectorf& v, int x, int y, int z, int w) const
		{
			switch (__SHUFFLE (x, y, z, w))
			{
			case __SHUFFLE (0, 0, 0, 0): return vcombine_f32 (vdup_lane_f32 (vget_low_f32 (_vector), 0), vdup_lane_f32 (vget_low_f32 (v._vector), 0));
			case __SHUFFLE (1, 1, 1, 1): return vcombine_f32 (vdup_lane_f32 (vget_low_f32 (_vector), 1), vdup_lane_f32 (vget_low_f32 (v._vector), 1));
			case __SHUFFLE (2, 2, 2, 2): return vcombine_f32 (vdup_lane_f32 (vget_high_f32 (_vector), 0), vdup_lane_f32 (vget_high_f32 (v._vector), 0));
			case __SHUFFLE (3, 3, 3, 3): return vcombine_f32 (vdup_lane_f32 (vget_high_f32 (_vector), 1), vdup_lane_f32 (vget_high_f32 (v._vector), 1));
			case __SHUFFLE (2, 3, 0, 1): return vcombine_f32 (vget_high_f32 (_vector), vget_low_f32 (v._vector));
			case __SHUFFLE (1, 0, 3, 2): return vcombine_f32 (vrev64_f32 (vget_low_f32 (_vector)), vrev64_f32 (vget_high_f32 (v._vector)));
			case __SHUFFLE (1, 2, 3, 0): return vcombine_f32 (vget_high_f32 (vextq_f32 (_vector, _vector, 3)), vget_low_f32 (vextq_f32 (v._vector, v._vector, 3)));
			case __SHUFFLE (3, 0, 1, 2): return vcombine_f32 (vget_low_f32 (vextq_f32 (_vector, _vector, 3)), vget_high_f32 (vextq_f32 (v._vector, v._vector, 3)));
			case __SHUFFLE (0, 1, 0, 1): return vcombine_f32 (vget_low_f32 (_vector), vget_low_f32 (v._vector));
			case __SHUFFLE (1, 0, 0, 1): return vcombine_f32 (vrev64_f32 (vget_low_f32 (_vector)), vget_low_f32 (v._vector));
			case __SHUFFLE (1, 0, 1, 0): return vcombine_f32 (vrev64_f32 (vget_low_f32 (_vector)), vrev64_f32 (vget_low_f32 (v._vector)));
			case __SHUFFLE (0, 1, 2, 3): return vcombine_f32 (vget_low_f32 (_vector), vget_high_f32 (v._vector));
			case __SHUFFLE (1, 1, 0, 0): return vcombine_f32 (vdup_lane_f32 (vget_low_f32 (_vector), 1), vdup_lane_f32 (vget_low_f32 (v._vector), 0));
			case __SHUFFLE (2, 2, 0, 0): return vcombine_f32 (vdup_lane_f32 (vget_high_f32 (_vector), 0), vdup_lane_f32 (vget_low_f32 (v._vector), 0));
			case __SHUFFLE (0, 0, 2, 2): return vcombine_f32 (vdup_lane_f32 (vget_low_f32 (_vector), 0), vdup_lane_f32 (vget_high_f32 (v._vector), 0));
			case __SHUFFLE (2, 0, 2, 3): return vcombine_f32 (vset_lane_f32 (vgetq_lane_f32 (_vector, 0), vdup_lane_f32 (vget_high_f32 (_vector), 0), 1), vget_high_f32 (v._vector));
			case __SHUFFLE (3, 3, 1, 1): return vcombine_f32 (vdup_lane_f32 (vget_high_f32 (_vector), 1), vdup_lane_f32 (vget_low_f32 (v._vector), 1));
			case __SHUFFLE (0, 1, 0, 2):
				return vcombine_f32 (vget_low_f32 (_vector), vset_lane_f32 (vgetq_lane_f32 (v._vector, 2)
						, vdup_lane_f32 (vget_low_f32 (v._vector), 0), 1));
			case __SHUFFLE (1, 0, 0, 2):
				return vcombine_f32 (vrev64_f32 (vget_low_f32 (_vector)), vset_lane_f32 (vgetq_lane_f32 (v._vector, 2)
						, vdup_lane_f32 (vget_low_f32 (v._vector), 0), 1));
			case __SHUFFLE(2, 3, 0, 2):
				return vcombine_f32 (vget_high_f32 (_vector), vset_lane_f32 (vgetq_lane_f32 (v._vector, 2)
						, vdup_lane_f32 (vget_low_f32 (v._vector), 0), 1));
			default:
				return vsetq_lane_f32 (vgetq_lane_f32 (v._vector, w), vsetq_lane_f32 (vgetq_lane_f32 (v._vector, z)
						, vsetq_lane_f32 (vgetq_lane_f32 (_vector, y), vmovq_n_f32 (vgetq_lane_f32 (_vector, x)), 1), 2), 3);
			};
		}
		inline float x () const { return vgetq_lane_f32 (_vector, 0); }
		inline float y () const { return vgetq_lane_f32 (_vector, 1); }
		inline float z () const { return vgetq_lane_f32 (_vector, 2); }
		inline float w () const { return vgetq_lane_f32 (_vector, 3); }
#else
	public:
		float4 _vector;

	public:
		inline vectorf splat_x () const { return float4 (_vector.x).operator dseed::vectorf (); }
		inline vectorf splat_y () const { return float4 (_vector.y).operator dseed::vectorf (); }
		inline vectorf splat_z () const { return float4 (_vector.z).operator dseed::vectorf (); }
		inline vectorf splat_w () const { return float4 (_vector.w).operator dseed::vectorf (); }
		inline vectorf permute (int x, int y, int z, int w) const
		{
			return float4 (_vector[x], _vector[y], _vector[z], _vector[w]);
		}
		inline vectorf shuffle (const vectorf& v, int x, int y, int z, int w) const
		{
			return float4 (_vector[x], _vector[y], v._vector[z], v._vector[w]);
		}
		inline float x () const { return _vector.x; }
		inline float y () const { return _vector.y; }
		inline float z () const { return _vector.z; }
		inline float w () const { return _vector.w; }
#endif
	};

	struct matrixf
	{
	public:
		vectorf column1, column2, column3, column4;

	public:
		matrixf () = default;
		inline matrixf (const vectorf& c1, const vectorf& c2, const vectorf& c3, const vectorf& c4)
			: column1 (c1), column2 (c2), column3 (c3), column4 (c4)
		{ }
	};

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Bounding shape Type definitions
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct bounding_box
	{
	public:
		float3 center, extends;
		quaternion orientation;

	public:
		bounding_box () = default;
		inline bounding_box (const float3& center, const float3& extends,
			const quaternion& orientation)
			: center (center), extends (extends), orientation (orientation)
		{ }

	public:
		intersect_t intersects (const bounding_box& bb) const;
		intersect_t intersects (const bounding_sphere& bs) const;
		intersect_t intersects (const bounding_frustum& bf) const;
	};

	struct bounding_sphere
	{
	public:
		float3 center;
		float radius;

	public:
		bounding_sphere () = default;
		inline bounding_sphere (const float3& center, float radius)
			: center (center), radius (radius)
		{ }

	public:
		intersect_t intersects (const bounding_box& bb) const;
		intersect_t intersects (const bounding_sphere& bs) const;
		intersect_t intersects (const bounding_frustum& bf) const;
	};

	struct bounding_frustum
	{
	public:
		float3 center;
		quaternion orientation;

		float right, left, top, bottom;
		float znear, zfar;

	public:
		bounding_frustum () = default;
		inline bounding_frustum (const float3& center, const quaternion& orientation,
			float left, float top, float bottom, float right, float znear, float zfar)
			: center (center), orientation (orientation)
			, left (left), top (top), bottom (bottom), right (right)
			, znear (znear), zfar (zfar)
		{}
		inline bounding_frustum (const float4x4& projection);

	public:
		intersect_t intersects (const bounding_box& bb) const;
		intersect_t intersects (const bounding_sphere& bs) const;
		intersect_t intersects (const bounding_frustum& bf) const;
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
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vaddq_f32 (v1, v2);
#else
		return float4 { v1._vector.x + v2._vector.x, v1._vector.y + v2._vector.y, v1._vector.z + v2._vector.z, v1._vector.w + v2._vector.w };
#endif
	}
	inline vectorf operator- (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_sub_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vsubq_f32 (v1, v2);
#else
		return float4 { v1._vector.x - v2._vector.x, v1._vector.y - v2._vector.y, v1._vector.z - v2._vector.z, v1._vector.w - v2._vector.w };
#endif
	}
	inline vectorf operator- (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_sub_ps (_mm_load1_ps (0), v);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vsubq_f32 (vmovq_n_f32 (0), v);
#else
		return float4 { -v._vector.x, -v._vector.y, -v._vector.z, -v._vector.w };
#endif
	}
	inline vectorf operator* (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_mul_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vmulq_f32 (v1, v2);
#else
		return float4 { v1._vector.x * v2._vector.x, v1._vector.y * v2._vector.y, v1._vector.z * v2._vector.z, v1._vector.w * v2._vector.w };
#endif
	}
	inline vectorf operator* (const vectorf& v, float s) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_mul_ps (v, _mm_set1_ps (s));
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vmulq_f32 (v, vmovq_n_f32 (s));
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
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vdivq_f32 (v1, v2);
#else
		return float4 { v1._vector.x / v2._vector.x, v1._vector.y / v2._vector.y, v1._vector.z / v2._vector.z, v1._vector.w / v2._vector.w };
#endif
	}
	inline vectorf operator/ (const vectorf& v, float s)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_div_ps (v, _mm_set1_ps (s));
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vdivq_f32 (v, vmovq_n_f32 (s));
#else
		return float4{ v._vector.x / s, v._vector.y / s, v._vector.z / s, v._vector.w / s };
#endif
	}
	inline bool operator== (const vectorf& v1, const vectorf& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_movemask_ps (_mm_cmpeq_ps (v1, v2)) == 0xF;
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		uint32x4_t temp = vceqq_f32 (v1, v2);
		return temp[0] == 1 && temp[1] == 1 && temp[2] == 1 && temp[3] == 1;
#else
		return v1.x () == v2.x () && v1.y () == v2.y ()
			&& v1.z () == v2.z () && v1.w () == v2.w ();
#endif
	}
	inline bool operator!= (const vectorf& v1, const vectorf& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_movemask_ps (_mm_cmpneq_ps (v1, v2)) != 0;
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		uint32x4_t temp = vmvnq_u32 (vceqq_f32 (v1, v2));
		return temp[0] == 0 || temp[1] == 0 || temp[2] == 0 || temp[3] == 0;
#else
		return v1.x () != v2.x () || v1.y () != v2.y ()
			|| v1.z () != v2.z () || v1.w () != v2.w ();
#endif
	}
	inline bool operator< (const vectorf& v1, const vectorf& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_movemask_ps (_mm_cmplt_ps (v1, v2)) == 0xF;
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		uint32x4_t temp = vcltq_f32 (v1, v2);
		return temp[0] == 1 && temp[1] == 1 && temp[2] == 1 && temp[3] == 1;
#else
		return v1.x () < v2.x () && v1.y () < v2.y ()
			&& v1.z () < v2.z () && v1.w () < v2.w ();
#endif
	}
	inline bool operator<= (const vectorf& v1, const vectorf& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_movemask_ps (_mm_cmple_ps (v1, v2)) == 0xF;
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		uint32x4_t temp = vcleq_f32 (v1, v2);
		return temp[0] == 1 && temp[1] == 1 && temp[2] == 1 && temp[3] == 1;
#else
		return v1.x () <= v2.x () && v1.y () <= v2.y ()
			&& v1.z () <= v2.z () && v1.w () <= v2.w ();
#endif
	}
	inline bool operator> (const vectorf& v1, const vectorf& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_movemask_ps (_mm_cmpgt_ps (v1, v2)) == 0xF;
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		uint32x4_t temp = vcgtq_f32 (v1, v2);
		return temp[0] == 1 && temp[1] == 1 && temp[2] == 1 && temp[3] == 1;
#else
		return v1.x () > v2.x () && v1.y () > v2.y ()
			&& v1.z () > v2.z () && v1.w () > v2.w ();
#endif
	}
	inline bool operator>= (const vectorf& v1, const vectorf& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_movemask_ps (_mm_cmpge_ps (v1, v2)) == 0xF;
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		uint32x4_t temp = vcgeq_f32 (v1, v2);
		return temp[0] == 1 && temp[1] == 1 && temp[2] == 1 && temp[3] == 1;
#else
		return v1.x () >= v2.x () && v1.y () >= v2.y ()
			&& v1.z () >= v2.z () && v1.w () >= v2.w ();
#endif
	}
	inline vectorf minimum (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_min_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vminq_f32 (v1, v2);
#else
		return float4 (
			minimum<float> (v1.x (), v2.x ()),
			minimum<float> (v1.y (), v2.y ()),
			minimum<float> (v1.z (), v2.z ()),
			minimum<float> (v1.w (), v2.w ())
		);
#endif
	}
	inline vectorf maximum (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_max_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vmaxq_f32 (v1, v2);
#else
		return float4 (
			maximum<float> (v1.x (), v2.x ()),
			maximum<float> (v1.y (), v2.y ()),
			maximum<float> (v1.z (), v2.z ()),
			maximum<float> (v1.w (), v2.w ())
		);
#endif
	}
	inline vectorf fma (const vectorf& mv1, const vectorf& mv2, const vectorf& av) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC ) && !defined ( NO_FMA3 )
		return _mm_fmadd_ps (mv1, mv2, av);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
#	if ( ARCH_ARM64 )
		return vfmaq_f32 (av, mv1, mv2);
#	else
		return vmlaq_f32 (av, mv1, mv2);
#	endif
#else
		return (mv1 * mv2) + av;
#endif
	}
	inline vectorf fms (const vectorf& mv1, const vectorf& mv2, const vectorf& sv) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC ) && !defined ( NO_FMA3 )
		return _mm_fmsub_ps (mv1, mv2, sv);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
#	if ( ARCH_ARM64 )
		return vfmsq_f32 (sv, mv1, mv2);
#	else
		return vmlsq_f32 (sv, mv1, mv2);
#	endif
#else
		return (mv1 * mv2) - sv;
#endif
	}
	inline float dot2 (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtss_f32 (_mm_dp_ps (v1, v2, 0x3f));
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		float32x4_t temp = vmulq_f32 (vget_low_f32 (v1), vget_low_f32 (v2));
		return vgetq_lane_f32 (vaddq_f32 (temp, temp));
#else
		return (v1._vector.x * v2._vector.x) + (v1._vector.y * v2._vector.y);
#endif
	}
	inline float dot2 (const vectorf& v, float s) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtss_f32 (_mm_dp_ps (v, _mm_set1_ps (s), 0x3f));
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return dot2 (v, float4 (s));
#else
		return (v._vector.x * s) + (v._vector.y * s);
#endif
	}
	inline float dot3 (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtss_f32 (_mm_dp_ps (v1, v2, 0x7f));
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		float32x4_t temp = vmulq_f32 (v1, v2);
		float32x2_t v1 = vpadd_f32 (vget_low_f32 (temp), vget_low_f32 (temp));
		float32x2_t v2 = vdup_lane_f32 (vget_high_f32 (temp), 0);
		v1 = vadd_f32 (v1, v2);
		return vcombine_f32 (v1, v1);
#else
		return (v1._vector.x * v2._vector.x) + (v1._vector.y * v2._vector.y) + (v1._vector.z * v2._vector.z);
#endif
	}
	inline float dot3 (const vectorf& v, float s) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtss_f32 (_mm_dp_ps (v, _mm_set1_ps (s), 0x7f));
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return dot3 (v, float4 (s));
#else
		return (v._vector.x * s) + (v._vector.y * s) + (v._vector.z * s);
#endif
	}
	inline float dot4 (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtss_f32 (_mm_dp_ps (v1, v2, 0xff));
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		float32x4_t temp = vmulq_f32 (v1, v2);
		float32x2_t v1 = vget_low_f32 (temp);
		float32x2_t v2 = vget_high_f32 (temp);
		v1 = vadd_f32 (v1, v2);
		v1 = vpadd_f32 (v1, v1);
		return vcombine_f32 (v1, v1);
#else
		return (v1._vector.x * v2._vector.x) + (v1._vector.y * v2._vector.y) + (v1._vector.z * v2._vector.z) + (v1._vector.w * v2._vector.w);
#endif
	}

	inline vectorf cross2 (const vectorf& v1, const vectorf& v2) noexcept
	{
		vectorf ret = v2.permute (1, 0, 1, 0);
		ret = ret * v1;
		vectorf temp = ret.splat_y ();
		ret = ret - temp;
		ret = ret.permute (0, 0, 3, 3);
		return ret;
	}
	inline vectorf cross3 (const vectorf& v1, const vectorf& v2) noexcept
	{
		vectorf temp1 = v1.permute (1, 2, 0, 3);
		vectorf temp2 = v2.permute (2, 0, 1, 3);
		vectorf ret = temp1 * temp2;
		temp1 = temp1.permute (1, 2, 0, 3);
		temp2 = temp2.permute (2, 0, 1, 3);
		temp1 = temp1 * temp2;
		return ret - temp1;
	}
	inline vectorf cross4 (const vectorf& v1, const vectorf& v2, const vectorf& v3) noexcept
	{
		vectorf ret = v2.permute (2, 3, 1, 3);
		vectorf temp3 = v3.permute (3, 2, 3, 1);
		ret = ret * temp3;

		vectorf temp2 = v2.permute (3, 2, 3, 1);
		temp3 = temp3.permute (1, 0, 3, 1);
		temp2 = temp2 * temp3;
		ret = ret - temp2;

		vectorf temp1 = v1.permute (1, 0, 0, 0);
		ret = ret * temp1;

		temp2 = v2.permute (1, 3, 0, 2);
		temp3 = v3.permute (3, 0, 3, 0);
		temp3 = temp3 * temp2;

		temp2 = temp2.permute (1, 2, 1, 2);
		temp1 = v3.permute (1, 3, 0, 2);
		temp2 = temp2 * temp1;
		temp3 = temp3 - temp2;

		temp1 = v1.permute (2, 2, 1, 1);
		temp1 = temp1 * temp3;
		ret = ret - temp1;

		temp2 = v2.permute (1, 2, 0, 1);
		temp3 = v3.permute (2, 0, 1, 0);
		temp3 = temp3 * temp2;

		temp2 = temp2.permute (1, 2, 0, 2);
		temp1 = v3.permute (1, 2, 0, 1);
		temp1 = temp1 * temp2;
		temp3 = temp3 - temp1;

		temp1 = v1.permute (3, 3, 3, 2);
		temp3 = temp3 * temp1;
		ret = ret + temp3;

		return ret;
	}

	inline float length_squared2 (const vectorf& v) noexcept
	{
		return dot2 (v, v);
	}
	inline float length_squared3 (const vectorf& v) noexcept
	{
		return dot3 (v, v);
	}
	inline float length_squared4 (const vectorf& v) noexcept
	{
		return dot4 (v, v);
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

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Quaternion, Plane Type operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline vectorf operator* (const quaternion& q1, const quaternion& q2)
	{
		vectorf q2x = ((vectorf)q2).splat_x ();
		vectorf q2y = ((vectorf)q2).splat_y ();
		vectorf q2z = ((vectorf)q2).splat_z ();
		vectorf q2w = ((vectorf)q2).splat_w ();

		vectorf result = q2w * q1;
		q2x = (q2x * ((vectorf)q1).permute (3, 2, 1, 0)) * float4 (1, -1, 1, -1);
		q2y = (q2y * ((vectorf)q1).permute (1, 0, 3, 2)) * float4 (1, 1, -1, -1);
		q2z = (q2z * ((vectorf)q1).permute (3, 2, 1, 0)) * float4 (-1, 1, 1, -1);
		q2y = q2y + q2z;

		return result + q2x + q2y;
	}
	inline vectorf quaternion_conjugate (const vectorf& q)
	{
		return q * float4 (-1, -1, -1, 1);
	}
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
		vectorf temp0 = m.column1.shuffle (m.column2, 0, 1, 0, 1);
		vectorf temp2 = m.column1.shuffle (m.column2, 2, 3, 2, 3);
		vectorf temp1 = m.column3.shuffle (m.column4, 0, 1, 0, 1);
		vectorf temp3 = m.column3.shuffle (m.column4, 2, 3, 2, 3);

		matrixf ret;
		ret.column1 = temp0.shuffle (temp1, 0, 2, 0, 2);
		ret.column2 = temp0.shuffle (temp1, 1, 3, 1, 3);
		ret.column3 = temp2.shuffle (temp3, 0, 2, 0, 2);
		ret.column4 = temp2.shuffle (temp3, 1, 3, 1, 3);

		return ret;
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
		static vectorf one = float4 (1, 1, 1, 1);

		matrixf transposed = transpose (m);
		vectorf v00 = transposed.column3.permute (0, 0, 1, 1);
		vectorf v10 = transposed.column4.permute (2, 3, 2, 3);
		vectorf v01 = transposed.column1.permute (0, 0, 1, 1);
		vectorf v11 = transposed.column2.permute (2, 3, 2, 3);
		vectorf v02 = transposed.column3.shuffle (transposed.column1, 0, 2, 0, 2);
		vectorf v12 = transposed.column4.shuffle (transposed.column2, 1, 3, 1, 3);

		vectorf d0 = v00 * v10;
		vectorf d1 = v01 * v11;
		vectorf d2 = v02 * v12;

		v00 = transposed.column3.permute (2, 3, 2, 3);
		v10 = transposed.column4.permute (0, 0, 1, 1);
		v01 = transposed.column1.permute (2, 3, 2, 3);
		v11 = transposed.column2.permute (0, 0, 1, 1);
		v02 = transposed.column3.shuffle (transposed.column1, 1, 3, 1, 3);
		v12 = transposed.column4.shuffle (transposed.column2, 0, 2, 0, 2);

		v00 = v00 * v10;
		v01 = v01 * v11;
		v02 = v02 * v12;

		d0 = d0 - v00;
		d1 = d1 - v01;
		d2 = d2 - v02;

		v11 = d0.shuffle (d2, 1, 3, 1, 1);
		v00 = transposed.column2.permute (1, 2, 0, 1);
		v10 = v11.shuffle (d0, 2, 0, 3, 0);
		v01 = transposed.column1.permute (2, 0, 1, 0);
		v11 = v11.shuffle (d0, 1, 2, 1, 2);

		vectorf v13 = d1.shuffle (d2, 1, 3, 3, 3);
		v02 = transposed.column4.permute (1, 2, 0, 1);
		v12 = v13.shuffle (d1, 2, 0, 3, 0);
		vectorf v03 = transposed.column3.permute (2, 0, 1, 0);
		v13 = v13.shuffle (d1, 1, 2, 1, 2);

		vectorf c0 = v00 * v10;
		vectorf c2 = v01 * v11;
		vectorf c4 = v02 * v12;
		vectorf c6 = v03 * v13;

		v11 = d0.shuffle (d2, 0, 1, 0, 0);
		v00 = transposed.column2.permute (2, 3, 1, 2);
		v10 = d0.shuffle (v11, 3, 0, 1, 2);
		v01 = transposed.column1.permute (3, 2, 3, 1);
		v11 = d0.shuffle (v11, 2, 1, 2, 0);

		v13 = d1.shuffle (d2, 0, 1, 2, 2);
		v02 = transposed.column4.permute (2, 3, 1, 2);
		v12 = d1.shuffle (v13, 3, 0, 1, 2);
		v03 = transposed.column3.permute (3, 2, 3, 1);
		v13 = d1.shuffle (v13, 2, 1, 2, 0);

		v00 = v00 * v10;
		v01 = v01 * v11;
		v02 = v02 * v12;
		v03 = v03 * v13;
		c0 = c0 - v00;
		c2 = c2 - v01;
		c4 = c4 - v02;
		c6 = c6 - v03;

		v00 = transposed.column2.permute (3, 0, 3, 0);
		v10 = d0.shuffle (d2, 2, 2, 0, 1);
		v10 = v10.permute (0, 3, 2, 0);
		v01 = transposed.column1.permute (1, 3, 0, 2);
		v11 = d0.shuffle (d2, 0, 3, 0, 1);
		v11 = v11.permute (3, 0, 1, 2);
		v02 = transposed.column4.permute (3, 0, 3, 0);
		v12 = d1.shuffle (d2, 2, 2, 2, 3);
		v12 = v12.permute (0, 3, 2, 0);
		v03 = transposed.column3.permute (1, 3, 0, 2);
		v13 = d1.shuffle (d2, 0, 3, 2, 3);
		v13 = v13.permute (3, 0, 1, 2);

		v00 = v00 * v10;
		v01 = v01 * v11;
		v02 = v02 * v12;
		v03 = v03 * v13;
		vectorf c1 = c0 - v00;
		c0 = c0 + v00;
		vectorf c3 = c2 + v01;
		c2 = c2 - v01;
		vectorf c5 = c4 - v02;
		c4 = c4 + v02;
		vectorf c7 = c6 + v03;
		c6 = c6 - v03;

		c0 = c0.shuffle (c1, 0, 2, 1, 3);
		c2 = c2.shuffle (c3, 0, 2, 1, 3);
		c4 = c4.shuffle (c5, 0, 2, 1, 3);
		c6 = c6.shuffle (c7, 0, 2, 1, 3);
		c0 = c0.permute (0, 2, 1, 3);
		c2 = c2.permute (0, 2, 1, 3);
		c4 = c4.permute (0, 2, 1, 3);
		c6 = c6.permute (0, 2, 1, 3);

		vectorf temp = float4 (dot4 (c0, transposed.column1));
		temp = one / temp;

		matrixf ret;
		ret.column1 = c0 * temp;
		ret.column2 = c2 * temp;
		ret.column3 = c4 * temp;
		ret.column4 = c6 * temp;

		return ret;
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
	inline dseed::float2::float2 (const vectorf& v) noexcept
	{
		float4 temp = v;
		x = temp.x;
		y = temp.y;
	}

	inline float2::operator vectorf () const noexcept
	{
		return float4 (x, y, 0, 0);
	}

	inline vectorf float2::normalize () const noexcept
	{
		return normalize2 (*this);
	}

	inline float float2::length_squared () const noexcept
	{
		return length_squared2 (*this);
	}

	inline float float2::length () const noexcept
	{
		return length2 (*this);
	}

	inline dseed::float3::float3 (const vectorf& v) noexcept
	{
		float4 temp = v;
		x = temp.x;
		y = temp.y;
		z = temp.z;
	}

	inline float3::operator vectorf () const noexcept
	{
		return float4 (x, y, z, 0);
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

	inline dseed::float4::float4 (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		_mm_store_ps ((float*)arr, v);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		vst1q_f32 ((float*)arr, v);
#else
		* this = v._vector;
#endif
	}

	inline float4::operator vectorf () const noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_load_ps ((float*)this);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vld1q_f32 ((float*)this);
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

	inline quaternion quaternion::conjugate () const noexcept
	{
		return quaternion_conjugate (*this);
	}

	inline quaternion quaternion::invert () const noexcept
	{
		vectorf length = float4 (length4 (*this));
		vectorf conjugate = this->conjugate ();

		if (length.x () <= single_epsilon)
			return quaternion (0, 0, 0, 0);

		return conjugate / length;
	}

	inline plane::plane (const float3& p1, const float3& p2, const float3& p3)
		: plane (p1, normalize3 (cross3 ((vectorf) p2 - p1, (vectorf) p3 - p1)))
	{ }
	inline plane::plane (const float3 & p, const float3 & n)
		: plane (n.x, n.y, n.z, -dot3 (p, n))
	{ }
	inline vectorf plane::normalize () { return normalize_plane ((float4)* this); }

	inline bool plane::intersects (const plane& p)
	{
		float3 ip1, ip2;
		intersects (p, &ip1, &ip2);
		return !( ip1 == ip2 && ip1 == float3 (dseed::single_nan));
	}

	inline void plane::intersects (const plane& p, float3* ip1, float3* ip2)
	{
		if (ip1 == nullptr || ip2 == nullptr) return;

		vectorf v1 = cross3 ((float4)p, (float4)*this);
		float lengthsq = length_squared3 (v1);

		vectorf v2 = cross3 ((float4)p, v1);
		vectorf point = v1 * float4 (w, w, w, w);

		vectorf v3 = cross3 (v1, (float4)* this);
		point = fma (v3, float4 (p.w, p.w, p.w, p.w), point);

		if (dseed::equals (lengthsq, 0))
		{
			*ip1 = point / lengthsq;
			*ip2 = *ip1 + v1;
		}
		else
		{
			*ip1 = *ip2 = float3 (dseed::single_nan);
		}
	}

	inline bool plane::intersects (const float3& lp1, const float3& lp2)
	{
		float3 ip;
		intersects (lp1, lp2, &ip);
		return ip != float3 (dseed::single_nan);
	}
	inline void plane::intersects (const float3& lp1, const float3& lp2, float3* ip)
	{
		float d = dot3 ((float4)* this, lp1) - dot3 ((float4)* this, lp2);
		*ip = dseed::equals (d, 0) ? fma (lp2 - lp1, float3 (dot_plane_coord ((float4)* this, lp1) / d), lp1) : float3 (dseed::single_nan);
	}

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

	inline float float4x4::determinant () const noexcept { return dseed::determinant (*this); }
	inline matrixf float4x4::transpose () const noexcept { return dseed::transpose (*this); }
	inline matrixf float4x4::invert () const noexcept { return dseed::invert (*this); }

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
		return float4x4 ( 2 / w, 0, 0, 0, 0, 2 / h, 0, 0, 0, 0, fRange, 0, 0, 0, zn * fRange, 1 );
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
		return float4x4 (
			twoNearZ * width, 0, 0, 0,
			0, twoNearZ * height, 0, 0,
			-(l + r) * width, -(t + b) * height, fRange, -1,
			0, 0, zn * fRange, 0
		);
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
		vector = (num < 0.0001f) ? vector = -camForward : vector * (1.0f / sqrtf (num));

		vector3 = normalize3 (cross3 (camUp, vector));
		vector2 = cross3 (vector, vector3);
		return float4x4 (float4 (vector3, 0), float4 (vector2, 0), float4 (vector, 0), float4 (pos, 1));
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Bounding Shape Type Implement
	//
	////////////////////////////////////////////////////////////////////////////////////////
	const static vectorf ____bounding_box_polygon[] = {
		float3 (-1.0f, -1.0f, +1.0f),
		float3 (+1.0f, -1.0f, +1.0f),
		float3 (+1.0f, +1.0f, +1.0f),
		float3 (-1.0f, +1.0f, +1.0f),
		float3 (-1.0f, -1.0f, -1.0f),
		float3 (+1.0f, -1.0f, -1.0f),
		float3 (+1.0f, +1.0f, -1.0f),
		float3 (-1.0f, +1.0f, -1.0f),
	};

	inline intersect_t bounding_box::intersects (const bounding_box& bb) const
	{
		// TODO
	}
	inline intersect_t bounding_box::intersects (const bounding_sphere& bs) const
	{
		// TODO
	}
	inline intersect_t bounding_box::intersects (const bounding_frustum& bf) const
	{
		// TODO
	}

	inline intersect_t bounding_sphere::intersects (const bounding_box& bb) const
	{
		if (!bb.intersects (*this))
			return intersect_disjoint;

		vectorf center = this->center;
		vectorf radius = float3 (this->radius);
		vectorf radiusSqured = radius * radius;

		vectorf boxCenter = bb.center;
		vectorf boxExtends = bb.extends;
		vectorf boxOrientation = bb.orientation;

		for (auto i = 0; i < 8; ++i)
		{
			vectorf c = ((quaternion_conjugate (bb.orientation) * (boxExtends * ____bounding_box_polygon[i])) * bb.orientation) + bb.center;
			vectorf d = float4 (length3 (center - c));
		}

		return intersect_t ();
	}
	inline intersect_t bounding_sphere::intersects (const bounding_sphere& bs) const
	{
		vectorf center1 = center;
		vectorf center2 = bs.center;

		vectorf v = center2 - center1;
		float distance = length3 (v);

		return (radius + bs.radius >= distance)
			? (radius - bs.radius >= distance ? intersect_contains : intersect_intersects)
			: intersect_disjoint;
	}
	inline intersect_t bounding_sphere::intersects (const bounding_frustum& bf) const
	{
		return bf.intersects (*this);
	}

	inline bounding_frustum::bounding_frustum (const float4x4& projection)
	{
		// TODO
	}
	inline intersect_t bounding_frustum::intersects (const bounding_box& bb) const
	{
		// TODO
	}
	inline intersect_t bounding_frustum::intersects (const bounding_sphere& bs) const
	{
		// TODO
	}
	inline intersect_t bounding_frustum::intersects (const bounding_frustum& bf) const
	{
		// TODO
	}
}

#endif