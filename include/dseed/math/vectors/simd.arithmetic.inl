#pragma warning (disable: 4309)

///////////////////////////////////////////////////////////////////////////////////////////
//
// 4D Vector Arithmetic Operations
//
///////////////////////////////////////////////////////////////////////////////////////////
namespace dseed
{
#if !DONT_USE_SSE
	static inline f32x4_t add(const f32x4_t& v1, const f32x4_t& v2) noexcept { return _mm_add_ps(v1, v2); }
	static inline f32x4_t subtract(const f32x4_t& v1, const f32x4_t& v2) noexcept { return _mm_sub_ps(v1, v2); }
	static inline f32x4_t negate(const f32x4_t& v) noexcept { static auto zero = _mm_set1_ps(0); return subtract(zero, v); }
	static inline f32x4_t multiply(const f32x4_t& v1, const f32x4_t& v2) noexcept { return _mm_mul_ps(v1, v2); }
	static inline f32x4_t multiply(const f32x4_t& v, float s) noexcept { return _mm_mul_ps(v, _mm_set1_ps(s)); }
	static inline f32x4_t fma(const f32x4_t& mv1, const f32x4_t& mv2, const f32x4_t& av) noexcept { return _mm_fmadd_ps(mv1, mv2, av); }
	static inline f32x4_t fms(const f32x4_t& mv1, const f32x4_t& mv2, const f32x4_t& sv) noexcept { return _mm_fmsub_ps(mv1, mv2, sv); }
	static inline f32x4_t fnms(const f32x4_t& sv, const f32x4_t& mv1, const f32x4_t& mv2) noexcept { return _mm_fnmsub_ps(mv1, mv2, sv); }
	static inline f32x4_t divide(const f32x4_t& v1, const f32x4_t& v2) noexcept { return _mm_div_ps(v1, v2); }
	static inline f32x4_t divide(const f32x4_t& v, float s) noexcept { return divide(v, _mm_set1_ps(s)); }

	static inline i32x4_t add32(const i32x4_t& v1, const i32x4_t& v2) noexcept { return _mm_add_epi32(v1, v2); }
	static inline i32x4_t subtract32(const i32x4_t& v1, const i32x4_t& v2) noexcept { return _mm_sub_epi32(v1, v2); }
	static inline i32x4_t negate32(const i32x4_t& v) noexcept { static auto zero = _mm_set1_epi32(0); return subtract32(zero, v); }
	static inline i32x4_t multiply32(const i32x4_t& v1, const i32x4_t& v2) noexcept { return _mm_mul_epi32(v1, v2); }
	static inline i32x4_t multiply32(const i32x4_t& v, int s) noexcept { return _mm_mul_epi32(v, _mm_set1_epi32(s)); }

	static inline i32x4_t add16(const i32x4_t& v1, const i32x4_t& v2) noexcept { return _mm_add_epi16(v1, v2); }
	static inline i32x4_t subtract16(const i32x4_t& v1, const i32x4_t& v2) noexcept { return _mm_sub_epi16(v1, v2); }
	static inline i32x4_t negate16(const i32x4_t& v) noexcept { static auto zero = _mm_set1_epi16(0); return _mm_add_epi16(zero, v); }
	static inline i32x4_t multiply16(const i32x4_t& v1, const i32x4_t& v2) noexcept
	{
		const auto hi = _mm_mulhi_epi16(v1, v2);
		const auto lo = _mm_mullo_epi16(v1, v2);
		return _mm_or_si128(lo, hi);
	}
	static inline i32x4_t multiply16(const i32x4_t& v, int s) noexcept { return multiply16(v, _mm_set1_epi16(s)); }

	static inline i32x4_t add8(const i32x4_t& v1, const i32x4_t& v2) noexcept { return _mm_add_epi8(v1, v2); }
	static inline i32x4_t subtract8(const i32x4_t& v1, const i32x4_t& v2) noexcept { return _mm_sub_epi8(v1, v2); }
	static inline i32x4_t negate8(const i32x4_t& v) noexcept { static auto zero = _mm_set1_epi8(0); return _mm_add_epi8(zero, v); }
	static inline i32x4_t multiply8(const i32x4_t& v1, const i32x4_t& v2) noexcept
	{
		static auto zero = _mm_setzero_si128();
		static auto lo_mask = _mm_set_epi8(0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 14, 12, 10, 8, 6, 4, 2, 0);
		static auto hi_mask = _mm_set_epi8(14, 12, 10, 8, 6, 4, 2, 0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);

		const auto lo = _mm_mullo_epi16(_mm_cvtepi8_epi16(v1), _mm_cvtepi8_epi16(v2));
		const auto hi = _mm_mullo_epi16(_mm_unpackhi_epi8(v1, zero), _mm_unpackhi_epi8(v2, zero));
		return _mm_or_si128(_mm_shuffle_epi8(lo, lo_mask), _mm_shuffle_epi8(hi, hi_mask));
	}
	static inline i32x4_t multiply8(const i32x4_t& v, int s) noexcept { return multiply8(v, _mm_set1_epi8(s)); }
#elif !DONT_USE_NEON
	static inline f32x4_t add(const f32x4_t& v1, const f32x4_t& v2) noexcept { return vaddq_f32(v1, v2); }
	static inline f32x4_t subtract(const f32x4_t& v1, const f32x4_t& v2) noexcept { return vsubq_f32(v1, v2); }
	static inline f32x4_t negate(const f32x4_t& v) noexcept { return vnegq_f32(v); }
	static inline f32x4_t multiply(const f32x4_t& v1, const f32x4_t& v2) noexcept { return vmulq_f32(v1, v2); }
	static inline f32x4_t multiply(const f32x4_t& v, float s) noexcept { return vmulq_f32(v, vmovq_n_f32(s)); }
	static inline f32x4_t fma(const f32x4_t& mv1, const f32x4_t& mv2, const f32x4_t& av) noexcept
	{
#if ARCH_ARM
		return vmlaq_f32(av, mv1, mv2);
#elif ARCH_ARM64
		return vfmaq_f32(av, mv1, mv2);
#endif
	}
	static inline f32x4_t fms(const f32x4_t& mv1, const f32x4_t& mv2, const f32x4_t& sv) noexcept
	{
		return subtract(multiply(mv1, mv2), sv);
	}
	static inline f32x4_t fnms(const f32x4_t& sv, const f32x4_t& mv1, const f32x4_t& mv2) noexcept
	{
#if ARCH_ARM
		return vmlsq_f32(sv, mv1, mv2);
#elif ARCH_ARM64
		return vfmsq_f32(sv, mv1, mv2);
#endif
	}
	static inline f32x4_t divide(const f32x4_t& v1, const f32x4_t& v2) noexcept
	{
		float32x4_t reciprocal = vrecpeq_f32(v2);
		reciprocal = vmulq_f32(vrecpsq_f32(v2, reciprocal), reciprocal);
		reciprocal = vmulq_f32(vrecpsq_f32(v2, reciprocal), reciprocal);
		return vmulq_f32(v1, reciprocal);
	}
	static inline f32x4_t divide(const f32x4_t& v, float s) noexcept { return divide(v, vmovq_n_f32(s)); }

	static inline i32x4_t add32(const i32x4_t& v1, const i32x4_t& v2) noexcept { return vaddq_s32(v1, v2); }
	static inline i32x4_t subtract32(const i32x4_t& v1, const i32x4_t& v2) noexcept { return vsubq_s32(v1, v2); }
	static inline i32x4_t negate32(const i32x4_t& v) noexcept { return vnegq_s32(v); }
	static inline i32x4_t multiply32(const i32x4_t& v1, const i32x4_t& v2) noexcept { return vmulq_s32(v1, v2); }
	static inline i32x4_t multiply32(const i32x4_t& v, int s) noexcept { return vmulq_s32(v, vmovq_n_s32(s)); }

	static inline i32x4_t add16(const i32x4_t& v1, const i32x4_t& v2) noexcept { return vaddq_s16(v1, v2); }
	static inline i32x4_t subtract16(const i32x4_t& v1, const i32x4_t& v2) noexcept { return vsubq_s16(v1, v2); }
	static inline i32x4_t negate16(const i32x4_t& v) noexcept { return vnegq_s16(v); }
	static inline i32x4_t multiply16(const i32x4_t& v1, const i32x4_t& v2) noexcept { return vmulq_s16(v1, v2); }
	static inline i32x4_t multiply16(const i32x4_t& v, int s) noexcept { return vmulq_s16(v, vmovq_n_s16(s)); }

	static inline i32x4_t add8(const i32x4_t& v1, const i32x4_t& v2) noexcept { return vaddq_s8(v1, v2); }
	static inline i32x4_t subtract8(const i32x4_t& v1, const i32x4_t& v2) noexcept { return vsubq_s8(v1, v2); }
	static inline i32x4_t negate8(const i32x4_t& v) noexcept { return vnegq_s8(v); }
	static inline i32x4_t multiply8(const i32x4_t& v1, const i32x4_t& v2) noexcept { return vmulq_s8(v1, v2); }
	static inline i32x4_t multiply8(const i32x4_t& v, int s) noexcept { return vmulq_s8(v, vmovq_n_s8(s)); }
#elif DONT_USE_SIMD
	static inline f32x4_t add(const f32x4_t& v1, const f32x4_t& v2) noexcept { return f32x4_t(v1.x() + v2.x(), v1.y() + v2.y(), v1.z() + v2.z(), v1.w() + v2.w()); }
	static inline f32x4_t subtract(const f32x4_t& v1, const f32x4_t& v2) noexcept { return f32x4_t(v1.x() - v2.x(), v1.y() - v2.y(), v1.z() - v2.z(), v1.w() - v2.w()); }
	static inline f32x4_t negate(const f32x4_t& v) noexcept { return f32x4_t(-v.x(), -v.y(), -v.z(), -v.w()); }
	static inline f32x4_t multiply(const f32x4_t& v1, const f32x4_t& v2) noexcept { return f32x4_t(v1.x() * v2.x(), v1.y() * v2.y(), v1.z() * v2.z(), v1.w() * v2.w()); }
	static inline f32x4_t multiply(const f32x4_t& v, float s) noexcept { return f32x4_t(v.x() * s, v.y() * s, v.z() * s, v.w() * s); }
	static inline f32x4_t fma(const f32x4_t& mv1, const f32x4_t& mv2, const f32x4_t& av) noexcept { return add(multiply(mv1, mv2), av); }
	static inline f32x4_t fms(const f32x4_t& mv1, const f32x4_t& mv2, const f32x4_t& sv) noexcept { return subtract(multiply(mv1, mv2), sv); }
	static inline f32x4_t fnms(const f32x4_t& sv, const f32x4_t& mv1, const f32x4_t& mv2) noexcept { return subtract(sv, multiply(mv1, mv2)); }
	static inline f32x4_t divide(const f32x4_t& v1, const f32x4_t& v2) noexcept { return f32x4_t(v1.x() / v2.x(), v1.y() / v2.y(), v1.z() / v2.z(), v1.w() / v2.w()); }
	static inline f32x4_t divide(const f32x4_t& v, float s) noexcept { return f32x4_t(v.x() / s, v.y() / s, v.z() / s, v.w() / s); }

	static inline i32x4_t add32(const i32x4_t& v1, const i32x4_t& v2) noexcept { return i32x4_t(v1.x() + v2.x(), v1.y() + v2.y(), v1.z() + v2.z(), v1.w() + v2.w()); }
	static inline i32x4_t subtract32(const i32x4_t& v1, const i32x4_t& v2) noexcept { return i32x4_t(v1.x() - v2.x(), v1.y() - v2.y(), v1.z() - v2.z(), v1.w() - v2.w()); }
	static inline i32x4_t negate32(const i32x4_t& v) noexcept { return i32x4_t(-v.x(), -v.y(), -v.z(), -v.w()); }
	static inline i32x4_t multiply32(const i32x4_t& v1, const i32x4_t& v2) noexcept { return i32x4_t(v1.x() * v2.x(), v1.y() * v2.y(), v1.z() * v2.z(), v1.w() * v2.w()); }
	static inline i32x4_t multiply32(const i32x4_t& v, int s) noexcept { return i32x4_t(v.x() * s, v.y() * s, v.z() * s, v.w() * s); }

	static inline i32x4_t add16(const i32x4_t& v1, const i32x4_t& v2) noexcept
	{
		const short* arr1 = (const short*)&v1;
		const short* arr2 = (const short*)&v2;
		short result[8] = {
			arr1[0] + arr2[0],
			arr1[1] + arr2[1],
			arr1[2] + arr2[2],
			arr1[3] + arr2[3],
			arr1[4] + arr2[4],
			arr1[5] + arr2[5],
			arr1[6] + arr2[6],
			arr1[7] + arr2[7],
		};
		return i32x4_t(result);
	}
	static inline i32x4_t subtract16(const i32x4_t& v1, const i32x4_t& v2) noexcept
	{
		const short* arr1 = (const short*)&v1;
		const short* arr2 = (const short*)&v2;
		short result[8] = {
			arr1[0] - arr2[0],
			arr1[1] - arr2[1],
			arr1[2] - arr2[2],
			arr1[3] - arr2[3],
			arr1[4] - arr2[4],
			arr1[5] - arr2[5],
			arr1[6] - arr2[6],
			arr1[7] - arr2[7],
		};
		return i32x4_t(result);
	}
	static inline i32x4_t negate16(const i32x4_t& v) noexcept
	{
		const short* arr = (const short*)&v;
		short result[8] = {
			-arr[0],
			-arr[1],
			-arr[2],
			-arr[3],
			-arr[4],
			-arr[5],
			-arr[6],
			-arr[7],
		};
		return i32x4_t(result);
	}
	static inline i32x4_t multiply16(const i32x4_t& v1, const i32x4_t& v2) noexcept
	{
		const short* arr1 = (const short*)&v1;
		const short* arr2 = (const short*)&v2;
		short result[8] = {
			arr1[0] * arr2[0],
			arr1[1] * arr2[1],
			arr1[2] * arr2[2],
			arr1[3] * arr2[3],
			arr1[4] * arr2[4],
			arr1[5] * arr2[5],
			arr1[6] * arr2[6],
			arr1[7] * arr2[7],
		};
		return i32x4_t(result);
	}
	static inline i32x4_t multiply16(const i32x4_t& v, int s) noexcept
	{
		const short* arr = (const short*)&v;
		short result[8] = {
			arr[0] * s,
			arr[1] * s,
			arr[2] * s,
			arr[3] * s,
			arr[4] * s,
			arr[5] * s,
			arr[6] * s,
			arr[7] * s,
		};
		return i32x4_t(result);
	}

	static inline i32x4_t add8(const i32x4_t& v1, const i32x4_t& v2) noexcept
	{
		const char* arr1 = (const char*)&v1;
		const char* arr2 = (const char*)&v2;
		char result[16] = {
			arr1[0] + arr2[0],
			arr1[1] + arr2[1],
			arr1[2] + arr2[2],
			arr1[3] + arr2[3],
			arr1[4] + arr2[4],
			arr1[5] + arr2[5],
			arr1[6] + arr2[6],
			arr1[7] + arr2[7],
			arr1[8] + arr2[8],
			arr1[9] + arr2[9],
			arr1[10] + arr2[10],
			arr1[11] + arr2[11],
			arr1[12] + arr2[12],
			arr1[13] + arr2[13],
			arr1[14] + arr2[14],
			arr1[15] + arr2[15],
		};
		return i32x4_t(result);
	}
	static inline i32x4_t subtract8(const i32x4_t& v1, const i32x4_t& v2) noexcept
	{
		const char* arr1 = (const char*)&v1;
		const char* arr2 = (const char*)&v2;
		char result[16] = {
			arr1[0] - arr2[0],
			arr1[1] - arr2[1],
			arr1[2] - arr2[2],
			arr1[3] - arr2[3],
			arr1[4] - arr2[4],
			arr1[5] - arr2[5],
			arr1[6] - arr2[6],
			arr1[7] - arr2[7],
			arr1[8] - arr2[8],
			arr1[9] - arr2[9],
			arr1[10] - arr2[10],
			arr1[11] - arr2[11],
			arr1[12] - arr2[12],
			arr1[13] - arr2[13],
			arr1[14] - arr2[14],
			arr1[15] - arr2[15],
		};
		return i32x4_t(result);
	}
	static inline i32x4_t negate8(const i32x4_t& v) noexcept
	{
		const char* arr = (const char*)&v1;
		char result[16] = {
			-arr[0],
			-arr[1],
			-arr[2],
			-arr[3],
			-arr[4],
			-arr[5],
			-arr[6],
			-arr[7],
			-arr[8],
			-arr[9],
			-arr[10],
			-arr[11],
			-arr[12],
			-arr[13],
			-arr[14],
			-arr[15],
		};
		return i32x4_t(result);
	}
	static inline i32x4_t multiply8(const i32x4_t& v1, const i32x4_t& v2) noexcept
	{
		const char* arr1 = (const char*)&v1;
		const char* arr2 = (const char*)&v2;
		char result[16] = {
			arr1[0] * arr2[0],
			arr1[1] * arr2[1],
			arr1[2] * arr2[2],
			arr1[3] * arr2[3],
			arr1[4] * arr2[4],
			arr1[5] * arr2[5],
			arr1[6] * arr2[6],
			arr1[7] * arr2[7],
			arr1[8] * arr2[8],
			arr1[9] * arr2[9],
			arr1[10] * arr2[10],
			arr1[11] * arr2[11],
			arr1[12] * arr2[12],
			arr1[13] * arr2[13],
			arr1[14] * arr2[14],
			arr1[15] * arr2[15],
		};
		return i32x4_t(result);
	}
	static inline i32x4_t multiply8(const i32x4_t& v, int s) noexcept
	{
		const char* arr = (const char*)&v1;
		char result[16] = {
			arr[0] * s,
			arr[1] * s,
			arr[2] * s,
			arr[3] * s,
			arr[4] * s,
			arr[5] * s,
			arr[6] * s,
			arr[7] * s,
			arr[8] * s,
			arr[9] * s,
			arr[10] * s,
			arr[11] * s,
			arr[12] * s,
			arr[13] * s,
			arr[14] * s,
			arr[15] * s,
		};
		return i32x4_t(result);
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// 4x4 Matrix Arithmetic Operations
//
///////////////////////////////////////////////////////////////////////////////////////////
namespace dseed
{
	static inline f32x4x4_t add(const f32x4x4_t& m1, const f32x4x4_t& m2) noexcept
	{
		return f32x4x4_t(
			add(m1.column1(), m2.column1()),
			add(m1.column2(), m2.column2()),
			add(m1.column3(), m2.column3()),
			add(m1.column4(), m2.column4())
		);
	}
	static inline f32x4x4_t subtract(const f32x4x4_t& m1, const f32x4x4_t& m2) noexcept
	{
		return f32x4x4_t(
			subtract(m1.column1(), m2.column1()),
			subtract(m1.column2(), m2.column2()),
			subtract(m1.column3(), m2.column3()),
			subtract(m1.column4(), m2.column4())
		);
	}
	static inline f32x4x4_t negate(const f32x4x4_t& m) noexcept
	{
		return f32x4x4_t(
			negate(m.column1()),
			negate(m.column2()),
			negate(m.column3()),
			negate(m.column4())
		);
	}
	static inline f32x4x4_t multiply(const f32x4x4_t& m1, const f32x4x4_t& m2) noexcept
	{
		return f32x4x4_t(
			fma(m1.column1().vx(), m2.column1(),
				fma(m1.column1().vy(), m2.column2(),
					fma(m1.column1().vz(), m2.column3(),
						multiply(m1.column1().vw(), m2.column4())))),
			fma(m1.column2().vx(), m2.column1(),
				fma(m1.column2().vy(), m2.column2(),
					fma(m1.column2().vz(), m2.column3(),
						multiply(m1.column2().vw(), m2.column4())))),
			fma(m1.column3().vx(), m2.column1(),
				fma(m1.column3().vy(), m2.column2(),
					fma(m1.column3().vz(), m2.column3(),
						multiply(m1.column3().vw(), m2.column4())))),
			fma(m1.column4().vx(), m2.column1(),
				fma(m1.column4().vy(), m2.column2(),
					fma(m1.column4().vz(), m2.column3(),
						multiply(m1.column4().vw(), m2.column4()))))
		);
	}
	static inline f32x4x4_t multiply(const f32x4x4_t& m, float s) noexcept
	{
		return f32x4x4_t(
			multiply(m.column1(), s),
			multiply(m.column2(), s),
			multiply(m.column3(), s),
			multiply(m.column4(), s)
		);
	}
	static inline f32x4x4_t divide(const f32x4x4_t& m1, const f32x4x4_t& m2) noexcept
	{
		return f32x4x4_t(
			divide(m1.column1(), m2.column1()),
			divide(m1.column2(), m2.column2()),
			divide(m1.column3(), m2.column3()),
			divide(m1.column4(), m2.column4())
		);
	}
	static inline f32x4x4_t divide(const f32x4x4_t& m, float s) noexcept
	{
		return f32x4x4_t(
			divide(m.column1(), s),
			divide(m.column2(), s),
			divide(m.column3(), s),
			divide(m.column4(), s)
		);
	}
}