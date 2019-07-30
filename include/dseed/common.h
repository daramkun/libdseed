#ifndef __DSEED_COMMON_H__
#define __DSEED_COMMON_H__

#include <atomic>
#include <string>
#include <cstdint>
#include <climits>
#include <cfloat>
#include <cmath>
#include <functional>
using namespace std::string_literals;

#define COMPILER_MSVC										defined ( _MSC_VER )
#define COMPILER_GCC										defined ( __GNUC__ ) && !defined ( __clang__ )
#define COMPILER_CLANG										defined ( __clang__ )

#define PLATFORM_ANDROID									( defined ( __ANDROID__ ) )
#define PLATFORM_FUCHSIA									( defined ( __Fuchsia__ ) )
#define PLATFORM_GOOGLE										( PLATFORM_ANDROID || PLATFORM_FUCHSIA )
#define PLATFORM_UNIX										( defined ( __unix__ ) || defined ( __linux__ ) ) && !defined ( __ANDROID__ )
#define PLATFORM_WEBASSEMBLY								( defined ( __EMSCRIPTEN__ ) || defined ( __asmjs__ ) )
#if defined ( __APPLE__ )
#	include <TargetConditionals.h>
#	define PLATFORM_IOS										( TARGET_OS_IOS || TARGET_OS_SIMULATOR )
#	define PLATFORM_MACOS									( TARGET_OS_MAC && !PLATFORM_IOS )
#else
#	define PLATFORM_IOS										0
#	define PLATFORM_MACOS									0
#endif
#define PLATFORM_APPLE										( PLATFORM_IOS || PLATFORM_MACOS )
#if ( defined ( _WINDOWS ) || defined ( _WIN32 ) || defined ( _WIN64 ) ) \
	&& !( PLATFORM_APPLE || PLATFORM_GOOGLE || PLATFORM_UNIX || PLATFORM_WEBASSEMBLY )
#	include <Windows.h>
#	define PLATFORM_WINDOWS									( WINAPI_FAMILY_PARTITION ( WINAPI_PARTITION_DESKTOP ) || !defined ( WINAPI_FAMILY_DESKTOP_APP ) )
#	define PLATFORM_UWP										( WINAPI_FAMILY_PARTITION ( WINAPI_FAMILY_PC_APP ) && !PLATFORM_WINDOWS )
#	if PLATFORM_UWP
#		include <wrl.h>
#	endif
#else
#	define PLATFORM_WINDOWS									0
#	define PLATFORM_UWP										0
#endif
#define PLATFORM_MICROSOFT									( PLATFORM_WINDOWS || PLATFORM_UWP )

#define PLATFORM_DESKTOP									( PLATFORM_WINDOWS || PLATFORM_MACOS || PLATFORM_UNIX )
#define PLATFORM_HANDHELD									( PLATFORM_IOS || PLATFORM_ANDROID )
#define PLATFORM_UNIVERSAL									( PLATFORM_UWP || PLATFORM_FUCHSIA )

#if PLATFORM_MICROSOFT
constexpr char PATH_SEPARATOR = '\\';
#else
constexpr char PATH_SEPARATOR = '/';
#endif

#define ARCH_IA32											( defined ( _M_IX86 ) || defined ( __i386__ ) )
#define ARCH_AMD64											( defined ( _M_AMD64 ) || defined ( __amd64__ ) )
#define ARCH_X86SET											( ARCH_IA32 || ARCH_AMD64 )
#define ARCH_ARM											( defined ( _M_ARM ) || defined ( __arm__ ) )
#define ARCH_ARM64											( defined ( _M_ARM64 ) || defined ( __aarch64__ ) )
#define ARCH_ARMSET											( ARCH_ARM || ARCH_ARM64 )
#define ARCH_RISCV32										( ( defined ( __riscv ) || defined ( __riscv__ ) ) && ( __riscv_xlen == 32 ) )
#define ARCH_RISCV64										( ( defined ( __riscv ) || defined ( __riscv__ ) ) && ( __riscv_xlen == 64 ) )
#define ARCH_RISCVSET										( ARCH_RISCV32 || ARCH_RISCV64 )

#if COMPILER_MSVC && defined ( DSEED_WINDOWS_DLL_EXPORT )
#   define DSEEDEXP											__declspec ( dllexport )
#elif COMPILER_MSVC && !defined ( DSEED_WINDOWS_DLL_EXPORT )
#   define DSEEDEXP											__declspec ( dllimport )
#else
#   define DSEEDEXP
#endif

#ifndef PURE
#define PURE												= 0
#endif

#if COMPILER_MSVC
#	define ALIGN(x)											__declspec(align(x))
#else
#	define ALIGN(x)											__attribute__ ((aligned (x)))
#endif

namespace dseed
{
	using error_t = int32_t;
	constexpr bool succeeded (error_t err) { return err >= 0; }
	constexpr bool failed (error_t err) { return err < 0; }

	constexpr error_t error_good = 0;

	constexpr error_t error_fail = 0x80000000;
	constexpr error_t error_version_mismatch = 0x80000001;

	constexpr error_t error_invalid_args = 0x80010001;
	constexpr error_t error_invalid_op = 0x80010002;
	constexpr error_t error_out_of_range = 0x80010003;
	constexpr error_t error_out_of_memory = 0x80010004;
	constexpr error_t error_not_contained = 0x80010005;
	constexpr error_t error_type_incorrect = 0x80010006;

	constexpr error_t error_io = 0x80020001;
	constexpr error_t error_file_not_found = 0x80020002;
	constexpr error_t error_end_of_file = 0x80020003;
	constexpr error_t error_hash_not_correct = 0x80020004;
	constexpr error_t error_access_denied = 0x80020005;
	constexpr error_t error_not_support_file_format = 0x80020006;

	constexpr error_t error_not_impl = 0x80030001;
	constexpr error_t error_not_support = 0x80030002;
	constexpr error_t error_platform_not_support = 0x80030003;
	constexpr error_t error_feature_not_support = 0x80030004;

	constexpr error_t error_device_not_conn = 0x80040001;
	constexpr error_t error_device_disconn = 0x80040002;
	constexpr error_t error_device_reset = 0x80040003;

	constexpr error_t error_network_not_conn = 0x80050001;
	constexpr error_t error_network_disconn = 0x80060002;
	constexpr error_t error_network_sync_fail = 0x80060003;
}

#endif