#include <dseed.h>

#include <array>
#include <bitset>

const dseed::instructions::x86_instruction_info& dseed::instructions::x86_instruction_info::instance ()
{
	static dseed::instructions::x86_instruction_info info;
	return info;
}

#if ARCH_X86SET
#	include <intrin.h>

inline void cpuid (int cpuinfo[4], int funcId)
{
#	if COMPILER_MSVC
	__cpuid (cpuinfo, funcId);
#	else
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
#	endif
}
inline void cpuidex (int cpuinfo[4], int funcId, int subFuncId)
{
#	if COMPILER_MSVC
	__cpuidex (cpuinfo, funcId, subFuncId);
#	else
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
#	endif
}
#endif

dseed::instructions::x86_instruction_info::x86_instruction_info ()
{
#if ARCH_X86SET
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
	avx = ecx1[28];
	avx2 = ebx7[5];
	fma3 = ecx1[12];

	rdrand = ebx7[18];
	aes = ecx1[25];
	sha = ebx7[29];

	f16c = ecx1[29];

	tsx = isIntel && ebx7[11];
	asf = false;
#else
	memset (this, 0, sizeof (dseed::instructions::x86_instruction_info));
#endif
}

const dseed::instructions::arm_instruction_info& dseed::instructions::arm_instruction_info::instance ()
{
	static dseed::instructions::arm_instruction_info info;
	return info;
}

#if ARCH_ARMSET
#	if ARCH_ARM && PLATFORM_UNIX
#		include <elf.h>
#		include <fcntl.h>
#		include <linux/auxvec.h>
#		include <unistd.h>
#	endif
#endif

dseed::instructions::arm_instruction_info::arm_instruction_info ()
{
#if ARCH_ARMSET
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
#else
	memset (this, 0, sizeof (dseed::instructions::arm_instruction_info));
#endif
}