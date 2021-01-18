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

PROJECT(
	${DSEED_PROJECT_NAME}
	VERSION ${DSEED_VERSION_CMAKE}
	LANGUAGES C CXX
	DESCRIPTION "Light-weight Native Game Engine"
)

SET(TARGET_ARCH_REGEX "^.*-march[= ]([^ ]+).*$")
STRING(REGEX MATCH "${TARGET_ARCH_REGEX}" TARGET_ARCH_MATCH ${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS})
IF(TARGET_ARCH_MATCH)
    STRING(REGEX REPLACE "${TARGET_ARCH_REGEX}" "\\1" TARGET_ARCH ${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS})
ELSE()
    SET(TARGET_ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})
ENDIF()

MESSAGE(STATUS "==============================================================")
MESSAGE(STATUS "   _ _ _         _                   _ ")
MESSAGE(STATUS "  | (_) |__   __| |___  ___  ___  __| |")
MESSAGE(STATUS "  | | | '_ \\ / _` / __|/ _ \\/ _ \\/ _` |")
MESSAGE(STATUS "  | | | |_) | (_| \\__ \\  __/  __/ (_| |")
MESSAGE(STATUS "  |_|_|_.__/ \\__,_|___/\\___|\\___|\\__,_|")
MESSAGE(STATUS "")
MESSAGE(STATUS "==============================================================")

MESSAGE(STATUS "libdseed Version:  ${DSEED_VERSION}")
MESSAGE(STATUS "CMake version:    ${CMAKE_VERSION}")
MESSAGE(STATUS "git Version:      ${GIT_VERSION_STRING}")
MESSAGE(STATUS "Build Target:     ${CMAKE_SYSTEM_NAME} (${TARGET_ARCH})")
MESSAGE(STATUS "Build Type:       ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "Compiler:         C++ = ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}, C = ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
MESSAGE(STATUS "Packages Path:    ${CMAKE_SYSTEM_PREFIX_PATH}")

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

IF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" OR ${CMAKE_SYSTEM_NAME} STREQUAL "MSYS")
	LIST(APPEND DSEED_DEFINITIONS -DWINDOWS)
	LIST(APPEND DSEED_DEFINITIONS -DUNICODE)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
	SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
	list(APPEND DSEED_DEFINITIONS -DDSEED_WINDOWS_DLL_EXPORT)
	LIST(APPEND DSEED_LINK_LIBS xinput.lib portabledeviceguids.lib sensorsapi.lib dxgi.lib d3d11.lib d3dcompiler.lib XAudio2.lib winmm.lib)
ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL "WindowsStore")
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

FIND_PACKAGE(SSE QUIET)
IF(C_SSE2_FOUND)
	SET(CMAKE_CXX_FLAGS "${C_SSE2_FLAGS} -DUSE_SSE2 ${CMAKE_CXX_FLAGS}")
ENDIF()
IF(C_SSE3_FOUND)
	SET(CMAKE_CXX_FLAGS "${C_SSE3_FLAGS} -DUSE_SSE3 ${CMAKE_CXX_FLAGS}")
ENDIF()
IF(C_SSE4_1_FOUND)
	SET(CMAKE_CXX_FLAGS "${C_SSE4_1_FLAGS} -DUSE_SSE4_1 ${CMAKE_CXX_FLAGS}")
ENDIF()
IF(C_SSE4_2_FOUND)
	SET(CMAKE_CXX_FLAGS "${C_SSE4_2_FLAGS} -DUSE_SSE4_2 ${CMAKE_CXX_FLAGS}")
ENDIF()

FIND_PACKAGE(ARM QUIET)
IF(NEON_FOUND)
	SET(CMAKE_CXX_FLAGS "-mfpu=neon -D__NEON__ ${CMAKE_CXX_FLAGS}")
ENDIF()
IF(CORTEXA8_FOUND)
	SET(CMAKE_CXX_FLAGS "-mcpu=cortex-a8 -fprefetch-loop-arrays ${CMAKE_CXX_FLAGS}")
ENDIF()
IF(CORTEXA9_FOUND)
	SET(CMAKE_CXX_FLAGS "-mcpu=cortex-a9 ${CMAKE_CXX_FLAGS}")
ENDIF()

IF(MSVC)
	LIST(APPEND DSEED_DEFINITIONS -DDSEED_DLL_EXP)
ENDIF()

# PNG
IF(${DSEED_USE_PNG})
	FIND_PACKAGE(ZLIB QUIET)
	FIND_PACKAGE(PNG QUIET)
	
	IF(NOT ZLIB_FOUND)
		MESSAGE(STATUS "# Not found zlib")
		SET(DSEED_USE_PNG OFF)
	ENDIF()

	IF(NOT PNG_FOUND)
		MESSAGE(STATUS "# Not found libpng")
		SET(DSEED_USE_PNG OFF)
	ENDIF()

	IF(${DSEED_USE_PNG})
		MESSAGE(STATUS "# Use libpng")
		LIST(APPEND DSEED_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS})
		LIST(APPEND DSEED_LINK_LIBS ${ZLIB_LIBRARY} ${PNG_LIBRARY})
		LIST(APPEND DSEED_DEFINITIONS -DUSE_PNG)
	ENDIF()
ENDIF()

# JPEG
IF(${DSEED_USE_JPEG})
	FIND_PACKAGE(JPEG QUIET)
	IF(NOT JPEG_FOUND)
		SET(DSEED_USE_JPEG OFF)
		MESSAGE(STATUS "# Not found libjpeg or mozjpeg")
	ENDIF()

	IF(${DSEED_USE_JPEG})
		MESSAGE(STATUS "# Use libjpeg")
		LIST(APPEND DSEED_INCLUDE_DIRS ${JPEG_INCLUDE_DIRS})
		LIST(APPEND DSEED_LINK_LIBS ${JPEG_LIBRARY})
		LIST(APPEND DSEED_DEFINITIONS -DUSE_JPEG)
	ENDIF()
ENDIF()

# JPEG2000
IF(${DSEED_USE_JPEG2000})
	FIND_PACKAGE(openjpeg QUIET)
	IF(NOT OPENJPEG_FOUND)
		SET(DSEED_USE_JPEG2000 OFF)
		MESSAGE(STATUS "# Not found openjpeg")
	ENDIF()

	IF(${DSEED_USE_JPEG2000})
		MESSAGE(STATUS "# Use openjpeg")
		LIST(APPEND DSEED_INCLUDE_DIRS ${OPENJPEG_INCLUDE_DIRS})
		LIST(APPEND DSEED_LINK_LIBS ${OPENJPEG_LIBRARY} ${OPENJPEG_LIBRARIES})
		LIST(APPEND DSEED_DEFINITIONS -DUSE_JPEG2000)
	ENDIF()
ENDIF()

# WebP
IF(${DSEED_USE_WEBP})
	FIND_PACKAGE(webp QUIET)
	IF(NOT WEBP_FOUND)
		SET(DSEED_USE_WEBP OFF)
		MESSAGE(STATUS "# Not found libwebp")
	ENDIF()

	IF(${DSEED_USE_WEBP})
		MESSAGE(STATUS "# Use libwebp")
		LIST(APPEND DSEED_INCLUDE_DIRS ${WEBP_INCLUDE_DIRS})
		LIST(APPEND DSEED_LINK_LIBS ${WEBP_LIBRARY} ${WEBP_LIBRARIES})
		LIST(APPEND DSEED_DEFINITIONS -DUSE_WEBP)
	ENDIF()
ENDIF()

# libtiff
IF(${DSEED_USE_TIFF})
	FIND_PACKAGE(LibLZMA QUIET)
	FIND_PACKAGE(ZLIB QUIET)
	FIND_PACKAGE(tiff QUIET)
	IF(NOT ZLIB_FOUND OR NOT LZMA_FOUND OR NOT TIFF_FOUND)
		SET(DSEED_USE_TIFF OFF)
		MESSAGE(STATUS "# Not found zlib or liblzma or libtiff")
	ENDIF()

	IF(${DSEED_USE_TIFF})
		MESSAGE(STATUS "# Use libtiff")
		LIST(APPEND DSEED_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS} ${LIBLZMA_INCLUDE_DIRS} ${TIFF_INCLUDE_DIRS})
		LIST(APPEND DSEED_LINK_LIBS ${ZLIB_LIBRARY} ${LIBLZMA_LIBRARY} ${TIFF_LIBRARY})
		LIST(APPEND DSEED_DEFINITIONS -DUSE_TIFF)
	ENDIF()
ENDIF()

# giflib
IF(${DSEED_USE_GIF})
	FIND_PACKAGE(GIF QUIET)
	IF(NOT GIF_FOUND)
		SET(DSEED_USE_GIF OFF)
		MESSAGE(STATUS "# Not found giflib")
	ENDIF()

	IF(${DSEED_USE_GIF})
		MESSAGE(STATUS "# Use giflib")
		LIST(APPEND DSEED_INCLUDE_DIRS ${GIF_INCLUDE_DIRS})
		LIST(APPEND DSEED_LINK_LIBS ${GIF_LIBRARY})
		LIST(APPEND DSEED_DEFINITIONS -DUSE_GIF)
	ENDIF()
ENDIF()

# Windows Imaging Codecs
IF(${DSEED_USE_WINDOWS_IMAGING_CODECS})
	IF(NOT (CMAKE_SYSTEM_NAME STREQUAL "Windows") AND NOT (CMAKE_SYSTEM_NAME STREQUAL "MSYS") AND NOT (CMAKE_SYSTEM_NAME STREQUAL "WindowsStore"))
		SET(DSEED_USE_WINDOWS_IMAGING_CODECS OFF)
		MESSAGE(STATUS "# Not found Microsoft Windows Imaging Codecs")
	ENDIF()

	IF(${DSEED_USE_WINDOWS_IMAGING_CODECS})
		MESSAGE(STATUS "# Use Microsoft Windows Imaging Codecs")
		LIST(APPEND DSEED_LINK_LIBS windowscodecs.lib)
	ENDIF()
ENDIF()

# Vorbis
IF(${DSEED_USE_VORBIS})
	FIND_PACKAGE(ogg QUIET)
	FIND_PACKAGE(vorbis QUIET)
	IF(NOT OGG_FOUND OR NOT vorbis_FOUND)
		SET(DSEED_USE_VORBIS OFF)
		MESSAGE(STATUS "# Not found vorbis")
	ENDIF()

	IF(${DSEED_USE_VORBIS})
		MESSAGE(STATUS "# Use vorbis")
		LIST(APPEND DSEED_INCLUDE_DIRS ${OGG_INCLUDE_DIRS} ${VORBIS_INCLUDE_DIRS})
		LIST(APPEND DSEED_LINK_LIBS ${OGG_LIBRARY} ${VORBIS_LIBRARY} Vorbis::vorbis Vorbis::vorbisenc Vorbis::vorbisfile)
		LIST(APPEND DSEED_DEFINITIONS -DUSE_VORBIS)
	ENDIF()
ENDIF()

# libopus
IF(${DSEED_USE_OPUS})
	FIND_PACKAGE(ogg QUIET)
	FIND_PACKAGE(opus QUIET)
	FIND_PACKAGE(opusfile QUIET)
	IF(NOT OGG_FOUND OR NOT OPUS_FOUND)
		SET(DSEED_USE_OPUS OFF)
		MESSAGE(STATUS "# Not found libogg or libopus or opusfile")
	ENDIF()

	IF(${DSEED_USE_OPUS})
		MESSAGE(STATUS "# Use libopus")
		LIST(APPEND DSEED_INCLUDE_DIRS ${OGG_INCLUDE_DIRS} ${OPUS_INCLUDE_DIRS} ${OPUSFILE_INCLUDE_DIRS})
		LIST(APPEND DSEED_LINK_LIBS ${OGG_LIBRARY} ${OPUS_LIBRARY} ${OPUSFILE_LIBRARY})
		LIST(APPEND DSEED_DEFINITIONS -DUSE_OPUS)
	ENDIF()
ENDIF()

# libFLAC
IF(${DSEED_USE_FLAC})
	FIND_PACKAGE(ogg QUIET)
	FIND_PACKAGE(FLAC QUIET)
	IF(NOT OGG_FOUND OR NOT FLAC_FOUND)
		SET(DSEED_USE_FLAC OFF)
		MESSAGE(STATUS "# Not found libFLAC")
	ENDIF()

	IF(${DSEED_USE_FLAC})
		MESSAGE(STATUS "# Use libFLAC")
		LIST(APPEND DSEED_INCLUDE_DIRS ${OGG_INCLUDE_DIRS} ${FLAC_INCLUDE_DIRS})
		LIST(APPEND DSEED_LINK_LIBS ${OGG_LIBRARY} ${FLAC_LIBRARIES} FLAC::FLAC FLAC::FLAC++)
		LIST(APPEND DSEED_DEFINITIONS -DUSE_FLAC)
	ENDIF()
ENDIF()

# Media Foundation
IF(${DSEED_USE_MEDIA_FOUNDATION})
	IF(NOT (CMAKE_SYSTEM_NAME STREQUAL "Windows") AND NOT (CMAKE_SYSTEM_NAME STREQUAL "MSYS") AND NOT (CMAKE_SYSTEM_NAME STREQUAL "WindowsStore"))
		SET(DSEED_USE_MEDIA_FOUNDATION OFF)
		MESSAGE(STATUS "# Not found Microsoft Media Foundation")
	ENDIF()

	IF(${DSEED_USE_MEDIA_FOUNDATION})
		MESSAGE(STATUS "# Use Microsoft Media Foundation")
		LIST(APPEND DSEED_LINK_LIBS mf.lib mfplat.lib mfuuid.lib mfreadwrite.lib)
	ENDIF()
ENDIF()

IF(${DSEED_USE_BITMAP_ENCODERS})
	MESSAGE(STATUS "# Use Bitmap Encoders")
	LIST(APPEND DSEED_DEFINITIONS -DUSE_BITMAP_ENCODERS)
ENDIF()

FIND_PACKAGE(glew QUIET)
IF(NOT GLEW_FOUND)
	MESSAGE(STATUS "# Not found GLEW")
ELSE()
	MESSAGE(STATUS "# Use GLEW")
	LIST(APPEND DSEED_INCLUDE_DIRS ${GLEW_INCLUDE_DIRS})
	LIST(APPEND DSEED_LINK_LIBS ${GLEW_LIBRARY})
	LIST(APPEND DSEED_DEFINITIONS -DUSE_GLEW)
ENDIF()

FIND_PACKAGE(Vulkan QUIET)
IF(NOT VULKAN_FOUND)
	MESSAGE(STATUS "# Not found Vulkan SDK")
ELSE()
	MESSAGE(STATUS "# Use Vulkan")
	LIST(APPEND DSEED_INCLUDE_DIRS ${VULKAN_INCLUDE_DIRS})
	LIST(APPEND DSEED_LINK_LIBS ${VULKAN_LIBRARY})
	LIST(APPEND DSEED_DEFINITIONS -DUSE_VULKAN)
ENDIF()

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