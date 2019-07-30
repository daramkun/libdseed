#ifndef __DSEED_MATH_SIMD_NOSIMD_INL__
#define __DSEED_MATH_SIMD_NOSIMD_INL__

namespace dseed
{
	////////////////////////////////////////////////////////////////////////////////////////
	//
	// 128-bit Floating-point Vector
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct alignas(16) vectorf_nosimd
	{
		float v[4];

		inline vectorf_nosimd () noexcept = default;
		inline vectorf_nosimd (const float* arr) noexcept { load (arr); }
		inline vectorf_nosimd (float x, float y, float z, float w) noexcept { v[0] = x; v[1] = y; v[2] = z; v[3] = w; }
		inline vectorf_nosimd (float scalar) noexcept { v[0] = v[1] = v[2] = v[3] = scalar; }

		inline operator float* () noexcept { return v; }
		inline operator const float* () const noexcept { return v; }

		inline void FASTCALL load (const float* arr) noexcept { memcpy (v, arr, sizeof (float) * 4); }
		inline void FASTCALL store (float* arr) noexcept { memcpy (arr, v, sizeof (float) * 4); }

		template<uint32_t x, uint32_t y, uint32_t z, uint32_t w>
		static inline vectorf_nosimd FASTCALL shuffle32 (const vectorf_nosimd & v1, const vectorf_nosimd & v2) noexcept
		{
			return vectorf_nosimd (
				(x >= 0 && x <= 3) ? v1.v[x] : v2.v[4 - x],
				(y >= 0 && y <= 3) ? v1.v[y] : v2.v[4 - y],
				(z >= 0 && z <= 3) ? v2.v[z] : v1.v[4 - z],
				(w >= 0 && w <= 3) ? v2.v[w] : v1.v[4 - w]
			);
		}

		template<uint32_t x, uint32_t y, uint32_t z, uint32_t w>
		inline vectorf_nosimd FASTCALL permute32 () const noexcept
		{
			return shuffle32<x, y, z, w> (*this);
		}

		inline vectorf_nosimd FASTCALL splat_x () const noexcept { return vectorf_nosimd (v[0]); }
		inline vectorf_nosimd FASTCALL splat_y () const noexcept { return vectorf_nosimd (v[1]); }
		inline vectorf_nosimd FASTCALL splat_z () const noexcept { return vectorf_nosimd (v[2]); }
		inline vectorf_nosimd FASTCALL splat_w () const noexcept { return vectorf_nosimd (v[3]); }

		inline float FASTCALL x () const noexcept { return v[0]; }
		inline float FASTCALL y () const noexcept { return v[1]; }
		inline float FASTCALL z () const noexcept { return v[2]; }
		inline float FASTCALL w () const noexcept { return v[3]; }

		static inline bool support () noexcept { return true; }
	};

	inline vectorf_nosimd FASTCALL addvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept;
	inline vectorf_nosimd FASTCALL subtractvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept;
	inline vectorf_nosimd FASTCALL negatevf (const vectorf_nosimd& v) noexcept;
	inline vectorf_nosimd FASTCALL multiplyvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept;
	inline vectorf_nosimd FASTCALL multiplyvf (const vectorf_nosimd& v, float s) noexcept;
	inline vectorf_nosimd FASTCALL multiplyvf (float s, const vectorf_nosimd& v) noexcept;
	inline vectorf_nosimd FASTCALL dividevf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept;
	inline vectorf_nosimd FASTCALL dividevf (const vectorf_nosimd& v, float s) noexcept;
	inline vectorf_nosimd FASTCALL fmavf (const vectorf_nosimd& mv1, const vectorf_nosimd& mv2, const vectorf_nosimd& av) noexcept;
	inline vectorf_nosimd FASTCALL fmsvf (const vectorf_nosimd& mv1, const vectorf_nosimd& mv2, const vectorf_nosimd& sv) noexcept;
	inline vectorf_nosimd FASTCALL fnmsvf (const vectorf_nosimd& sv, const vectorf_nosimd& mv1, const vectorf_nosimd& mv2) noexcept;
	inline vectorf_nosimd FASTCALL andvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept;
	inline vectorf_nosimd FASTCALL orvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept;
	inline vectorf_nosimd FASTCALL xorvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept;
	inline vectorf_nosimd FASTCALL notvf (const vectorf_nosimd& v) noexcept;
	inline vectorf_nosimd FASTCALL shiftlvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept;
	inline vectorf_nosimd FASTCALL shiftrvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept;
	inline vectorf_nosimd FASTCALL equalsvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept;
	inline vectorf_nosimd FASTCALL not_equalsvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept;
	inline vectorf_nosimd FASTCALL lesservf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept;
	inline vectorf_nosimd FASTCALL lesser_equalsvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept;
	inline vectorf_nosimd FASTCALL greatervf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept;
	inline vectorf_nosimd FASTCALL greater_equalsvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept;
	
	////////////////////////////////////////////////////////////////////////////////////////
	//
	// 128-bit Signed Integer Vector
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct alignas(16) vectori_nosimd
	{
		int v[4];
		inline vectori_nosimd (const int* vector) noexcept { memcpy (v, vector, sizeof (int) * 4); }
		inline vectori_nosimd (const float* vector) noexcept
		{
			v[0] = (int)vector[0];
			v[1] = (int)vector[1];
			v[2] = (int)vector[2];
			v[3] = (int)vector[3];
		}
		inline vectori_nosimd (int x, int y, int z, int w) noexcept { v[0] = x; v[1] = y; v[2] = z; v[3] = w; }
		inline vectori_nosimd (int i) noexcept { v[0] = v[1] = v[2] = v[3] = i; }

		inline operator int* () noexcept { return v; }
		inline operator const int* () const noexcept { return v; }

		inline void FASTCALL load (const int* vector) noexcept { memcpy (v, vector, sizeof (int) * 4); }
		inline void FASTCALL store (int* vector) noexcept { memcpy (vector, v, sizeof (int) * 4); }

		template<uint8_t x, uint8_t y, uint8_t z, uint8_t w>
		static inline vectori_nosimd FASTCALL shuffle32 (const vectori_nosimd & v1, const vectori_nosimd & v2) noexcept
		{
			return vectori (
				(x >= 0 && x <= 3) ? v1.v[x] : v2.v[4 - x],
				(y >= 0 && y <= 3) ? v1.v[y] : v2.v[4 - y],
				(z >= 0 && z <= 3) ? v2.v[z] : v1.v[4 - z],
				(w >= 0 && w <= 3) ? v2.v[w] : v1.v[4 - w]
			);
		}
		template<uint8_t x, uint8_t y, uint8_t z, uint8_t w>
		inline vectori_nosimd FASTCALL permute32 () const
		{
			return shuffle32<x, y, z, w> (*this, *this);
		}
		template<uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4, uint8_t s5, uint8_t s6, uint8_t s7, uint8_t s8>
		inline vectori_nosimd FASTCALL permute16 () const
		{
			auto arr = reinterpret_cast<const short*>(v);
			vectori_nosimd ret;
			auto destarr = reinterpret_cast<short*>(ret.v);
			destarr[0] = arr[s1]; destarr[1] = arr[s2]; destarr[2] = arr[s3]; destarr[3] = arr[s4];
			destarr[4] = arr[s5]; destarr[5] = arr[s6]; destarr[6] = arr[s7]; destarr[7] = arr[s8];
			return ret;
		}
		template<uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4, uint8_t s5, uint8_t s6, uint8_t s7, uint8_t s8,
			uint8_t s9, uint8_t s10, uint8_t s11, uint8_t s12, uint8_t s13, uint8_t s14, uint8_t s15, uint8_t s16>
			inline vectori_nosimd FASTCALL permute8 () const
		{
			auto arr = reinterpret_cast<const char*>(v);
			vectori_nosimd ret;
			auto destarr = reinterpret_cast<char*>(ret.v);
			destarr[0] = arr[s1]; destarr[1] = arr[s2]; destarr[2] = arr[s3]; destarr[3] = arr[s4];
			destarr[4] = arr[s5]; destarr[5] = arr[s6]; destarr[6] = arr[s7]; destarr[7] = arr[s8];
			destarr[8] = arr[s9]; destarr[9] = arr[s10]; destarr[10] = arr[s11]; destarr[11] = arr[s12];
			destarr[12] = arr[s13]; destarr[13] = arr[s14]; destarr[14] = arr[s15]; destarr[15] = arr[s16];
			return ret;
		}

		inline vectori_nosimd FASTCALL splat_x () const { return vectori_nosimd (v[0]); }
		inline vectori_nosimd FASTCALL splat_y () const { return vectori_nosimd (v[1]); }
		inline vectori_nosimd FASTCALL splat_z () const { return vectori_nosimd (v[2]); }
		inline vectori_nosimd FASTCALL splat_w () const { return vectori_nosimd (v[3]); }

		inline int FASTCALL x () const { return v[0]; }
		inline int FASTCALL y () const { return v[1]; }
		inline int FASTCALL z () const { return v[2]; }
		inline int FASTCALL w () const { return v[3]; }

		static inline bool support () noexcept { return true; }
	};

	inline vectori_nosimd FASTCALL addvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept;
	inline vectori_nosimd FASTCALL subtractvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept;
	inline vectori_nosimd FASTCALL negatevi (const vectori_nosimd& v) noexcept;
	inline vectori_nosimd FASTCALL multiplyvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept;
	inline vectori_nosimd FASTCALL multiplyvi (const vectori_nosimd& v, float s) noexcept;
	inline vectori_nosimd FASTCALL multiplyvi (float s, const vectori_nosimd& v) noexcept;
	inline vectori_nosimd FASTCALL dividevi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept;
	inline vectori_nosimd FASTCALL dividevi (const vectori_nosimd& v, float s) noexcept;
	inline vectori_nosimd FASTCALL fmavi (const vectori_nosimd& mv1, const vectori_nosimd& mv2, const vectori_nosimd& av) noexcept;
	inline vectori_nosimd FASTCALL fmsvi (const vectori_nosimd& mv1, const vectori_nosimd& mv2, const vectori_nosimd& sv) noexcept;
	inline vectori_nosimd FASTCALL fnmsvi (const vectori_nosimd& sv, const vectori_nosimd& mv1, const vectori_nosimd& mv2) noexcept;
	inline vectori_nosimd FASTCALL andvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept;
	inline vectori_nosimd FASTCALL orvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept;
	inline vectori_nosimd FASTCALL xorvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept;
	inline vectori_nosimd FASTCALL shiftlvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept;
	inline vectori_nosimd FASTCALL shiftrvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept;
	inline vectori_nosimd FASTCALL notvi (const vectori_nosimd& v) noexcept;
	inline vectori_nosimd FASTCALL equalsvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept;
	inline vectori_nosimd FASTCALL not_equalsvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept;
	inline vectori_nosimd FASTCALL lesservi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept;
	inline vectori_nosimd FASTCALL lesser_equalsvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept;
	inline vectori_nosimd FASTCALL greatervi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept;
	inline vectori_nosimd FASTCALL greater_equalsvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept;

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// 256-bit Floating-point Vector
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct alignas(32) vector8f_nosimd
	{
		float v[8];

		inline vector8f_nosimd () noexcept = default;
		inline vector8f_nosimd (const float* arr) noexcept { load (arr); }
		inline vector8f_nosimd (float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2) noexcept
		{
			v[0] = x1; v[1] = y1; v[2] = z1; v[3] = w1;
			v[4] = x2; v[5] = y2; v[6] = z2; v[7] = w2;
		}
		inline vector8f_nosimd (float scalar) noexcept
		{
			v[0] = v[1] = v[2] = v[3] = v[4] = v[5] = v[6] = v[7] = scalar;
		}
		inline vector8f_nosimd (const vectorf_nosimd & v1, const vectorf_nosimd & v2) noexcept
		{
			memcpy (v, v1.v, sizeof (float) * 4);
			memcpy (v + 4, v2.v, sizeof (float) * 4);
		}

		inline operator float* () noexcept { return v; }
		inline operator const float* () const noexcept { return v; }

		inline void FASTCALL load (const float* arr) noexcept { memcpy (v, arr, sizeof (float) * 8); }
		inline void FASTCALL store (float* arr) noexcept { memcpy (arr, v, sizeof (float) * 8); }

		template<uint32_t x, uint32_t y, uint32_t z, uint32_t w>
		static inline vector8f_nosimd FASTCALL shuffle32 (const vector8f_nosimd & v1, const vector8f_nosimd & v2) noexcept
		{
			return vector8f_nosimd (
				(x >= 0 && x <= 3) ? v1.v[x] : v2.v[4 - x],
				(y >= 0 && y <= 3) ? v1.v[y] : v2.v[4 - y],
				(z >= 0 && z <= 3) ? v2.v[z] : v1.v[4 - z],
				(w >= 0 && w <= 3) ? v2.v[w] : v1.v[4 - w],
				(x >= 0 && x <= 3) ? v1.v[x + 4] : v2.v[x],
				(y >= 0 && y <= 3) ? v1.v[y + 4] : v2.v[y],
				(z >= 0 && z <= 3) ? v2.v[z + 4] : v1.v[z],
				(w >= 0 && w <= 3) ? v2.v[w + 4] : v1.v[w]
			);
		}

		template<uint32_t x, uint32_t y, uint32_t z, uint32_t w>
		inline vector8f_nosimd FASTCALL permute32 () const noexcept
		{
			return shuffle32<x, y, z, w> (*this);
		}

		inline vector8f_nosimd FASTCALL splat_x1 () const noexcept { return vector8f_nosimd (v[0]); }
		inline vector8f_nosimd FASTCALL splat_y1 () const noexcept { return vector8f_nosimd (v[1]); }
		inline vector8f_nosimd FASTCALL splat_z1 () const noexcept { return vector8f_nosimd (v[2]); }
		inline vector8f_nosimd FASTCALL splat_w1 () const noexcept { return vector8f_nosimd (v[3]); }
		inline vector8f_nosimd FASTCALL splat_x2 () const noexcept { return vector8f_nosimd (v[4]); }
		inline vector8f_nosimd FASTCALL splat_y2 () const noexcept { return vector8f_nosimd (v[5]); }
		inline vector8f_nosimd FASTCALL splat_z2 () const noexcept { return vector8f_nosimd (v[6]); }
		inline vector8f_nosimd FASTCALL splat_w2 () const noexcept { return vector8f_nosimd (v[7]); }

		inline float FASTCALL x1 () const noexcept { return v[0]; }
		inline float FASTCALL y1 () const noexcept { return v[1]; }
		inline float FASTCALL z1 () const noexcept { return v[2]; }
		inline float FASTCALL w1 () const noexcept { return v[3]; }
		inline float FASTCALL x2 () const noexcept { return v[4]; }
		inline float FASTCALL y2 () const noexcept { return v[5]; }
		inline float FASTCALL z2 () const noexcept { return v[6]; }
		inline float FASTCALL w2 () const noexcept { return v[7]; }

		inline vectorf_nosimd FASTCALL vector1 () const noexcept { return vectorf_nosimd (v); }
		inline vectorf_nosimd FASTCALL vector2 () const noexcept { return vectorf_nosimd (v + 4); }

		static inline bool support () noexcept { return true; }
	};

	inline vector8f_nosimd FASTCALL addv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept;
	inline vector8f_nosimd FASTCALL subtractv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept;
	inline vector8f_nosimd FASTCALL negatev8f (const vector8f_nosimd& v) noexcept;
	inline vector8f_nosimd FASTCALL multiplyv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept;
	inline vector8f_nosimd FASTCALL multiplyv8f (const vector8f_nosimd& v, float s) noexcept;
	inline vector8f_nosimd FASTCALL multiplyv8f (float s, const vector8f_nosimd& v) noexcept;
	inline vector8f_nosimd FASTCALL dividev8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept;
	inline vector8f_nosimd FASTCALL dividev8f (const vector8f_nosimd& v, float s) noexcept;
	inline vector8f_nosimd FASTCALL fmav8f (const vector8f_nosimd& mv1, const vector8f_nosimd& mv2, const vector8f_nosimd& av) noexcept;
	inline vector8f_nosimd FASTCALL fmsv8f (const vector8f_nosimd& mv1, const vector8f_nosimd& mv2, const vector8f_nosimd& sv) noexcept;
	inline vector8f_nosimd FASTCALL fnmsv8f (const vector8f_nosimd& sv, const vector8f_nosimd& mv1, const vector8f_nosimd& mv2) noexcept;
	inline vector8f_nosimd FASTCALL andv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept;
	inline vector8f_nosimd FASTCALL orv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept;
	inline vector8f_nosimd FASTCALL xorv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept;
	inline vector8f_nosimd FASTCALL shiftlv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept;
	inline vector8f_nosimd FASTCALL shiftrv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept;
	inline vector8f_nosimd FASTCALL notv8f (const vector8f_nosimd& v) noexcept;
	inline vector8f_nosimd FASTCALL equalsv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept;
	inline vector8f_nosimd FASTCALL not_equalsv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept;
	inline vector8f_nosimd FASTCALL lesserv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept;
	inline vector8f_nosimd FASTCALL lesser_equalsv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept;
	inline vector8f_nosimd FASTCALL greaterv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept;
	inline vector8f_nosimd FASTCALL greater_equalsv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept;

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// 256-bit Signed Integer Vector
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct alignas(32) vector8i_nosimd
	{
		int v[8];

		inline vector8i_nosimd () noexcept = default;
		inline vector8i_nosimd (const int* arr) noexcept { load (arr); }
		inline vector8i_nosimd (int x1, int y1, int z1, int w1, int x2, int y2, int z2, int w2) noexcept
		{
			v[0] = x1; v[1] = y1; v[2] = z1; v[3] = w1;
			v[4] = x2; v[5] = y2; v[6] = z2; v[7] = w2;
		}
		inline vector8i_nosimd (int scalar) noexcept
		{
			v[0] = v[1] = v[2] = v[3] = v[4] = v[5] = v[6] = v[7] = scalar;
		}
		inline vector8i_nosimd (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept
		{
			memcpy (v, v1.v, sizeof (int) * 4);
			memcpy (v + 4, v2.v, sizeof (int) * 4);
		}

		inline operator int* () noexcept { return v; }
		inline operator const int* () const noexcept { return v; }

		inline void FASTCALL load (const int* arr) noexcept { memcpy (v, arr, sizeof (int) * 8); }
		inline void FASTCALL store (int* arr) noexcept { memcpy (arr, v, sizeof (int) * 8); }

		template<uint32_t x, uint32_t y, uint32_t z, uint32_t w>
		static inline vector8i_nosimd FASTCALL shuffle32 (const vector8i_nosimd & v1, const vector8i_nosimd & v2) noexcept
		{
			return vector8i_nosimd (
				(x >= 0 && x <= 3) ? v1.v[x] : v2.v[4 - x],
				(y >= 0 && y <= 3) ? v1.v[y] : v2.v[4 - y],
				(z >= 0 && z <= 3) ? v2.v[z] : v1.v[4 - z],
				(w >= 0 && w <= 3) ? v2.v[w] : v1.v[4 - w],
				(x >= 0 && x <= 3) ? v1.v[x + 4] : v2.v[x],
				(y >= 0 && y <= 3) ? v1.v[y + 4] : v2.v[y],
				(z >= 0 && z <= 3) ? v2.v[z + 4] : v1.v[z],
				(w >= 0 && w <= 3) ? v2.v[w + 4] : v1.v[w]
			);
		}

		template<uint32_t x, uint32_t y, uint32_t z, uint32_t w>
		inline vector8i_nosimd FASTCALL permute32 () const noexcept
		{
			return shuffle32<x, y, z, w> (*this);
		}
		template<uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4, uint8_t s5, uint8_t s6, uint8_t s7, uint8_t s8,
			uint8_t s9, uint8_t s10, uint8_t s11, uint8_t s12, uint8_t s13, uint8_t s14, uint8_t s15, uint8_t s16>
		inline vectori_nosimd FASTCALL permute16 () const
		{
			auto arr = reinterpret_cast<const short*>(v);
			vectori_nosimd ret;
			auto destarr = reinterpret_cast<short*>(ret.v);
			destarr[0] = arr[s1]; destarr[1] = arr[s2]; destarr[2] = arr[s3]; destarr[3] = arr[s4];
			destarr[4] = arr[s5]; destarr[5] = arr[s6]; destarr[6] = arr[s7]; destarr[7] = arr[s8];
			destarr[8] = arr[s9]; destarr[9] = arr[s10]; destarr[10] = arr[s11]; destarr[11] = arr[s12];
			destarr[12] = arr[s13]; destarr[13] = arr[s14]; destarr[14] = arr[s15]; destarr[15] = arr[s16];
			return ret;
		}
		template<uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4, uint8_t s5, uint8_t s6, uint8_t s7, uint8_t s8,
			uint8_t s9, uint8_t s10, uint8_t s11, uint8_t s12, uint8_t s13, uint8_t s14, uint8_t s15, uint8_t s16,
			uint8_t s17, uint8_t s18, uint8_t s19, uint8_t s20, uint8_t s21, uint8_t s22, uint8_t s23, uint8_t s24,
			uint8_t s25, uint8_t s26, uint8_t s27, uint8_t s28, uint8_t s29, uint8_t s30, uint8_t s31, uint8_t s32>
			inline vectori_nosimd FASTCALL permute8 () const
		{
			auto arr = reinterpret_cast<const char*>(v);
			vectori_nosimd ret;
			auto destarr = reinterpret_cast<char*>(ret.v);
			destarr[0] = arr[s1]; destarr[1] = arr[s2]; destarr[2] = arr[s3]; destarr[3] = arr[s4];
			destarr[4] = arr[s5]; destarr[5] = arr[s6]; destarr[6] = arr[s7]; destarr[7] = arr[s8];
			destarr[8] = arr[s9]; destarr[9] = arr[s10]; destarr[10] = arr[s11]; destarr[11] = arr[s12];
			destarr[12] = arr[s13]; destarr[13] = arr[s14]; destarr[14] = arr[s15]; destarr[15] = arr[s16];
			destarr[16] = arr[s17]; destarr[17] = arr[s18]; destarr[18] = arr[s19]; destarr[19] = arr[s20];
			destarr[20] = arr[s21]; destarr[21] = arr[s22]; destarr[22] = arr[s23]; destarr[23] = arr[s24];
			destarr[24] = arr[s25]; destarr[25] = arr[s26]; destarr[26] = arr[s27]; destarr[27] = arr[s28];
			destarr[28] = arr[s29]; destarr[29] = arr[s30]; destarr[30] = arr[s31]; destarr[31] = arr[s32];
			return ret;
		}

		inline vector8i_nosimd FASTCALL splat_x1 () const noexcept { return vector8i_nosimd (v[0]); }
		inline vector8i_nosimd FASTCALL splat_y1 () const noexcept { return vector8i_nosimd (v[1]); }
		inline vector8i_nosimd FASTCALL splat_z1 () const noexcept { return vector8i_nosimd (v[2]); }
		inline vector8i_nosimd FASTCALL splat_w1 () const noexcept { return vector8i_nosimd (v[3]); }
		inline vector8i_nosimd FASTCALL splat_x2 () const noexcept { return vector8i_nosimd (v[4]); }
		inline vector8i_nosimd FASTCALL splat_y2 () const noexcept { return vector8i_nosimd (v[5]); }
		inline vector8i_nosimd FASTCALL splat_z2 () const noexcept { return vector8i_nosimd (v[6]); }
		inline vector8i_nosimd FASTCALL splat_w2 () const noexcept { return vector8i_nosimd (v[7]); }

		inline int FASTCALL x1 () const noexcept { return v[0]; }
		inline int FASTCALL y1 () const noexcept { return v[1]; }
		inline int FASTCALL z1 () const noexcept { return v[2]; }
		inline int FASTCALL w1 () const noexcept { return v[3]; }
		inline int FASTCALL x2 () const noexcept { return v[4]; }
		inline int FASTCALL y2 () const noexcept { return v[5]; }
		inline int FASTCALL z2 () const noexcept { return v[6]; }
		inline int FASTCALL w2 () const noexcept { return v[7]; }

		inline vectori_nosimd FASTCALL vector1 () const noexcept { return vectori_nosimd (v); }
		inline vectori_nosimd FASTCALL vector2 () const noexcept { return vectori_nosimd (v + 4); }

		static inline bool support () noexcept { return true; }
	};

	inline vector8i_nosimd FASTCALL addv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept;
	inline vector8i_nosimd FASTCALL subtractv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept;
	inline vector8i_nosimd FASTCALL negatev8i (const vector8i_nosimd& v) noexcept;
	inline vector8i_nosimd FASTCALL multiplyv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept;
	inline vector8i_nosimd FASTCALL multiplyv8i (const vector8i_nosimd& v, float s) noexcept;
	inline vector8i_nosimd FASTCALL multiplyv8i (float s, const vector8i_nosimd& v) noexcept;
	inline vector8i_nosimd FASTCALL dividev8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept;
	inline vector8i_nosimd FASTCALL dividev8i (const vector8i_nosimd& v, float s) noexcept;
	inline vector8i_nosimd FASTCALL fmav8i (const vector8i_nosimd& mv1, const vector8i_nosimd& mv2, const vector8i_nosimd& av) noexcept;
	inline vector8i_nosimd FASTCALL fmsv8i (const vector8i_nosimd& mv1, const vector8i_nosimd& mv2, const vector8i_nosimd& sv) noexcept;
	inline vector8i_nosimd FASTCALL fnmsv8i (const vector8i_nosimd& sv, const vector8i_nosimd& mv1, const vector8i_nosimd& mv2) noexcept;
	inline vector8i_nosimd FASTCALL andv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept;
	inline vector8i_nosimd FASTCALL orv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept;
	inline vector8i_nosimd FASTCALL xorv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept;
	inline vector8i_nosimd FASTCALL shiftlv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept;
	inline vector8i_nosimd FASTCALL shiftrv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept;
	inline vector8i_nosimd FASTCALL notv8i (const vector8i_nosimd& v) noexcept;
	inline vector8i_nosimd FASTCALL equalsv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept;
	inline vector8i_nosimd FASTCALL not_equalsv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept;
	inline vector8i_nosimd FASTCALL lesserv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept;
	inline vector8i_nosimd FASTCALL lesser_equalsv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept;
	inline vector8i_nosimd FASTCALL greaterv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept;
	inline vector8i_nosimd FASTCALL greater_equalsv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept;

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Implementation vector conversions
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline vectori_nosimd FASTCALL conv_f32_to_i32 (const vectorf_nosimd& v) noexcept
	{
		return vectori_nosimd ((int)v.v[0], (int)v.v[1], (int)v.v[2], (int)v.v[3]);
	}
	inline vector8i_nosimd FASTCALL conv_f32_to_i32 (const vector8f_nosimd& v) noexcept
	{
		return vector8i_nosimd (
			(int)v.v[0], (int)v.v[1], (int)v.v[2], (int)v.v[3],
			(int)v.v[4], (int)v.v[5], (int)v.v[6], (int)v.v[7]
		);
	}
	inline vectori_nosimd FASTCALL reinterpret_f32_to_i32 (const vectorf_nosimd& v) noexcept
	{
		const int* reinterpret = reinterpret_cast<const int*>(v.v);
		return vectori_nosimd (reinterpret);
	}
	inline vector8i_nosimd FASTCALL reinterpret_f32_to_i32 (const vector8f_nosimd& v) noexcept
	{
		const int* reinterpret = reinterpret_cast<const int*>(v.v);
		return vector8i_nosimd (reinterpret);
	}

	inline vectorf_nosimd FASTCALL conv_i32_to_f32 (const vectori_nosimd& v) noexcept
	{
		return vectorf_nosimd ((float)v.v[0], (float)v.v[1], (float)v.v[2], (float)v.v[3]);
	}
	inline vector8f_nosimd FASTCALL conv_i32_to_f32 (const vector8i_nosimd& v) noexcept
	{
		return vector8f_nosimd (
			(float)v.v[0], (float)v.v[1], (float)v.v[2], (float)v.v[3],
			(float)v.v[4], (float)v.v[5], (float)v.v[6], (float)v.v[7]
		);
	}
	inline vectorf_nosimd FASTCALL reinterpret_i32_to_f32 (const vectori_nosimd& v) noexcept
	{
		const float* reinterpret = reinterpret_cast<const float*>(v.v);
		return vectorf_nosimd (reinterpret);
	}
	inline vector8f_nosimd FASTCALL reinterpret_i32_to_f32 (const vector8i_nosimd& v) noexcept
	{
		const float* reinterpret = reinterpret_cast<const float*>(v.v);
		return vector8f_nosimd (reinterpret);
	}

	inline vectori_nosimd FASTCALL conv_i32_to_i24 (const vectori_nosimd& v) noexcept
	{
		int24_t conv[6] = {};
		conv[0] = (int24_t)v.v[0]; conv[1] = (int24_t)v.v[1];
		conv[2] = (int24_t)v.v[2]; conv[3] = (int24_t)v.v[3];
		return vectori_nosimd ((const int*)conv);
	}
	inline vector8i_nosimd FASTCALL conv_i32_to_i24 (const vector8i_nosimd& v) noexcept
	{
		int24_t conv[11] = {};
		conv[0] = (int24_t)v.v[0]; conv[1] = (int24_t)v.v[1];
		conv[2] = (int24_t)v.v[2]; conv[3] = (int24_t)v.v[3];
		conv[4] = (int24_t)v.v[4]; conv[5] = (int24_t)v.v[5];
		conv[6] = (int24_t)v.v[6]; conv[7] = (int24_t)v.v[7];
		return vector8i_nosimd ((const int*)conv);
	}
	inline vectori_nosimd FASTCALL conv_i32_to_i16 (const vectori_nosimd& v) noexcept
	{
		short conv[8] = {};
		conv[0] = (short)v.v[0]; conv[1] = (short)v.v[1];
		conv[2] = (short)v.v[2]; conv[3] = (short)v.v[3];
		return vectori_nosimd ((const int*)conv);
	}
	inline vector8i_nosimd FASTCALL conv_i32_to_i16 (const vector8i_nosimd& v) noexcept
	{
		short conv[16] = {};
		conv[0] = (short)v.v[0]; conv[1] = (short)v.v[1];
		conv[2] = (short)v.v[2]; conv[3] = (short)v.v[3];
		conv[4] = (short)v.v[4]; conv[5] = (short)v.v[5];
		conv[6] = (short)v.v[6]; conv[7] = (short)v.v[7];
		return vector8i_nosimd ((const int*)conv);
	}
	inline vectori_nosimd FASTCALL conv_i32_to_i8 (const vectori_nosimd& v) noexcept
	{
		char conv[16] = {};
		conv[0] = (char)v.v[0]; conv[1] = (char)v.v[1];
		conv[2] = (char)v.v[2]; conv[3] = (char)v.v[3];
		return vectori_nosimd ((const int*)conv);
	}
	inline vector8i_nosimd FASTCALL conv_i32_to_i8 (const vector8i_nosimd& v) noexcept
	{
		char conv[32] = {};
		conv[0] = (char)v.v[0]; conv[1] = (char)v.v[1];
		conv[2] = (char)v.v[2]; conv[3] = (char)v.v[3];
		conv[4] = (char)v.v[4]; conv[5] = (char)v.v[5];
		conv[6] = (char)v.v[6]; conv[7] = (char)v.v[7];
		return vector8i_nosimd ((const int*)conv);
	}

	inline vectori_nosimd FASTCALL conv_i24_to_i32 (const vectori_nosimd& v) noexcept
	{
		const int24_t* origin = (const int24_t*)v.v;
		int32_t conv[4] = {};
		conv[0] = (int32_t)origin[0]; conv[1] = (int32_t)origin[1];
		conv[2] = (int32_t)origin[2]; conv[3] = (int32_t)origin[3];
		return vectori_nosimd ((const int*)conv);
	}
	inline vector8i_nosimd FASTCALL conv_i24_to_i32 (const vector8i_nosimd& v) noexcept
	{
		const int24_t* origin = (const int24_t*)v.v;
		int32_t conv[8] = {};
		conv[0] = (int32_t)origin[0]; conv[1] = (int32_t)origin[1];
		conv[2] = (int32_t)origin[2]; conv[3] = (int32_t)origin[3];
		conv[4] = (int32_t)origin[4]; conv[5] = (int32_t)origin[5];
		conv[6] = (int32_t)origin[6]; conv[7] = (int32_t)origin[7];
		return vector8i_nosimd ((const int*)conv);
	}
	inline vectori_nosimd FASTCALL conv_i24_to_i16 (const vectori_nosimd& v) noexcept
	{
		const int24_t* origin = (const int24_t*)v.v;
		short conv[8] = {};
		conv[0] = (short)(int)origin[0]; conv[1] = (short)(int)origin[1]; conv[2] = (short)(int)origin[2];
		conv[3] = (short)(int)origin[3];
		return vectori_nosimd ((const int*)conv);
	}
	inline vector8i_nosimd FASTCALL conv_i24_to_i16 (const vector8i_nosimd& v) noexcept
	{
		const int24_t* origin = (const int24_t*)v.v;
		short conv[16] = {};
		conv[0] = (short)(int)v.v[0]; conv[1] = (short)(int)v.v[1];
		conv[2] = (short)(int)v.v[2]; conv[3] = (short)(int)v.v[3];
		conv[4] = (short)(int)v.v[4]; conv[5] = (short)(int)v.v[5];
		conv[6] = (short)(int)v.v[6]; conv[7] = (short)(int)v.v[7];
		return vector8i_nosimd ((const int*)conv);
	}
	inline vectori_nosimd FASTCALL conv_i24_to_i8 (const vectori_nosimd& v) noexcept
	{
		const int24_t* origin = (const int24_t*)v.v;
		char conv[16] = {};
		conv[0] = (char)(int)origin[0]; conv[1] = (char)(int)origin[1];
		conv[2] = (char)(int)origin[2]; conv[3] = (char)(int)origin[3];
		return vectori_nosimd ((const int*)conv);
	}
	inline vector8i_nosimd FASTCALL conv_i24_to_i8 (const vector8i_nosimd& v) noexcept
	{
		const int24_t* origin = (const int24_t*)v.v;
		char conv[32] = {};
		conv[0] = (char)(int)origin[0]; conv[1] = (char)(int)origin[1];
		conv[2] = (char)(int)origin[2]; conv[3] = (char)(int)origin[3];
		conv[4] = (char)(int)origin[4]; conv[5] = (char)(int)origin[5];
		conv[6] = (char)(int)origin[6]; conv[7] = (char)(int)origin[7];
		return vector8i_nosimd ((const int*)conv);
	}

	inline vectori_nosimd FASTCALL conv_i16_to_i32 (const vectori_nosimd& v) noexcept
	{
		const int16_t* origin = (const int16_t*)v.v;
		int conv[4] = {};
		conv[0] = (int)origin[0]; conv[1] = (int)origin[1];
		conv[2] = (int)origin[2]; conv[3] = (int)origin[3];
		return vectori_nosimd ((const int*)conv);
	}
	inline vector8i_nosimd FASTCALL conv_i16_to_i32 (const vector8i_nosimd& v) noexcept
	{
		const int16_t* origin = (const int16_t*)v.v;
		int conv[8] = {};
		conv[0] = (int)origin[0]; conv[1] = (int)origin[1];
		conv[2] = (int)origin[2]; conv[3] = (int)origin[3];
		conv[4] = (int)origin[4]; conv[5] = (int)origin[5];
		conv[6] = (int)origin[6]; conv[7] = (int)origin[7];
		return vector8i_nosimd ((const int*)conv);
	}
	inline vectori_nosimd FASTCALL conv_i16_to_i24 (const vectori_nosimd& v) noexcept
	{
		const int16_t* origin = (const int16_t*)v.v;
		int24_t conv[6] = {};
		conv[0] = (int24_t)origin[0]; conv[1] = (int24_t)origin[1];
		conv[2] = (int24_t)origin[2]; conv[3] = (int24_t)origin[3];
		conv[4] = (int24_t)origin[4];
		return vectori_nosimd ((const int*)conv);
	}
	inline vector8i_nosimd FASTCALL conv_i16_to_i24 (const vector8i_nosimd& v) noexcept
	{
		const int16_t* origin = (const int16_t*)v.v;
		int24_t conv[11] = {};
		conv[0] = (int24_t)origin[0]; conv[1] = (int24_t)origin[1];
		conv[2] = (int24_t)origin[2]; conv[3] = (int24_t)origin[3];
		conv[4] = (int24_t)origin[4]; conv[5] = (int24_t)origin[5];
		conv[6] = (int24_t)origin[6]; conv[7] = (int24_t)origin[7];
		return vector8i_nosimd ((const int*)conv);
	}
	inline vectori_nosimd FASTCALL conv_i16_to_i8 (const vectori_nosimd& v) noexcept
	{
		const int16_t* origin = (const int16_t*)v.v;
		char conv[16] = {};
		conv[0] = (char)origin[0]; conv[1] = (char)origin[1];
		conv[2] = (char)origin[2]; conv[3] = (char)origin[3];
		conv[4] = (char)origin[4]; conv[5] = (char)origin[5];
		conv[6] = (char)origin[6]; conv[7] = (char)origin[7];
		return vectori_nosimd ((const int*)conv);
	}
	inline vector8i_nosimd FASTCALL conv_i16_to_i8 (const vector8i_nosimd& v) noexcept
	{
		const int16_t* origin = (const int16_t*)v.v;
		char conv[32] = {};
		conv[0] = (char)origin[0]; conv[1] = (char)origin[1];
		conv[2] = (char)origin[2]; conv[3] = (char)origin[3];
		conv[4] = (char)origin[4]; conv[5] = (char)origin[5];
		conv[6] = (char)origin[6]; conv[7] = (char)origin[7];
		conv[8] = (char)origin[8]; conv[9] = (char)origin[9];
		conv[10] = (char)origin[10]; conv[11] = (char)origin[11];
		conv[12] = (char)origin[12]; conv[13] = (char)origin[13];
		conv[14] = (char)origin[14]; conv[15] = (char)origin[15];
		return vector8i_nosimd ((const int*)conv);
	}

	inline vectori_nosimd FASTCALL conv_i8_to_i32 (const vectori_nosimd& v) noexcept
	{
		const int8_t* origin = (const int8_t*)v.v;
		int conv[4] = {};
		conv[0] = (int)origin[0]; conv[1] = (int)origin[1];
		conv[2] = (int)origin[2]; conv[3] = (int)origin[3];
		return vectori_nosimd ((const int*)conv);
	}
	inline vector8i_nosimd FASTCALL conv_i8_to_i32 (const vector8i_nosimd& v) noexcept
	{
		const int8_t* origin = (const int8_t*)v.v;
		int conv[8] = {};
		conv[0] = (int)origin[0]; conv[1] = (int)origin[1];
		conv[2] = (int)origin[2]; conv[3] = (int)origin[3];
		conv[4] = (int)origin[4]; conv[5] = (int)origin[5];
		conv[6] = (int)origin[6]; conv[7] = (int)origin[7];
		return vector8i_nosimd ((const int*)conv);
	}
	inline vectori_nosimd FASTCALL conv_i8_to_i24 (const vectori_nosimd& v) noexcept
	{
		const int8_t* origin = (const int8_t*)v.v;
		int24_t conv[6] = {};
		conv[0] = (int24_t)origin[0]; conv[1] = (int24_t)origin[1];
		conv[2] = (int24_t)origin[2]; conv[3] = (int24_t)origin[3];
		conv[4] = (int24_t)origin[4];
		return vectori_nosimd ((const int*)conv);
	}
	inline vector8i_nosimd FASTCALL conv_i8_to_i24 (const vector8i_nosimd& v) noexcept
	{
		const int8_t* origin = (const int8_t*)v.v;
		int24_t conv[11] = {};
		conv[0] = (int24_t)origin[0]; conv[1] = (int24_t)origin[1];
		conv[2] = (int24_t)origin[2]; conv[3] = (int24_t)origin[3];
		conv[4] = (int24_t)origin[4]; conv[5] = (int24_t)origin[5];
		conv[6] = (int24_t)origin[6]; conv[7] = (int24_t)origin[7];
		return vector8i_nosimd ((const int*)conv);
	}
	inline vectori_nosimd FASTCALL conv_i8_to_i16 (const vectori_nosimd& v) noexcept
	{
		const int8_t* origin = (const int8_t*)v.v;
		short conv[8] = {};
		conv[0] = (char)origin[0]; conv[1] = (char)origin[1];
		conv[2] = (char)origin[2]; conv[3] = (char)origin[3];
		conv[4] = (char)origin[4]; conv[5] = (char)origin[5];
		conv[6] = (char)origin[6]; conv[7] = (char)origin[7];
		return vectori_nosimd ((const int*)conv);
	}
	inline vector8i_nosimd FASTCALL conv_i8_to_i16 (const vector8i_nosimd& v) noexcept
	{
		const int8_t* origin = (const int8_t*)v.v;
		short conv[16] = {};
		conv[0] = (char)origin[0]; conv[1] = (char)origin[1];
		conv[2] = (char)origin[2]; conv[3] = (char)origin[3];
		conv[4] = (char)origin[4]; conv[5] = (char)origin[5];
		conv[6] = (char)origin[6]; conv[7] = (char)origin[7];
		conv[8] = (char)origin[8]; conv[9] = (char)origin[9];
		conv[10] = (char)origin[10]; conv[11] = (char)origin[11];
		conv[12] = (char)origin[12]; conv[13] = (char)origin[13];
		conv[14] = (char)origin[14]; conv[15] = (char)origin[15];
		return vector8i_nosimd ((const int*)conv);
	}

	inline int movemaskvf (const vectorf_nosimd& v) noexcept
	{
		auto ia = reinterpret_cast<const uint32_t*> (v.v);
		return (ia[0] >> 31) | ((ia[1] >> 30) & 2) | ((ia[2] >> 29) & 4) | ((ia[3] >> 28) & 8);
	}
	inline int movemaskvi (const vectori_nosimd& v) noexcept
	{
		auto ia = reinterpret_cast<const uint8_t*> (v.v);
		return (ia[0] >> 31)
			| ((ia[1] >> 30) & 2) | ((ia[2] >> 29) & 4) | ((ia[3] >> 28) & 8)
			| ((ia[4] >> 27) & 16) | ((ia[5] >> 26) & 32) | ((ia[6] >> 25) & 64)
			| ((ia[7] >> 24) & 128) | ((ia[8] >> 23) & 256) | ((ia[9] >> 22) & 512)
			| ((ia[10] >> 21) & 1024) | ((ia[11] >> 20) & 2048) | ((ia[12] >> 19) & 4096)
			| ((ia[13] >> 18) & 8192) | ((ia[14] >> 17) & 16384) | ((ia[15] >> 16) & 32768);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Implementation operators for 128-bit Floating-point Vector
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline vectorf_nosimd FASTCALL addvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept
	{
		return vectorf_nosimd (v1.v[0] + v2.v[0], v1.v[1] + v2.v[1], v1.v[2] + v2.v[2], v1.v[3] + v2.v[3]);
	}
	inline vectorf_nosimd FASTCALL subtractvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept
	{
		return vectorf_nosimd (v1.v[0] - v2.v[0], v1.v[1] - v2.v[1], v1.v[2] - v2.v[2], v1.v[3] - v2.v[3]);
	}
	inline vectorf_nosimd FASTCALL negatevf (const vectorf_nosimd& v) noexcept
	{
		return vectorf_nosimd (-v.v[0], -v.v[1], -v.v[2], -v.v[3]);
	}
	inline vectorf_nosimd FASTCALL multiplyvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept
	{
		return vectorf_nosimd (v1.v[0] * v2.v[0], v1.v[1] * v2.v[1], v1.v[2] * v2.v[2], v1.v[3] * v2.v[3]);
	}
	inline vectorf_nosimd FASTCALL multiplyvf (const vectorf_nosimd& v, float s) noexcept
	{
		return vectorf_nosimd (v.v[0] * s, v.v[1] * s, v.v[2] * s, v.v[3] * s);
	}
	inline vectorf_nosimd FASTCALL multiplyvf (float s, const vectorf_nosimd& v) noexcept
	{
		return multiplyvf(v, s);
	}
	inline vectorf_nosimd FASTCALL dividevf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept
	{
		return vectorf_nosimd (v1.v[0] / v2.v[0], v1.v[1] / v2.v[1], v1.v[2] / v2.v[2], v1.v[3] / v2.v[3]);
	}
	inline vectorf_nosimd FASTCALL dividevf (const vectorf_nosimd& v, float s) noexcept
	{
		return vectorf_nosimd (v.v[0] / s, v.v[1] / s, v.v[2] / s, v.v[3] / s);
	}
	inline vectorf_nosimd FASTCALL fmavf (const vectorf_nosimd& mv1, const vectorf_nosimd& mv2, const vectorf_nosimd& av) noexcept
	{
		return addvf(multiplyvf(mv1, mv2), av);
	}
	inline vectorf_nosimd FASTCALL fmsvf (const vectorf_nosimd& mv1, const vectorf_nosimd& mv2, const vectorf_nosimd& sv) noexcept
	{
		return subtractvf(multiplyvf(mv1, mv2), sv);
	}
	inline vectorf_nosimd FASTCALL fnmsvf (const vectorf_nosimd& sv, const vectorf_nosimd& mv1, const vectorf_nosimd& mv2) noexcept
	{
		return subtractvf (sv, multiplyvf (mv1, mv2));
	}
	inline vectorf_nosimd FASTCALL andvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (andvi (reinterpret_f32_to_i32 (v1), reinterpret_f32_to_i32 (v2)));
	}
	inline vectorf_nosimd FASTCALL orvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (orvi (reinterpret_f32_to_i32 (v1), reinterpret_f32_to_i32 (v2)));
	}
	inline vectorf_nosimd FASTCALL xorvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (xorvi (reinterpret_f32_to_i32 (v1), reinterpret_f32_to_i32 (v2)));
	}
	inline vectorf_nosimd FASTCALL notvf (const vectorf_nosimd& v) noexcept
	{
		return reinterpret_i32_to_f32 (notvi (reinterpret_f32_to_i32 (v)));
	}
	inline vectorf_nosimd FASTCALL shiftlvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (shiftlvi (reinterpret_f32_to_i32 (v1), reinterpret_f32_to_i32 (v2)));
	}
	inline vectorf_nosimd FASTCALL shiftrvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (shiftrvi (reinterpret_f32_to_i32 (v1), reinterpret_f32_to_i32 (v2)));
	}
	inline vectorf_nosimd FASTCALL equalsvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (vectori_nosimd (
			v1.x () == v2.x () ? -1 : 0,
			v1.y () == v2.y () ? -1 : 0,
			v1.z () == v2.z () ? -1 : 0,
			v1.w () == v2.w () ? -1 : 0));
	}
	inline vectorf_nosimd FASTCALL not_equalsvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (vectori_nosimd (
			v1.x () != v2.x () ? -1 : 0,
			v1.y () != v2.y () ? -1 : 0,
			v1.z () != v2.z () ? -1 : 0,
			v1.w () != v2.w () ? -1 : 0));
	}
	inline vectorf_nosimd FASTCALL lesservf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (vectori_nosimd (
			v1.x () < v2.x () ? -1 : 0,
			v1.y () < v2.y () ? -1 : 0,
			v1.z () < v2.z () ? -1 : 0,
			v1.w () < v2.w () ? -1 : 0));
	}
	inline vectorf_nosimd FASTCALL lesser_equalsvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (vectori_nosimd (
			v1.x () <= v2.x () ? -1 : 0,
			v1.y () <= v2.y () ? -1 : 0,
			v1.z () <= v2.z () ? -1 : 0,
			v1.w () <= v2.w () ? -1 : 0));
	}
	inline vectorf_nosimd FASTCALL greatervf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (vectori_nosimd (
			v1.x () > v2.x () ? -1 : 0,
			v1.y () > v2.y () ? -1 : 0,
			v1.z () > v2.z () ? -1 : 0,
			v1.w () > v2.w () ? -1 : 0));
	}
	inline vectorf_nosimd FASTCALL greater_equalsvf (const vectorf_nosimd& v1, const vectorf_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (vectori_nosimd (
			v1.x () >= v2.x () ? -1 : 0,
			v1.y () >= v2.y () ? -1 : 0,
			v1.z () >= v2.z () ? -1 : 0,
			v1.w () >= v2.w () ? -1 : 0));
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Implementation operators for 128-bit Signed Integer Vector
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline vectori_nosimd FASTCALL addvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept
	{
		return vectori_nosimd (v1.v[0] + v2.v[0], v1.v[1] + v2.v[1], v1.v[2] + v2.v[2], v1.v[3] + v2.v[3]);
	}
	inline vectori_nosimd FASTCALL subtractvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept
	{
		return vectori_nosimd (v1.v[0] - v2.v[0], v1.v[1] - v2.v[1], v1.v[2] - v2.v[2], v1.v[3] - v2.v[3]);
	}
	inline vectori_nosimd FASTCALL negatevi (const vectori_nosimd& v) noexcept
	{
		return vectori_nosimd (-v.v[0], -v.v[1], -v.v[2], -v.v[3]);
	}
	inline vectori_nosimd FASTCALL multiplyvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept
	{
		return vectori_nosimd (v1.v[0] * v2.v[0], v1.v[1] * v2.v[1], v1.v[2] * v2.v[2], v1.v[3] * v2.v[3]);
	}
	inline vectori_nosimd FASTCALL multiplyvi (const vectori_nosimd& v, float s) noexcept
	{
		return vectori_nosimd (v.v[0] * s, v.v[1] * s, v.v[2] * s, v.v[3] * s);
	}
	inline vectori_nosimd FASTCALL multiplyvi (float s, const vectori_nosimd& v) noexcept
	{
		return multiplyvi (v, s);
	}
	inline vectori_nosimd FASTCALL dividevi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept
	{
		return vectori_nosimd (v1.v[0] / v2.v[0], v1.v[1] / v2.v[1], v1.v[2] / v2.v[2], v1.v[3] / v2.v[3]);
	}
	inline vectori_nosimd FASTCALL dividevi (const vectori_nosimd& v, float s) noexcept
	{
		return vectori_nosimd (v.v[0] / s, v.v[1] / s, v.v[2] / s, v.v[3] / s);
	}
	inline vectori_nosimd FASTCALL fmavi (const vectori_nosimd& mv1, const vectori_nosimd& mv2, const vectori_nosimd& av) noexcept
	{
		return addvi (multiplyvi (mv1, mv2), av);
	}
	inline vectori_nosimd FASTCALL fmsvi (const vectori_nosimd& mv1, const vectori_nosimd& mv2, const vectori_nosimd& sv) noexcept
	{
		return subtractvi (multiplyvi (mv1, mv2), sv);
	}
	inline vectori_nosimd FASTCALL fnmsvi (const vectori_nosimd& sv, const vectori_nosimd& mv1, const vectori_nosimd& mv2) noexcept
	{
		return subtractvi (sv, multiplyvi (mv1, mv2));
	}
	inline vectori_nosimd FASTCALL andvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept
	{
		return vectori_nosimd (v1.v[0] & v2.v[0], v1.v[1] & v2.v[1], v1.v[2] & v2.v[2], v1.v[3] & v2.v[3]);
	}
	inline vectori_nosimd FASTCALL orvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept
	{
		return vectori_nosimd (v1.v[0] | v2.v[0], v1.v[1] | v2.v[1], v1.v[2] | v2.v[2], v1.v[3] | v2.v[3]);
	}
	inline vectori_nosimd FASTCALL xorvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept
	{
		return vectori_nosimd (v1.v[0] ^ v2.v[0], v1.v[1] ^ v2.v[1], v1.v[2] ^ v2.v[2], v1.v[3] ^ v2.v[3]);
	}
	inline vectori_nosimd FASTCALL shiftlvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept
	{
		return vectori_nosimd (v1.v[0] << v2.v[0], v1.v[1] << v2.v[1], v1.v[2] << v2.v[2], v1.v[3] << v2.v[3]);
	}
	inline vectori_nosimd FASTCALL shiftrvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept
	{
		return vectori_nosimd (v1.v[0] >> v2.v[0], v1.v[1] >> v2.v[1], v1.v[2] >> v2.v[2], v1.v[3] >> v2.v[3]);
	}
	inline vectori_nosimd FASTCALL notvi (const vectori_nosimd& v) noexcept
	{
		return vectori_nosimd (~v.v[0], ~v.v[1], ~v.v[2], ~v.v[3]);
	}
	inline vectori_nosimd FASTCALL equalsvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept
	{
		return vectori_nosimd (
			v1.x () == v2.x () ? -1 : 0,
			v1.y () == v2.y () ? -1 : 0,
			v1.z () == v2.z () ? -1 : 0,
			v1.w () == v2.w () ? -1 : 0
		);
	}
	inline vectori_nosimd FASTCALL not_equalsvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept
	{
		return vectori_nosimd (
			v1.x () != v2.x () ? -1 : 0,
			v1.y () != v2.y () ? -1 : 0,
			v1.z () != v2.z () ? -1 : 0,
			v1.w () != v2.w () ? -1 : 0
		);
	}
	inline vectori_nosimd FASTCALL lesservi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept
	{
		return vectori_nosimd (
			v1.x () < v2.x () ? -1 : 0,
			v1.y () < v2.y () ? -1 : 0,
			v1.z () < v2.z () ? -1 : 0,
			v1.w () < v2.w () ? -1 : 0
		);
	}
	inline vectori_nosimd FASTCALL lesser_equalsvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept
	{
		return vectori_nosimd (
			v1.x () <= v2.x () ? -1 : 0,
			v1.y () <= v2.y () ? -1 : 0,
			v1.z () <= v2.z () ? -1 : 0,
			v1.w () <= v2.w () ? -1 : 0
		);
	}
	inline vectori_nosimd FASTCALL greatervi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept
	{
		return vectori_nosimd (
			v1.x () > v2.x () ? -1 : 0,
			v1.y () > v2.y () ? -1 : 0,
			v1.z () > v2.z () ? -1 : 0,
			v1.w () > v2.w () ? -1 : 0
		);
	}
	inline vectori_nosimd FASTCALL greater_equalsvi (const vectori_nosimd& v1, const vectori_nosimd& v2) noexcept
	{
		return vectori_nosimd (
			v1.x () >= v2.x () ? -1 : 0,
			v1.y () >= v2.y () ? -1 : 0,
			v1.z () >= v2.z () ? -1 : 0,
			v1.w () >= v2.w () ? -1 : 0
		);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Implementation operators for 256-bit Floating-point Vector
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline vector8f_nosimd FASTCALL addv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept
	{
		return vector8f_nosimd (
			v1.v[0] + v2.v[0], v1.v[1] + v2.v[1], v1.v[2] + v2.v[2], v1.v[3] + v2.v[3],
			v1.v[4] + v2.v[4], v1.v[5] + v2.v[5], v1.v[6] + v2.v[6], v1.v[7] + v2.v[7]
		);
	}
	inline vector8f_nosimd FASTCALL subtractv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept
	{
		return vector8f_nosimd (
			v1.v[0] - v2.v[0], v1.v[1] - v2.v[1], v1.v[2] - v2.v[2], v1.v[3] - v2.v[3],
			v1.v[4] - v2.v[4], v1.v[5] - v2.v[5], v1.v[6] - v2.v[6], v1.v[7] - v2.v[7]
		);
	}
	inline vector8f_nosimd FASTCALL negatev8f (const vector8f_nosimd& v) noexcept
	{
		return vector8f_nosimd (-v.v[0], -v.v[1], -v.v[2], -v.v[3], -v.v[4], -v.v[5], -v.v[6], -v.v[7]);
	}
	inline vector8f_nosimd FASTCALL multiplyv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept
	{
		return vector8f_nosimd (
			v1.v[0] * v2.v[0], v1.v[1] * v2.v[1], v1.v[2] * v2.v[2], v1.v[3] * v2.v[3],
			v1.v[4] * v2.v[4], v1.v[5] * v2.v[5], v1.v[6] * v2.v[6], v1.v[7] * v2.v[7]
		);
	}
	inline vector8f_nosimd FASTCALL multiplyv8f (const vector8f_nosimd& v, float s) noexcept
	{
		return vector8f_nosimd (
			v.v[0] * s, v.v[1] * s, v.v[2] * s, v.v[3] * s,
			v.v[4] * s, v.v[5] * s, v.v[6] * s, v.v[7] * s
		);
	}
	inline vector8f_nosimd FASTCALL multiplyv8f (float s, const vector8f_nosimd& v) noexcept
	{
		return multiplyv8f (v, s);
	}
	inline vector8f_nosimd FASTCALL dividev8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept
	{
		return vector8f_nosimd (
			v1.v[0] / v2.v[0], v1.v[1] / v2.v[1], v1.v[2] / v2.v[2], v1.v[3] / v2.v[3],
			v1.v[4] / v2.v[4], v1.v[5] / v2.v[5], v1.v[6] / v2.v[6], v1.v[7] / v2.v[7]
		);
	}
	inline vector8f_nosimd FASTCALL dividev8f (const vector8f_nosimd& v, float s) noexcept
	{
		return vector8f_nosimd (
			v.v[0] / s, v.v[1] / s, v.v[2] / s, v.v[3] / s,
			v.v[4] / s, v.v[5] / s, v.v[6] / s, v.v[7] / s
		);
	}
	inline vector8f_nosimd FASTCALL fmav8f (const vector8f_nosimd& mv1, const vector8f_nosimd& mv2, const vector8f_nosimd& av) noexcept
	{
		return addv8f (multiplyv8f (mv1, mv2), av);
	}
	inline vector8f_nosimd FASTCALL fmsv8f (const vector8f_nosimd& mv1, const vector8f_nosimd& mv2, const vector8f_nosimd& sv) noexcept
	{
		return subtractv8f (multiplyv8f (mv1, mv2), sv);
	}
	inline vector8f_nosimd FASTCALL fnmsv8f (const vector8f_nosimd& sv, const vector8f_nosimd& mv1, const vector8f_nosimd& mv2) noexcept
	{
		return subtractv8f (sv, multiplyv8f (mv1, mv2));
	}
	inline vector8f_nosimd FASTCALL andv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (andv8i (reinterpret_f32_to_i32 (v1), reinterpret_f32_to_i32 (v2)));
	}
	inline vector8f_nosimd FASTCALL orv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (orv8i (reinterpret_f32_to_i32 (v1), reinterpret_f32_to_i32 (v2)));
	}
	inline vector8f_nosimd FASTCALL xorv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (xorv8i (reinterpret_f32_to_i32 (v1), reinterpret_f32_to_i32 (v2)));
	}
	inline vector8f_nosimd FASTCALL notv8f (const vector8f_nosimd& v) noexcept
	{
		return reinterpret_i32_to_f32 (notv8i (reinterpret_f32_to_i32 (v)));
	}
	inline vector8f_nosimd FASTCALL shiftlv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (shiftlv8i (reinterpret_f32_to_i32 (v1), reinterpret_f32_to_i32 (v2)));
	}
	inline vector8f_nosimd FASTCALL shiftrv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (shiftrv8i (reinterpret_f32_to_i32 (v1), reinterpret_f32_to_i32 (v2)));
	}
	inline vector8f_nosimd FASTCALL equalsv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (vector8i_nosimd (
			v1.x1 () == v2.x1 () ? -1 : 0,
			v1.y1 () == v2.y1 () ? -1 : 0,
			v1.z1 () == v2.z1 () ? -1 : 0,
			v1.w1 () == v2.w1 () ? -1 : 0,
			v1.x2 () == v2.x2 () ? -1 : 0,
			v1.y2 () == v2.y2 () ? -1 : 0,
			v1.z2 () == v2.z2 () ? -1 : 0,
			v1.w2 () == v2.w2 () ? -1 : 0));
	}
	inline vector8f_nosimd FASTCALL not_equalsv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (vector8i_nosimd (
			v1.x1 () != v2.x1 () ? -1 : 0,
			v1.y1 () != v2.y1 () ? -1 : 0,
			v1.z1 () != v2.z1 () ? -1 : 0,
			v1.w1 () != v2.w1 () ? -1 : 0,
			v1.x2 () != v2.x2 () ? -1 : 0,
			v1.y2 () != v2.y2 () ? -1 : 0,
			v1.z2 () != v2.z2 () ? -1 : 0,
			v1.w2 () != v2.w2 () ? -1 : 0));
	}
	inline vector8f_nosimd FASTCALL lesserv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (vector8i_nosimd (
			v1.x1 () < v2.x1 () ? -1 : 0,
			v1.y1 () < v2.y1 () ? -1 : 0,
			v1.z1 () < v2.z1 () ? -1 : 0,
			v1.w1 () < v2.w1 () ? -1 : 0,
			v1.x2 () < v2.x2 () ? -1 : 0,
			v1.y2 () < v2.y2 () ? -1 : 0,
			v1.z2 () < v2.z2 () ? -1 : 0,
			v1.w2 () < v2.w2 () ? -1 : 0));
	}
	inline vector8f_nosimd FASTCALL lesser_equalsv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (vector8i_nosimd (
			v1.x1 () <= v2.x1 () ? -1 : 0,
			v1.y1 () <= v2.y1 () ? -1 : 0,
			v1.z1 () <= v2.z1 () ? -1 : 0,
			v1.w1 () <= v2.w1 () ? -1 : 0,
			v1.x2 () <= v2.x2 () ? -1 : 0,
			v1.y2 () <= v2.y2 () ? -1 : 0,
			v1.z2 () <= v2.z2 () ? -1 : 0,
			v1.w2 () <= v2.w2 () ? -1 : 0));
	}
	inline vector8f_nosimd FASTCALL greaterv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (vector8i_nosimd (
			v1.x1 () > v2.x1 () ? -1 : 0,
			v1.y1 () > v2.y1 () ? -1 : 0,
			v1.z1 () > v2.z1 () ? -1 : 0,
			v1.w1 () > v2.w1 () ? -1 : 0,
			v1.x2 () > v2.x2 () ? -1 : 0,
			v1.y2 () > v2.y2 () ? -1 : 0,
			v1.z2 () > v2.z2 () ? -1 : 0,
			v1.w2 () > v2.w2 () ? -1 : 0));
	}
	inline vector8f_nosimd FASTCALL greater_equalsv8f (const vector8f_nosimd& v1, const vector8f_nosimd& v2) noexcept
	{
		return reinterpret_i32_to_f32 (vector8i_nosimd (
			v1.x1 () >= v2.x1 () ? -1 : 0,
			v1.y1 () >= v2.y1 () ? -1 : 0,
			v1.z1 () >= v2.z1 () ? -1 : 0,
			v1.w1 () >= v2.w1 () ? -1 : 0,
			v1.x2 () >= v2.x2 () ? -1 : 0,
			v1.y2 () >= v2.y2 () ? -1 : 0,
			v1.z2 () >= v2.z2 () ? -1 : 0,
			v1.w2 () >= v2.w2 () ? -1 : 0));
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Implementation operators for 256-bit Signed Integer Vector
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline vector8i_nosimd FASTCALL addv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept
	{
		return vector8i_nosimd (
			v1.v[0] + v2.v[0], v1.v[1] + v2.v[1], v1.v[2] + v2.v[2], v1.v[3] + v2.v[3],
			v1.v[4] + v2.v[4], v1.v[5] + v2.v[5], v1.v[6] + v2.v[6], v1.v[7] + v2.v[7]
		);
	}
	inline vector8i_nosimd FASTCALL subtractv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept
	{
		return vector8i_nosimd (
			v1.v[0] - v2.v[0], v1.v[1] - v2.v[1], v1.v[2] - v2.v[2], v1.v[3] - v2.v[3],
			v1.v[4] - v2.v[4], v1.v[5] - v2.v[5], v1.v[6] - v2.v[6], v1.v[7] - v2.v[7]
		);
	}
	inline vector8i_nosimd FASTCALL negatev8i (const vector8i_nosimd& v) noexcept
	{
		return vector8i_nosimd (-v.v[0], -v.v[1], -v.v[2], -v.v[3], -v.v[4], -v.v[5], -v.v[6], -v.v[7]);
	}
	inline vector8i_nosimd FASTCALL multiplyv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept
	{
		return vector8i_nosimd (
			v1.v[0] * v2.v[0], v1.v[1] * v2.v[1], v1.v[2] * v2.v[2], v1.v[3] * v2.v[3],
			v1.v[4] * v2.v[4], v1.v[5] * v2.v[5], v1.v[6] * v2.v[6], v1.v[7] * v2.v[7]
		);
	}
	inline vector8i_nosimd FASTCALL multiplyv8i (const vector8i_nosimd& v, float s) noexcept
	{
		return vector8i_nosimd (
			v.v[0] * s, v.v[1] * s, v.v[2] * s, v.v[3] * s,
			v.v[4] * s, v.v[5] * s, v.v[6] * s, v.v[7] * s
		);
	}
	inline vector8i_nosimd FASTCALL multiplyv8i (float s, const vector8i_nosimd& v) noexcept
	{
		return multiplyv8i (v, s);
	}
	inline vector8i_nosimd FASTCALL dividev8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept
	{
		return vector8i_nosimd (
			v1.v[0] / v2.v[0], v1.v[1] / v2.v[1], v1.v[2] / v2.v[2], v1.v[3] / v2.v[3],
			v1.v[4] / v2.v[4], v1.v[5] / v2.v[5], v1.v[6] / v2.v[6], v1.v[7] / v2.v[7]
		);
	}
	inline vector8i_nosimd FASTCALL dividev8i (const vector8i_nosimd& v, float s) noexcept
	{
		return vector8i_nosimd (
			v.v[0] / s, v.v[1] / s, v.v[2] / s, v.v[3] / s,
			v.v[4] / s, v.v[5] / s, v.v[6] / s, v.v[7] / s
		);
	}
	inline vector8i_nosimd FASTCALL fmav8i (const vector8i_nosimd& mv1, const vector8i_nosimd& mv2, const vector8i_nosimd& av) noexcept
	{
		return addv8i (multiplyv8i (mv1, mv2), av);
	}
	inline vector8i_nosimd FASTCALL fmsv8i (const vector8i_nosimd& mv1, const vector8i_nosimd& mv2, const vector8i_nosimd& sv) noexcept
	{
		return subtractv8i (multiplyv8i (mv1, mv2), sv);
	}
	inline vector8i_nosimd FASTCALL fnmsv8i (const vector8i_nosimd& sv, const vector8i_nosimd& mv1, const vector8i_nosimd& mv2) noexcept
	{
		return subtractv8i (sv, multiplyv8i (mv1, mv2));
	}
	inline vector8i_nosimd FASTCALL andv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept
	{
		return vector8i_nosimd (
			v1.v[0] & v2.v[0], v1.v[1] & v2.v[1], v1.v[2] & v2.v[2], v1.v[3] & v2.v[3],
			v1.v[4] & v2.v[4], v1.v[5] & v2.v[5], v1.v[6] & v2.v[6], v1.v[7] & v2.v[7]
		);
	}
	inline vector8i_nosimd FASTCALL orv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept
	{
		return vector8i_nosimd (
			v1.v[0] | v2.v[0], v1.v[1] | v2.v[1], v1.v[2] | v2.v[2], v1.v[3] | v2.v[3],
			v1.v[4] | v2.v[4], v1.v[5] | v2.v[5], v1.v[6] | v2.v[6], v1.v[7] | v2.v[7]
		);
	}
	inline vector8i_nosimd FASTCALL xorv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept
	{
		return vector8i_nosimd (
			v1.v[0] ^ v2.v[0], v1.v[1] ^ v2.v[1], v1.v[2] ^ v2.v[2], v1.v[3] ^ v2.v[3],
			v1.v[4] ^ v2.v[4], v1.v[5] ^ v2.v[5], v1.v[6] ^ v2.v[6], v1.v[7] ^ v2.v[7]
		);
	}
	inline vector8i_nosimd FASTCALL shiftlv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept
	{
		return vector8i_nosimd (
			v1.v[0] << v2.v[0], v1.v[1] << v2.v[1], v1.v[2] << v2.v[2], v1.v[3] << v2.v[3],
			v1.v[4] << v2.v[4], v1.v[5] << v2.v[5], v1.v[6] << v2.v[6], v1.v[7] << v2.v[7]
		);
	}
	inline vector8i_nosimd FASTCALL shiftrv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept
	{
		return vector8i_nosimd (
			v1.v[0] >> v2.v[0], v1.v[1] >> v2.v[1], v1.v[2] >> v2.v[2], v1.v[3] >> v2.v[3],
			v1.v[4] >> v2.v[4], v1.v[5] >> v2.v[5], v1.v[6] >> v2.v[6], v1.v[7] >> v2.v[7]
		);
	}
	inline vector8i_nosimd FASTCALL notv8i (const vector8i_nosimd& v) noexcept
	{
		return vector8i_nosimd (~v.v[0], ~v.v[1], ~v.v[2], ~v.v[3], ~v.v[4], ~v.v[5], ~v.v[6], ~v.v[7]);
	}
	inline vector8i_nosimd FASTCALL equalsv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept
	{
		return vector8i_nosimd (
			v1.x1 () == v2.x1 () ? -1 : 0,
			v1.y1 () == v2.y1 () ? -1 : 0,
			v1.z1 () == v2.z1 () ? -1 : 0,
			v1.w1 () == v2.w1 () ? -1 : 0,
			v1.x2 () == v2.x2 () ? -1 : 0,
			v1.y2 () == v2.y2 () ? -1 : 0,
			v1.z2 () == v2.z2 () ? -1 : 0,
			v1.w2 () == v2.w2 () ? -1 : 0
		);
	}
	inline vector8i_nosimd FASTCALL not_equalsv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept
	{
		return vector8i_nosimd (
			v1.x1 () != v2.x1 () ? -1 : 0,
			v1.y1 () != v2.y1 () ? -1 : 0,
			v1.z1 () != v2.z1 () ? -1 : 0,
			v1.w1 () != v2.w1 () ? -1 : 0,
			v1.x2 () != v2.x2 () ? -1 : 0,
			v1.y2 () != v2.y2 () ? -1 : 0,
			v1.z2 () != v2.z2 () ? -1 : 0,
			v1.w2 () != v2.w2 () ? -1 : 0
		);
	}
	inline vector8i_nosimd FASTCALL lesserv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept
	{
		return vector8i_nosimd (
			v1.x1 () < v2.x1 () ? -1 : 0,
			v1.y1 () < v2.y1 () ? -1 : 0,
			v1.z1 () < v2.z1 () ? -1 : 0,
			v1.w1 () < v2.w1 () ? -1 : 0,
			v1.x2 () < v2.x2 () ? -1 : 0,
			v1.y2 () < v2.y2 () ? -1 : 0,
			v1.z2 () < v2.z2 () ? -1 : 0,
			v1.w2 () < v2.w2 () ? -1 : 0
		);
	}
	inline vector8i_nosimd FASTCALL lesser_equalsv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept
	{
		return vector8i_nosimd (
			v1.x1 () <= v2.x1 () ? -1 : 0,
			v1.y1 () <= v2.y1 () ? -1 : 0,
			v1.z1 () <= v2.z1 () ? -1 : 0,
			v1.w1 () <= v2.w1 () ? -1 : 0,
			v1.x2 () <= v2.x2 () ? -1 : 0,
			v1.y2 () <= v2.y2 () ? -1 : 0,
			v1.z2 () <= v2.z2 () ? -1 : 0,
			v1.w2 () <= v2.w2 () ? -1 : 0
		);
	}
	inline vector8i_nosimd FASTCALL greaterv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept
	{
		return vector8i_nosimd (
			v1.x1 () > v2.x1 () ? -1 : 0,
			v1.y1 () > v2.y1 () ? -1 : 0,
			v1.z1 () > v2.z1 () ? -1 : 0,
			v1.w1 () > v2.w1 () ? -1 : 0,
			v1.x2 () > v2.x2 () ? -1 : 0,
			v1.y2 () > v2.y2 () ? -1 : 0,
			v1.z2 () > v2.z2 () ? -1 : 0,
			v1.w2 () > v2.w2 () ? -1 : 0
		);
	}
	inline vector8i_nosimd FASTCALL greater_equalsv8i (const vector8i_nosimd& v1, const vector8i_nosimd& v2) noexcept
	{
		return vector8i_nosimd (
			v1.x1 () >= v2.x1 () ? -1 : 0,
			v1.y1 () >= v2.y1 () ? -1 : 0,
			v1.z1 () >= v2.z1 () ? -1 : 0,
			v1.w1 () >= v2.w1 () ? -1 : 0,
			v1.x2 () >= v2.x2 () ? -1 : 0,
			v1.y2 () >= v2.y2 () ? -1 : 0,
			v1.z2 () >= v2.z2 () ? -1 : 0,
			v1.w2 () >= v2.w2 () ? -1 : 0
		);
	}
}

#endif