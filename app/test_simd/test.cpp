#include <dseed.h>

int main (void)
{
	const dseed::x86_instruction_info& info = dseed::x86_instruction_info::instance ();
	printf (
		"CPU Vender: %s\n"
		"CPU Name:   %s\n"
		"SSE:        %c\n"
		"SSE2:       %c\n"
		"SSE3:       %c\n"
		"SSSE3:      %c\n"
		"SSE4.1:     %c\n"
		"SSE4.2:     %c\n"
		"SSE4a:      %c\n"
		"AVX:        %c\n"
		"AVX2:       %c\n"
		"FMA3:       %c\n"
		"F16C:       %c\n"
		"AES:        %c\n"
		"SHA:        %c\n"
		"RDRAND:     %c\n"
		"TSX:        %c\n"
		"ASF:        %c\n"
		, info.cpu_vender
		, info.cpu_brand
		, info.sse ? 'T' : 'F'
		, info.sse2 ? 'T' : 'F'
		, info.sse3 ? 'T' : 'F'
		, info.ssse3 ? 'T' : 'F'
		, info.sse4_1 ? 'T' : 'F'
		, info.sse4_2 ? 'T' : 'F'
		, info.sse4a ? 'T' : 'F'
		, info.avx ? 'T' : 'F'
		, info.avx2 ? 'T' : 'F'
		, info.fma3 ? 'T' : 'F'
		, info.f16c ? 'T' : 'F'
		, info.aes ? 'T' : 'F'
		, info.sha ? 'T' : 'F'
		, info.rdrand ? 'T' : 'F'
		, info.tsx ? 'T' : 'F'
		, info.asf ? 'T' : 'F'
	);

	return 0;
}