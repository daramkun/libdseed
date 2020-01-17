cmake_minimum_required ( VERSION 3.10 )

INCLUDE ( GenerateExportHeader )
INCLUDE ( CMakePackageConfigHelpers )

############################################################################################
#
# Define Versions
#
############################################################################################
SET ( DSEED_VERSION_MAJOR 1 )
SET ( DSEED_VERSION_MINOR 0 )
SET ( DSEED_VERSION_REVISION 0 )
SET ( DSEED_VERSION ${DSEED_VERSION_MAJOR}.${DSEED_VERSION_MINOR}.${DSEED_VERSION_REVISION} )

############################################################################################
#
# Define Project Informations
#
############################################################################################
PROJECT ( dseed
	VERSION ${DSEED_VERSION}
	LANGUAGES C CXX
	DESCRIPTION "This is Core library for personal."
)

############################################################################################
#
# Setting Install Pathes
#
############################################################################################
SET ( INSTALL_BIN_DIR				"${CMAKE_INSTALL_PREFIX}/bin"			CACHE PATH "Installation directory for executables." )
SET ( INSTALL_LIB_DIR				"${CMAKE_INSTALL_PREFIX}/lib"			CACHE PATH "Installation directory for libraries." )
SET ( INSTALL_INC_DIR				"${CMAKE_INSTALL_PREFIX}/include"		CACHE PATH "Installation directory for headers." )
SET ( INSTALL_PKGCONFIG_DIR			"${CMAKE_INSTALL_PREFIX}/lib/pkgconfig"	CACHE PATH "Installation directory for pkgconfig (.pc) files" )

############################################################################################
#
# Setting CMake Build Options
#
############################################################################################
OPTION ( DSEED_SKIP_INSTALL			"libdseed do not install."				OFF )
OPTION ( DSEED_SKIP_BUILD_APP		"libdseed do not build app."			OFF )

OPTION ( DSEED_USE_PNG				"libdseed use PNG Decode/Encode."		ON  )
OPTION ( DSEED_USE_JPEG				"libdseed use JPEG Decode/Encode."		ON  )
OPTION ( DSEED_USE_JPEG2000			"libdseed use JPEG2000 Decode."			ON  )
OPTION ( DSEED_USE_WEBP				"libdseed use Webp Decode/Encode."		ON  )
OPTION ( DSEED_USE_TIFF				"libdseed use TIFF Decode/Encode."		ON  )
OPTION ( DSEED_USE_GIF				"libdseed use GIF Decode/Encode."		ON  )
OPTION ( DSEED_USE_SQUISH			"libdseed use BC1~5 Decode/Encode."		ON  )

OPTION ( DSEED_USE_VORBIS			"libdseed use Vorbis Decode/Encode."	ON  )
OPTION ( DSEED_USE_OPUS				"libdseed use Opus Decode/Encode."		ON  )
OPTION ( DSEED_USE_FLAC				"libdseed use FLAC Decode."				ON  )

OPTION ( DSEED_USE_BITMAP_ENCODERS	"libdseed use Encoder APIs."			ON  )
OPTION ( DSEED_USE_MEDIA_ENCODERS	"libdseed use Encoder APIs."			ON  )

SET_PROPERTY ( GLOBAL PROPERTY USE_FOLDERS ON )

############################################################################################
#
# Preconfiguration External Libraries
#
############################################################################################
IF ( ${DSEED_USE_PNG} )
	FIND_PACKAGE ( zlib )
	FIND_PACKAGE ( png )

	IF ( ( NOT DEFINED ZLIB_LIBRARY ) OR ( "${ZLIB_LIBRARY}" STREQUAL "ZLIB_LIBRARY-NOTFOUND" ) )
		FIND_LIBRARY ( ZLIB_LIBRARY NAMES zlibstatic zlibstatic.lib )
	ENDIF ()
	
	IF ( ( NOT DEFINED PNG_LIBRARY ) OR ( "${PNG_LIBRARY}" STREQUAL "PNG_LIBRARY-NOTFOUND") )
		FIND_LIBRARY ( PNG_LIBRARY NAMES png16_static png_static libpng16_static libpng16_static.lib png16_static.lib libpng16_static.a png16_static.a )
		MESSAGE ( "PNG_LIBRARY: ${PNG_LIBRARY}" )
	ENDIF ()

	IF ( ( ( NOT DEFINED ZLIB_INCLUDE_DIRS ) OR ( ${ZLIB_INCLUDE_DIRS} STREQUAL "ZLIB_INCLUDE_DIRS-NOTFOUND" ) )
		OR ( ( NOT DEFINED ZLIB_LIBRARY ) OR ( ${ZLIB_LIBRARY} STREQUAL "ZLIB_LIBRARY-NOTFOUND" ) )
		OR ( ( NOT DEFINED PNG_INCLUDE_DIRS ) OR ( ${PNG_INCLUDE_DIRS} STREQUAL "PNG_INCLUDE_DIRS-NOTFOUND" ) )
		OR ( ( NOT DEFINED PNG_LIBRARY ) OR ( ${PNG_LIBRARY} STREQUAL "PNG_LIBRARY-NOTFOUND" ) ) )
		MESSAGE ( "-- Not found zlib or libpng" )
		SET ( DSEED_USE_PNG OFF )
	ENDIF ()
ENDIF ()
IF ( ${DSEED_USE_JPEG} )
	FIND_PACKAGE ( jpeg QUIET )
	IF ( NOT DEFINED JPEG_FOUND )
		SET ( DSEED_USE_JPEG OFF )
		MESSAGE ( "-- Not found libjpeg-turbo or mozjpeg" )
	ENDIF ()
ENDIF ()
IF ( ${DSEED_USE_JPEG2000} )
	FIND_PACKAGE ( openjpeg QUIET )
	IF ( NOT DEFINED OPENGJPEG_FOUND )
		FIND_PATH ( OPENJPEG_INCLUDE_DIRS NAMES openjpeg.h )
		FIND_LIBRARY ( OPENJPEG_LIBRARY NAMES openjp2 )
		
		IF ( ( ( NOT DEFINED OPENJPEG_INCLUDE_DIRS ) OR ( NOT DEFINED OPENJPEG_LIBRARY ) ) OR ( "${OPENJPEG_LIBRARY}" STREQUAL "OPENJPEG_LIBRARY-NOTFOUND" ) )
			SET ( DSEED_USE_JPEG2000 OFF )
			MESSAGE ( "-- Not found openjpeg" )
		ENDIF ()
	ENDIF ()
ENDIF ()
IF ( ${DSEED_USE_WEBP} )
	FIND_PACKAGE ( webp QUIET )
	FIND_LIBRARY ( WEBP_LIBRARY NAMES webp )
	FIND_LIBRARY ( WEBP_DECODER_LIBRARY NAMES webpdecoder )
	FIND_LIBRARY ( WEBP_MUXER_LIBRARY NAMES webpmux )
	FIND_LIBRARY ( WEBP_DEMUXER_LIBRARY NAMES webpdemux )
	IF( ( NOT ( "${WEBP_LIBRARY}" STREQUAL "WEBP_LIBRARY-NOTFOUND" ) ) AND
		( NOT ( "${WEBP_DECODER_LIBRARY}" STREQUAL "WEBP_DECODER_LIBRARY-NOTFOUND" ) ) AND
		( NOT ( "${WEBP_MUXER_LIBRARY}" STREQUAL "WEBP_MUXER_LIBRARY-NOTFOUND" ) ) AND
		( NOT ( "${WEBP_DEMUXER_LIBRARY}" STREQUAL "WEBP_DEMUXER_LIBRARY-NOTFOUND" ) ) )
		SET ( WEBP_LIBRARIES ${WEBP_LIBRARY} ${WEBP_DECODER_LIBRARY} ${WEBP_MUXER_LIBRARY} ${WEBP_DEMUXER_LIBRARY} )
	ENDIF ()

	IF ( ( NOT DEFINED WEBP_INCLUDE_DIRS ) OR ( NOT DEFINED WEBP_LIBRARIES ) )
		MESSAGE ( "-- Not found libwebp" )
		SET ( DSEED_USE_WEBP OFF )
	ENDIF ()
ENDIF ()
IF ( ${DSEED_USE_TIFF} )
	FIND_PACKAGE ( liblzma )
	IF ( ( NOT DEFINED LZMA_LIBRARY ) OR ( "${LZMA_LIBRARY}" STREQUAL "LZMA_LIBRARY-NOTFOUND" ) )
		FIND_LIBRARY ( LZMA_LIBRARY NAMES lzma liblzma lzma.lib liblzma.lib lzma.a liblzma.a )
	ENDIF ()
	FIND_PACKAGE ( zlib )
	IF ( ( NOT DEFINED ZLIB_LIBRARY ) OR ( "${ZLIB_LIBRARY}" STREQUAL "ZLIB_LIBRARY-NOTFOUND" ) )
		FIND_LIBRARY ( ZLIB_LIBRARY NAMES zlibstatic zlibstatic.lib )
	ENDIF ()
	FIND_PACKAGE ( tiff )
	
	IF ( ( NOT DEFINED ZLIB_INCLUDE_DIRS ) OR ( ( NOT DEFINED ZLIB_LIBRARY ) OR ( "${ZLIB_LIBRARY}" STREQUAL "ZLIB_LIBRARY-NOTFOUND" ) )
		AND ( NOT DEFINED LZMA_INCLUDE_DIRS ) OR ( ( NOT DEFINED LZMA_LIBRARY ) OR ( "${LZMA_LIBRARY}" STREQUAL "LZMA_LIBRARY-NOTFOUND" ) )
		AND ( NOT DEFINED TIFF_INCLUDE_DIRS ) OR ( ( NOT DEFINED TIFF_LIBRARY ) OR ( "${TIFF_LIBRARY}" STREQUAL "TIFF_LIBRARY-NOTFOUND" ) ) )
		MESSAGE ( "-- Not found xz or zlib or libtiff" )
		SET ( DSEED_USE_TIFF OFF )
	ENDIF ()
ENDIF ()
IF ( ${DSEED_USE_GIF} )
	FIND_PACKAGE ( gif )
	IF ( NOT DEFINED GIF_FOUND )
		MESSAGE ( "-- Not found giflib" )
		SET ( DSEED_USE_GIF OFF )
	ENDIF ()
ENDIF ()
IF ( ${DSEED_USE_SQUISH} )
	FIND_LIBRARY ( SQUISH_LIBRARY NAMES squish squishd squish.lib squishd.lib QUIET )
	IF ( ( NOT DEFINED SQUISH_LIBRARY ) OR ( "${SQUISH_LIBRARY}" STREQUAL "SQUISH_LIBRARY-NOTFOUND" ) )
		MESSAGE ( "-- Not found squish" )
		SET ( DSEED_USE_SQUISH OFF )
	ENDIF ()
ENDIF ()

IF ( ${DSEED_USE_VORBIS} )
	FIND_PACKAGE ( ogg )
	IF ( NOT DEFINED OGG_FOUND )
		MESSAGE ( "-- Not found ogg or vorbis" )
		SET ( DSEED_USE_VORBIS OFF )
	ELSE ()
		FIND_PATH ( VORBIS_INCLUDE_DIRS vorbis/vorbisfile.h )
		FIND_LIBRARY ( VORBIS_LIBRARY NAMES vorbis vorbis.lib )
		FIND_LIBRARY ( VORBIS_ENCODER_LIBRARY NAMES vorbisenc vorbisenc.lib )
		FIND_LIBRARY ( VORBIS_FILE_LIBRARY NAMES vorbisfile vorbisfile.lib )
		IF( ( NOT ( "${VORBIS_LIBRARY}" STREQUAL "VORBIS_LIBRARY-NOTFOUND" ) ) OR
			( NOT ( "${VORBIS_ENCODER_LIBRARY}" STREQUAL "VORBIS_ENCODER_LIBRARY-NOTFOUND" ) ) OR
			( NOT ( "${VORBIS_FILE_LIBRARY}" STREQUAL "VORBIS_FILE_LIBRARY-NOTFOUND" ) ) )
			SET ( VORBIS_LIBRARIES ${VORBIS_LIBRARY} ${VORBIS_ENCODER_LIBRARY} ${VORBIS_FILE_LIBRARY} )
		ELSE ()
			SET ( VORBIS_LIBRARIES "VORBIS_LIBRARIES-NOTFOUND" )
		ENDIF ()

		IF ( ( ( NOT DEFINED VORBIS_INCLUDE_DIRS ) OR ( "${VORBIS_INCLUDE_DIRS}" STREQUAL "VORBIS_INCLUDE_DIRS-NOTFOUND" ) )
			OR ( ( NOT DEFINED VORBIS_LIBRARIES ) OR ( "${VORBIS_LIBRARIES}" STREQUAL "VORBIS_LIBRARIES-NOTFOUND" ) ) )
			MESSAGE ( "-- Not found ogg or vorbis" )
			SET ( DSEED_USE_VORBIS OFF )
		ENDIF ()
	ENDIF ()
ENDIF ()
IF ( ${DSEED_USE_OPUS} )
	FIND_PACKAGE ( ogg )
	FIND_PACKAGE ( opus )
	IF ( ( NOT DEFINED OGG_FOUND ) OR ( NOT DEFINED OPUS_FOUND ) )
		MESSAGE ( "-- Not found ogg or opus" )
		SET ( DSEED_USE_OPUS OFF )
	ENDIF ()
ENDIF ()
IF ( ${DSEED_USE_FLAC} )
	FIND_PACKAGE ( ogg )
	IF ( NOT DEFINED OGG_FOUND )
		SET ( DSEED_USE_FLAC OFF )
	ELSE ()
		FIND_PATH ( FLAC_INCLUDE_DIRS FLAC/stream_decoder.h )
		FIND_LIBRARY ( FLAC_LIBRARY NAMES FLAC )
		IF ( ( NOT DEFINED VORBIS_INCLUDE_DIRS ) AND ( NOT DEFINED VORBIS_LIBRARY ) )
			MESSAGE ( "-- Not found ogg or flac" )
			SET ( DSEED_USE_FLAC OFF )
		ENDIF ()
	ENDIF ()
ENDIF ()

############################################################################################
#
# Define Engine Sources
#
############################################################################################
SET ( DSEED_BUILD_FILES 
	# Processor Instructions
    src/instructions/instruction_info.cpp
	src/instructions/utility_instructions.cpp

	# Static Components
	src/time.cpp
	src/strings.cpp
	src/math/half.cpp
	src/math/int24.cpp
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
)

############################################################################################
#
# Define Linking Libraries
#
############################################################################################
link_directories ( ${INSTALL_LIB_DIR} )

SET ( DSEED_DEFINITIONS  )
SET ( DSEED_LINK_LIBS  )

MESSAGE ( "CMake System Name is ${CMAKE_SYSTEM_NAME}" )
IF ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" OR ${CMAKE_SYSTEM_NAME} STREQUAL "MSYS" )
	LIST ( APPEND DSEED_DEFINITIONS -DWINDOWS )
	LIST ( APPEND DSEED_DEFINITIONS -DUNICODE )
	SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8" )
	SET ( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO" )
	SET ( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO" )
	SET ( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO" )
	LIST ( APPEND DSEED_LINK_LIBS windowscodecs.lib mf.lib mfplat.lib mfuuid.lib mfreadwrite.lib XInput.lib PortableDeviceGUIDs.lib sensorsapi.lib dxgi.lib d3d11.lib d3dcompiler.lib )
ELSEIF ( ${CMAKE_SYSTEM_NAME} STREQUAL "WindowsStore" )
	SET ( PLATFORM STORE )
	LIST ( APPEND DSEED_DEFINITIONS -DUNICODE )
	SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /ZW /utf-8" )
	# Find Windows.winmd
	SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -AI\"C:/Program Files (x86)/Windows Kits/10/UnionMetadata\"")
	SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -AI\"C:/Program Files (x86)/Windows Kits/10/UnionMetadata/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}\"")
	# Find Platform.winmd for VS2017
	SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -AI\"C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/IDE/VC/vcpackages\"")
	# Find Platform.winmd for VS2019
	SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -AI\"C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/VC/vcpackages\"")
	LIST ( APPEND DSEED_LINK_LIBS windowscodecs.lib mf.lib mfplat.lib mfuuid.lib mfreadwrite.lib XInput.lib )
ENDIF ()

IF ( ${DSEED_USE_BITMAP_ENCODERS} )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_BITMAP_ENCODERS )
ENDIF ()
IF ( ${DSEED_USE_PNG} )
	MESSAGE ( "-- Use libpng" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_PNG )
	LIST ( APPEND DSEED_PRIVATE_INCS ${ZLIB_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS} )
	LIST ( APPEND DSEED_LINK_LIBS ${ZLIB_LIBRARY} ${PNG_LIBRARY} )
ENDIF ()
IF ( ${DSEED_USE_JPEG} )
	MESSAGE ( "-- Use libjpeg-turbo or mozjpeg" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_JPEG )
	LIST ( APPEND DSEED_PRIVATE_INCS ${JPEG_INCLUDE_DIRS} )
	LIST ( APPEND DSEED_LINK_LIBS ${JPEG_LIBRARY} )
ENDIF ()
IF ( ${DSEED_USE_JPEG2000} )
	MESSAGE ( "-- Use openjpeg" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_JPEG2000 )
	LIST ( APPEND DSEED_PRIVATE_INCS ${OPENJPEG_INCLUDE_DIRS} )
	LIST ( APPEND DSEED_LINK_LIBS ${OPENJPEG_LIBRARY} )
ENDIF ()
IF ( ${DSEED_USE_WEBP} )
	MESSAGE ( "-- Use libwebp" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_WEBP )
	LIST ( APPEND DSEED_PRIVATE_INCS ${WEBP_INCLUDE_DIRS} )
	LIST ( APPEND DSEED_LINK_LIBS ${WEBP_LIBRARIES} )
ENDIF ()
IF ( ${DSEED_USE_TIFF} )
	MESSAGE ( "-- Use libtiff" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_TIFF )
	LIST ( APPEND DSEED_PRIVATE_INCS ${LZMA_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${TIFF_INCLUDE_DIRS} )
	LIST ( APPEND DSEED_LINK_LIBS ${LZMA_LIBRARY} ${ZLIB_LIBRARY} ${TIFF_LIBRARY} )
ENDIF ()
IF ( ${DSEED_USE_GIF} )
	MESSAGE ( "-- Use giflib" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_GIF )
	LIST ( APPEND DSEED_PRIVATE_INCS ${GIF_INCLUDE_DIRS} )
	LIST ( APPEND DSEED_LINK_LIBS ${GIF_LIBRARY} )
ENDIF ()
IF ( ${DSEED_USE_SQUISH} )
	MESSAGE ( "-- Use Squish" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_SQUISH )
	LIST ( APPEND DSEED_PRIVATE_INCS ${SQUISH_INCLUDE_DIRS} )
	LIST ( APPEND DSEED_LINK_LIBS ${SQUISH_LIBRARY} )
ENDIF ()

IF ( ${DSEED_USE_MEDIA_ENCODERS} )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_MEDIA_ENCODERS )
ENDIF ()
IF ( ${DSEED_USE_VORBIS} )
	MESSAGE ( "-- Use vorbis" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_VORBIS )
	LIST ( APPEND DSEED_PRIVATE_INCS ${VORBIS_INCLUDE_DIRS} )
	LIST ( APPEND DSEED_LINK_LIBS ${OGG_LIBRARY} ${VORBIS_LIBRARIES} )
ENDIF ()
IF ( ${DSEED_USE_OPUS} )
	MESSAGE ( "-- Use opus" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_OPUS )
	LIST ( APPEND DSEED_PRIVATE_INCS ${OPUS_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src/libs/opusfile )
	LIST ( APPEND DSEED_LINK_LIBS ${OGG_LIBRARY} ${OPUS_LIBRARY} )
ENDIF ()
IF ( ${DSEED_USE_FLAC} )
	MESSAGE ( "-- Use flac" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_FLAC )
	LIST ( APPEND DSEED_PRIVATE_INCS ${FLAC_INCLUDE_DIRS} )
	LIST ( APPEND DSEED_LINK_LIBS ${OGG_LIBRARY} ${FLAC_LIBRARY} )
ENDIF ()

SET ( DSEED_BUILD_TYPE SHARED )
IF ( MSVC )
	LIST ( APPEND DSEED_DEFINITIONS -DDSEED_WINDOWS_DLL_EXPORT )
ENDIF ()

############################################################################################
#
# Build Engine
#
############################################################################################
ADD_LIBRARY ( dseed ${DSEED_BUILD_TYPE} ${DSEED_BUILD_FILES} )
TARGET_INCLUDE_DIRECTORIES (
    dseed
    PUBLIC

    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
	$<INSTALL_INTERFACE:${INSTALL_INC_DIR}>
)
TARGET_INCLUDE_DIRECTORIES (
	dseed
	PRIVATE

	${DSEED_PRIVATE_INCS}
)
TARGET_COMPILE_DEFINITIONS ( dseed PRIVATE ${DSEED_DEFINITIONS} )
TARGET_LINK_LIBRARIES ( dseed ${DSEED_LINK_LIBS} )

IF ( ${CMAKE_SYSTEM_NAME} STREQUAL "WindowsStore" )
	MESSAGE ( "Windows Store Target Platform Version: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}" )
	SET_PROPERTY ( TARGET dseed PROPERTY VS_DESKTOP_EXTENSIONS_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION} )
	SET_PROPERTY ( TARGET dseed PROPERTY VS_MOBILE_EXTENSIONS_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION} )
ENDIF ()

SET_TARGET_PROPERTIES ( dseed PROPERTIES CXX_STANDARD 17 )

############################################################################################
#
# Build Apps
#
############################################################################################
IF ( NOT ${DSEED_SKIP_BUILD_APP} )
	IF ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" OR ${CMAKE_SYSTEM_NAME} STREQUAL "MSYS" )
		SET ( TARGET_TO  "WIN32" )
	ENDIF ()

	SET ( TEST_APP_NAME dseed_test_app )
	SET ( TEST_APP_PACKAGE_GUID "6514377e-dfd4-4cdb-80df-4e0366346eff" )
	ADD_EXECUTABLE ( ${TEST_APP_NAME} ${TARGET_TO} app/test_app/test.cpp app/test_app/app.manifest )
	TARGET_INCLUDE_DIRECTORIES ( ${TEST_APP_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/ )
	TARGET_LINK_LIBRARIES ( ${TEST_APP_NAME} dseed )
	SET_TARGET_PROPERTIES ( ${TEST_APP_NAME} PROPERTIES CXX_STANDARD 17 )
	#IF ( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
	#	MESSAGE ( "Enable add manifest to test_app." )
	#	ADD_CUSTOM_COMMAND (
	#		TARGET ${TEST_APP_NAME}
	#		POST_BUILD
	#		COMMAND "mt.exe" -nologo -manifest \"${CMAKE_CURRENT_SOURCE_DIR}/app/test_app/app.manifest\" -outputresource:\"$<TARGET_FILE:${TEST_APP_NAME}>\"\;\#1
	#		COMMENT "Adding Win32 manifest resource..." 
	#	)
	#ENDIF ()

	#IF ( ${CMAKE_SYSTEM_NAME} STREQUAL "WindowsStore" )
	#	SET_PROPERTY ( TARGET ${TEST_APP_NAME} PROPERTY VS_WINRT_COMPONENT TRUE )
	#	CONFIGURE_FILE (
	#		${CMAKE_CURRENT_SOURCE_DIR}/app/test_app/app.appxmanifest.in
	#		${CMAKE_CURRENT_BINARY_DIR}/Package.appxmanifest
	#		@ONLY
	#	)
	#	SET_PROPERTY ( SOURCE ${CMAKE_CURRENT_BINARY_DIR}/Package.appxmanifest PROPERTY VS_DEPLOYMENT_CONTENT 1 )
	#ENDIF ()
ENDIF ()

############################################################################################
#
# Installation
#
############################################################################################
IF ( NOT ${DSEED_SKIP_INSTALL} )
	generate_export_header ( dseed )
	SET_PROPERTY ( TARGET dseed PROPERTY VERSION ${DSEED_VERSION} )
	SET_PROPERTY ( TARGET dseed PROPERTY SOVERSION ${DSEED_VERSION_MAJOR} )
	SET_PROPERTY ( TARGET dseed APPEND PROPERTY COMPATIBLE_INTERFACE_STRING ${DSEED_VERSION_MAJOR} )

	SET ( DSEED_PC ${CMAKE_CURRENT_BINARY_DIR}/dseed.pc )
	CONFIGURE_FILE ( ${CMAKE_CURRENT_SOURCE_DIR}/dseed.pc.in ${DSEED_PC} @ONLY )

	INSTALL (
		TARGETS dseed
		EXPORT dseedTargets
		RUNTIME DESTINATION ${INSTALL_BIN_DIR}
		LIBRARY DESTINATION ${INSTALL_LIB_DIR}
		ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
		FRAMEWORK DESTINATION ${INSTALL_LIB_DIR}
		PUBLIC_HEADER DESTINATION ${INSTALL_INC_DIR}
		INCLUDES DESTINATION ${INSTALL_INC_DIR}
	)
	INSTALL (
		DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
		DESTINATION ${INSTALL_INC_DIR}
	)
	INSTALL (
		FILES ${DSEED_PC}
		DESTINATION ${INSTALL_PKGCONFIG_DIR}
	)
	
	write_basic_package_version_file (
		"${CMAKE_CURRENT_BINARY_DIR}/dseedConfigVersion.cmake"
		VERSION ${DSEED_VERSION}
		COMPATIBILITY AnyNewerVersion
	)

	EXPORT (
		EXPORT dseedTargets
		FILE "${CMAKE_CURRENT_BINARY_DIR}/dseedTargets.cmake"
	)
	INSTALL (
		EXPORT dseedTargets
		FILE dseedConfig.cmake
		DESTINATION "${INSTALL_LIB_DIR}/cmake/dseed"
	)
	INSTALL (
		FILES
			"${CMAKE_CURRENT_BINARY_DIR}/dseedConfigVersion.cmake"
		DESTINATION "${INSTALL_LIB_DIR}/cmake/dseed"
	)
ENDIF ()