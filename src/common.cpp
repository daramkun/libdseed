#include <dseed.h>

#include <array>
#include <bitset>

#if ARCH_X86SET
const dseed::x86_instruction_info& dseed::x86_instruction_info::instance ()
{
	static dseed::x86_instruction_info info;
	return info;
}

inline void cpuid (int cpuinfo[4], int funcId)
{
#if COMPILER_MSVC
	__cpuid (cpuinfo, funcId);
#else
	__asm
	{
		mov    esi, cpuinfo
		mov    eax, funcId
		xor ecx, ecx
		cpuid
		mov    dword ptr[esi + 0], eax
		mov    dword ptr[esi + 4], ebx
		mov    dword ptr[esi + 8], ecx
		mov    dword ptr[esi + 12], edx
	};
#endif
}
inline void cpuidex (int cpuinfo[4], int funcId, int subFuncId)
{
#if COMPILER_MSVC
	__cpuidex (cpuinfo, funcId, subFuncId);
#else
	__asm
	{
		mov    esi, cpuinfo
		mov    eax, funcId
		mov    ecx, subFuncId
		cpuid
		mov    dword ptr[esi + 0], eax
		mov    dword ptr[esi + 4], ebx
		mov    dword ptr[esi + 8], ecx
		mov    dword ptr[esi + 12], edx
	};
#endif
}

dseed::x86_instruction_info::x86_instruction_info ()
{
	memset (this, 0, sizeof (x86_instruction_info));

	std::array<int, 4> cpuinfo;

	cpuid (cpuinfo.data (), 0);
	int nIds = cpuinfo[0];
	cpuid (cpuinfo.data (), 0x80000000);
	int nExIds = cpuinfo[0];

	std::vector<std::array<int, 4>> data;
	std::vector<std::array<int, 4>> extData;

	for (int i = 0; i < nIds; ++i)
	{
		cpuidex (cpuinfo.data (), i, 0);
		data.push_back (cpuinfo);
	}

	memset (cpu_vender, 0, sizeof (cpu_vender));
	*reinterpret_cast<int*>(cpu_vender) = data[0][1];
	*reinterpret_cast<int*>(cpu_vender + 4) = data[0][3];
	*reinterpret_cast<int*>(cpu_vender + 8) = data[0][2];

	bool isIntel = false, isAMD = false;
	if (strcmp ("GenuineIntel", cpu_vender))
		isIntel = true;
	else if (strcmp ("AuthenticAMD", cpu_vender))
		isAMD = true;

	for (int i = 0x80000000; i <= nExIds; ++i)
	{
		cpuidex (cpuinfo.data (), i, 0);
		extData.push_back (cpuinfo);
	}

	memset (cpu_brand, 0, sizeof (cpu_brand));
	if (nExIds >= 0x80000004)
	{
		memcpy (cpu_brand + 00, extData[2].data (), sizeof (cpuinfo));
		memcpy (cpu_brand + 16, extData[3].data (), sizeof (cpuinfo));
		memcpy (cpu_brand + 32, extData[4].data (), sizeof (cpuinfo));
	}

	std::bitset<32> ecx1, edx1, ebx7, ecx7, ecx8, edx8;
	if (nIds >= 1)
	{
		ecx1 = data[1][2];
		edx1 = data[1][3];
	}
	if (nIds >= 7)
	{
		ebx7 = data[7][1];
		ecx7 = data[7][2];
	}
	if (nExIds >= 1)
	{
		ecx8 = extData[1][2];
		edx8 = extData[1][3];
	}

	sse = edx1[25];
	sse2 = edx1[26];
	sse3 = ecx1[0];
	ssse3 = ecx1[9];
	sse4_1 = ecx1[19];
	sse4_2 = ecx1[20];
	sse4a = isAMD && ecx8[6];
	avx = ecx1[28];
	avx2 = ebx7[5];
	fma3 = ecx1[12];

	rdrand = ebx7[18];
	aes = ecx1[25];
	sha = ebx7[29];

	f16c = ecx1[29];
}
#endif

#if ARCH_ARMSET
#	if ARCH_ARM && PLATFORM_UNIX
#		include <elf.h>
#		include <fcntl.h>
#		include <linux/auxvec.h>
#		include <unistd.h>
#	endif

const dseed::arm_instruction_info& dseed::arm_instruction_info::instance ()
{
	static dseed::arm_instruction_info info;
	return info;
}

dseed::arm_instruction_info::arm_instruction_info ()
{
#	if ARCH_ARM64
	neon = true;
	crc32 = true;
#	else
	crc32 = false;
#		if PLATFORM_UNIX && COMPILER_GCC
	auto cpufile = open ("/proc/self/auxv", O_RDONLY);
	assert (cpufile);

	Elf32_auxv_t auxv;

	if (cpufile >= 0)
	{
		const auto size_auxv_t = sizeof (Elf32_auxv_t);
		while (read (cpufile, &auxv, size_auxv_t) == size_auxv_t)
		{
			if (auxv.a_type == AT_HWCAP)
			{
				neon = (auxv.a_un.a_val & 4096) != 0;
				break;
			}
		}

		close (cpufile);
	}
	else
	{
		neon = false;
	}
#		elif COMPILER_CLANG
#			if defined (__ARM_NEON__)
	neon = true;
#			else
	neon = false;
#			endif
#		else
	neon = false;
#		endif
#	endif
}
#endif

#if ARCH_ARM64 && !(COMPILER_MSVC)
#	include <arm_acle.h>
#endif

uint32_t crc32_table[256] = {
		0x00000000, 0xF26B8303, 0xE13B70F7, 0x1350F3F4, 0xC79A971F, 0x35F1141C, 0x26A1E7E8, 0xD4CA64EB,
		0x8AD958CF, 0x78B2DBCC, 0x6BE22838, 0x9989AB3B, 0x4D43CFD0, 0xBF284CD3, 0xAC78BF27, 0x5E133C24,
		0x105EC76F, 0xE235446C, 0xF165B798, 0x030E349B, 0xD7C45070, 0x25AFD373, 0x36FF2087, 0xC494A384,
		0x9A879FA0, 0x68EC1CA3, 0x7BBCEF57, 0x89D76C54, 0x5D1D08BF, 0xAF768BBC, 0xBC267848, 0x4E4DFB4B,
		0x20BD8EDE, 0xD2D60DDD, 0xC186FE29, 0x33ED7D2A, 0xE72719C1, 0x154C9AC2, 0x061C6936, 0xF477EA35,
		0xAA64D611, 0x580F5512, 0x4B5FA6E6, 0xB93425E5, 0x6DFE410E, 0x9F95C20D, 0x8CC531F9, 0x7EAEB2FA,
		0x30E349B1, 0xC288CAB2, 0xD1D83946, 0x23B3BA45, 0xF779DEAE, 0x05125DAD, 0x1642AE59, 0xE4292D5A,
		0xBA3A117E, 0x4851927D, 0x5B016189, 0xA96AE28A, 0x7DA08661, 0x8FCB0562, 0x9C9BF696, 0x6EF07595,
		0x417B1DBC, 0xB3109EBF, 0xA0406D4B, 0x522BEE48, 0x86E18AA3, 0x748A09A0, 0x67DAFA54, 0x95B17957,
		0xCBA24573, 0x39C9C670, 0x2A993584, 0xD8F2B687, 0x0C38D26C, 0xFE53516F, 0xED03A29B, 0x1F682198,
		0x5125DAD3, 0xA34E59D0, 0xB01EAA24, 0x42752927, 0x96BF4DCC, 0x64D4CECF, 0x77843D3B, 0x85EFBE38,
		0xDBFC821C, 0x2997011F, 0x3AC7F2EB, 0xC8AC71E8, 0x1C661503, 0xEE0D9600, 0xFD5D65F4, 0x0F36E6F7,
		0x61C69362, 0x93AD1061, 0x80FDE395, 0x72966096, 0xA65C047D, 0x5437877E, 0x4767748A, 0xB50CF789,
		0xEB1FCBAD, 0x197448AE, 0x0A24BB5A, 0xF84F3859, 0x2C855CB2, 0xDEEEDFB1, 0xCDBE2C45, 0x3FD5AF46,
		0x7198540D, 0x83F3D70E, 0x90A324FA, 0x62C8A7F9, 0xB602C312, 0x44694011, 0x5739B3E5, 0xA55230E6,
		0xFB410CC2, 0x092A8FC1, 0x1A7A7C35, 0xE811FF36, 0x3CDB9BDD, 0xCEB018DE, 0xDDE0EB2A, 0x2F8B6829,
		0x82F63B78, 0x709DB87B, 0x63CD4B8F, 0x91A6C88C, 0x456CAC67, 0xB7072F64, 0xA457DC90, 0x563C5F93,
		0x082F63B7, 0xFA44E0B4, 0xE9141340, 0x1B7F9043, 0xCFB5F4A8, 0x3DDE77AB, 0x2E8E845F, 0xDCE5075C,
		0x92A8FC17, 0x60C37F14, 0x73938CE0, 0x81F80FE3, 0x55326B08, 0xA759E80B, 0xB4091BFF, 0x466298FC,
		0x1871A4D8, 0xEA1A27DB, 0xF94AD42F, 0x0B21572C, 0xDFEB33C7, 0x2D80B0C4, 0x3ED04330, 0xCCBBC033,
		0xA24BB5A6, 0x502036A5, 0x4370C551, 0xB11B4652, 0x65D122B9, 0x97BAA1BA, 0x84EA524E, 0x7681D14D,
		0x2892ED69, 0xDAF96E6A, 0xC9A99D9E, 0x3BC21E9D, 0xEF087A76, 0x1D63F975, 0x0E330A81, 0xFC588982,
		0xB21572C9, 0x407EF1CA, 0x532E023E, 0xA145813D, 0x758FE5D6, 0x87E466D5, 0x94B49521, 0x66DF1622,
		0x38CC2A06, 0xCAA7A905, 0xD9F75AF1, 0x2B9CD9F2, 0xFF56BD19, 0x0D3D3E1A, 0x1E6DCDEE, 0xEC064EED,
		0xC38D26C4, 0x31E6A5C7, 0x22B65633, 0xD0DDD530, 0x0417B1DB, 0xF67C32D8, 0xE52CC12C, 0x1747422F,
		0x49547E0B, 0xBB3FFD08, 0xA86F0EFC, 0x5A048DFF, 0x8ECEE914, 0x7CA56A17, 0x6FF599E3, 0x9D9E1AE0,
		0xD3D3E1AB, 0x21B862A8, 0x32E8915C, 0xC083125F, 0x144976B4, 0xE622F5B7, 0xF5720643, 0x07198540,
		0x590AB964, 0xAB613A67, 0xB831C993, 0x4A5A4A90, 0x9E902E7B, 0x6CFBAD78, 0x7FAB5E8C, 0x8DC0DD8F,
		0xE330A81A, 0x115B2B19, 0x020BD8ED, 0xF0605BEE, 0x24AA3F05, 0xD6C1BC06, 0xC5914FF2, 0x37FACCF1,
		0x69E9F0D5, 0x9B8273D6, 0x88D28022, 0x7AB90321, 0xAE7367CA, 0x5C18E4C9, 0x4F48173D, 0xBD23943E,
		0xF36E6F75, 0x0105EC76, 0x12551F82, 0xE03E9C81, 0x34F4F86A, 0xC69F7B69, 0xD5CF889D, 0x27A40B9E,
		0x79B737BA, 0x8BDCB4B9, 0x988C474D, 0x6AE7C44E, 0xBE2DA0A5, 0x4C4623A6, 0x5F16D052, 0xAD7D5351,
};

uint32_t dseed::crc32 (uint32_t crc32, const uint8_t* bytes, size_t bytesCount) noexcept
{
	uint32_t crc = ~crc32;
#if ARCH_X86SET
	if (x86_instruction_info::instance ().sse4_2)
	{
#	if ARCH_AMD64
		size_t s64BytesCount = bytesCount / 8;
		for (size_t i = 0; i < s64BytesCount; ++i)
			crc = _mm_crc32_u64 (crc, reinterpret_cast<const uint64_t*>(bytes)[i]);
#	else
		size_t s64BytesCount = 0;
#	endif

		size_t s32BytesCount = bytesCount / 4;
		for (size_t i = s64BytesCount * 8; i < s32BytesCount; ++i)
			crc = _mm_crc32_u32 (crc, reinterpret_cast<const uint32_t*>(bytes)[i]);

		size_t s16BytesCount = bytesCount / 2;
		for (size_t i = s32BytesCount * 4; i < s16BytesCount; ++i)
			crc = _mm_crc32_u16 (crc, reinterpret_cast<const uint16_t*>(bytes)[i]);

		for (size_t i = s16BytesCount * 2; i < bytesCount; ++i)
			crc = _mm_crc32_u8 (crc, bytes[i]);
	}
#elif ARCH_ARM64 && !(COMPILER_MSVC)
	if (arm_instruction_info::instance ().neon)
	{
		size_t s64BytesCount = bytesCount / 8;
		for (size_t i = 0; i < s64BytesCount; ++i)
			crc = __crc32cd (crc, reinterpret_cast<const uint64_t*>(bytes)[i]);

		size_t s32BytesCount = bytesCount / 4;
		for (size_t i = s64BytesCount * 8; i < s32BytesCount; ++i)
			crc = __crc32cw (crc, reinterpret_cast<const uint32_t*>(bytes)[i]);

		size_t s16BytesCount = bytesCount / 2;
		for (size_t i = s32BytesCount * 4; i < s16BytesCount; ++i)
			crc = __crc32ch (crc, reinterpret_cast<const uint16_t*>(bytes)[i]);

		for (size_t i = s16BytesCount * 2; i < bytesCount; ++i)
			crc = __crc32cb (crc, bytes[i]);
	}
#else
	if (false)
	{

	}
#endif
	else
	{
		for (size_t i = 0; i < bytesCount; ++i)
			crc = crc32_table[(crc ^ bytes[i]) & 0xFF] ^ (crc >> 8);
	}

	return ~crc;
}
