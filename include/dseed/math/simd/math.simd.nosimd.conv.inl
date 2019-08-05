#ifndef __DSEED_MATH_SIMD_NOSIMD_CONV_INL__
#define __DSEED_MATH_SIMD_NOSIMD_CONV_INL__

namespace dseed
{
	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Implementation vector conversions
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline vectori_def FASTCALL conv_f32_to_i32 (const vectorf_def& v) noexcept
	{
		return vectori_def ((int)v.v[0], (int)v.v[1], (int)v.v[2], (int)v.v[3]);
	}
	inline vector8i_def FASTCALL conv_f32_to_i32 (const vector8f_def& v) noexcept
	{
		return vector8i_def (
			(int)v.v[0], (int)v.v[1], (int)v.v[2], (int)v.v[3],
			(int)v.v[4], (int)v.v[5], (int)v.v[6], (int)v.v[7]
		);
	}
	inline vectori_def FASTCALL reinterpret_f32_to_i32 (const vectorf_def& v) noexcept
	{
		const int* reinterpret = reinterpret_cast<const int*>(v.v);
		return vectori_def (reinterpret);
	}
	inline vector8i_def FASTCALL reinterpret_f32_to_i32 (const vector8f_def& v) noexcept
	{
		const int* reinterpret = reinterpret_cast<const int*>(v.v);
		return vector8i_def (reinterpret);
	}

	inline vectorf_def FASTCALL conv_i32_to_f32 (const vectori_def& v) noexcept
	{
		return vectorf_def ((float)v.v[0], (float)v.v[1], (float)v.v[2], (float)v.v[3]);
	}
	inline vector8f_def FASTCALL conv_i32_to_f32 (const vector8i_def& v) noexcept
	{
		return vector8f_def (
			(float)v.v[0], (float)v.v[1], (float)v.v[2], (float)v.v[3],
			(float)v.v[4], (float)v.v[5], (float)v.v[6], (float)v.v[7]
		);
	}
	inline vectorf_def FASTCALL reinterpret_i32_to_f32 (const vectori_def& v) noexcept
	{
		const float* reinterpret = reinterpret_cast<const float*>(v.v);
		return vectorf_def (reinterpret);
	}
	inline vector8f_def FASTCALL reinterpret_i32_to_f32 (const vector8i_def& v) noexcept
	{
		const float* reinterpret = reinterpret_cast<const float*>(v.v);
		return vector8f_def (reinterpret);
	}

	inline vectori_def FASTCALL conv_i64_to_i32 (const vectori_def& v) noexcept
	{
		const int32_t* origin = (const int32_t*)v.v;
		int64_t conv[2] = {};
		conv[0] = origin[0];
		conv[1] = origin[1];
		return vectori_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i64_to_i32 (const vector8i_def& v) noexcept
	{
		const int32_t* origin = (const int32_t*)v.v;
		int64_t conv[4] = {};
		conv[0] = origin[0];
		conv[1] = origin[1];
		conv[2] = origin[2];
		conv[3] = origin[3];
		return vector8i_def ((const int*)conv);
	}
	inline vectori_def FASTCALL conv_i64_to_i24 (const vectori_def& v) noexcept
	{
		const int32_t* origin = (const int32_t*)v.v;
		int24_t conv[5] = {};
		conv[0] = origin[0];
		conv[1] = origin[1];
		return vectori_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i64_to_i24 (const vector8i_def& v) noexcept
	{
		const int32_t* origin = (const int32_t*)v.v;
		int24_t conv[8] = {};
		conv[0] = origin[0];
		conv[1] = origin[1];
		conv[2] = origin[2];
		conv[3] = origin[3];
		return vector8i_def ((const int*)conv);
	}
	inline vectori_def FASTCALL conv_i64_to_i16 (const vectori_def& v) noexcept
	{
		const int32_t* origin = (const int32_t*)v.v;
		int16_t conv[8] = {};
		conv[0] = origin[0];
		conv[1] = origin[1];
		return vectori_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i64_to_i16 (const vector8i_def& v) noexcept
	{
		const int32_t* origin = (const int32_t*)v.v;
		int16_t conv[8] = {};
		conv[0] = origin[0];
		conv[1] = origin[1];
		conv[2] = origin[2];
		conv[3] = origin[3];
		return vectori_def ((const int*)conv);
	}
	inline vectori_def FASTCALL conv_i64_to_i8 (const vectori_def& v) noexcept
	{
		const int32_t* origin = (const int32_t*)v.v;
		int8_t conv[16] = {};
		conv[0] = origin[0];
		conv[1] = origin[1];
		return vectori_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i64_to_i8 (const vector8i_def& v) noexcept
	{
		const int32_t* origin = (const int32_t*)v.v;
		int8_t conv[32] = {};
		conv[0] = origin[0];
		conv[1] = origin[1];
		conv[2] = origin[2];
		conv[3] = origin[3];
		return vectori_def ((const int*)conv);
	}

	inline vectori_def FASTCALL conv_i32_to_i64 (const vectori_def& v) noexcept
	{
		int64_t conv[2] = {};
		conv[0] = (int64_t)v.v[0]; conv[1] = (int64_t)v.v[1];
		return vector8i_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i32_to_i64 (const vector8i_def& v) noexcept
	{
		int64_t conv[4] = {};
		conv[0] = (int64_t)v.v[0]; conv[1] = (int64_t)v.v[1];
		conv[2] = (int64_t)v.v[2]; conv[3] = (int64_t)v.v[3];
		return vector8i_def ((const int*)conv);
	}
	inline vectori_def FASTCALL conv_i32_to_i24 (const vectori_def& v) noexcept
	{
		int24_t conv[6] = {};
		conv[0] = (int24_t)v.v[0]; conv[1] = (int24_t)v.v[1];
		conv[2] = (int24_t)v.v[2]; conv[3] = (int24_t)v.v[3];
		return vectori_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i32_to_i24 (const vector8i_def& v) noexcept
	{
		int24_t conv[11] = {};
		conv[0] = (int24_t)v.v[0]; conv[1] = (int24_t)v.v[1];
		conv[2] = (int24_t)v.v[2]; conv[3] = (int24_t)v.v[3];
		conv[4] = (int24_t)v.v[4]; conv[5] = (int24_t)v.v[5];
		conv[6] = (int24_t)v.v[6]; conv[7] = (int24_t)v.v[7];
		return vector8i_def ((const int*)conv);
	}
	inline vectori_def FASTCALL conv_i32_to_i16 (const vectori_def& v) noexcept
	{
		short conv[8] = {};
		conv[0] = (short)v.v[0]; conv[1] = (short)v.v[1];
		conv[2] = (short)v.v[2]; conv[3] = (short)v.v[3];
		return vectori_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i32_to_i16 (const vector8i_def& v) noexcept
	{
		short conv[16] = {};
		conv[0] = (short)v.v[0]; conv[1] = (short)v.v[1];
		conv[2] = (short)v.v[2]; conv[3] = (short)v.v[3];
		conv[4] = (short)v.v[4]; conv[5] = (short)v.v[5];
		conv[6] = (short)v.v[6]; conv[7] = (short)v.v[7];
		return vector8i_def ((const int*)conv);
	}
	inline vectori_def FASTCALL conv_i32_to_i8 (const vectori_def& v) noexcept
	{
		char conv[16] = {};
		conv[0] = (char)v.v[0]; conv[1] = (char)v.v[1];
		conv[2] = (char)v.v[2]; conv[3] = (char)v.v[3];
		return vectori_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i32_to_i8 (const vector8i_def& v) noexcept
	{
		char conv[32] = {};
		conv[0] = (char)v.v[0]; conv[1] = (char)v.v[1];
		conv[2] = (char)v.v[2]; conv[3] = (char)v.v[3];
		conv[4] = (char)v.v[4]; conv[5] = (char)v.v[5];
		conv[6] = (char)v.v[6]; conv[7] = (char)v.v[7];
		return vector8i_def ((const int*)conv);
	}

	inline vectori_def FASTCALL conv_i24_to_i64 (const vectori_def& v) noexcept
	{
		const int24_t* origin = (const int24_t*)v.v;
		int64_t conv[2] = {};
		conv[0] = (int64_t)(int32_t)origin[0];
		conv[1] = (int64_t)(int32_t)origin[1];
		return vector8i_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i24_to_i64 (const vector8i_def& v) noexcept
	{
		const int24_t* origin = (const int24_t*)v.v;
		int64_t conv[4] = {};
		conv[0] = (int64_t)(int32_t)origin[0];
		conv[1] = (int64_t)(int32_t)origin[1];
		conv[2] = (int64_t)(int32_t)origin[2];
		conv[3] = (int64_t)(int32_t)origin[3];
		return vector8i_def ((const int*)conv);
	}
	inline vectori_def FASTCALL conv_i24_to_i32 (const vectori_def& v) noexcept
	{
		const int24_t* origin = (const int24_t*)v.v;
		int32_t conv[4] = {};
		conv[0] = (int32_t)origin[0]; conv[1] = (int32_t)origin[1];
		conv[2] = (int32_t)origin[2]; conv[3] = (int32_t)origin[3];
		return vectori_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i24_to_i32 (const vector8i_def& v) noexcept
	{
		const int24_t* origin = (const int24_t*)v.v;
		int32_t conv[8] = {};
		conv[0] = (int32_t)origin[0]; conv[1] = (int32_t)origin[1];
		conv[2] = (int32_t)origin[2]; conv[3] = (int32_t)origin[3];
		conv[4] = (int32_t)origin[4]; conv[5] = (int32_t)origin[5];
		conv[6] = (int32_t)origin[6]; conv[7] = (int32_t)origin[7];
		return vector8i_def ((const int*)conv);
	}
	inline vectori_def FASTCALL conv_i24_to_i16 (const vectori_def& v) noexcept
	{
		const int24_t* origin = (const int24_t*)v.v;
		short conv[8] = {};
		conv[0] = (short)(int)origin[0]; conv[1] = (short)(int)origin[1]; conv[2] = (short)(int)origin[2];
		conv[3] = (short)(int)origin[3];
		return vectori_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i24_to_i16 (const vector8i_def& v) noexcept
	{
		const int24_t* origin = (const int24_t*)v.v;
		short conv[16] = {};
		conv[0] = (short)(int)v.v[0]; conv[1] = (short)(int)v.v[1];
		conv[2] = (short)(int)v.v[2]; conv[3] = (short)(int)v.v[3];
		conv[4] = (short)(int)v.v[4]; conv[5] = (short)(int)v.v[5];
		conv[6] = (short)(int)v.v[6]; conv[7] = (short)(int)v.v[7];
		return vector8i_def ((const int*)conv);
	}
	inline vectori_def FASTCALL conv_i24_to_i8 (const vectori_def& v) noexcept
	{
		const int24_t* origin = (const int24_t*)v.v;
		char conv[16] = {};
		conv[0] = (char)(int)origin[0]; conv[1] = (char)(int)origin[1];
		conv[2] = (char)(int)origin[2]; conv[3] = (char)(int)origin[3];
		return vectori_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i24_to_i8 (const vector8i_def& v) noexcept
	{
		const int24_t* origin = (const int24_t*)v.v;
		char conv[32] = {};
		conv[0] = (char)(int)origin[0]; conv[1] = (char)(int)origin[1];
		conv[2] = (char)(int)origin[2]; conv[3] = (char)(int)origin[3];
		conv[4] = (char)(int)origin[4]; conv[5] = (char)(int)origin[5];
		conv[6] = (char)(int)origin[6]; conv[7] = (char)(int)origin[7];
		return vector8i_def ((const int*)conv);
	}

	inline vectori_def FASTCALL conv_i16_to_i64 (const vectori_def& v) noexcept
	{
		const int16_t* origin = (const int16_t*)v.v;
		int64_t conv[2] = {};
		conv[0] = (int64_t)origin[0];
		conv[1] = (int64_t)origin[1];
		return vector8i_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i16_to_i64 (const vector8i_def& v) noexcept
	{
		const int16_t* origin = (const int16_t*)v.v;
		int64_t conv[4] = {};
		conv[0] = (int64_t)origin[0];
		conv[1] = (int64_t)origin[1];
		conv[2] = (int64_t)origin[2];
		conv[3] = (int64_t)origin[3];
		return vector8i_def ((const int*)conv);
	}
	inline vectori_def FASTCALL conv_i16_to_i32 (const vectori_def& v) noexcept
	{
		const int16_t* origin = (const int16_t*)v.v;
		int conv[4] = {};
		conv[0] = (int)origin[0]; conv[1] = (int)origin[1];
		conv[2] = (int)origin[2]; conv[3] = (int)origin[3];
		return vectori_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i16_to_i32 (const vector8i_def& v) noexcept
	{
		const int16_t* origin = (const int16_t*)v.v;
		int conv[8] = {};
		conv[0] = (int)origin[0]; conv[1] = (int)origin[1];
		conv[2] = (int)origin[2]; conv[3] = (int)origin[3];
		conv[4] = (int)origin[4]; conv[5] = (int)origin[5];
		conv[6] = (int)origin[6]; conv[7] = (int)origin[7];
		return vector8i_def ((const int*)conv);
	}
	inline vectori_def FASTCALL conv_i16_to_i24 (const vectori_def& v) noexcept
	{
		const int16_t* origin = (const int16_t*)v.v;
		int24_t conv[6] = {};
		conv[0] = (int24_t)origin[0]; conv[1] = (int24_t)origin[1];
		conv[2] = (int24_t)origin[2]; conv[3] = (int24_t)origin[3];
		conv[4] = (int24_t)origin[4];
		return vectori_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i16_to_i24 (const vector8i_def& v) noexcept
	{
		const int16_t* origin = (const int16_t*)v.v;
		int24_t conv[11] = {};
		conv[0] = (int24_t)origin[0]; conv[1] = (int24_t)origin[1];
		conv[2] = (int24_t)origin[2]; conv[3] = (int24_t)origin[3];
		conv[4] = (int24_t)origin[4]; conv[5] = (int24_t)origin[5];
		conv[6] = (int24_t)origin[6]; conv[7] = (int24_t)origin[7];
		return vector8i_def ((const int*)conv);
	}
	inline vectori_def FASTCALL conv_i16_to_i8 (const vectori_def& v) noexcept
	{
		const int16_t* origin = (const int16_t*)v.v;
		char conv[16] = {};
		conv[0] = (char)origin[0]; conv[1] = (char)origin[1];
		conv[2] = (char)origin[2]; conv[3] = (char)origin[3];
		conv[4] = (char)origin[4]; conv[5] = (char)origin[5];
		conv[6] = (char)origin[6]; conv[7] = (char)origin[7];
		return vectori_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i16_to_i8 (const vector8i_def& v) noexcept
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
		return vector8i_def ((const int*)conv);
	}

	inline vectori_def FASTCALL conv_i8_to_i64 (const vectori_def& v) noexcept
	{
		const int8_t* origin = (const int8_t*)v.v;
		int64_t conv[2] = {};
		conv[0] = (int64_t)origin[0];
		conv[1] = (int64_t)origin[1];
		return vector8i_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i8_to_i64 (const vector8i_def& v) noexcept
	{
		const int8_t* origin = (const int8_t*)v.v;
		int64_t conv[4] = {};
		conv[0] = (int64_t)origin[0];
		conv[1] = (int64_t)origin[1];
		conv[2] = (int64_t)origin[2];
		conv[3] = (int64_t)origin[3];
		return vector8i_def ((const int*)conv);
	}
	inline vectori_def FASTCALL conv_i8_to_i32 (const vectori_def& v) noexcept
	{
		const int8_t* origin = (const int8_t*)v.v;
		int conv[4] = {};
		conv[0] = (int)origin[0]; conv[1] = (int)origin[1];
		conv[2] = (int)origin[2]; conv[3] = (int)origin[3];
		return vectori_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i8_to_i32 (const vector8i_def& v) noexcept
	{
		const int8_t* origin = (const int8_t*)v.v;
		int conv[8] = {};
		conv[0] = (int)origin[0]; conv[1] = (int)origin[1];
		conv[2] = (int)origin[2]; conv[3] = (int)origin[3];
		conv[4] = (int)origin[4]; conv[5] = (int)origin[5];
		conv[6] = (int)origin[6]; conv[7] = (int)origin[7];
		return vector8i_def ((const int*)conv);
	}
	inline vectori_def FASTCALL conv_i8_to_i24 (const vectori_def& v) noexcept
	{
		const int8_t* origin = (const int8_t*)v.v;
		int24_t conv[6] = {};
		conv[0] = (int24_t)origin[0]; conv[1] = (int24_t)origin[1];
		conv[2] = (int24_t)origin[2]; conv[3] = (int24_t)origin[3];
		conv[4] = (int24_t)origin[4];
		return vectori_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i8_to_i24 (const vector8i_def& v) noexcept
	{
		const int8_t* origin = (const int8_t*)v.v;
		int24_t conv[11] = {};
		conv[0] = (int24_t)origin[0]; conv[1] = (int24_t)origin[1];
		conv[2] = (int24_t)origin[2]; conv[3] = (int24_t)origin[3];
		conv[4] = (int24_t)origin[4]; conv[5] = (int24_t)origin[5];
		conv[6] = (int24_t)origin[6]; conv[7] = (int24_t)origin[7];
		return vector8i_def ((const int*)conv);
	}
	inline vectori_def FASTCALL conv_i8_to_i16 (const vectori_def& v) noexcept
	{
		const int8_t* origin = (const int8_t*)v.v;
		short conv[8] = {};
		conv[0] = (char)origin[0]; conv[1] = (char)origin[1];
		conv[2] = (char)origin[2]; conv[3] = (char)origin[3];
		conv[4] = (char)origin[4]; conv[5] = (char)origin[5];
		conv[6] = (char)origin[6]; conv[7] = (char)origin[7];
		return vectori_def ((const int*)conv);
	}
	inline vector8i_def FASTCALL conv_i8_to_i16 (const vector8i_def& v) noexcept
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
		return vector8i_def ((const int*)conv);
	}

	inline int movemaskvf (const vectorf_def& v) noexcept
	{
		auto ia = reinterpret_cast<const uint32_t*> (v.v);
		return (ia[0] >> 31) | ((ia[1] >> 30) & 2) | ((ia[2] >> 29) & 4) | ((ia[3] >> 28) & 8);
	}
	inline int movemaskv8f (const vector8f_def& v) noexcept
	{
		auto ia = reinterpret_cast<const uint32_t*> (v.v);
		return (ia[0] >> 31) | ((ia[1] >> 30) & 2) | ((ia[2] >> 29) & 4) | ((ia[3] >> 28) & 8)
			| ((ia[4] >> 27) & 16) | ((ia[5] >> 26) & 32) | ((ia[6] >> 25) & 64) | ((ia[7] >> 24) & 128);
	}
	inline int movemaskvi (const vectori_def& v) noexcept
	{
		auto ia = reinterpret_cast<const uint8_t*> (v.v);
		return (ia[0] >> 31)
			| ((ia[1] >> 30) & 2) | ((ia[2] >> 29) & 4) | ((ia[3] >> 28) & 8)
			| ((ia[4] >> 27) & 16) | ((ia[5] >> 26) & 32) | ((ia[6] >> 25) & 64)
			| ((ia[7] >> 24) & 128) | ((ia[8] >> 23) & 256) | ((ia[9] >> 22) & 512)
			| ((ia[10] >> 21) & 1024) | ((ia[11] >> 20) & 2048) | ((ia[12] >> 19) & 4096)
			| ((ia[13] >> 18) & 8192) | ((ia[14] >> 17) & 16384) | ((ia[15] >> 16) & 32768);
	}
	inline int movemaskv8i (const vector8i_def& v) noexcept
	{
		auto ia = reinterpret_cast<const uint8_t*> (v.v);
		return (ia[0] >> 31)
			| ((ia[1] >> 30) & 2) | ((ia[2] >> 29) & 4) | ((ia[3] >> 28) & 8)
			| ((ia[4] >> 27) & 16) | ((ia[5] >> 26) & 32) | ((ia[6] >> 25) & 64)
			| ((ia[7] >> 24) & 128) | ((ia[8] >> 23) & 256) | ((ia[9] >> 22) & 512)
			| ((ia[10] >> 21) & 1024) | ((ia[11] >> 20) & 2048) | ((ia[12] >> 19) & 4096)
			| ((ia[13] >> 18) & 8192) | ((ia[14] >> 17) & 16384) | ((ia[15] >> 16) & 32768)
			| ((ia[16] >> 15) & 65536) | ((ia[17] >> 14) & 131072) | ((ia[18] >> 13) & 262144)
			| ((ia[19] >> 12) & 524288) | ((ia[20] >> 11) & 1048576) | ((ia[21] >> 10) & 2097152)
			| ((ia[22] >> 9) & 4194304) | ((ia[23] >> 8) & 8388608) | ((ia[24] >> 7) & 16777216)
			| ((ia[25] >> 6) & 33554432) | ((ia[26] >> 5) & 67108864) | ((ia[27] >> 4) & 134217728)
			| ((ia[28] >> 3) & 268435456) | ((ia[29] >> 2) & 536870912) | ((ia[30] >> 1) & 1073741824)
			| (ia[31] & 2147483648);
	}
}

#endif