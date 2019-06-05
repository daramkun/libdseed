#ifndef __DMATH_SIMD_INL__
#define __DMATH_SIMD_INL__

#include <dseed.h>

#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
#	include <xmmintrin.h>									// SSE
#	include <emmintrin.h>									// SSE 2
#	include <pmmintrin.h>									// SSE 3
#	include <smmintrin.h>									// SSE 4.x
#	include <nmmintrin.h>									// SSE 4.x
#	include <immintrin.h>									// AVX
#elif ARCH_ARM && !defined ( NO_INTRINSIC )
#	include <arm_neon.h>
#elif ARCH_ARM64 && !defined ( NO_INTRINSIC )
#	include <arm64_neon.h>
#endif

#include <cmath>
#include <cassert>

namespace dseed
{
	////////////////////////////////////////////////////////////////////////////////////////
	//
	// SIMD-operation Type definitions
	//
	////////////////////////////////////////////////////////////////////////////////////////

	// Shuffle Index
	enum shuffle_t : uint32_t {
		shuffle_ax1 = 0, shuffle_ay1 = 1, shuffle_az1 = 2, shuffle_aw1 = 3,
		shuffle_bx2 = 0, shuffle_by2 = 1, shuffle_bz2 = 2, shuffle_bw2 = 3,
		shuffle_ax2 = 4, shuffle_ay2 = 5, shuffle_az2 = 6, shuffle_aw2 = 7,
		shuffle_bx1 = 4, shuffle_by1 = 5, shuffle_bz1 = 6, shuffle_bw1 = 7,
	};
	// Permute Index
	enum permute_t : uint32_t {
		permute_x = 0, permute_y = 1, permute_z = 2, permute_w = 3,
	};

	// 32-bit Floating Point Vector-4
	struct vectorf
	{
		vectorf () = default;
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		__m128 v;
		inline vectorf (const float* vector) : v (_mm_load_ps (vector)) { }
		inline vectorf (float x, float y, float z, float w) : v (_mm_set_ps (w, z, y, x)) {}
		inline vectorf (const __m128& vector) : v (vector) { }
		inline vectorf (const __m128i& vector) : v (_mm_cvtepi32_ps (vector)) { }
		inline vectorf (float s) : v (_mm_set1_ps (s)) { }
		inline operator __m128 () const { return v; }
		inline void store (float* vector) const { _mm_store_ps (vector, v); }
		template<uint32_t x, uint32_t y, uint32_t z, uint32_t w>
		inline vectorf shuffle (const vectorf& v2) const { return _mm_shuffle_ps (v, v2.v, _MM_SHUFFLE (w, z, y, x)); }
		template<uint32_t x, uint32_t y, uint32_t z, uint32_t w>
		inline vectorf permute () const
		{
#if !defined ( NO_AVX )
			return _mm_permute_ps (v, _MM_SHUFFLE (w, z, y, x));
#else
			return shuffle (v, x, y, z, w);
#endif
		}
		inline vectorf splat_x () const { return permute<0, 0, 0, 0> (); }
		inline vectorf splat_y () const { return permute<1, 1, 1, 1> (); }
		inline vectorf splat_z () const { return permute<2, 2, 2, 2> (); }
		inline vectorf splat_w () const { return permute<3, 3, 3, 3> (); }
		inline float x () const { return _mm_cvtss_f32 (splat_x ()); }
		inline float y () const { return _mm_cvtss_f32 (splat_y ()); }
		inline float z () const { return _mm_cvtss_f32 (splat_z ()); }
		inline float w () const { return _mm_cvtss_f32 (splat_w ()); }
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
	private:
		template<uint32_t x, uint32_t y>
		inline float32x2_t __getpart (const float32x4_t& v2) const
		{
			float32x2_t ret;
			if (x <= 0 && x >= 3) ret = vmov_n_f32 (vgetq_lane_f32 (v, x));
			else if (x <= 4 && x >= 7) ret = vmov_n_f32 (vgetq_lane_f32 (v2, 4 - x));
			else assert (true);

			if (y <= 0 && y >= 3) ret = vset_lane_f32 (vgetq_lane_f32 (v2, y), ret, 1);
			else if (y <= 4 && y >= 7) ret = vset_lane_f32 (vgetq_lane_f32 (v, 4 - y), ret, 1);
			else assert (true);

			return ret;
		}
		template<> inline float32x2_t __getpart<0, 1> (const float32x4_t & v2) const { return vget_low_f32 (v); }
		template<> inline float32x2_t __getpart<2, 3> (const float32x4_t & v2) const { return vget_high_f32 (v); }
		template<> inline float32x2_t __getpart<4, 5> (const float32x4_t & v2) const { return vget_low_f32 (v2); }
		template<> inline float32x2_t __getpart<6, 7> (const float32x4_t & v2) const { return vget_high_f32 (v2); }
		template<> inline float32x2_t __getpart<1, 0> (const float32x4_t & v2) const { return vrev64_f32 (vget_low_f32 (v)); }
		template<> inline float32x2_t __getpart<3, 2> (const float32x4_t & v2) const { return vrev64_f32 (vget_high_f32 (v)); }
		template<> inline float32x2_t __getpart<5, 4> (const float32x4_t & v2) const { return vrev64_f32 (vget_low_f32 (v2)); }
		template<> inline float32x2_t __getpart<7, 6> (const float32x4_t & v2) const { return vrev64_f32 (vget_high_f32 (v2)); }
		template<> inline float32x2_t __getpart<0, 0> (const float32x4_t & v2) const { return vdup_lane_f32 (vget_low_f32 (v), 0); }
		template<> inline float32x2_t __getpart<1, 1> (const float32x4_t & v2) const { return vdup_lane_f32 (vget_low_f32 (v), 1); }
		template<> inline float32x2_t __getpart<2, 2> (const float32x4_t & v2) const { return vdup_lane_f32 (vget_high_f32 (v), 0); }
		template<> inline float32x2_t __getpart<3, 3> (const float32x4_t & v2) const { return vdup_lane_f32 (vget_high_f32 (v), 1); }
		template<> inline float32x2_t __getpart<4, 4> (const float32x4_t & v2) const { return vdup_lane_f32 (vget_low_f32 (v2), 0); }
		template<> inline float32x2_t __getpart<5, 5> (const float32x4_t & v2) const { return vdup_lane_f32 (vget_low_f32 (v2), 1); }
		template<> inline float32x2_t __getpart<6, 6> (const float32x4_t & v2) const { return vdup_lane_f32 (vget_high_f32 (v2), 0); }
		template<> inline float32x2_t __getpart<7, 7> (const float32x4_t & v2) const { return vdup_lane_f32 (vget_high_f32 (v2), 1); }

	public:
		float32x4_t v;
		inline vectorf (const float* vector) : v (vld1q_f32 (vector)) { }
		inline vectorf (float x, float y, float z, float w) : vectorf (float{ x, y, z, w }) { }
		inline vectorf (const float32x4_t& vector) : v (vector) { }
		inline vectorf (const int32x4_t& vector) : v (vcvtq_s32_f32 (vector)) { }
		inline vectorf (float s) : v (vmovq_n_f32 (s)) { }
		inline operator float32x4_t () const { return v; }
		inline void store (float* vector) const { vst1q_f32 (vector, v); }
		template<uint32_t x, uint32_t y, uint32_t z, uint32_t w>
		inline vectorf shuffle (const vectorf& v2) const
		{
			float32x2_t a = this->__getpart<x, y> (v2.v);
			float32x2_t b = v2.__getpart<z, w> (v);
			return vcombine_f32 (a, b);
		}
		template<uint32_t x, uint32_t y, uint32_t z, uint32_t w>
		inline vectorf permute () const { return shuffle<x, y, z, w> (*this); }
		inline vectorf splat_x () const { vdupq_lane_f32 (vget_low_f32 (v), 0); }
		inline vectorf splat_y () const { vdupq_lane_f32 (vget_low_f32 (v), 1); }
		inline vectorf splat_z () const { vdupq_lane_f32 (vget_high_f32 (v), 0); }
		inline vectorf splat_w () const { vdupq_lane_f32 (vget_high_f32 (v), 1); }
		inline float x () const { return vgetq_lane_f32 (v, 0); }
		inline float y () const { return vgetq_lane_f32 (v, 1); }
		inline float z () const { return vgetq_lane_f32 (v, 2); }
		inline float w () const { return vgetq_lane_f32 (v, 3); }
#else
		float v[4];
		inline vectorf (const float* vector) { memcpy (v, vector, sizeof (float) * 4); }
		inline vectorf (float x, float y, float z, float w) : v ({ x, y, z, w }) { }
		inline vectorf (float s) : v ({ s, s, s, s }) { }
		inline operator float* () const { return v; }
		inline void store (float* vector) const { memcpy (vector, v, sizeof (float) * 4); }
		template<uint32_t x, uint32_t y, uint32_t z, uint32_t w>
		inline vectorf shuffle (const vectorf& v2) const
		{
			return vectorf ((x >= 0 && x <= 3) ? v[x] : v2.v[4 - x], (y >= 0 && y <= 3) ? v[y] : v2.v[4 - y],
				(z >= 0 && z <= 3) ? v2.v[z] : v[4 - z], (w >= 0 && w <= 3) ? v2.v[w] : v[4 - w]);
		}
		template<uint32_t x, uint32_t y, uint32_t z, uint32_t w>
		inline vectorf permute () const { return shuffle (*this, x, y, z, w); }
		inline vectorf splat_x () const { return vectorf (v[0]); }
		inline vectorf splat_y () const { return vectorf (v[1]); }
		inline vectorf splat_z () const { return vectorf (v[2]); }
		inline vectorf splat_w () const { return vectorf (v[3]); }
		inline float x () const { return v[0]; }
		inline float y () const { return v[1]; }
		inline float z () const { return v[2]; }
		inline float w () const { return v[3]; }
#endif
	};

	// 32-bit Integer Vector-4
	struct vectori
	{
		vectori () = default;
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		__m128i v;
		inline vectori (const int* vector) : v (_mm_load_si128 ((const __m128i*) vector)) { }
		inline vectori (int x, int y, int z, int w) : v (_mm_set_epi32 (w, z, y, x)) { }
		inline vectori (const __m128i& vector) : v (vector) { }
		inline vectori (const __m128& vector) : v (_mm_cvtps_epi32 (vector)) { }
		inline vectori (int i) : v (_mm_set1_epi32 (i)) { }
		inline operator __m128i () const { return v; }
		inline void store (int* vector) const { _mm_store_si128 ((__m128i*)vector, v); }
		template<int x, int y, int z, int w>
		inline vectori shuffle (const vectori& v2) const
		{
			return _mm_castps_si128 (_mm_shuffle_ps (
				_mm_castsi128_ps (v), _mm_castsi128_ps (v2.v), _MM_SHUFFLE (w, z, y, x)
			));
		}
		template<int s1, int s2, int s3, int s4, int s5, int s6, int s7, int s8>
		inline vectori permute () const
		{
			__m128i shuffle = _mm_set_epi16 (s8, s7, s6, s5, s4, s3, s2, s1);
			return _mm_shuffle_epi8 (v, mask);
		}
		template<int s1, int s2, int s3, int s4, int s5, int s6, int s7, int s8,
			int s9, int s10, int s11, int s12, int s13, int s14, int s15, int s16>
			inline vectori permute () const
		{
			__m128i shuffle = _mm_set_epi8 (s16, s15, s14, s13, s12, s11, s10, s9, s8, s7, s6, s5, s4, s3, s2, s1);
			return _mm_shuffle_epi8 (v, mask);
		}
		template<int x, int y, int z, int w>
		inline vectori permute () const { return _mm_shuffle_epi32 (v, _MM_SHUFFLE (w, z, y, x)); }
		inline vectori splat_x () const { return permute<0, 0, 0, 0> (); }
		inline vectori splat_y () const { return permute<1, 1, 1, 1> (); }
		inline vectori splat_z () const { return permute<2, 2, 2, 2> (); }
		inline vectori splat_w () const { return permute<3, 3, 3, 3> (); }
		inline int x () const { return _mm_cvtss_i32 (_mm_castsi128_ps (splat_x ())); }
		inline int y () const { return _mm_cvtss_i32 (_mm_castsi128_ps (splat_y ())); }
		inline int z () const { return _mm_cvtss_i32 (_mm_castsi128_ps (splat_z ())); }
		inline int w () const { return _mm_cvtss_i32 (_mm_castsi128_ps (splat_w ())); }
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
	private:
		template<int x, int y>
		inline int32x2_t __getpart (const float32x4_t& v2) const
		{
			int32x2_t ret;
			if (x <= 0 && x >= 3) ret = vmov_n_s32 (vgetq_lane_s32 (v, x));
			else if (x <= 4 && x >= 7) ret = vmov_n_s32 (vgetq_lane_s32 (v2, 4 - x));
			else assert (true);

			if (y <= 0 && y >= 3) ret = vset_lane_s32 (vgetq_lane_s32 (v2, y), ret, 1);
			else if (y <= 4 && y >= 7) ret = vset_lane_s32 (vgetq_lane_s32 (v, 4 - y), ret, 1);
			else assert (true);

			return ret;
		}
		template<> inline int32x2_t __getpart<0, 1> (const int32x4_t & v2) const { return vget_low_s32 (v); }
		template<> inline int32x2_t __getpart<2, 3> (const int32x4_t & v2) const { return vget_high_s32 (v); }
		template<> inline int32x2_t __getpart<4, 5> (const int32x4_t & v2) const { return vget_low_s32 (v2); }
		template<> inline int32x2_t __getpart<6, 7> (const int32x4_t & v2) const { return vget_high_s32 (v2); }
		template<> inline int32x2_t __getpart<1, 0> (const int32x4_t & v2) const { return vrev64_s32 (vget_low_s32 (v)); }
		template<> inline int32x2_t __getpart<3, 2> (const int32x4_t & v2) const { return vrev64_s32 (vget_high_s32 (v)); }
		template<> inline int32x2_t __getpart<5, 4> (const int32x4_t & v2) const { return vrev64_s32 (vget_low_s32 (v2)); }
		template<> inline int32x2_t __getpart<7, 6> (const int32x4_t & v2) const { return vrev64_s32 (vget_high_s32 (v2)); }
		template<> inline int32x2_t __getpart<0, 0> (const int32x4_t & v2) const { return vdup_lane_s32 (vget_low_s32 (v), 0); }
		template<> inline int32x2_t __getpart<1, 1> (const int32x4_t & v2) const { return vdup_lane_s32 (vget_low_s32 (v), 1); }
		template<> inline int32x2_t __getpart<2, 2> (const int32x4_t & v2) const { return vdup_lane_s32 (vget_high_s32 (v), 0); }
		template<> inline int32x2_t __getpart<3, 3> (const int32x4_t & v2) const { return vdup_lane_s32 (vget_high_s32 (v), 1); }
		template<> inline int32x2_t __getpart<4, 4> (const int32x4_t & v2) const { return vdup_lane_s32 (vget_low_s32 (v2), 0); }
		template<> inline int32x2_t __getpart<5, 5> (const int32x4_t & v2) const { return vdup_lane_s32 (vget_low_s32 (v2), 1); }
		template<> inline int32x2_t __getpart<6, 6> (const int32x4_t & v2) const { return vdup_lane_s32 (vget_high_s32 (v2), 0); }
		template<> inline int32x2_t __getpart<7, 7> (const int32x4_t & v2) const { return vdup_lane_s32 (vget_high_s32 (v2), 1); }

	public:
		int32x4_t v;
		inline vectori (const int* vector) : v (vld1q_s32 (vector)) { }
		inline vectori (int x, int y, int z, int w) : v (vld1q_s32 (int{ x, y, z, w })) { }
		inline vectori (const int32x4_t& vector) : v (vector) { }
		inline vectori (const float32x4_t& vector) : v (vcvtq_f32_s32 (vector)) { }
		inline vectori (int i) : v (vmovq_n_s32 (i)) { }
		inline operator int32x4_t () const { return v; }
		inline void store (int* vector) const { vst1q_s32 (vector, v); }
		template<int x, int y, int z, int w>
		inline vectori shuffle (const vectori& v2) const
		{
			int32x2_t a = this->__getpart<x, y> (v2.v);
			int32x2_t b = v2.__getpart<z, w> (v);
			return vcombine_s32 (a, b);
		}
		template<int x, int y, int z, int w>
		inline vectori permute () const { return shuffle<x, y, z, w> (*this); }
		inline vectori splat_x () const { vdupq_lane_s32 (vget_low_s32 (v), 0); }
		inline vectori splat_y () const { vdupq_lane_s32 (vget_low_s32 (v), 1); }
		inline vectori splat_z () const { vdupq_lane_s32 (vget_high_s32 (v), 0); }
		inline vectori splat_w () const { vdupq_lane_s32 (vget_high_s32 (v), 1); }
		inline float x () const { return vgetq_lane_s32 (v, 0); }
		inline float y () const { return vgetq_lane_s32 (v, 1); }
		inline float z () const { return vgetq_lane_s32 (v, 2); }
		inline float w () const { return vgetq_lane_s32 (v, 3); }
#else
		int v[4];
		inline vectori (const int* vector) { memcpy (v, vector, sizeof (int) * 4); }
		inline vectori (const float* vector)
		{
			v[0] = (int)vector[0];
			v[1] = (int)vector[1];
			v[2] = (int)vector[2];
			v[3] = (int)vector[3];
		}
		inline vectori (int x, int y, int z, int w) : v ({ x, y, z, w }) { }
		inline vectori (int i) : v ({ i, i, i, i }) { }
		inline operator int* () const { return v; }
		inline void store (int* vector) { memcpy (vector, v, sizeof (int) * 4); }
		template<int x, int y, int z, int w>
		inline vectori shuffle (const vectori& v2) const
		{
			return vectori ((x >= 0 && x <= 3) ? v[x] : v2.v[4 - x], (y >= 0 && y <= 3) ? v[y] : v2.v[4 - y],
				(z >= 0 && z <= 3) ? v2.v[z] : v[4 - z], (w >= 0 && w <= 3) ? v2.v[w] : v[4 - w]);
		}
		template<int x, int y, int z, int w>
		inline vectori permute () const { return shuffle (*this, x, y, z, w); }
		inline vectori splat_x () const { return vectori (v[0]); }
		inline vectori splat_y () const { return vectori (v[1]); }
		inline vectori splat_z () const { return vectori (v[2]); }
		inline vectori splat_w () const { return vectori (v[3]); }
		inline int x () const { return v[0]; }
		inline int y () const { return v[1]; }
		inline int z () const { return v[2]; }
		inline int w () const { return v[3]; }
#endif
	};

	// 32-bit Floating Point 4x4 Matrix
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

	// 32-bit Integer 4x4 Matrix
	struct matrixi
	{
	public:
		vectori column1, column2, column3, column4;

	public:
		matrixi () = default;
		inline matrixi (const vectori& c1, const vectori& c2, const vectori& c3, const vectori& c4)
			: column1 (c1), column2 (c2), column3 (c3), column4 (c4)
		{ }
	};

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Cast Operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline vectorf convert_vector (const vectori& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtepi32_ps (v);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vcvtq_f32_s32 (v);
#else
		return vectorf ((float)v.x (), (float)v.y (), (float)v.z (), (float)v.w ());
#endif
	}
	inline vectori convert_vector (const vectorf v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cvtps_epi32 (v);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
#	if ( ARCH_ARM64 )
		return vcvtnq_f32_s32 (v);
#	elif ( ARCH_ARM )
		float32x4_t half = vbslq_f32 (vdupq_n_u32 (0x80000000), v, vdupq_n_f32 (0.5f));
		int32x4_t r_trunc = vcvtq_s32_f32 (v);
		int32x4_t r_even = vbicq_s32 (vaddq_s32 (r_trunc, vshrq_n_u32 (vnegq_s32 (r_trunc), 31)), vdupq_n_s32 (1));
		return vbslq_s32 (vceqq_f32 (vsubq_f32 (v, vcvtq_f32_s32 (r_trunc)), half),
			r_even, vcvtq_s32_f32 (vaddq_f32 (v, half)));
#	endif
#else
		return vectori ((int)v.x (), (int)v.y (), (int)v.z (), (int)v.w ());
#endif
	}
	inline vectorf reinterpret_vector (const vectori& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_castsi128_ps (v);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vreinterpretq_f32_s32 (v);
#else
		return *reinterpret_cast<const vectorf*>(&v);
#endif
	}
	inline vectori reinterpret_vector (const vectorf v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_castps_si128 (v);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vreinterpretq_s32_f32 (v);
#else
		return *reinterpret_cast<const vectori*>(&v);
#endif
	}
	inline int movemask_vector (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_movemask_ps (v);
#else
		uint32_t& ia = *reinterpret_cast<uint32_t*> (&v);
		return (ia[0] >> 31) | ((ia[1] >> 30) & 2) | ((ia[2] >> 29) & 4) | ((ia[3] >> 28) & 8);
#endif
	}
	inline int movemask_vector (const vectori& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_movemask_epi8 (v);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		uint8x16_t input = v;
		static const int8_t __attribute__ ((aligned (16))) xr[8] = { -7, -6, -5, -4, -3, -2, -1, 0 };
		uint8x8_t mask_and = vdup_n_u8 (0x80);
		int8x8_t mask_shift = vld1_s8 (xr);

		uint8x8_t lo = vget_low_u8 (input);
		uint8x8_t hi = vget_high_u8 (input);

		lo = vand_u8 (lo, mask_and);
		lo = vshl_u8 (lo, mask_shift);

		hi = vand_u8 (hi, mask_and);
		hi = vshl_u8 (hi, mask_shift);

		lo = vpadd_u8 (lo, lo);
		lo = vpadd_u8 (lo, lo);
		lo = vpadd_u8 (lo, lo);

		hi = vpadd_u8 (hi, hi);
		hi = vpadd_u8 (hi, hi);
		hi = vpadd_u8 (hi, hi);

		return ((hi[0] << 8) | (lo[0] & 0xFF));
#else
		uint8_t& ia = *reinterpret_cast<uint8_t*> (&v);
		return (ia[0] >> 31)
			| ((ia[1] >> 30) & 2) | ((ia[2] >> 29) & 4) | ((ia[3] >> 28) & 8)
			| ((ia[4] >> 27) & 16) | ((ia[5] >> 26) & 32) | ((ia[6] >> 25) & 64)
			| ((ia[7] >> 24) & 128) | ((ia[8] >> 23) & 256) | ((ia[9] >> 22) & 512)
			| ((ia[10] >> 21) & 1024) | ((ia[11] >> 20) & 2048) | ((ia[12] >> 19) & 4096)
			| ((ia[13] >> 18) & 8192) | ((ia[14] >> 17) & 16384) | ((ia[15] >> 16) & 32768);
#endif
	}
	inline matrixf convert_matrix (const matrixi& m) noexcept
	{
		return matrixf (
			convert_vector (m.column1),
			convert_vector (m.column2),
			convert_vector (m.column3),
			convert_vector (m.column4)
		);
	}
	inline matrixi convert_matrix (const matrixf& m) noexcept
	{
		return matrixi (
			convert_vector (m.column1),
			convert_vector (m.column2),
			convert_vector (m.column3),
			convert_vector (m.column4)
		);
	}
	inline matrixf reinterpret_matrix (const matrixi& m) noexcept
	{
		return matrixf (
			reinterpret_vector (m.column1),
			reinterpret_vector (m.column2),
			reinterpret_vector (m.column3),
			reinterpret_vector (m.column4)
		);
	}
	inline matrixi reinterpret_matrix (const matrixf& m) noexcept
	{
		return matrixi (
			reinterpret_vector (m.column1),
			reinterpret_vector (m.column2),
			reinterpret_vector (m.column3),
			reinterpret_vector (m.column4)
		);
	}
	inline int movemask_matrix (const matrixf& m) noexcept
	{
		return (movemask_vector (m.column1) & 0x000F)
			+ ((movemask_vector (m.column2) << 8) & 0x00F0)
			+ ((movemask_vector (m.column3) << 16) & 0x0F00)
			+ ((movemask_vector (m.column4) << 24) & 0xF000);
	}
	inline long long int movemask_matrix (const matrixi& m) noexcept
	{
		return ((long long)movemask_vector (m.column1) & 0x000000000000FFFF)
			+ (((long long)movemask_vector (m.column2) << 16) & 0x00000000FFFF0000)
			+ (((long long)movemask_vector (m.column3) << 32) & 0x0000FFFF00000000)
			+ (((long long)movemask_vector (m.column4) << 48) & 0xFFFF000000000000);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Vector Type Arithmetic operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline vectorf addvf (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_add_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vaddq_f32 (v1, v2);
#else
		return vectorf (v1.x () + v2.x (), v1.y () + v2.y (), v1.z () + v2.z (), v1.w () + v2.w ());
#endif
	}
	inline vectorf subtractvf (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_sub_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vsubq_f32 (v1, v2);
#else
		return vectorf (v1.x () - v2.x (), v1.y () - v2.y (), v1.z () - v2.z (), v1.w () - v2.w ());
#endif
	}
	inline vectorf negatevf (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_sub_ps (_mm_load1_ps (0), v);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vsubq_f32 (vmovq_n_f32 (0), v);
#else
		return vectorf (-v.x (), -v.y (), -v.z (), -v.w ());
#endif
	}
	inline vectorf multiplyvf (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_mul_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vmulq_f32 (v1, v2);
#else
		return vectorf (v1.x () * v2.x (), v1.y () * v2.y (), v1.z () * v2.z (), v1.w () * v2.w ());
#endif
	}
	inline vectorf multiplyvf (const vectorf& v, float s) noexcept { return multiplyvf (v, vectorf (s)); }
	inline vectorf multiplyvf (float s, const vectorf& v) noexcept { return multiplyvf (vectorf (s), v); }
	inline vectorf dividevf (const vectorf& v1, const vectorf& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_div_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		float32x4_t x = vrecpeq_f32 (v2);
		for (int i = 0; i < 2; i++)
			x = vmulq_f32 (vrecpsq_f32 (v2, x), x);
		return vmulq_f32 (v1, x);
#else
		return vectorf (v1.x () / v2.x (), v1.y () / v2.y (), v1.z () / v2.z (), v1.w () / v2.w ());
#endif
	}
	inline vectorf dividevf (const vectorf& v, float s) noexcept { return dividevf (v, vectorf (s)); }
	inline vectorf andvf (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_and_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vandq_s32 (v1, v2);
#else
		return vectori (v1.x () & v2.x (), v1.y () & v2.y (), v1.z () & v2.z (), v1.w () & v2.w ());
#endif
	}
	inline vectorf orvf (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_or_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vorrq_s32 (v1, v2);
#else
		return vectori (v1.x () | v2.x (), v1.y () | v2.y (), v1.z () | v2.z (), v1.w () | v2.w ());
#endif
	}
	inline vectorf xorvf (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_xor_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return veorq_s32 (v1, v2);
#else
		return vectori (v1.x () ^ v2.x (), v1.y () ^ v2.y (), v1.z () ^ v2.z (), v1.w () ^ v2.w ());
#endif
	}
	inline vectorf notvf (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_xor_ps (v, reinterpret_vector (vectori (0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff)));
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vmvnq_s32 (v);
#else
		return vectori (~v.x (), ~v.y (), ~v.z (), ~v.w ());
#endif
	}
	inline vectorf equalsvf (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cmpeq_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vceqq_f32 (v1, v2);
#else
		return reinterpret_vector (vectori (
			v1.x () == v2.x () ? -1 : 0,
			v1.y () == v2.y () ? -1 : 0,
			v1.z () == v2.z () ? -1 : 0,
			v1.w () == v2.w () ? -1 : 0));
#endif
	}
	inline vectorf not_equalsvf (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cmpneq_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vmvnq_u32 (vceqq_f32 (v1, v2));
#else
		return reinterpret_vector (vectori (
			v1.x () != v2.x () ? -1 : 0,
			v1.y () != v2.y () ? -1 : 0,
			v1.z () != v2.z () ? -1 : 0,
			v1.w () != v2.w () ? -1 : 0));
#endif
	}
	inline vectorf lesservf (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cmplt_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vcltq_f32 (v1, v2);
#else
		return reinterpret_vector (vectori (
			v1.x () < v2.x () ? -1 : 0,
			v1.y () < v2.y () ? -1 : 0,
			v1.z () < v2.z () ? -1 : 0,
			v1.w () < v2.w () ? -1 : 0));
#endif
	}
	inline vectorf lesser_equalsvf (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_movemask_ps (_mm_cmple_ps (v1, v2)) == 0xF;
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vcleq_f32 (v1, v2);
#else
		return reinterpret_vector (vectori (
			v1.x () <= v2.x () ? -1 : 0,
			v1.y () <= v2.y () ? -1 : 0,
			v1.z () <= v2.z () ? -1 : 0,
			v1.w () <= v2.w () ? -1 : 0));
#endif
	}
	inline vectorf greatervf (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cmpgt_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vcgtq_f32 (v1, v2);
#else
		return reinterpret_vector (vectori (
			v1.x () > v2.x () ? -1 : 0,
			v1.y () > v2.y () ? -1 : 0,
			v1.z () > v2.z () ? -1 : 0,
			v1.w () > v2.w () ? -1 : 0));
#endif
	}
	inline vectorf greater_equalsvf (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cmpge_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vcgeq_f32 (v1, v2);
#else
		return reinterpret_vector (vectori (
			v1.x () >= v2.x () ? -1 : 0,
			v1.y () >= v2.y () ? -1 : 0,
			v1.z () >= v2.z () ? -1 : 0,
			v1.w () >= v2.w () ? -1 : 0));
#endif
	}

	inline vectorf fmavf (const vectorf& mv1, const vectorf& mv2, const vectorf& av) noexcept
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
		return addvf (multiplyvf (mv1, mv2), av);
#endif
	}
	inline vectorf fmsvf (const vectorf& mv1, const vectorf& mv2, const vectorf& sv) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC ) && !defined ( NO_FMA3 )
		return _mm_fmsub_ps (mv1, mv2, sv);
#else
		return subtractvf (multiplyvf (mv1, mv2), sv);
#endif
	}
	inline vectorf fnmsvf (const vectorf& mv1, const vectorf& mv2, const vectorf& sv) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC ) && !defined ( NO_FMA3 )
		return _mm_fnmsub_ps (mv1, mv2, sv);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
#	if ( ARCH_ARM64 )
		return vfmsq_f32 (sv, mv1, mv2);
#	else
		return vmlsq_f32 (sv, mv1, mv2);
#	endif
#else
		return subtractvf (sv, multiplyvf (mv1, mv2));
#endif
	}

	inline vectorf operator+ (const vectorf& v1, const vectorf& v2) noexcept { return addvf (v1, v2); }
	inline vectorf operator- (const vectorf& v1, const vectorf& v2) noexcept { return subtractvf (v1, v2); }
	inline vectorf operator- (const vectorf& v) noexcept { return negatevf (v); }
	inline vectorf operator* (const vectorf& v1, const vectorf& v2) noexcept { return multiplyvf (v1, v2); }
	inline vectorf operator* (const vectorf& v, float s) noexcept { return multiplyvf (v, s); }
	inline vectorf operator* (float s, const vectorf& v) noexcept { return multiplyvf (s, v); }
	inline vectorf operator/ (const vectorf& v1, const vectorf& v2) noexcept { return dividevf (v1, v2); }
	inline vectorf operator/ (const vectorf& v, float s) noexcept { return dividevf (v, s); }
	inline vectorf operator& (const vectorf& v1, const vectorf& v2) noexcept { return andvf (v1, v2); }
	inline vectorf operator| (const vectorf& v1, const vectorf& v2) noexcept { return orvf (v1, v2); }
	inline vectorf operator^ (const vectorf& v1, const vectorf& v2) noexcept { return xorvf (v1, v2); }
	inline vectorf operator~ (const vectorf& v) noexcept { return notvf (v); }
	inline bool operator== (const vectorf& v1, const vectorf& v2) noexcept { return movemask_vector (equalsvf (v1, v2)) == 0xF; }
	inline bool operator!= (const vectorf& v1, const vectorf& v2) noexcept { return movemask_vector (not_equalsvf (v1, v2)) != 0; }
	inline bool operator< (const vectorf& v1, const vectorf& v2) noexcept { return movemask_vector (lesservf (v1, v2)) == 0xF; }
	inline bool operator<= (const vectorf& v1, const vectorf& v2) noexcept { return movemask_vector (lesser_equalsvf (v1, v2)) == 0xF; }
	inline bool operator> (const vectorf& v1, const vectorf& v2) noexcept { return movemask_vector (greatervf (v1, v2)) == 0xF; }
	inline bool operator>= (const vectorf& v1, const vectorf& v2) noexcept { return movemask_vector (greater_equalsvf (v1, v2)) == 0xF; }
	
	inline vectori addvi (const vectori& v1, const vectori& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_add_epi32 (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vaddq_s32 (v1, v2);
#else
		return vectori (v1.x () + v2.x (), v1.y () + v2.y (), v1.z () + v2.z (), v1.w () + v2.w ());
#endif
	}
	inline vectori subtractvi (const vectori& v1, const vectori& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_sub_epi32 (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vsubq_s32 (v1, v2);
#else
		return vectori (v1.x () - v2.x (), v1.y () - v2.y (), v1.z () - v2.z (), v1.w () - v2.w ());
#endif
	}
	inline vectori negatevi (const vectori& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_sub_epi32 (_mm_set1_epi32 (0), v);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vsubq_s32 (vmovq_n_f32 (0), v);
#else
		return vectori (-v.x (), -v.y (), -v.z (), -v.w ());
#endif
	}
	inline vectori multiplyvi (const vectori& v1, const vectori& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_mul_epi32 (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vmulq_s32 (v1, v2);
#else
		return vectori (v1.x () * v2.x (), v1.y () * v2.y (), v1.z () * v2.z (), v1.w () * v2.w ());
#endif
	}
	inline vectori multiplyvi (const vectori& v, int s) noexcept { return multiplyvi (v, vectori (s)); }
	inline vectori multiplyvi (int s, const vectori& v) noexcept { return multiplyvi (vectori (s), v); }
	inline vectori dividevi (const vectori& v1, const vectori& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_div_epi32 (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		float32x4_t x = vrecpeq_s32 (v2);
		for (int i = 0; i < 2; i++)
			x = vmulq_s32 (vrecpsq_s32 (v2, x), x);
		return vmulq_s32 (v1, x);
#else
		return vectori (v1.x () / v2.x (), v1.y () / v2.y (), v1.z () / v2.z (), v1.w () / v2.w ());
#endif
	}
	inline vectori dividevi (const vectori& v, int s) noexcept { return dividevi (v, vectori (s)); }
	inline vectori left_shiftvi (const vectori& v1, int s)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_slli_epi32 (v1, s);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vshlq_n_s32 (v1, s);
#else
		return vectori (v1.x () << s, v1.y () << s, v1.z () << s, v1.w () << s);
#endif
	}
	inline vectori right_shiftvi (const vectori& v1, int s)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_srai_epi32 (v1, s);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vshrq_n_s32 (v1, s);
#else
		return vectori (v1.x () >> s, v1.y () >> s, v1.z () >> s, v1.w () >> s);
#endif
	}
	inline vectori andvi (const vectori& v1, const vectori& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_and_si128 (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vandq_s32 (v1, x);
#else
		return vectori (v1.x () & v2.x (), v1.y () & v2.y (), v1.z () & v2.z (), v1.w () & v2.w ());
#endif
	}
	inline vectori orvi (const vectori& v1, const vectori& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_or_si128 (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vorrq_s32 (v1, x);
#else
		return vectori (v1.x () | v2.x (), v1.y () | v2.y (), v1.z () | v2.z (), v1.w () | v2.w ());
#endif
	}
	inline vectori xorvi (const vectori& v1, const vectori& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_xor_si128 (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return veorq_s32 (v1, x);
#else
		return vectori (v1.x () ^ v2.x (), v1.y () ^ v2.y (), v1.z () ^ v2.z (), v1.w () ^ v2.w ());
#endif
	}
	inline vectori notvi (const vectori& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_xor_si128 (v, vectori (0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff));
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vmvnq_s32 (v);
#else
		return vectori (~v.x (), ~v.y (), ~v.z (), ~v.w ());
#endif
	}
	inline vectori equalsvi (const vectori& v1, const vectori& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cmpeq_epi32 (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vceqq_s32 (v1, v2);
#else
		return vectori (
			v1.x () == v2.x () ? -1 : 0,
			v1.y () == v2.y () ? -1 : 0,
			v1.z () == v2.z () ? -1 : 0,
			v1.w () == v2.w () ? -1 : 0 );
#endif
	}
	inline vectori not_equalsvi (const vectori& v1, const vectori& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cmpeq_epi32 (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vmvnq_u32 (vceqq_s32 (v1, v2));
#else
		return vectori (
			v1.x () != v2.x () ? -1 : 0,
			v1.y () != v2.y () ? -1 : 0,
			v1.z () != v2.z () ? -1 : 0,
			v1.w () != v2.w () ? -1 : 0 );
#endif
	}
	inline vectori lesservi (const vectori& v1, const vectori& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cmplt_epi32 (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vcltq_s32 (v1, v2);
#else
		return vectori (
			v1.x () < v2.x () ? -1 : 0,
			v1.y () < v2.y () ? -1 : 0,
			v1.z () < v2.z () ? -1 : 0,
			v1.w () < v2.w () ? -1 : 0 );
#endif
	}
	inline vectori lesser_equalsvi (const vectori& v1, const vectori& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return orvi (lesservi (v1, v2), equalsvi (v1, v2));
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vcleq_s32 (v1, v2);
#else
		return vectori (
			v1.x () <= v2.x () ? -1 : 0,
			v1.y () <= v2.y () ? -1 : 0,
			v1.z () <= v2.z () ? -1 : 0,
			v1.w () <= v2.w () ? -1 : 0 );
#endif
	}
	inline vectori greatervi (const vectori& v1, const vectori& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_cmpgt_epi32 (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vcgtq_s32 (v1, v2);
#else
		return vectori (
			v1.x () > v2.x () ? -1 : 0,
			v1.y () > v2.y () ? -1 : 0,
			v1.z () > v2.z () ? -1 : 0,
			v1.w () > v2.w () ? -1 : 0 );
#endif
	}
	inline vectori greater_equalsvi (const vectori& v1, const vectori& v2)
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return orvi (greatervi (v1, v2), equalsvi (v1, v2));
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vcgeq_s32 (v1, v2);
#else
		return vectori (
			v1.x () >= v2.x () ? -1 : 0,
			v1.y () >= v2.y () ? -1 : 0,
			v1.z () >= v2.z () ? -1 : 0,
			v1.w () >= v2.w () ? -1 : 0 );
#endif
	}

	inline vectori fmavi (const vectori& mv1, const vectori& mv2, const vectori& av) noexcept
	{
#if ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vmlal_s32 (mv1, mv2, av);
#else
		return addvi (multiplyvi (mv1, mv2), av);
#endif
	}
	inline vectori fmsvi (const vectori& mv1, const vectori& mv2, const vectori& sv) noexcept
	{
		return subtractvi (multiplyvi (mv1, mv2), sv);
	}
	inline vectori fnmsvi (const vectori& mv1, const vectori& mv2, const vectori& sv) noexcept
	{
#if ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vmls_s32 (sv, mv1, mv2);
#else
		return subtractvi (sv, multiplyvi (mv1, mv2));
#endif
	}

	inline vectori operator+ (const vectori& v1, const vectori& v2) noexcept { return addvi (v1, v2); }
	inline vectori operator- (const vectori& v1, const vectori& v2) noexcept { return subtractvi (v1, v2); }
	inline vectori operator- (const vectori& v) noexcept { return negatevi (v); }
	inline vectori operator* (const vectori& v1, const vectori& v2) noexcept { return multiplyvi (v1, v2); }
	inline vectori operator* (const vectori& v, int s) noexcept { return multiplyvi (v, s); }
	inline vectori operator* (int s, const vectori& v) noexcept { return multiplyvi (s, v); }
	inline vectori operator/ (const vectori& v1, const vectori& v2) noexcept { return dividevi (v1, v2); }
	inline vectori operator/ (const vectori& v, int s) noexcept { return dividevi (v, s); }
	inline vectori operator& (const vectori& v1, const vectori& v2) noexcept { return andvi (v1, v2); }
	inline vectori operator| (const vectori& v1, const vectori& v2) noexcept { return orvi (v1, v2); }
	inline vectori operator^ (const vectori& v1, const vectori& v2) noexcept { return xorvi (v1, v2); }
	inline vectori operator~ (const vectori& v) noexcept { return notvi (v); }
	inline bool operator== (const vectori& v1, const vectori& v2) noexcept { return movemask_vector (equalsvi (v1, v2)) == 0xFFFF; }
	inline bool operator!= (const vectori& v1, const vectori& v2) noexcept { return movemask_vector (not_equalsvi (v1, v2)) != 0; }
	inline bool operator< (const vectori& v1, const vectori& v2) noexcept { return movemask_vector (lesservi (v1, v2)) == 0xFFFF; }
	inline bool operator<= (const vectori& v1, const vectori& v2) noexcept { return movemask_vector (lesser_equalsvi (v1, v2)) == 0xFFFF; }
	inline bool operator> (const vectori& v1, const vectori& v2) noexcept { return movemask_vector (greatervi (v1, v2)) == 0xFFFF; }
	inline bool operator>= (const vectori& v1, const vectori& v2) noexcept { return movemask_vector (greater_equalsvi (v1, v2)) == 0xFFFF; }

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Vector type Vector operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline vectorf dot2 (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_dp_ps (v1, v2, 0x3f);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		float32x2_t temp = vmul_f32 (vget_low_f32 (v1), vget_low_f32 (v2));
		temp = vadd_f32 (temp, temp);
		return temp;
#else
		return vectorf ((v1.x () * v2.x ()) + (v1.y () * v2.y ()));
#endif
	}
	inline vectorf dot2 (const vectorf& v, float s) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_dp_ps (v, _mm_set1_ps (s), 0x3f);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return dot2 (v, float4 (s));
#else
		return vectorf ((v.x () * s) + (v.y () * s));
#endif
	}
	inline vectorf dot3 (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_dp_ps (v1, v2, 0x7f);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		float32x4_t temp = vmulq_f32 (v1, v2);
		float32x2_t tv1 = vpadd_f32 (vget_low_f32 (temp), vget_low_f32 (temp));
		float32x2_t tv2 = vdup_lane_f32 (vget_high_f32 (temp), 0);
		return tv1 = vadd_f32 (tv1, tv2);
#else
		return vectorf ((v1.x () * v2.x ()) + (v1.y () * v2.y ()) + (v1.z () * v2.z ()));
#endif
	}
	inline vectorf dot3 (const vectorf& v, float s) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_dp_ps (v, _mm_set1_ps (s), 0x7f);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return dot3 (v, float3 (s));
#else
		return vectorf ((v.x () * s) + (v.y () * s) + (v.z () * s));
#endif
	}
	inline vectorf dot4 (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_dp_ps (v1, v2, 0xff);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		float32x4_t temp = vmulq_f32 (v1, v2);
		float32x2_t tv1 = vget_low_f32 (temp);
		float32x2_t tv2 = vget_high_f32 (temp);
		tv1 = vadd_f32 (tv1, tv2);
		tv1 = vpadd_f32 (tv1, tv1);
		return tv1;
#else
		return vectorf ((v1.x () * v2.x ()) + (v1.y () * v2.y ()) + (v1.z () * v2.z ()) + (v1.w () * v2.w ()));
#endif
	}

	inline vectorf cross2 (const vectorf& v1, const vectorf& v2) noexcept
	{
		vectorf ret = v2.permute<1, 0, 1, 0> ();
		ret = ret * v1;
		vectorf temp = ret.splat_y ();
		ret = ret - temp;
		ret = ret.permute<0, 0, 3, 3> ();
		return ret;
	}
	inline vectorf cross3 (const vectorf& v1, const vectorf& v2) noexcept
	{
		vectorf temp1 = v1.permute<1, 2, 0, 3> ();
		vectorf temp2 = v2.permute<2, 0, 1, 3> ();
		vectorf ret = temp1 * temp2;
		temp1 = temp1.permute<1, 2, 0, 3> ();
		temp2 = temp2.permute<2, 0, 1, 3> ();
		temp1 = temp1 * temp2;
		return ret - temp1;
	}
	inline vectorf cross4 (const vectorf& v1, const vectorf& v2, const vectorf& v3) noexcept
	{
		vectorf ret = v2.permute <2, 3, 1, 3> ();
		vectorf temp3 = v3.permute <3, 2, 3, 1> ();
		ret = ret * temp3;

		vectorf temp2 = v2.permute <3, 2, 3, 1> ();
		temp3 = temp3.permute <1, 0, 3, 1> ();
		temp2 = temp2 * temp3;
		ret = ret - temp2;

		vectorf temp1 = v1.permute<1, 0, 0, 0> ();
		ret = ret * temp1;

		temp2 = v2.permute<1, 3, 0, 2> ();
		temp3 = v3.permute<3, 0, 3, 0> ();
		temp3 = temp3 * temp2;

		temp2 = temp2.permute<1, 2, 1, 2> ();
		temp1 = v3.permute<1, 3, 0, 2> ();
		temp2 = temp2 * temp1;
		temp3 = temp3 - temp2;

		temp1 = v1.permute <2, 2, 1, 1> ();
		temp1 = temp1 * temp3;
		ret = ret - temp1;

		temp2 = v2.permute<1, 2, 0, 1> ();
		temp3 = v3.permute<2, 0, 1, 0> ();
		temp3 = temp3 * temp2;

		temp2 = temp2.permute<1, 2, 0, 2> ();
		temp1 = v3.permute<1, 2, 0, 1> ();
		temp1 = temp1 * temp2;
		temp3 = temp3 - temp1;

		temp1 = v1.permute<3, 3, 3, 2> ();
		temp3 = temp3 * temp1;
		ret = ret + temp3;

		return ret;
	}

	inline vectorf length_squared2 (const vectorf& v) noexcept
	{
		return dot2 (v, v);
	}
	inline vectorf length_squared3 (const vectorf& v) noexcept
	{
		return dot3 (v, v);
	}
	inline vectorf length_squared4 (const vectorf& v) noexcept
	{
		return dot4 (v, v);
	}
	inline vectorf length2 (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_sqrt_ps (_mm_dp_ps (v, v, 0x3f));
#else
		return sqrtvf (length_squared2 (v));
#endif
	}
	inline vectorf length3 (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_sqrt_ps (_mm_dp_ps (v, v, 0x7f));
#else
		return sqrtvf (length_squared3 (v));
#endif
	}
	inline vectorf length4 (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_sqrt_ps (_mm_dp_ps (v, v, 0xff));
#else
		return sqrtvf (length_squared4 (v));
#endif
	}

	inline vectorf normalize2 (const vectorf& v) noexcept { return v / length2 (v); }
	inline vectorf normalize3 (const vectorf& v) noexcept { return v / length3 (v); }
	inline vectorf normalize4 (const vectorf& v) noexcept { return v / length4 (v); }
	inline vectorf normalize_plane (const vectorf& v) noexcept { return normalize3 (v); }

	inline vectorf is_unit2 (const vectorf& v) noexcept { return equalsvf (length_squared2 (v), vectorf (1)); }
	inline vectorf is_unit3 (const vectorf& v) noexcept { return equalsvf (length_squared3 (v), vectorf (1)); }
	inline vectorf is_unit4 (const vectorf& v) noexcept { return equalsvf (length_squared4 (v), vectorf (1)); }
	
	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Vector Type Utility operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	
	// Get Minimum value (max(v1, v2))
	inline vectorf minvf (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_min_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vminq_f32 (v1, v2);
#else
		return vectorf (minimum<float> (v1.x (), v2.x ()), minimum<float> (v1.y (), v2.y ()),
			minimum<float> (v1.z (), v2.z ()), minimum<float> (v1.w (), v2.w ()));
#endif
	}
	// Get Maximum value (max(v1, v2))
	inline vectorf maxvf (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_max_ps (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vmaxq_f32 (v1, v2);
#else
		return vectorf (maximum<float> (v1.x (), v2.x ()), maximum<float> (v1.y (), v2.y ()),
			maximum<float> (v1.z (), v2.z ()), maximum<float> (v1.w (), v2.w ()));
#endif
	}
	// Get Minimum value (max(v1, v2))
	inline vectori minvi (const vectori& v1, const vectori& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_min_epi32 (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vminq_f32 (v1, v2);
#else
		return float4 (minimum<float> (v1.x (), v2.x ()), minimum<float> (v1.y (), v2.y ()),
			minimum<float> (v1.z (), v2.z ()), minimum<float> (v1.w (), v2.w ()));
#endif
	}
	// Get Maximum value (max(v1, v2))
	inline vectori maxvi (const vectori& v1, const vectori& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_max_epi32 (v1, v2);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vmaxq_f32 (v1, v2);
#else
		return float4 (maximum<float> (v1.x (), v2.x ()), maximum<float> (v1.y (), v2.y ()),
			maximum<float> (v1.z (), v2.z ()), maximum<float> (v1.w (), v2.w ()));
#endif
	}
	// Get Squared vector (sqrt(v))
	inline vectorf sqrtvf (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_sqrt_ps (v);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vrecpeq_f32 (vrsqrteq_f32 (v));
#else
		return vectorf (sqrtf(v.x ()), sqrtf(v.y ()), sqrtf(v.z ()), sqrtf(v.w ()));
#endif
	}
	// Get Squared Reciprocal vector (1/sqrt(v))
	inline vectorf rsqrtvf (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_rsqrt_ps (v);
#elif ( ARCH_ARMSET ) && !defined ( NO_INTRINSIC )
		return vrsqrteq_f32 (v);
#else
		return dividevf (vectorf (1), sqrt (v));
#endif
	}
	// Get Power vector (v1^v2)
	inline vectorf powvf (const vectorf& v1, const vectorf& v2) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_pow_ps (v1, v2);
#else
		float v[4] = { powf (v1.x (), v2.x ()), powf (v1.y (), v2.y ()), powf (v1.z (), v2.z ()), powf (v1.w (), v2.w ()) };
		return vectorf (v);
#endif
	}
	// Get Reciprocal vector (1/v)
	inline vectorf rcpvf (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_rcp_ps (v);
#else
		return dividevf (vectorf (1), v);
#endif
	}
	// Get Rounded vector
	inline vectorf roundvf (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_round_ps (v, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
#else
		float v[4] = { roundf (v.x ()), roundf (v.y ()), roundf (v.z ()), roundf (v.w ()) };
		return vectorf (v);
#endif
	}
	// Get Floored vector
	inline vectorf floorvf (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_floor_ps (v);
#else
		float v[4] = { floorf (v.x ()), floorf (v.y ()), floorf (v.z ()), floorf (v.w ()) };
		return vectorf (v);
#endif
	}
	// Get Ceiled vector
	inline vectorf ceilvf (const vectorf& v) noexcept
	{
#if ( ARCH_X86SET ) && !defined ( NO_INTRINSIC )
		return _mm_ceil_ps (v);
#else
		float v[4] = { ceilf (v.x ()), ceilf (v.y ()), ceilf (v.z ()), ceilf (v.w ()) };
		return vectorf (v);
#endif
	}
	// Get Sine vector (sin(v))
	inline vectorf sinvf (const vectorf& v) noexcept
	{
		return vectorf (sinf (v.x ()), sinf (v.y ()), sinf (v.z ()), sinf (v.w ()));
	}
	inline vectorf sinhvf (const vectorf& v) noexcept
	{
		return vectorf (sinhf (v.x ()), sinhf (v.y ()), sinhf (v.z ()), sinhf (v.w ()));
	}
	// Get Cosine vector (cos(v))
	inline vectorf cosvf (const vectorf& v) noexcept
	{
		return vectorf (cosf (v.x ()), cosf (v.y ()), cosf (v.z ()), cosf (v.w ()));
	}
	inline vectorf coshvf (const vectorf& v) noexcept
	{
		return vectorf (coshf (v.x ()), coshf (v.y ()), coshf (v.z ()), coshf (v.w ()));
	}
	// Get Tangent vector (tan(v))
	inline vectorf tanvf (const vectorf& v) noexcept
	{
		return vectorf (tanf (v.x ()), tanf (v.y ()), tanf (v.z ()), tanf (v.w ()));
	}
	inline vectorf tanhvf (const vectorf& v) noexcept
	{
		return vectorf (tanhf (v.x ()), tanhf (v.y ()), tanhf (v.z ()), tanhf (v.w ()));
	}
	// Get Arc-Sine vector (asin(v))
	inline vectorf asinvf (const vectorf& v) noexcept
	{
		return vectorf (asinf (v.x ()), asinf (v.y ()), asinf (v.z ()), asinf (v.w ()));
	}
	inline vectorf asinhvf (const vectorf& v) noexcept
	{
		return vectorf (asinhf (v.x ()), asinhf (v.y ()), asinhf (v.z ()), asinhf (v.w ()));
	}
	// Get Arc-Cosine vector (acos(v))
	inline vectorf acosvf (const vectorf& v) noexcept
	{
		return vectorf (acosf (v.x ()), acosf (v.y ()), acosf (v.z ()), acosf (v.w ()));
	}
	inline vectorf acoshvf (const vectorf& v) noexcept
	{
		return vectorf (acoshf (v.x ()), acoshf (v.y ()), acoshf (v.z ()), acoshf (v.w ()));
	}
	// Get Arc-Tangent vector (atan(v))
	inline vectorf atanvf (const vectorf& v) noexcept
	{
		return vectorf (atanf (v.x ()), atanf (v.y ()), atanf (v.z ()), atanf (v.w ()));
	}
	inline vectorf atanhvf (const vectorf& v) noexcept
	{
		return vectorf (atanhf (v.x ()), atanhf (v.y ()), atanhf (v.z ()), atanhf (v.w ()));
	}
	inline vectorf atan2vf (const vectorf& y, const vectorf& x) noexcept
	{
		return vectorf (atan2f (y.x (), x.x ()), atan2f (y.y (), x.y ()),
			atan2f (y.z (), x.z ()), atan2f (y.w (), x.w ()));
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Quaternion operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline vectorf quaternion_add (const vectorf& q1, const vectorf& q2) noexcept
	{
		return addvf (q1, q2);
	}
	inline vectorf quaternion_subtract (const vectorf& q1, const vectorf& q2)noexcept
	{
		return subtractvf (q1, q2);
	}
	inline vectorf quaternion_multiply (const vectorf& q1, const vectorf& q2) noexcept
	{
		vectorf q2x = q2.splat_x ();
		vectorf q2y = q2.splat_y ();
		vectorf q2z = q2.splat_z ();
		vectorf q2w = q2.splat_w ();

		vectorf result = q2w * q1;
		q2x = (q2x * q1.permute<3, 2, 1, 0> ()) * vectorf (1, -1, 1, -1);
		q2y = (q2y * q1.permute<1, 0, 3, 2> ()) * vectorf (1, 1, -1, -1);
		q2z = (q2z * q1.permute<3, 2, 1, 0> ()) * vectorf (-1, 1, 1, -1);
		q2y = q2y + q2z;

		return result + q2x + q2y;
	}
	inline vectorf quaternion_divide (const vectorf& q1, const vectorf& q2) noexcept
	{
		return dividevf (q1, q2);
	}

	inline vectorf lengthq (const vectorf& q) noexcept
	{
		return length4 (q);
	}
	inline vectorf normalizeq (const vectorf& q) noexcept
	{
		return normalize4 (q);
	}
	inline vectorf conjugateq (const vectorf& q) noexcept
	{
		return multiplyvf (q, vectorf (-1, -1, -1, 1));
	}
	inline vectorf invertq (const vectorf& q) noexcept
	{
		vectorf length = length4 (q);
		vectorf conjugate = conjugateq (q);

		if (length.x () <= single_epsilon)
			return vectorf (0, 0, 0, 0);

		return dividevf (conjugate, length);
	}
	inline vectorf from_matrixq (const matrixf& m) noexcept
	{
		static const vectorf XMPMMP ( +1.0f, -1.0f, -1.0f, +1.0f );
		static const vectorf XMMPMP ( -1.0f, +1.0f, -1.0f, +1.0f );
		static const vectorf XMMMPP ( -1.0f, -1.0f, +1.0f, +1.0f );

		vectorf r0 = m.column1;
		vectorf r1 = m.column2;
		vectorf r2 = m.column3;

		vectorf r00 = r0.splat_x ();
		vectorf r11 = r1.splat_y ();
		vectorf r22 = r2.splat_z ();

		vectorf r11mr00 = r11 - r00;
		vectorf x2gey2 = lesser_equalsvf (r11mr00, vectorf (0, 0, 0, 0));

		vectorf r11pr00 = r11 + r00;
		vectorf z2gew2 = lesser_equalsvf (r11pr00, vectorf (0, 0, 0, 0));

		vectorf x2py2gez2pw2 = lesser_equalsvf (r22, vectorf (0, 0, 0, 0));

		vectorf t0 = XMPMMP * r00;
		vectorf t1 = XMMPMP * r11;
		vectorf t2 = XMMMPP * r22;

		vectorf x2y2z2w2 = t0 + t1;
		x2y2z2w2 = t2 + x2y2z2w2;
		x2y2z2w2 = x2y2z2w2 + vectorf (1, 1, 1, 1);

		t0 = r0.shuffle<1, 2, 2, 1> (r1);
		t1 = r1.shuffle<0, 0, 0, 1> (r2);
		t1 = t1.permute<0, 2, 3, 1> ();
		vectorf xyxzyz = t0 + t1;

		t0 = r2.shuffle<1, 0, 0, 0> (r1);
		t1 = r1.shuffle<2, 2, 2, 1> (r0);
		t1 = t1.permute<0, 2, 3, 1> ();
		vectorf xwywzw = t0 - t1;
		xwywzw = XMMPMP * xwywzw;

		t0 = x2y2z2w2.shuffle<0, 1, 0, 0> (xyxzyz);
		t1 = x2y2z2w2.shuffle<2, 3, 2, 0> (xwywzw);
		t2 = xyxzyz.shuffle<1, 2, 0, 1> (xwywzw);

		vectorf tensor0 = t0.shuffle<0, 2, 0, 2> (t2);
		vectorf tensor1 = t0.shuffle<2, 1, 1, 3> (t2);
		vectorf tensor2 = t2.shuffle<0, 1, 0, 2> (t1);
		vectorf tensor3 = t2.shuffle<2, 3, 2, 1> (t1);

		t0 = x2gey2 & tensor0;
		t1 = ~x2gey2 & tensor1;
		t0 = t0 | t1;
		t1 = z2gew2 & tensor2;
		t2 = ~z2gew2 & tensor3;
		t1 = t1 | t2;
		t0 = x2py2gez2pw2 & t0;
		t1 = ~x2py2gez2pw2 & t1;
		t2 = t0 | t1;

		t0 = length4 (t2);
		return t2 / t0;
	}
	inline vectorf from_yaw_pitch_rollq (float yaw, float pitch, float roll) noexcept
	{
		static vectorf sign = vectorf (1, -1, -1, 1);
		vectorf halfAngles = vectorf (pitch, yaw, roll, 0) * vectorf (0.5f, 0.5f, 0.5f, 0.5f);

		vectorf sinAngles = sinvf (halfAngles);
		vectorf cosAngles = cosvf (halfAngles);

		vectorf p0 = sinAngles.shuffle<shuffle_ax1, shuffle_bx1, shuffle_bx2, shuffle_bx2> (cosAngles);
		vectorf y0 = cosAngles.shuffle<shuffle_ax1, shuffle_bx1, shuffle_ax2, shuffle_ax2> (sinAngles);
		vectorf r0 = cosAngles.shuffle<shuffle_ax1, shuffle_ax1, shuffle_bx2, shuffle_ax2> (sinAngles);
		vectorf p1 = cosAngles.shuffle<shuffle_ax1, shuffle_bx1, shuffle_bx2, shuffle_bx2> (sinAngles);
		vectorf y1 = sinAngles.shuffle<shuffle_ax1, shuffle_bx1, shuffle_ax2, shuffle_ax2> (cosAngles);
		vectorf r1 = sinAngles.shuffle<shuffle_ax1, shuffle_ax1, shuffle_bx2, shuffle_ax2> (cosAngles);

		return (p0 * y0 * r0) + (p1 * y1 * r1);
	}
	inline matrixf to_matrixq (const vectorf& q) noexcept
	{
		static vectorf constant1110 = vectorf (1, 1, 1, 0);

		vectorf q0 = q + q;
		vectorf q1 = q * q0;

		vectorf v0 = q1.shuffle<1, 0, 4, 3> (constant1110);
		vectorf v1 = q1.shuffle<2, 2, 5, 3> (constant1110);
		vectorf r0 = constant1110 - v0;
		r0 = r0 - v1;

		v0 = q.permute<0, 0, 1, 3> ();
		v1 = q0.permute<2, 1, 2, 3> ();
		v0 = v0 * v1;

		v1 = q.splat_w ();
		vectorf v2 = q0.permute<1, 2, 0, 3> ();
		v1 = v1 * v2;

		vectorf r1 = v0 + v1;
		vectorf r2 = v0 - v1;

		v0 = r1.shuffle<1, 4, 1, 7> (r2);
		v1 = r1.shuffle<0, 6, 4, 2> (r2);

		matrixf ret;
		ret.column1 = r0.shuffle<0, 4, 1, 7> (v0);
		ret.column2 = r0.shuffle<2, 1, 3, 7> (v0);
		ret.column3 = r0.shuffle<4, 5, 6, 7> (v1);
		ret.column4 = vectorf ( 0, 0, 0, 1 );
		return ret;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Matrix type Arithmetic operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline matrixf addmf (const matrixf& m1, const matrixf& m2) noexcept
	{
		return matrixf (m1.column1 + m2.column1, m1.column2 + m2.column2, m1.column3 + m2.column3, m1.column4 + m2.column4);
	}
	inline matrixf subtractmf (const matrixf& m1, const matrixf& m2) noexcept
	{
		return matrixf (m1.column1 + m2.column1, m1.column2 + m2.column2, m1.column3 + m2.column3, m1.column4 + m2.column4);
	}
	inline matrixf negatemf (const matrixf& m) noexcept
	{
		return matrixf (-m.column1, -m.column2, -m.column3, -m.column4);
	}
	inline matrixf multiplymf (const matrixf& m1, const matrixf& m2) noexcept
	{
		return matrixf (
			fmavf (m1.column1.splat_x (), m2.column1,
				fmavf (m1.column1.splat_y (), m2.column2,
					fmavf (m1.column1.splat_z (), m2.column3,
						m1.column1.splat_w () * m2.column4))),
			fmavf (m1.column2.splat_x (), m2.column1,
				fmavf (m1.column2.splat_y (), m2.column2,
					fmavf (m1.column2.splat_z (), m2.column3,
						m1.column2.splat_w () * m2.column4))),
			fmavf (m1.column3.splat_x (), m2.column1,
				fmavf (m1.column3.splat_y (), m2.column2,
					fmavf (m1.column3.splat_z (), m2.column3,
						m1.column3.splat_w () * m2.column4))),
			fmavf (m1.column4.splat_x (), m2.column1,
				fmavf (m1.column4.splat_y (), m2.column2,
					fmavf (m1.column4.splat_z (), m2.column3,
						m1.column4.splat_w () * m2.column4)))
		);
	}
	inline matrixf multiplymf (const matrixf& m, float s) noexcept
	{
		return matrixf (m.column1 * s, m.column2 * s, m.column3 * s, m.column4 * s);
	}
	inline matrixf multiplymf (float s, const matrixf& m) noexcept
	{
		return multiplymf (m, s);
	}
	inline matrixf dividemf (const matrixf& m1, const matrixf& m2) noexcept
	{
		return matrixf (m1.column1 / m2.column1, m1.column2 / m2.column2, m1.column3 / m2.column3, m1.column4 / m2.column4);
	}
	inline matrixf dividemf (const matrixf& m, float s) noexcept
	{
		return matrixf (m.column1 / s, m.column2 / s, m.column3 / s, m.column4 / s);
	}
	inline matrixf andmf (const matrixf& m1, const matrixf& m2) noexcept
	{
		return matrixf (m1.column1 & m2.column1, m1.column2 & m2.column2, m1.column3 & m2.column3, m1.column4 & m2.column4);
	}
	inline matrixf ormf (const matrixf& m1, const matrixf& m2) noexcept
	{
		return matrixf (m1.column1 | m2.column1, m1.column2 | m2.column2, m1.column3 | m2.column3, m1.column4 | m2.column4);
	}
	inline matrixf xormf (const matrixf& m1, const matrixf& m2) noexcept
	{
		return matrixf (m1.column1 ^ m2.column1, m1.column2 ^ m2.column2, m1.column3 ^ m2.column3, m1.column4 ^ m2.column4);
	}
	inline matrixf equalsmf (const matrixf& m1, const matrixf& m2) noexcept
	{
		return matrixf (equalsvf (m1.column1, m2.column1), equalsvf (m1.column2, m2.column2),
			equalsvf (m1.column3, m2.column3), equalsvf (m1.column4, m2.column4));
	}
	inline matrixf not_equalsmf (const matrixf& m1, const matrixf& m2) noexcept
	{
		return matrixf (equalsvf (m1.column1, m2.column1), not_equalsvf (m1.column2, m2.column2),
			not_equalsvf (m1.column3, m2.column3), not_equalsvf (m1.column4, m2.column4));
	}

	inline matrixf operator+ (const matrixf& m1, const matrixf& m2) noexcept { return addmf (m1, m2); }
	inline matrixf operator- (const matrixf& m1, const matrixf& m2) noexcept { return subtractmf (m1, m2); }
	inline matrixf operator- (const matrixf& m) noexcept { return negatemf (m); }
	inline matrixf operator* (const matrixf& m1, const matrixf& m2) noexcept { return multiplymf (m1, m2); }
	inline matrixf operator* (const matrixf& m, float s) noexcept { return multiplymf (m, s); }
	inline matrixf operator* (float s, const matrixf& m) noexcept { return multiplymf (s, m); }
	inline matrixf operator/ (const matrixf& m1, const matrixf& m2) noexcept { return dividemf (m1, m2); }
	inline matrixf operator/ (const matrixf& m, float s) noexcept { return dividemf (m, s); }
	inline matrixf operator& (const matrixf& m1, const matrixf& m2) noexcept { return andmf (m1, m2); }
	inline matrixf operator| (const matrixf& m1, const matrixf& m2) noexcept { return ormf (m1, m2); }
	inline matrixf operator^ (const matrixf& m1, const matrixf& m2) noexcept { return xormf (m1, m2); }
	inline bool operator== (const matrixf& m1, const matrixf& m2) noexcept { return movemask_matrix (equalsmf (m1, m2)) == 0xFFFF; }
	inline bool operator!= (const matrixf& m1, const matrixf& m2) noexcept { return movemask_matrix (not_equalsmf (m1, m2)) != 0; }

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Matrix type Matrix operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline matrixf transposemf (const matrixf& m) noexcept
	{
		vectorf temp0 = m.column1.shuffle<0, 1, 0, 1> (m.column2);
		vectorf temp2 = m.column1.shuffle<2, 3, 2, 3> (m.column2);
		vectorf temp1 = m.column3.shuffle<0, 1, 0, 1> (m.column4);
		vectorf temp3 = m.column3.shuffle<2, 3, 2, 3> (m.column4);

		matrixf ret;
		ret.column1 = temp0.shuffle<0, 2, 0, 2> (temp1);
		ret.column2 = temp0.shuffle<1, 3, 1, 3> (temp1);
		ret.column3 = temp2.shuffle<0, 2, 0, 2> (temp3);
		ret.column4 = temp2.shuffle<1, 3, 1, 3> (temp3);

		return ret;
	}
	inline float determinantmf (const matrixf& m) noexcept
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
	inline matrixf invertmf (const matrixf& m, float& det) noexcept
	{
		static vectorf one = vectorf (1);

		matrixf transposed = transposemf (m);
		vectorf v00 = transposed.column3.permute<0, 0, 1, 1> ();
		vectorf v10 = transposed.column4.permute<2, 3, 2, 3> ();
		vectorf v01 = transposed.column1.permute<0, 0, 1, 1> ();
		vectorf v11 = transposed.column2.permute<2, 3, 2, 3> ();
		vectorf v02 = transposed.column3.shuffle<0, 2, 0, 2> (transposed.column1);
		vectorf v12 = transposed.column4.shuffle<1, 3, 1, 3> (transposed.column2);

		vectorf d0 = v00 * v10;
		vectorf d1 = v01 * v11;
		vectorf d2 = v02 * v12;

		v00 = transposed.column3.permute<2, 3, 2, 3> ();
		v10 = transposed.column4.permute<0, 0, 1, 1> ();
		v01 = transposed.column1.permute<2, 3, 2, 3> ();
		v11 = transposed.column2.permute<0, 0, 1, 1> ();
		v02 = transposed.column3.shuffle<1, 3, 1, 3> (transposed.column1);
		v12 = transposed.column4.shuffle<0, 2, 0, 2> (transposed.column2);

		v00 = v00 * v10;
		v01 = v01 * v11;
		v02 = v02 * v12;

		d0 = d0 - v00;
		d1 = d1 - v01;
		d2 = d2 - v02;

		v11 = d0.shuffle<1, 3, 1, 1> (d2);
		v00 = transposed.column2.permute<1, 2, 0, 1> ();
		v10 = v11.shuffle<2, 0, 3, 0> (d0);
		v01 = transposed.column1.permute<2, 0, 1, 0> ();
		v11 = v11.shuffle<1, 2, 1, 2> (d0);

		vectorf v13 = d1.shuffle<1, 3, 3, 3> (d2);
		v02 = transposed.column4.permute<1, 2, 0, 1> ();
		v12 = v13.shuffle<2, 0, 3, 0> (d1);
		vectorf v03 = transposed.column3.permute<2, 0, 1, 0> ();
		v13 = v13.shuffle<1, 2, 1, 2> (d1);

		vectorf c0 = v00 * v10;
		vectorf c2 = v01 * v11;
		vectorf c4 = v02 * v12;
		vectorf c6 = v03 * v13;

		v11 = d0.shuffle<0, 1, 0, 0> (d2);
		v00 = transposed.column2.permute<2, 3, 1, 2> ();
		v10 = d0.shuffle<3, 0, 1, 2> (v11);
		v01 = transposed.column1.permute<3, 2, 3, 1> ();
		v11 = d0.shuffle<2, 1, 2, 0> (v11);

		v13 = d1.shuffle<0, 1, 2, 2> (d2);
		v02 = transposed.column4.permute<2, 3, 1, 2> ();
		v12 = d1.shuffle<3, 0, 1, 2> (v13);
		v03 = transposed.column3.permute<3, 2, 3, 1> ();
		v13 = d1.shuffle<2, 1, 2, 0> (v13);

		v00 = v00 * v10;
		v01 = v01 * v11;
		v02 = v02 * v12;
		v03 = v03 * v13;
		c0 = c0 - v00;
		c2 = c2 - v01;
		c4 = c4 - v02;
		c6 = c6 - v03;

		v00 = transposed.column2.permute<3, 0, 3, 0> ();
		v10 = d0.shuffle<2, 2, 0, 1> (d2);
		v10 = v10.permute<0, 3, 2, 0> ();
		v01 = transposed.column1.permute<1, 3, 0, 2> ();
		v11 = d0.shuffle<0, 3, 0, 1> (d2);
		v11 = v11.permute<3, 0, 1, 2> ();
		v02 = transposed.column4.permute<3, 0, 3, 0> ();
		v12 = d1.shuffle<2, 2, 2, 3> (d2);
		v12 = v12.permute<0, 3, 2, 0> ();
		v03 = transposed.column3.permute<1, 3, 0, 2> ();
		v13 = d1.shuffle<0, 3, 2, 3> (d2);
		v13 = v13.permute<3, 0, 1, 2> ();

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

		c0 = c0.shuffle<0, 2, 1, 3> (c1);
		c2 = c2.shuffle<0, 2, 1, 3> (c3);
		c4 = c4.shuffle<0, 2, 1, 3> (c5);
		c6 = c6.shuffle<0, 2, 1, 3> (c7);
		c0 = c0.permute<0, 2, 1, 3> ();
		c2 = c2.permute<0, 2, 1, 3> ();
		c4 = c4.permute<0, 2, 1, 3> ();
		c6 = c6.permute<0, 2, 1, 3> ();

		vectorf temp = vectorf(dot4 (c0, transposed.column1));
		det = temp.x ();
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
	// Vector * Quaternion Operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline vectorf rotate3 (const vectorf& v, const vectorf& q) noexcept
	{
		return (conjugateq (q) * v) * q;
	}
	inline vectorf inverse_rotate3 (const vectorf& v, const vectorf& q) noexcept
	{
		return (conjugateq (q * v)) * q;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Vector * Matrix Operations
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline vectorf transform2 (const vectorf& v, const matrixf& m) noexcept
	{
		return fmavf (v.splat_x (), m.column1, fmavf (v.splat_y (), m.column2, m.column4));
	}
	inline vectorf transform_normal2 (const vectorf& v, const matrixf& m) noexcept
	{
		return fmavf (v.splat_x (), m.column1, v.splat_y () * m.column2);
	}
	inline vectorf transform3 (const vectorf& v, const matrixf& m) noexcept
	{
		return fmavf (v.splat_x (), m.column1, fmavf (v.splat_y (), m.column2, fmavf (v.splat_z (), m.column3, m.column4)));
	}
	inline vectorf transform_normal3 (const vectorf& v, const matrixf& m) noexcept
	{
		return fmavf (v.splat_x (), m.column1, fmavf (v.splat_y (), m.column2, v.splat_z () * m.column3));
	}
	inline vectorf transform4 (const vectorf& v, const matrixf& m) noexcept
	{
		return fmavf (v.splat_x (), m.column1, fmavf (v.splat_y (), m.column2, fmavf (v.splat_z (), m.column3, v.splat_w () * m.column4)));
	}
	inline vectorf transform_plane (const vectorf& p, const matrixf& m) noexcept
	{
		return transform4 (p, m);
	}
}

#endif