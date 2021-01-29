CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

INCLUDE(GenerateExportHeader)
INCLUDE(CMakePackageConfigHelpers)

FIND_PACKAGE(Git QUIET)
IF(GIT_FOUND)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
		WORKING_DIRECTORY "${local_dir}"
		OUTPUT_VARIABLE DSEED_VERSION_GIT_HASH
		ERROR_QUIET
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	IF(DSEED_VERSION_GIT_HASH STREQUAL "")
		SET(DSEED_VERSION_GIT_HASH "nogit")
	ENDIF()
ELSE()
	SET(DSEED_VERSION_GIT_HASH "unknown")
ENDIF()

SET(DSEED_VERSION_MAJOR			0)
SET(DSEED_VERSION_MINOR			1)
SET(DSEED_VERSION_REVISION		0)
SET(DSEED_VERSION_CMAKE			${DSEED_VERSION_MAJOR}.${DSEED_VERSION_MINOR}.${DSEED_VERSION_REVISION})
SET(DSEED_VERSION				${DSEED_VERSION_CMAKE}-${DSEED_VERSION_GIT_HASH})

SET(INSTALL_BIN_DIR				${CMAKE_INSTALL_PREFIX}/bin				CACHE PATH "Installation Directory for Executable files.")
SET(INSTALL_LIB_DIR				${CMAKE_INSTALL_PREFIX}/lib				CACHE PATH "Installation Directory for Library files.")
SET(INSTALL_INC_DIR				${CMAKE_INSTALL_PREFIX}/include			CACHE PATH "Installation Directory for Header files.")
SET(INSTALL_PKGCONFIG_DIR		${CMAKE_INSTALL_PREFIX}/lib/pkgconfig	CACHE PATH "Installation Directory for pkgconfig file.")
LINK_DIRECTORIES(${INSTALL_LIB_DIR})

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET(DSEED_PROJECT_NAME dseed)
SET(DSEED_PROJECT_LANGUAGES C CXX)
IF(PLATFORM_MACOS OR PLATFORM_IOS)
	LIST(APPEND DSEED_PROJECT_LANGUAGES OBJC OBJCXX)
ENDIF()

PROJECT(
	${DSEED_PROJECT_NAME}
	VERSION ${DSEED_VERSION_CMAKE}
	LANGUAGES ${DSEED_PROJECT_LANGUAGES}
	DESCRIPTION "Light-weight Native Game Engine"
)

INCLUDE("cmake/detect_target.cmake")

MESSAGE(STATUS "==============================================================")
MESSAGE(STATUS "   _ _ _         _                   _ ")
MESSAGE(STATUS "  | (_) |__   __| |___  ___  ___  __| |")
MESSAGE(STATUS "  | | | '_ \\ / _` / __|/ _ \\/ _ \\/ _` |")
MESSAGE(STATUS "  | | | |_) | (_| \\__ \\  __/  __/ (_| |")
MESSAGE(STATUS "  |_|_|_.__/ \\__,_|___/\\___|\\___|\\__,_|")
MESSAGE(STATUS "")
MESSAGE(STATUS "==============================================================")

MESSAGE(STATUS "[dseed] libdseed Version: ${DSEED_VERSION}")
MESSAGE(STATUS "[dseed] CMake version:    ${CMAKE_VERSION}")
MESSAGE(STATUS "[dseed] git Version:      ${GIT_VERSION_STRING}")
MESSAGE(STATUS "[dseed] Build Target:     ${CMAKE_SYSTEM_NAME} (${TARGET_ARCH})")
MESSAGE(STATUS "[dseed] Build Type:       ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "[dseed] Compiler:         C++ = ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}, C = ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
IF(PLATFORM_MACOS OR PLATFORM_IOS)
	MESSAGE(STATUS "                          Objective-C++ = ${CMAKE_OBJCXX_COMPILER_ID} ${CMAKE_OBJCXX_COMPILER_VERSION}")
	MESSAGE(STATUS "                          Objective-C = ${CMAKE_OBJC_COMPILER_ID} ${CMAKE_OBJC_COMPILER_VERSION}")
ENDIF()
MESSAGE(STATUS "[dseed] Packages Path:    ${CMAKE_SYSTEM_PREFIX_PATH}")

OPTION(DSEED_SKIP_INSTALL					"libdseed do not install."						OFF)
OPTION(DSEED_SKIP_BUILD_APPS				"libdseed do not build apps."					OFF)

OPTION(DSEED_USE_PNG						"libdseed use libpng decoding and encoding"		ON)
OPTION(DSEED_USE_JPEG						"libdseed use libjpeg decoding and encoding"	ON)
OPTION(DSEED_USE_JPEG2000					"libdseed use openjpeg decoding and encoding"	ON)
OPTION(DSEED_USE_WEBP						"libdseed use libwebp decoding and encoding"	ON)
OPTION(DSEED_USE_TIFF						"libdseed use libtIFf decoding and encoding"	ON)
OPTION(DSEED_USE_GIF						"libdseed use gIFlib decoding and encoding"		ON)
OPTION(DSEED_USE_WINDOWS_IMAGING_CODECS		"libdseed use Microsoft Windows Imaging Codecs"	ON)

OPTION(DSEED_USE_VORBIS						"libdseed use libvorbis decoding and encoding"	ON)
OPTION(DSEED_USE_OPUS						"libdseed use libopus decoding and encoding"	ON)
OPTION(DSEED_USE_FLAC						"libdseed use libFLAC decoding and encoding"	ON)
OPTION(DSEED_USE_MEDIA_FOUNDATION			"libdseed use Microsoft Media Foundation"		ON)

option(DSEED_USE_BITMAP_ENCODERS			"libdseed use bitmap encoders"					ON)

option(DSEED_USE_X11						"libdseed use X11 in Linux/UNIX"				ON)

option(DSEED_USE_OPENGL						"libdseed use OpenGL(ES)"						ON)
option(DSEED_USE_VULKAN						"libdseed use Vulkan"							ON)
option(DSEED_USE_METAL						"libdseed use Metal"							ON)

SET(DSEED_LIB_SOURCES
	# Processor Instructions
    src/instructions/instruction_info.cpp
	src/instructions/utility_instructions.cpp

	# Static Components
	src/common.cpp
	src/time.cpp
	src/strings.cpp
	src/math/half.cpp
	
	src/math/fraction.cpp
	src/color.cpp

	# Reference Count Object
	src/object.cpp
	
	# Stream
	src/io/stream.cpp

	# Diagnostics
	src/diagnostics/diagnostics.cpp

	# Bitmap
	src/bitmap/bitmap.cpp
	src/bitmap/bitmap_determine.cpp
	src/bitmap/bitmap_reformat.cpp
	src/bitmap/bitmap_operation.cpp
	src/bitmap/bitmap_resize.cpp
	src/bitmap/bitmap_filter.cpp
	src/bitmap/bitmap_histogram.cpp
	src/bitmap/bitmap_flip.cpp
	src/bitmap/bitmap_element.cpp
	
	# Bitmap Decoders
	src/bitmap/decoders/wic_decoder.cpp
	src/bitmap/decoders/dib_decoder.cpp
	src/bitmap/decoders/tga_decoder.cpp
	src/bitmap/decoders/ico_decoder.cpp
	src/bitmap/decoders/dds_decoder.cpp
	src/bitmap/decoders/ktx_decoder.cpp
	src/bitmap/decoders/pkm_decoder.cpp
	src/bitmap/decoders/astc_decoder.cpp
	src/bitmap/decoders/png_decoder.cpp
	src/bitmap/decoders/jpeg_decoder.cpp
	src/bitmap/decoders/jp2_decoder.cpp
	src/bitmap/decoders/webp_decoder.cpp
	src/bitmap/decoders/tiff_decoder.cpp
	src/bitmap/decoders/gif_decoder.cpp

	# Bitmap Encoders
	src/bitmap/encoders/dib_encoder.cpp
	src/bitmap/encoders/png_encoder.cpp
	src/bitmap/encoders/gif_encoder.cpp
	src/bitmap/encoders/webp_encoder.cpp
	src/bitmap/encoders/dds_encoder.cpp
	src/bitmap/encoders/ico_encoder.cpp
	src/bitmap/encoders/jpeg_encoder.cpp
	src/bitmap/encoders/tiff_encoder.cpp
	src/bitmap/encoders/wic_encoder.cpp

	# Media (Audio)
	src/media/audio.cpp
	src/media/audio_stream.cpp
	src/media/audio_filter.cpp
	src/media/audio_reformat.cpp
	src/media/audio_resample.cpp
	src/media/audio_rechannel.cpp
	src/media/audio_spectrum.cpp
	src/media/media.cpp
	
	# Media Decoders
	src/media/decoders/mf_decoder.cpp
	src/media/decoders/wav_decoder.cpp
	src/media/decoders/mp3_decoder.cpp
	src/media/decoders/vorbis_decoder.cpp
	src/media/decoders/opus_decoder.cpp
	src/media/decoders/flac_decoder.cpp

	# Media Encoders
	src/media/encoders/wav_encoder.cpp

	# Platform
	src/platform/platform.cpp
	src/platform/platform_windows.cpp
	src/platform/platform_uwp.cpp
	src/platform/platform_macos.mm
	src/platform/platform_ios.mm
	src/platform/platform_linux.cpp
	src/platform/platform_android.cpp

	# Graphics
	src/graphics/graphics_dxgi.cpp
	src/graphics/graphics_d3d11.cpp
	src/graphics/graphics_d3d11_sprite.cpp

	# Audio
	src/audio/audio_wasapi_adapter.cpp
	src/audio/audio_xaudio2_player.cpp
	src/audio/audio_wasapi_requester.cpp
)
SET(DSEED_DEFINITIONS)
SET(DSEED_INCLUDE_DIRS)
SET(DSEED_LINK_LIBS)
SET(DSEED_BUILD_TYPE SHARED)

IF(PLATFORM_WINDOWS)
	LIST(APPEND DSEED_DEFINITIONS -DWINDOWS)
	LIST(APPEND DSEED_DEFINITIONS -DUNICODE)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
	SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
	LIST(APPEND DSEED_DEFINITIONS -DDSEED_WINDOWS_DLL_EXPORT)
	LIST(APPEND DSEED_LINK_LIBS xinput.lib portabledeviceguids.lib sensorsapi.lib dxgi.lib d3d11.lib d3dcompiler.lib XAudio2.lib winmm.lib)
ELSEIF(PLATFORM_UWP)
	SET(PLATFORM STORE)
	LIST(APPEND DSEED_DEFINITIONS -DUNICODE)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /ZW /utf-8")
	# Find Windows.winmd
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -AI\"C:/Program Files (x86)/Windows Kits/10/UnionMetadata\"")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -AI\"C:/Program Files (x86)/Windows Kits/10/UnionMetadata/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}\"")
	# Find Platform.winmd for VS2017
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -AI\"C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/IDE/VC/vcpackages\"")
	# Find Platform.winmd for VS2019
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -AI\"C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/VC/vcpackages\"")
	LIST(APPEND DSEED_LINK_LIBS XInput.lib XAudio2.lib winmm.lib)
ENDIF()

IF(COMPILER_GCC OR COMPILER_CLANG)
	MESSAGE(STATUS "[dseed] Compiler is GCC or Clang. Target architecture: ${TARGET_ARCH}")
	IF(ARCH_IA32 OR ARCH_AMD64)
		MESSAGE(STATUS "[dseed] SIMD and F16C instruction settings.")
		add_compile_options(-msse -msse2 -msse3 -msse4.1 -msse4.2 -mssse3 -mavx -mfma -mavx2 -mf16c -mrdrnd)
	ELSEIF(ARCH_ARM OR ARCH_ARM64)
		MESSAGE(STATUS "[dseed] SIMD instruction settings.")
		add_compile_options(-mfpu=neon)
	ENDIF()
ENDIF()

IF(MSVC)
	LIST(APPEND DSEED_DEFINITIONS -DDSEED_DLL_EXP)
ENDIF()

INCLUDE("cmake/detect_libraries.cmake")

############################################################################################
#
# Build libdseed Library
#
############################################################################################
ADD_LIBRARY(${DSEED_PROJECT_NAME} ${DSEED_BUILD_TYPE} ${DSEED_LIB_SOURCES})
TARGET_INCLUDE_DIRECTORIES(${DSEED_PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/> $<INSTALL_INTERFACE:${INSTALL_INC_DIR}>)
TARGET_INCLUDE_DIRECTORIES(${DSEED_PROJECT_NAME} PRIVATE ${DSEED_INCLUDE_DIRS})
TARGET_COMPILE_DEFINITIONS(${DSEED_PROJECT_NAME} PRIVATE ${DSEED_DEFINITIONS})
TARGET_LINK_LIBRARIES(${DSEED_PROJECT_NAME} ${DSEED_LINK_LIBS})
IF(${CMAKE_SYSTEM_NAME} STREQUAL "WindowsStore")
	MESSAGE(STATUS "Windows Store Target Platform Version: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
	SET_PROPERTY(TARGET ${DSEED_PROJECT_NAME} PROPERTY VS_DESKTOP_EXTENSIONS_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
	SET_PROPERTY(TARGET ${DSEED_PROJECT_NAME} PROPERTY VS_MOBILE_EXTENSIONS_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
ENDIF()
SET_TARGET_PROPERTIES(${DSEED_PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
IF(PLATFORM_MACOS)
	TARGET_LINK_LIBRARIES(${DSEED_PROJECT_NAME} "-framework Cocoa")
	TARGET_LINK_LIBRARIES(${DSEED_PROJECT_NAME} "-framework GameController")
	IF(DSEED_USE_OPENGL)
		TARGET_LINK_LIBRARIES(${DSEED_PROJECT_NAME} "-framework OpenGL")
	ENDIF()
	IF(DSEED_USE_METAL)
		TARGET_LINK_LIBRARIES(${DSEED_PROJECT_NAME} "-framework Metal")
	ENDIF()
ENDIF()
IF(PLATFORM_IOS)
	TARGET_LINK_LIBRARIES(${DSEED_PROJECT_NAME} "-framework AppKit")
	TARGET_LINK_LIBRARIES(${DSEED_PROJECT_NAME} "-framework GameController")
	IF(DSEED_USE_OPENGL)
		TARGET_LINK_LIBRARIES(${DSEED_PROJECT_NAME} "-framework OpenGLES")
	ENDIF()
	IF(DSEED_USE_METAL)
		TARGET_LINK_LIBRARIES(${DSEED_PROJECT_NAME} "-framework Metal")
	ENDIF()
ENDIF()
IF(DSEED_USE_X11)
	TARGET_LINK_LIBRARIES(${DSEED_PROJECT_NAME} GL)
	TARGET_LINK_LIBRARIES(${DSEED_PROJECT_NAME} GLU)
ENDIF()

############################################################################################
#
# Build Test Application
#
############################################################################################
IF(NOT ${DSEED_SKIP_BUILD_APPS})
	IF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" OR ${CMAKE_SYSTEM_NAME} STREQUAL "MSYS")
		SET(TARGET_TO "WIN32")
	ENDIF()

	SET(TEST_APP_NAME dseed_test_app)
	SET(TEST_APP_PACKAGE_GUID "6514377e-dfd4-4cdb-80df-4e0366346eff")
	ADD_EXECUTABLE(${TEST_APP_NAME} ${TARGET_TO} app/test_app/test.cpp app/test_app/app.manifest)
	TARGET_INCLUDE_DIRECTORIES(${TEST_APP_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
	TARGET_LINK_LIBRARIES(${TEST_APP_NAME} dseed)
	SET_TARGET_PROPERTIES(${TEST_APP_NAME} PROPERTIES CXX_STANDARD 17)
	
	SET(TEST_BASIC_WINDOW_NAME dseed_basic_window)
	SET(TEST_APP_PACKAGE_GUID "6514377e-dfd4-4cdb-80df-4e0366346eff")
	ADD_EXECUTABLE(${TEST_BASIC_WINDOW_NAME} ${TARGET_TO} app/test_basic_window/test_basic_window.cpp app/test_app/app.manifest)
	TARGET_INCLUDE_DIRECTORIES(${TEST_BASIC_WINDOW_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
	TARGET_LINK_LIBRARIES(${TEST_BASIC_WINDOW_NAME} dseed)
	SET_TARGET_PROPERTIES(${TEST_BASIC_WINDOW_NAME} PROPERTIES CXX_STANDARD 17)
	
	SET(TEST_BITMAP_NAME dseed_bitmap)
	ADD_EXECUTABLE(${TEST_BITMAP_NAME} app/test_bitmap/test.cpp)
	TARGET_INCLUDE_DIRECTORIES(${TEST_BITMAP_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
	TARGET_LINK_LIBRARIES(${TEST_BITMAP_NAME} dseed)
	SET_TARGET_PROPERTIES(${TEST_BITMAP_NAME} PROPERTIES CXX_STANDARD 17)
ENDIF()

############################################################################################
#
# Installation
#
############################################################################################
IF(NOT ${DSEED_SKIP_INSTALL})
	generate_export_header(${DSEED_PROJECT_NAME})
	SET_PROPERTY(TARGET ${DSEED_PROJECT_NAME} PROPERTY VERSION ${DSEED_VERSION})
	SET_PROPERTY(TARGET ${DSEED_PROJECT_NAME} PROPERTY SOVERSION ${DSEED_VERSION_MAJOR})
	SET_PROPERTY(TARGET ${DSEED_PROJECT_NAME} APPEND PROPERTY COMPATIBLE_INTERFACE_STRING ${DSEED_VERSION_MAJOR})

	SET(DSEED_PC ${CMAKE_CURRENT_BINARY_DIR}/${DSEED_PROJECT_NAME}.pc)
	CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${DSEED_PROJECT_NAME}.pc.in ${DSEED_PC} @ONLY)
	
	IF(PLATFORM_MACOS OR PLATFORM_IOS)
		SET_TARGET_PROPERTIES(${DSEED_PROJECT_NAME} PROPERTIES
			FRAMEWORK TRUE
			FRAMEWORK_VERSION A
			MACOSX_FRAMEWORK_IDENTIFIER in.daram.dseed
			MACOSX_FRAMEWORK_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist
			VERSION ${DSEED_VERSION_CMAKE}
			SOVERSION 1.0.0
		)
	ENDIF()

	INSTALL(
		TARGETS ${DSEED_PROJECT_NAME}
		EXPORT ${DSEED_PROJECT_NAME}Targets
		RUNTIME DESTINATION ${INSTALL_BIN_DIR}
		LIBRARY DESTINATION ${INSTALL_LIB_DIR}
		ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
		FRAMEWORK DESTINATION ${INSTALL_LIB_DIR}
		PUBLIC_HEADER DESTINATION ${INSTALL_INC_DIR}
		INCLUDES DESTINATION ${INSTALL_INC_DIR}
	)
	INSTALL(
		DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
		DESTINATION ${INSTALL_INC_DIR}
	)
	INSTALL(
		FILES ${DSEED_PC}
		DESTINATION ${INSTALL_PKGCONFIG_DIR}
	)
	
	write_basic_package_version_file(
		"${CMAKE_CURRENT_BINARY_DIR}/${DSEED_PROJECT_NAME}ConfigVersion.cmake"
		VERSION ${DSEED_VERSION}
		COMPATIBILITY AnyNewerVersion
	)

	EXPORT(
		EXPORT ${DSEED_PROJECT_NAME}Targets
		FILE "${CMAKE_CURRENT_BINARY_DIR}/${DSEED_PROJECT_NAME}Targets.cmake"
	)
	INSTALL(
		EXPORT ${DSEED_PROJECT_NAME}Targets
		FILE ${DSEED_PROJECT_NAME}Config.cmake
		DESTINATION "${INSTALL_LIB_DIR}/cmake/${DSEED_PROJECT_NAME}"
	)
	INSTALL(
		FILES "${CMAKE_CURRENT_BINARY_DIR}/${DSEED_PROJECT_NAME}ConfigVersion.cmake"
		DESTINATION "${INSTALL_LIB_DIR}/cmake/${DSEED_PROJECT_NAME}"
	)
ENDIF ()