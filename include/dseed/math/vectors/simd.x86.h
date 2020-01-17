#ifndef __DSEED_MATH_VECTORS_SIMD_X86_H__
#define __DSEED_MATH_VECTORS_SIMD_X86_H__

#if ARCH_X86SET

#	include <xmmintrin.h>									// SSE
#	include <emmintrin.h>									// SSE 2
#	include <pmmintrin.h>									// SSE 3
#	include <smmintrin.h>									// SSE 4.x
#	include <nmmintrin.h>									// SSE 4.x
#	include <immintrin.h>									// AVX

namespace dseed
{
	////////////////////////////////////////////////////////////////////////////////////////
	//
	// 32-bit Floating Point 4D Vector
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct f32x4_x86
	{
	public:
		__m128 _vector;

	public:
		inline f32x4_x86 () noexcept = default;
		inline f32x4_x86 (float v) noexcept : _vector (_mm_set1_ps (v)) { }
		inline f32x4_x86 (float x, float y, float z, float w) noexcept : _vector (_mm_set_ps (w, z, y, x)) { }
		inline f32x4_x86 (READS (4) const float* arr) noexcept : _vector (_mm_load_ps (arr)) { }
		inline f32x4_x86 (const f32x4_arith& vector) noexcept : _vector (_mm_load_ps ((const float*)&vector)) { }
		inline f32x4_x86 (const __m128& vector) noexcept : _vector (vector) { }

	public:
		inline operator const __m128& () const noexcept { return _vector; }
		inline explicit operator f32x4_arith () const noexcept { f32x4_arith ret; store ((float*)&ret); return ret; }

	public:
		inline f32x4_x86 splat_x () const noexcept { return _mm_permute_ps (_vector, _MM_SHUFFLE (0, 0, 0, 0)); }
		inline f32x4_x86 splat_y () const noexcept { return _mm_permute_ps (_vector, _MM_SHUFFLE (1, 1, 1, 1)); }
		inline f32x4_x86 splat_z () const noexcept { return _mm_permute_ps (_vector, _MM_SHUFFLE (2, 2, 2, 2)); }
		inline f32x4_x86 splat_w () const noexcept { return _mm_permute_ps (_vector, _MM_SHUFFLE (3, 3, 3, 3)); }

	public:
		inline float x () const noexcept { return _mm_cvtss_f32 (splat_x ()); }
		inline float y () const noexcept { return _mm_cvtss_f32 (splat_y ()); }
		inline float z () const noexcept { return _mm_cvtss_f32 (splat_z ()); }
		inline float w () const noexcept { return _mm_cvtss_f32 (splat_w ()); }

	public:
		inline void store (WRITES (4) float* arr) const noexcept { _mm_store_ps (arr, _vector); }
	};

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// 32-bit Signed Integer 4D Vector
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct i32x4_x86
	{
	public:
		__m128i _vector;

	public:
		inline i32x4_x86 () noexcept = default;
		inline i32x4_x86 (int v) noexcept : _vector (_mm_set1_epi32 (v)) { }
		inline i32x4_x86 (int x, int y, int z, int w) noexcept : _vector (_mm_set_epi32 (w, z, y, x)) { }
		inline i32x4_x86 (READS (4) const int* arr) noexcept : _vector (_mm_load_si128 ((const __m128i*)arr)) { }
		inline i32x4_x86 (const i32x4_arith& vector) noexcept : _vector (_mm_load_si128 ((const __m128i*)&vector)) { }
		inline i32x4_x86 (const __m128i& vector) noexcept : _vector (vector) { }

	public:
		inline operator const __m128i& () const noexcept { return _vector; }
		inline explicit operator i32x4_arith () const noexcept { i32x4_arith ret; store ((int*)&ret); return ret; }

	public:
		inline i32x4_x86 splat_x () const noexcept { return _mm_shuffle_epi32 (_vector, _MM_SHUFFLE (0, 0, 0, 0)); }
		inline i32x4_x86 splat_y () const noexcept { return _mm_shuffle_epi32 (_vector, _MM_SHUFFLE (1, 1, 1, 1)); }
		inline i32x4_x86 splat_z () const noexcept { return _mm_shuffle_epi32 (_vector, _MM_SHUFFLE (2, 2, 2, 2)); }
		inline i32x4_x86 splat_w () const noexcept { return _mm_shuffle_epi32 (_vector, _MM_SHUFFLE (3, 3, 3, 3)); }

	public:
		inline int x () const noexcept { return _mm_cvtss_si32 (_mm_castsi128_ps (splat_x ())); }
		inline int y () const noexcept { return _mm_cvtss_si32 (_mm_castsi128_ps (splat_y ())); }
		inline int z () const noexcept { return _mm_cvtss_si32 (_mm_castsi128_ps (splat_z ())); }
		inline int w () const noexcept { return _mm_cvtss_si32 (_mm_castsi128_ps (splat_w ())); }

	public:
		inline void store (WRITES (4) int* arr) const noexcept { _mm_store_si128 ((__m128i*)arr, _vector); }
	};

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// 32-bit Floating Point 4*4 Matrix
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct f32x4x4_x86
	{
	public:
		f32x4_x86 _column1, _column2, _column3, _column4;

		inline f32x4x4_x86 () noexcept = default;
		inline f32x4x4_x86 (float v) noexcept : _column1 (v), _column2 (v), _column3 (v), _column4 (v) { }
		inline f32x4x4_x86 (READS (16) const float* arr) noexcept
			: _column1 (&arr[0]), _column2 (&arr[4]), _column3 (&arr[8]), _column4 (&arr[12])
		{ }
		inline f32x4x4_x86 (const f32x4_x86& c1, const f32x4_x86& c2, const f32x4_x86& c3, const f32x4_x86& c4) noexcept
			: _column1 (c1), _column2 (c2), _column3 (c3), _column4 (c4)
		{ }
		inline f32x4x4_x86 (
			float m11, float m12, float m13, float m14,
			float m21, float m22, float m23, float m24,
			float m31, float m32, float m33, float m34,
			float m41, float m42, float m43, float m44) noexcept
			: _column1 (m11, m12, m13, m14)
			, _column2 (m21, m22, m23, m24)
			, _column3 (m31, m32, m33, m34)
			, _column4 (m41, m42, m43, m44)
		{ }

	public:
		inline f32x4_x86 column1 () const noexcept { return _column1; }
		inline f32x4_x86 column2 () const noexcept { return _column2; }
		inline f32x4_x86 column3 () const noexcept { return _column3; }
		inline f32x4_x86 column4 () const noexcept { return _column4; }

	public:
		inline float m11 () const noexcept { return column1 ().x (); }
		inline float m12 () const noexcept { return column1 ().y (); }
		inline float m13 () const noexcept { return column1 ().z (); }
		inline float m14 () const noexcept { return column1 ().w (); }

		inline float m21 () const noexcept { return column2 ().x (); }
		inline float m22 () const noexcept { return column2 ().y (); }
		inline float m23 () const noexcept { return column2 ().z (); }
		inline float m24 () const noexcept { return column2 ().w (); }

		inline float m31 () const noexcept { return column3 ().x (); }
		inline float m32 () const noexcept { return column3 ().y (); }
		inline float m33 () const noexcept { return column3 ().z (); }
		inline float m34 () const noexcept { return column3 ().w (); }

		inline float m41 () const noexcept { return column4 ().x (); }
		inline float m42 () const noexcept { return column4 ().y (); }
		inline float m43 () const noexcept { return column4 ().z (); }
		inline float m44 () const noexcept { return column4 ().w (); }

	public:
		inline void store (WRITES (16) float* arr) const noexcept
		{
			_column1.store (&arr[0]);
			_column2.store (&arr[4]);
			_column3.store (&arr[8]);
			_column4.store (&arr[12]);
		}
	};

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Vector/Matrix Casting and Reinterpreting Operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline f32x4_x86 cast_if (const i32x4_x86& v) noexcept { return _mm_cvtepi32_ps (v); }
	inline i32x4_x86 cast_fi (const f32x4_x86& v) noexcept { return _mm_cvtps_epi32 (v); }
	inline f32x4_x86 reinterpret_if (const i32x4_x86& v) noexcept { return _mm_castsi128_ps (v); }
	inline i32x4_x86 reinterpret_fi (const f32x4_x86& v) noexcept { return _mm_castps_si128 (v); }

	inline int movemask (const f32x4_x86& v) noexcept { return _mm_movemask_ps (v); }
	inline int movemask (const i32x4_x86& v) noexcept { return _mm_movemask_epi8 (v); }
	inline int movemask (const f32x4x4_x86& m) noexcept
	{
		return ((movemask (m.column1 ()) & 0xf) | ((movemask (m.column2 ()) << 4) & 0xf0)
			| ((movemask (m.column3 ()) << 8) & 0xf00) | ((movemask (m.column4 ()) << 12) & 0xf000));
	}

	template<int x, int y, int z, int w>
	inline f32x4_x86 permute (const f32x4_x86& v) noexcept
	{
		return _mm_permute_ps (v, _MM_SHUFFLE (w, z, y, x));
	}
	template<int x, int y, int z, int w>
	inline f32x4_x86 shuffle (const f32x4_x86& v1, const f32x4_x86& v2) noexcept
	{
		return _mm_shuffle_ps (v1, v2, _MM_SHUFFLE (w, z, y, x));
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// 32-bit Floating Point 4D Vector Operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline f32x4_x86 add (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_add_ps (v1, v2); }
	inline f32x4_x86 subtract (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_sub_ps (v1, v2); }
	inline f32x4_x86 negate (const f32x4_x86& v) noexcept { static f32x4_x86 zero (0.000000000f); return subtract (zero, v); }
	inline f32x4_x86 multiply (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_mul_ps (v1, v2); }
	inline f32x4_x86 multiply (const f32x4_x86& v, float s) noexcept { return _mm_mul_ps (v, _mm_set1_ps (s)); }
	inline f32x4_x86 multiply (float s, const f32x4_x86& v) noexcept { return multiply (v, s); }
	inline f32x4_x86 divide (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_div_ps (v1, v2); }
	inline f32x4_x86 divide (const f32x4_x86& v, float s) noexcept { return _mm_div_ps (v, _mm_set1_ps (s)); }

	inline f32x4_x86 fma (const f32x4_x86& mv1, const f32x4_x86& mv2, const f32x4_x86& av) noexcept { return _mm_fmadd_ps (mv1, mv2, av); }
	inline f32x4_x86 fms (const f32x4_x86& mv1, const f32x4_x86& mv2, const f32x4_x86& sv) noexcept { return _mm_fmsub_ps (mv1, mv2, sv); }
	inline f32x4_x86 fnms (const f32x4_x86& mv1, const f32x4_x86& mv2, const f32x4_x86& sv) noexcept { return _mm_fnmsub_ps (mv1, mv2, sv);; }

	inline f32x4_x86 and (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_and_ps (v1, v2); }
	inline f32x4_x86 or (const f32x4_x86 & v1, const f32x4_x86 & v2) noexcept { return _mm_or_ps (v1, v2); }
	inline f32x4_x86 xor (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_xor_ps (v1, v2); }
	inline f32x4_x86 not (const f32x4_x86& v) noexcept
	{
		i32x4_x86 iv = cast_fi (v);
		return _mm_castsi128_ps (_mm_xor_si128 (iv, _mm_cmpeq_epi32 (iv, iv)));
	}

	inline f32x4_x86 operator+ (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return add (v1, v2); }
	inline f32x4_x86 operator- (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return subtract (v1, v2); }
	inline f32x4_x86 operator- (const f32x4_x86& v) noexcept { return negate (v); }
	inline f32x4_x86 operator* (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return multiply (v1, v2); }
	inline f32x4_x86 operator* (const f32x4_x86& v, float s) noexcept { return multiply (v, s); }
	inline f32x4_x86 operator* (float s, const f32x4_x86& v) noexcept { return multiply (s, v); }
	inline f32x4_x86 operator/ (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return divide (v1, v2); }
	inline f32x4_x86 operator/ (const f32x4_x86& v, float s) noexcept { return divide (v, s); }
	inline f32x4_x86 operator& (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return and (v1, v2); }
	inline f32x4_x86 operator| (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return or (v1, v2); }
	inline f32x4_x86 operator^ (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return xor (v1, v2); }
	inline f32x4_x86 operator~ (const f32x4_x86& v) noexcept { return not (v); }

	inline f32x4_x86 equals (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_cmpeq_ps (v1, v2); }
	inline f32x4_x86 not_equals (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_cmpneq_ps (v1, v2); }
	inline f32x4_x86 lesser (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_cmplt_ps (v1, v2); }
	inline f32x4_x86 lesser_equals (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_cmple_ps (v1, v2); }
	inline f32x4_x86 greater (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_cmpgt_ps (v1, v2); }
	inline f32x4_x86 greater_equals (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_cmpge_ps (v1, v2); }

	inline bool operator== (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return 0xf == movemask (equals (v1, v2)); }
	inline bool operator!= (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return 0xf == movemask (not_equals (v1, v2)); }
	inline bool operator< (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return 0xf == movemask (lesser (v1, v2)); }
	inline bool operator<= (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return 0xf == movemask (lesser_equals (v1, v2)); }
	inline bool operator> (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return 0xf == movemask (greater (v1, v2)); }
	inline bool operator>= (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return 0xf == movemask (greater_equals (v1, v2)); }

	inline f32x4_x86 dot2d (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_dp_ps (v1, v2, 0x3f); }
	inline f32x4_x86 dot3d (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_dp_ps (v1, v2, 0x7f); }
	inline f32x4_x86 dot4d (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_dp_ps (v1, v2, 0xff); }

	inline float dot2df (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return dot2d (v1, v2).x (); }
	inline float dot3df (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return dot3d (v1, v2).x (); }
	inline float dot4df (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return dot4d (v1, v2).x (); }

	inline f32x4_x86 cross2d (const f32x4_x86& v1, const f32x4_x86& v2) noexcept
	{
		f32x4_x86 v2_ = permute<1, 0, 3, 3> (v2);
		return multiply (v1, v2_);
	}
	inline f32x4_x86 cross3d (const f32x4_x86& v1, const f32x4_x86& v2) noexcept
	{
		f32x4_x86 v1_1 = permute<1, 2, 0, 3> (v1);
		f32x4_x86 v2_1 = permute<2, 0, 1, 3> (v2);
		f32x4_x86 v1_2 = permute<2, 0, 1, 3> (v1);
		f32x4_x86 v2_2 = permute<1, 2, 0, 3> (v2);
		return subtract (multiply (v1_1, v2_1), multiply (v1_2, v2_2));
	}
	inline f32x4_x86 cross4d (const f32x4_x86& v1, const f32x4_x86& v2, const f32x4_x86& v3) noexcept
	{
		// NEED TO CONVERT TO SIMD OPERATIONS
		return f32x4_x86 (
			+(v1.y () * ((v2.z () * v3.w ()) - (v2.w () * v3.z ()))) - (v1.z () * ((v2.y () * v3.w ()) - (v2.w () * v3.y ()))) + (v1.w () * ((v2.y () * v3.z ()) - (v2.z () * v3.y ()))),
			-(v1.x () * ((v2.z () * v3.w ()) - (v2.w () * v3.z ()))) + (v1.z () * ((v2.x () * v3.w ()) - (v2.w () * v3.x ()))) - (v1.w () * ((v2.x () * v3.z ()) - (v2.z () * v3.x ()))),
			+(v1.x () * ((v2.y () * v3.w ()) - (v2.w () * v3.y ()))) - (v1.y () * ((v2.x () * v3.w ()) - (v2.w () * v3.x ()))) + (v1.w () * ((v2.x () * v3.y ()) - (v2.y () * v3.x ()))),
			-(v1.x () * ((v2.y () * v3.z ()) - (v2.z () * v3.y ()))) + (v1.y () * ((v2.x () * v3.z ()) - (v2.z () * v3.x ()))) - (v1.z () * ((v2.x () * v3.y ()) - (v2.y () * v3.x ())))
		);
	}

	inline f32x4_x86 transform2d (const f32x4_x86& v, const f32x4x4_x86& m) noexcept
	{
		return fma (v.splat_x (), m.column1 (), fma (v.splat_y (), m.column2 (), m.column4 ()));
	}
	inline f32x4_x86 transform3d (const f32x4_x86& v, const f32x4x4_x86& m) noexcept
	{
		return fma (v.splat_x (), m.column1 (), fma (v.splat_y (), m.column2 (), fma (v.splat_z (), m.column3 (), m.column4 ())));
	}
	inline f32x4_x86 transform4d (const f32x4_x86& v, const f32x4x4_x86& m) noexcept
	{
		return fma (v.splat_x (), m.column1 (), fma (v.splat_y (), m.column2 (), fma (v.splat_z (), m.column3 (), multiply (v.splat_w (), m.column4 ()))));
	}

	inline f32x4_x86 transform2dnorm (const f32x4_x86& v, const f32x4x4_x86& m) noexcept
	{
		return fma (v.splat_x (), m.column1 (), multiply (v.splat_y (), m.column2 ()));
	}
	inline f32x4_x86 transform3dnorm (const f32x4_x86& v, const f32x4x4_x86& m) noexcept
	{
		return fma (v.splat_x (), m.column1 (), fma (v.splat_y (), m.column2 (), multiply (v.splat_z (), m.column3 ())));
	}

	inline f32x4_x86 rcp (const f32x4_x86& v) noexcept { return _mm_rcp_ps (v); }
	inline f32x4_x86 pow (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_pow_ps (v1, v2); }
	inline f32x4_x86 sqrt (const f32x4_x86& v) noexcept { return _mm_sqrt_ps (v); }
	inline f32x4_x86 rsqrt (const f32x4_x86& v) noexcept { return _mm_rsqrt_ps (v); }
	inline f32x4_x86 round (const f32x4_x86& v) noexcept { return _mm_round_ps (v, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC); }
	inline f32x4_x86 floor (const f32x4_x86& v) noexcept { return _mm_floor_ps (v); }
	inline f32x4_x86 ceil (const f32x4_x86& v) noexcept { return _mm_ceil_ps (v); }
	inline f32x4_x86 abs (const f32x4_x86& v) noexcept
	{
		static f32x4_x86 signing = reinterpret_if (i32x4_x86 (0x7fffffff));
		return and (v, signing);
	}
	inline f32x4_x86 sin (const f32x4_x86& v) noexcept
	{
		// http://gruntthepeon.free.fr/ssemath/sse_mathfun.h
		static __m128 sign_mask = _mm_castsi128_ps (_mm_set1_epi32 (0x80000000)),
			quater_pi = _mm_set1_ps (4 / 3.14159265359f), onef = _mm_set1_ps (1), half = _mm_set1_ps (0.5f);
		static __m128i one = _mm_set1_epi32 (1), one_inv = _mm_set1_epi32 (~1), two = _mm_set1_epi32 (2), four = _mm_set1_epi32 (4);
		static __m128 minus_cephes_dp1 = _mm_set1_ps (-0.78515625f)
			, minus_cephes_dp2 = _mm_set1_ps ((float)-2.4187564849853515625e-4)
			, minus_cephes_dp3 = _mm_set1_ps ((float)-3.77489497744594108e-8);
		static __m128 sincof_p0 = _mm_set1_ps ((float)-1.9515295891e-4)
			, sincof_p1 = _mm_set1_ps ((float)8.3321608736e-3)
			, sincof_p2 = _mm_set1_ps ((float)-1.6666654611e-1);
		static __m128 coscof_p0 = _mm_set1_ps ((float)2.443315711809948e-005)
			, coscof_p1 = _mm_set1_ps ((float)-1.388731625493765e-003)
			, coscof_p2 = _mm_set1_ps ((float)4.166664568298827e-002);

		__m128 xmm1, xmm2 = _mm_setzero_ps (), xmm3, sign_bit, y;
		__m128i emm0, emm2;

		sign_bit = v;
		/* take the absolute value */
		__m128 x = abs (v);
		/* extract the sign bit (upper one) */
		sign_bit = _mm_and_ps (sign_bit, sign_mask);

		/* scale by 4/Pi */
		y = _mm_mul_ps (x, quater_pi);

		/* store the integer part of y in mm0 */
		emm2 = _mm_cvttps_epi32 (y);
		/* j=(j+1) & (~1) (see the cephes sources) */
		emm2 = _mm_add_epi32 (emm2, one);
		emm2 = _mm_and_si128 (emm2, one_inv);
		y = _mm_cvtepi32_ps (emm2);

		/* get the swap sign flag */
		emm0 = _mm_and_si128 (emm2, four);
		emm0 = _mm_slli_epi32 (emm0, 29);
		/* get the polynom selection mask
		   there is one polynom for 0 <= x <= Pi/4
		   and another one for Pi/4<x<=Pi/2

		   Both branches will be computed.
		*/
		emm2 = _mm_and_si128 (emm2, two);
		emm2 = _mm_cmpeq_epi32 (emm2, _mm_setzero_si128 ());

		__m128 swap_sign_bit = _mm_castsi128_ps (emm0);
		__m128 poly_mask = _mm_castsi128_ps (emm2);
		sign_bit = _mm_xor_ps (sign_bit, swap_sign_bit);

		/* The magic pass: "Extended precision modular arithmetic"
		x = ((x - y * DP1) - y * DP2) - y * DP3; */
		xmm1 = minus_cephes_dp1;
		xmm2 = minus_cephes_dp2;
		xmm3 = minus_cephes_dp3;
		xmm1 = _mm_mul_ps (y, xmm1);
		xmm2 = _mm_mul_ps (y, xmm2);
		xmm3 = _mm_mul_ps (y, xmm3);
		x = _mm_add_ps (x, xmm1);
		x = _mm_add_ps (x, xmm2);
		x = _mm_add_ps (x, xmm3);

		/* Evaluate the first polynom  (0 <= x <= Pi/4) */
		y = coscof_p0;
		__m128 z = _mm_mul_ps (x, x);

		y = _mm_mul_ps (y, z);
		y = _mm_add_ps (y, coscof_p1);
		y = _mm_mul_ps (y, z);
		y = _mm_add_ps (y, coscof_p2);
		y = _mm_mul_ps (y, z);
		y = _mm_mul_ps (y, z);
		__m128 tmp = _mm_mul_ps (z, half);
		y = _mm_sub_ps (y, tmp);
		y = _mm_add_ps (y, onef);

		/* Evaluate the second polynom  (Pi/4 <= x <= 0) */
		__m128 y2 = sincof_p0;
		y2 = _mm_mul_ps (y2, z);
		y2 = _mm_add_ps (y2, sincof_p1);
		y2 = _mm_mul_ps (y2, z);
		y2 = _mm_add_ps (y2, sincof_p2);
		y2 = _mm_mul_ps (y2, z);
		y2 = _mm_mul_ps (y2, x);
		y2 = _mm_add_ps (y2, x);

		/* select the correct result from the two polynoms */
		xmm3 = poly_mask;
		y2 = _mm_and_ps (xmm3, y2); //, xmm3);
		y = _mm_andnot_ps (xmm3, y);
		y = _mm_add_ps (y, y2);
		/* update the sign */
		y = _mm_xor_ps (y, sign_bit);
		return y;
	}
	inline f32x4_x86 cos (const f32x4_x86& v) noexcept
	{
		// http://gruntthepeon.free.fr/ssemath/sse_mathfun.h
		static __m128 sign_mask = _mm_castsi128_ps (_mm_set1_epi32 (0x80000000)),
			quater_pi = _mm_set1_ps (4 / 3.14159265359f), onef = _mm_set1_ps (1), half = _mm_set1_ps (0.5f);
		static __m128i one = _mm_set1_epi32 (1), one_inv = _mm_set1_epi32 (~1), two = _mm_set1_epi32 (2), four = _mm_set1_epi32 (4);
		static __m128 minus_cephes_dp1 = _mm_set1_ps (-0.78515625f)
			, minus_cephes_dp2 = _mm_set1_ps ((float)-2.4187564849853515625e-4)
			, minus_cephes_dp3 = _mm_set1_ps ((float)-3.77489497744594108e-8);
		static __m128 sincof_p0 = _mm_set1_ps ((float)-1.9515295891e-4)
			, sincof_p1 = _mm_set1_ps ((float)8.3321608736e-3)
			, sincof_p2 = _mm_set1_ps ((float)-1.6666654611e-1);
		static __m128 coscof_p0 = _mm_set1_ps ((float)2.443315711809948e-005)
			, coscof_p1 = _mm_set1_ps ((float)-1.388731625493765e-003)
			, coscof_p2 = _mm_set1_ps ((float)4.166664568298827e-002);

		__m128 xmm1, xmm2 = _mm_setzero_ps (), xmm3, sign_bit, y;
		__m128i emm0, emm2;

		sign_bit = v;
		/* take the absolute value */
		__m128 x = abs (v);
		/* extract the sign bit (upper one) */
		sign_bit = _mm_and_ps (sign_bit, sign_mask);

		/* scale by 4/Pi */
		y = _mm_mul_ps (x, quater_pi);

		/* store the integer part of y in mm0 */
		emm2 = _mm_cvttps_epi32 (y);
		/* j=(j+1) & (~1) (see the cephes sources) */
		emm2 = _mm_add_epi32 (emm2, one);
		emm2 = _mm_and_si128 (emm2, one_inv);
		y = _mm_cvtepi32_ps (emm2);

		emm2 = _mm_sub_epi32 (emm2, two);

		/* get the swap sign flag */
		emm0 = _mm_andnot_si128 (emm2, four);
		emm0 = _mm_slli_epi32 (emm0, 29);
		/* get the polynom selection mask */
		emm2 = _mm_and_si128 (emm2, two);
		emm2 = _mm_cmpeq_epi32 (emm2, _mm_setzero_si128 ());

		/*__m128 */sign_bit = _mm_castsi128_ps (emm0);
		__m128 poly_mask = _mm_castsi128_ps (emm2);

		/* The magic pass: "Extended precision modular arithmetic"
	 x = ((x - y * DP1) - y * DP2) - y * DP3; */
		xmm1 = minus_cephes_dp1;
		xmm2 = minus_cephes_dp2;
		xmm3 = minus_cephes_dp3;
		xmm1 = _mm_mul_ps (y, xmm1);
		xmm2 = _mm_mul_ps (y, xmm2);
		xmm3 = _mm_mul_ps (y, xmm3);
		x = _mm_add_ps (x, xmm1);
		x = _mm_add_ps (x, xmm2);
		x = _mm_add_ps (x, xmm3);

		/* Evaluate the first polynom  (0 <= x <= Pi/4) */
		y = coscof_p0;
		__m128 z = _mm_mul_ps (x, x);

		y = _mm_mul_ps (y, z);
		y = _mm_add_ps (y, coscof_p1);
		y = _mm_mul_ps (y, z);
		y = _mm_add_ps (y, coscof_p2);
		y = _mm_mul_ps (y, z);
		y = _mm_mul_ps (y, z);
		__m128 tmp = _mm_mul_ps (z, half);
		y = _mm_sub_ps (y, tmp);
		y = _mm_add_ps (y, onef);

		/* Evaluate the second polynom  (Pi/4 <= x <= 0) */
		__m128 y2 = sincof_p0;
		y2 = _mm_mul_ps (y2, z);
		y2 = _mm_add_ps (y2, sincof_p1);
		y2 = _mm_mul_ps (y2, z);
		y2 = _mm_add_ps (y2, sincof_p2);
		y2 = _mm_mul_ps (y2, z);
		y2 = _mm_mul_ps (y2, x);
		y2 = _mm_add_ps (y2, x);

		/* select the correct result from the two polynoms */
		xmm3 = poly_mask;
		y2 = _mm_and_ps (xmm3, y2); //, xmm3);
		y = _mm_andnot_ps (xmm3, y);
		y = _mm_add_ps (y, y2);
		/* update the sign */
		y = _mm_xor_ps (y, sign_bit);

		return y;
	}
	inline f32x4_x86 sincos (const f32x4_x86& v, f32x4_x86* osin, f32x4_x86* ocos) noexcept
	{
		// http://gruntthepeon.free.fr/ssemath/sse_mathfun.h
		static __m128 sign_mask = _mm_castsi128_ps (_mm_set1_epi32 (0x80000000)), sign_mask_inv = _mm_castsi128_ps (_mm_set1_epi32 (0x7fffffff)),
			quater_pi = _mm_set1_ps (4 / 3.14159265359f), onef = _mm_set1_ps (1), half = _mm_set1_ps (0.5f);
		static __m128i one = _mm_set1_epi32 (1), one_inv = _mm_set1_epi32 (~1), two = _mm_set1_epi32 (2), four = _mm_set1_epi32 (4);
		static __m128 minus_cephes_dp1 = _mm_set1_ps (-0.78515625f)
			, minus_cephes_dp2 = _mm_set1_ps ((float)-2.4187564849853515625e-4)
			, minus_cephes_dp3 = _mm_set1_ps ((float)-3.77489497744594108e-8);
		static __m128 sincof_p0 = _mm_set1_ps ((float)-1.9515295891e-4)
			, sincof_p1 = _mm_set1_ps ((float)8.3321608736e-3)
			, sincof_p2 = _mm_set1_ps ((float)-1.6666654611e-1);
		static __m128 coscof_p0 = _mm_set1_ps ((float)2.443315711809948e-005)
			, coscof_p1 = _mm_set1_ps ((float)-1.388731625493765e-003)
			, coscof_p2 = _mm_set1_ps ((float)4.166664568298827e-002);

		__m128 xmm1, xmm2 = _mm_setzero_ps (), xmm3, sign_bit_sin, y;
		__m128i emm0, emm2, emm4;

		sign_bit_sin = v;
		/* take the absolute value */
		__m128 x = _mm_and_ps (x, sign_mask_inv);
		/* extract the sign bit (upper one) */
		sign_bit_sin = _mm_and_ps (sign_bit_sin, sign_mask);

		/* scale by 4/Pi */
		y = _mm_mul_ps (x, quater_pi);

		/* store the integer part of y in emm2 */
		emm2 = _mm_cvttps_epi32 (y);

		/* j=(j+1) & (~1) (see the cephes sources) */
		emm2 = _mm_add_epi32 (emm2, one);
		emm2 = _mm_and_si128 (emm2, one_inv);
		y = _mm_cvtepi32_ps (emm2);

		emm4 = emm2;

		/* get the swap sign flag for the sine */
		emm0 = _mm_and_si128 (emm2, four);
		emm0 = _mm_slli_epi32 (emm0, 29);
		__m128 swap_sign_bit_sin = _mm_castsi128_ps (emm0);

		/* get the polynom selection mask for the sine*/
		emm2 = _mm_and_si128 (emm2, two);
		emm2 = _mm_cmpeq_epi32 (emm2, _mm_setzero_si128 ());
		__m128 poly_mask = _mm_castsi128_ps (emm2);

		/* The magic pass: "Extended precision modular arithmetic"
		 x = ((x - y * DP1) - y * DP2) - y * DP3; */
		xmm1 = minus_cephes_dp1;
		xmm2 = minus_cephes_dp2;
		xmm3 = minus_cephes_dp3;
		xmm1 = _mm_mul_ps (y, xmm1);
		xmm2 = _mm_mul_ps (y, xmm2);
		xmm3 = _mm_mul_ps (y, xmm3);
		x = _mm_add_ps (x, xmm1);
		x = _mm_add_ps (x, xmm2);
		x = _mm_add_ps (x, xmm3);

		emm4 = _mm_sub_epi32 (emm4, two);
		emm4 = _mm_andnot_si128 (emm4, four);
		emm4 = _mm_slli_epi32 (emm4, 29);
		__m128 sign_bit_cos = _mm_castsi128_ps (emm4);

		sign_bit_sin = _mm_xor_ps (sign_bit_sin, swap_sign_bit_sin);

		/* Evaluate the first polynom  (0 <= x <= Pi/4) */
		__m128 z = _mm_mul_ps (x, x);
		y = coscof_p0;

		y = _mm_mul_ps (y, z);
		y = _mm_add_ps (y, coscof_p1);
		y = _mm_mul_ps (y, z);
		y = _mm_add_ps (y, coscof_p2);
		y = _mm_mul_ps (y, z);
		y = _mm_mul_ps (y, z);
		__m128 tmp = _mm_mul_ps (z, half);
		y = _mm_sub_ps (y, tmp);
		y = _mm_add_ps (y, onef);

		/* Evaluate the second polynom  (Pi/4 <= x <= 0) */
		__m128 y2 = sincof_p0;
		y2 = _mm_mul_ps (y2, z);
		y2 = _mm_add_ps (y2, sincof_p1);
		y2 = _mm_mul_ps (y2, z);
		y2 = _mm_add_ps (y2, sincof_p2);
		y2 = _mm_mul_ps (y2, z);
		y2 = _mm_mul_ps (y2, x);
		y2 = _mm_add_ps (y2, x);

		/* select the correct result from the two polynoms */
		xmm3 = poly_mask;
		__m128 ysin2 = _mm_and_ps (xmm3, y2);
		__m128 ysin1 = _mm_andnot_ps (xmm3, y);
		y2 = _mm_sub_ps (y2, ysin2);
		y = _mm_sub_ps (y, ysin1);

		xmm1 = _mm_add_ps (ysin1, ysin2);
		xmm2 = _mm_add_ps (y, y2);

		/* update the sign */
		*osin = _mm_xor_ps (xmm1, sign_bit_sin);
		*ocos = _mm_xor_ps (xmm2, sign_bit_cos);
	}
	inline f32x4_x86 tan (const f32x4_x86& v) noexcept { f32x4_x86 s, c; sincos (v, &s, &c); return divide (s, c); }
	inline f32x4_x86 asin (const f32x4_x86& v) noexcept
	{
		// NEED TO CONVERT TO SIMD OPERATIONS
		return f32x4_x86 (asinf (v.x ()), asinf (v.y ()), asinf (v.z ()), asinf (v.w ()));
	}
	inline f32x4_x86 acos (const f32x4_x86& v) noexcept
	{
		// NEED TO CONVERT TO SIMD OPERATIONS
		return f32x4_x86 (acosf (v.x ()), acosf (v.y ()), acosf (v.z ()), acosf (v.w ()));
	}
	inline f32x4_x86 atan (const f32x4_x86& v) noexcept
	{
		// NEED TO CONVERT TO SIMD OPERATIONS
		return f32x4_x86 (atanf (v.x ()), atanf (v.y ()), atanf (v.z ()), atanf (v.w ()));
	}
	inline f32x4_x86 atan2 (const f32x4_x86& vy, const f32x4_x86& vx) noexcept
	{
		// NEED TO CONVERT TO SIMD OPERATIONS
		return f32x4_x86 (atan2f (vy.x (), vx.x ()), atan2f (vy.y (), vx.y ()), atan2f (vy.z (), vx.z ()), atan2f (vy.w (), vx.w ()));
	}

	inline f32x4_x86 maximumf (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_max_ps (v1, v2); }
	inline f32x4_x86 minimumf (const f32x4_x86& v1, const f32x4_x86& v2) noexcept { return _mm_min_ps (v1, v2); }

	inline float lengthsq2df (const f32x4_x86& v) noexcept { return dot2df (v, v); }
	inline float lengthsq3df (const f32x4_x86& v) noexcept { return dot3df (v, v); }
	inline float lengthsq4df (const f32x4_x86& v) noexcept { return dot4df (v, v); }

	inline f32x4_x86 lengthsq2d (const f32x4_x86& v) noexcept { return dot2d (v, v); }
	inline f32x4_x86 lengthsq3d (const f32x4_x86& v) noexcept { return dot3d (v, v); }
	inline f32x4_x86 lengthsq4d (const f32x4_x86& v) noexcept { return dot4d (v, v); }

	inline f32x4_x86 length2d (const f32x4_x86& v) noexcept { return sqrt (lengthsq2d (v)); }
	inline f32x4_x86 length3d (const f32x4_x86& v) noexcept { return sqrt (lengthsq3d (v)); }
	inline f32x4_x86 length4d (const f32x4_x86& v) noexcept { return sqrt (lengthsq4d (v)); }

	inline float length2df (const f32x4_x86& v) noexcept { return length2d (v).x (); }
	inline float length3df (const f32x4_x86& v) noexcept { return length3d (v).x (); }
	inline float length4df (const f32x4_x86& v) noexcept { return length4d (v).x (); }

	inline f32x4_x86 normalize2d (const f32x4_x86& v) noexcept { divide (v, length2d (v)); }
	inline f32x4_x86 normalize3d (const f32x4_x86& v) noexcept { divide (v, length3d (v)); }
	inline f32x4_x86 normalize4d (const f32x4_x86& v) noexcept { divide (v, length4d (v)); }

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// 32-bit Floating Point 4*4 Matrix Operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline f32x4x4_x86 add (const f32x4x4_x86& m1, const f32x4x4_x86& m2) noexcept
	{
		return f32x4x4_x86 (
			add (m1.column1 (), m2.column1 ()),
			add (m1.column2 (), m2.column2 ()),
			add (m1.column3 (), m2.column3 ()),
			add (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_x86 subtract (const f32x4x4_x86& m1, const f32x4x4_x86& m2) noexcept
	{
		return f32x4x4_x86 (
			subtract (m1.column1 (), m2.column1 ()),
			subtract (m1.column2 (), m2.column2 ()),
			subtract (m1.column3 (), m2.column3 ()),
			subtract (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_x86 negate (const f32x4x4_x86& m) noexcept
	{
		return f32x4x4_x86 (
			negate (m.column1 ()),
			negate (m.column2 ()),
			negate (m.column3 ()),
			negate (m.column4 ())
		);
	}
	inline f32x4x4_x86 multiply (const f32x4x4_x86& m1, const f32x4x4_x86& m2) noexcept
	{
		return f32x4x4_x86 (
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
	inline f32x4x4_x86 multiply (const f32x4x4_x86& m, float s) noexcept
	{
		return f32x4x4_x86 (
			multiply (m.column1 (), s),
			multiply (m.column2 (), s),
			multiply (m.column3 (), s),
			multiply (m.column4 (), s)
		);
	}
	inline f32x4x4_x86 multiply (float s, const f32x4x4_x86& m) noexcept { return multiply (m, s); }
	inline f32x4x4_x86 divide (const f32x4x4_x86& m1, const f32x4x4_x86& m2) noexcept
	{
		return f32x4x4_x86 (
			divide (m1.column1 (), m2.column1 ()),
			divide (m1.column2 (), m2.column2 ()),
			divide (m1.column3 (), m2.column3 ()),
			divide (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_x86 divide (const f32x4x4_x86& m, float s) noexcept
	{
		return f32x4x4_x86 (
			divide (m.column1 (), s),
			divide (m.column2 (), s),
			divide (m.column3 (), s),
			divide (m.column4 (), s)
		);
	}

	inline f32x4x4_x86 fma (const f32x4x4_x86& mm1, const f32x4x4_x86& mm2, const f32x4x4_x86& am) noexcept { return add (multiply (mm1, mm2), am); }
	inline f32x4x4_x86 fms (const f32x4x4_x86& mm1, const f32x4x4_x86& mm2, const f32x4x4_x86& sm) noexcept { return subtract (multiply (mm1, mm2), sm); }
	inline f32x4x4_x86 fnms (const f32x4x4_x86& mm1, const f32x4x4_x86& mm2, const f32x4x4_x86& sm) noexcept { return subtract (sm, multiply (mm1, mm2)); }

	inline f32x4x4_x86 and (const f32x4x4_x86& m1, const f32x4x4_x86& m2) noexcept
	{
		return f32x4x4_x86 (
			and (m1.column1 (), m2.column1 ()),
			and (m1.column2 (), m2.column2 ()),
			and (m1.column3 (), m2.column3 ()),
			and (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_x86 or (const f32x4x4_x86 & m1, const f32x4x4_x86 & m2) noexcept
	{
		return f32x4x4_x86 (
			or (m1.column1 (), m2.column1 ()),
			or (m1.column2 (), m2.column2 ()),
			or (m1.column3 (), m2.column3 ()),
			or (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_x86 xor (const f32x4x4_x86& m1, const f32x4x4_x86& m2) noexcept
	{
		return f32x4x4_x86 (
			xor (m1.column1 (), m2.column1 ()),
			xor (m1.column2 (), m2.column2 ()),
			xor (m1.column3 (), m2.column3 ()),
			xor (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_x86 not (const f32x4x4_x86& m) noexcept
	{
		return f32x4x4_x86 (
			not (m.column1 ()),
			not (m.column2 ()),
			not (m.column3 ()),
			not (m.column4 ())
		);
	}

	inline f32x4x4_x86 operator+ (const f32x4x4_x86& v1, const f32x4x4_x86& v2) noexcept { return add (v1, v2); }
	inline f32x4x4_x86 operator- (const f32x4x4_x86& v1, const f32x4x4_x86& v2) noexcept { return subtract (v1, v2); }
	inline f32x4x4_x86 operator- (const f32x4x4_x86& v) noexcept { return negate (v); }
	inline f32x4x4_x86 operator* (const f32x4x4_x86& v1, const f32x4x4_x86& v2) noexcept { return multiply (v1, v2); }
	inline f32x4x4_x86 operator* (const f32x4x4_x86& v, float s) noexcept { return multiply (v, s); }
	inline f32x4x4_x86 operator* (float s, const f32x4x4_x86& v) noexcept { return multiply (s, v); }
	inline f32x4x4_x86 operator/ (const f32x4x4_x86& v1, const f32x4x4_x86& v2) noexcept { return divide (v1, v2); }
	inline f32x4x4_x86 operator/ (const f32x4x4_x86& v, float s) noexcept { return divide (v, s); }
	inline f32x4x4_x86 operator& (const f32x4x4_x86& v1, const f32x4x4_x86& v2) noexcept { return and (v1, v2); }
	inline f32x4x4_x86 operator| (const f32x4x4_x86& v1, const f32x4x4_x86& v2) noexcept { return or (v1, v2); }
	inline f32x4x4_x86 operator^ (const f32x4x4_x86& v1, const f32x4x4_x86& v2) noexcept { return xor (v1, v2); }
	inline f32x4x4_x86 operator~ (const f32x4x4_x86& v) noexcept { return not (v); }

	inline f32x4x4_x86 transpose (const f32x4x4_x86& m) noexcept
	{
		f32x4_x86 temp0 = shuffle<0, 1, 0, 1> (m.column1 (), m.column2 ());
		f32x4_x86 temp2 = shuffle<2, 3, 2, 3> (m.column1 (), m.column2 ());
		f32x4_x86 temp1 = shuffle<0, 1, 0, 1> (m.column3 (), m.column4 ());
		f32x4_x86 temp3 = shuffle<2, 3, 2, 3> (m.column3 (), m.column4 ());

		f32x4x4_x86 ret;
		ret._column1 = shuffle<0, 2, 0, 2> (temp0, temp1);
		ret._column2 = shuffle<1, 3, 1, 3> (temp0, temp1);
		ret._column3 = shuffle<0, 2, 0, 2> (temp2, temp3);
		ret._column4 = shuffle<1, 3, 1, 3> (temp2, temp3);

		return ret;
	}
	inline float determinant (const f32x4x4_x86& m) noexcept
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
	inline f32x4x4_x86 invert (const f32x4x4_x86& m, float& det) noexcept
	{
		static f32x4_x86 one = f32x4_x86 (1);

		f32x4x4_x86 transposed = transpose (m);
		f32x4_x86 v00 = permute<0, 0, 1, 1> (transposed.column3 ());
		f32x4_x86 v10 = permute<2, 3, 2, 3> (transposed.column4 ());
		f32x4_x86 v01 = permute<0, 0, 1, 1> (transposed.column1 ());
		f32x4_x86 v11 = permute<2, 3, 2, 3> (transposed.column2 ());
		f32x4_x86 v02 = shuffle<0, 2, 0, 2> (transposed.column3 (), transposed.column1 ());
		f32x4_x86 v12 = shuffle<1, 3, 1, 3> (transposed.column4 (), transposed.column2 ());

		f32x4_x86 d0 = v00 * v10;
		f32x4_x86 d1 = v01 * v11;
		f32x4_x86 d2 = v02 * v12;

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

		f32x4_x86 v13 = shuffle<1, 3, 3, 3> (d1, d2);
		v02 = permute<1, 2, 0, 1> (transposed.column4 ());
		v12 = shuffle<2, 0, 3, 0> (v13, d1);
		f32x4_x86 v03 = permute<2, 0, 1, 0> (transposed.column3 ());
		v13 = shuffle<1, 2, 1, 2> (v13, d1);

		f32x4_x86 c0 = v00 * v10;
		f32x4_x86 c2 = v01 * v11;
		f32x4_x86 c4 = v02 * v12;
		f32x4_x86 c6 = v03 * v13;

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
		f32x4_x86 c1 = c0 - v00;
		c0 = c0 + v00;
		f32x4_x86 c3 = c2 + v01;
		c2 = c2 - v01;
		f32x4_x86 c5 = c4 - v02;
		c4 = c4 + v02;
		f32x4_x86 c7 = c6 + v03;
		c6 = c6 - v03;

		c0 = shuffle<0, 2, 1, 3> (c0, c1);
		c2 = shuffle<0, 2, 1, 3> (c2, c3);
		c4 = shuffle<0, 2, 1, 3> (c4, c5);
		c6 = shuffle<0, 2, 1, 3> (c6, c7);
		c0 = permute<0, 2, 1, 3> (c0);
		c2 = permute<0, 2, 1, 3> (c2);
		c4 = permute<0, 2, 1, 3> (c4);
		c6 = permute<0, 2, 1, 3> (c6);

		f32x4_x86 temp = f32x4_x86 (dot4d (c0, transposed.column1 ()));
		det = temp.x ();
		temp = one / temp;

		f32x4x4_x86 ret;
		ret._column1 = c0 * temp;
		ret._column2 = c2 * temp;
		ret._column3 = c4 * temp;
		ret._column4 = c6 * temp;

		return ret;
	}

	inline f32x4x4_x86 equals (const f32x4x4_x86& m1, const f32x4x4_x86& m2) noexcept
	{
		return f32x4x4_x86 (
			equals (m1.column1 (), m2.column1 ()),
			equals (m1.column2 (), m2.column2 ()),
			equals (m1.column3 (), m2.column3 ()),
			equals (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_x86 not_equals (const f32x4x4_x86& m1, const f32x4x4_x86& m2) noexcept
	{
		return f32x4x4_x86 (
			not_equals (m1.column1 (), m2.column1 ()),
			not_equals (m1.column2 (), m2.column2 ()),
			not_equals (m1.column3 (), m2.column3 ()),
			not_equals (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_x86 lesser (const f32x4x4_x86& m1, const f32x4x4_x86& m2) noexcept
	{
		return f32x4x4_x86 (
			lesser (m1.column1 (), m2.column1 ()),
			lesser (m1.column2 (), m2.column2 ()),
			lesser (m1.column3 (), m2.column3 ()),
			lesser (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_x86 lesser_equals (const f32x4x4_x86& m1, const f32x4x4_x86& m2) noexcept
	{
		return f32x4x4_x86 (
			lesser_equals (m1.column1 (), m2.column1 ()),
			lesser_equals (m1.column2 (), m2.column2 ()),
			lesser_equals (m1.column3 (), m2.column3 ()),
			lesser_equals (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_x86 greater (const f32x4x4_x86& m1, const f32x4x4_x86& m2) noexcept
	{
		return f32x4x4_x86 (
			greater (m1.column1 (), m2.column1 ()),
			greater (m1.column2 (), m2.column2 ()),
			greater (m1.column3 (), m2.column3 ()),
			greater (m1.column4 (), m2.column4 ())
		);
	}
	inline f32x4x4_x86 greater_equals (const f32x4x4_x86& m1, const f32x4x4_x86& m2) noexcept
	{
		return f32x4x4_x86 (
			greater_equals (m1.column1 (), m2.column1 ()),
			greater_equals (m1.column2 (), m2.column2 ()),
			greater_equals (m1.column3 (), m2.column3 ()),
			greater_equals (m1.column4 (), m2.column4 ())
		);
	}

	inline bool operator== (const f32x4x4_x86& v1, const f32x4x4_x86& v2) noexcept { return 0xffff == movemask (equals (v1, v2)); }
	inline bool operator!= (const f32x4x4_x86& v1, const f32x4x4_x86& v2) noexcept { return 0xffff == movemask (not_equals (v1, v2)); }
	inline bool operator< (const f32x4x4_x86& v1, const f32x4x4_x86& v2) noexcept { return 0xffff == movemask (lesser (v1, v2)); }
	inline bool operator<= (const f32x4x4_x86& v1, const f32x4x4_x86& v2) noexcept { return 0xffff == movemask (lesser_equals (v1, v2)); }
	inline bool operator> (const f32x4x4_x86& v1, const f32x4x4_x86& v2) noexcept { return 0xffff == movemask (greater (v1, v2)); }
	inline bool operator>= (const f32x4x4_x86& v1, const f32x4x4_x86& v2) noexcept { return 0xffff == movemask (greater_equals (v1, v2)); }
}

#endif

#endif