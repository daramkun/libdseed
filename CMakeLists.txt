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
OPTION ( DSEED_USE_ZOPFLI			"libdseed use PNG Zopfli Optimization."	ON  )

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
	FIND_PACKAGE ( zlib COMPONENTS zlibstatic )
	FIND_PACKAGE ( png COMPONENTS png_static )
	IF ( ( NOT DEFINED ZLIB_FOUND ) OR ( NOT DEFINED PNG_FOUND ) )
		FIND_PATH ( ZLIB_INCLUDE_DIRS zlib.h )
		FIND_LIBRARY ( ZLIB_LIBRARY NAMES zlibstatic )

		FIND_PATH ( PNG_INCLUDE_DIRS png.h )
		FIND_LIBRARY ( PNG_LIBRARY NAMES libpng16_static )

		IF ( ( NOT DEFINED ZLIB_INCLUDE_DIRS ) AND ( NOT DEFINED ZLIB_LIBRARY ) AND ( NOT DEFINED PNG_INCLUDE_DIRS ) AND ( NOT DEFINED PNG_LIBRARY ) )
			MESSAGE ( "-- Not found zlib or libpng" )
			SET ( DSEED_USE_PNG OFF )
		ENDIF ()
	ENDIF ()
ENDIF ()
IF ( ${DSEED_USE_JPEG} )
	FIND_PACKAGE ( jpeg )
	IF ( NOT DEFINED JPEG_FOUND )
		SET ( DSEED_USE_JPEG OFF )
		MESSAGE ( "-- Not found libjpeg-turbo or mozjpeg" )
	ENDIF ()
ENDIF ()
IF ( ${DSEED_USE_JPEG2000} )
	FIND_PACKAGE ( openjpeg )
	IF ( NOT DEFINED OPENGJPEG_FOUND )
		FIND_PATH ( OPENJPEG_INCLUDE_DIRS openjpeg.h )
		FIND_LIBRARY ( OPENJPEG_LIBRARY NAMES openjp2 )
		
		IF ( ( NOT DEFINED OPENJPEG_INCLUDE_DIRS ) AND ( NOT DEFINED OPENJPEG_LIBRARY ) )
			SET ( DSEED_USE_JPEG2000 OFF )
			MESSAGE ( "-- Not found openjpeg" )
		ENDIF ()
	ENDIF ()
ENDIF ()
IF ( ${DSEED_USE_WEBP} )
	FIND_PACKAGE ( webp )
	IF ( NOT DEFINED WEBP_FOUND )
		FIND_PATH ( WEBP_INCLUDE_DIRS webp/decode.h webp/encode.h webp/demux.h )
		FIND_LIBRARY ( WEBP_LIBRARY NAMES webp webpdecoder webpmux webpdemux )
		
		IF ( ( NOT DEFINED WEBP_INCLUDE_DIRS ) AND ( NOT DEFINED WEBP_LIBRARY ) )
			MESSAGE ( "-- Not found libwebp" )
			SET ( DSEED_USE_WEBP OFF )
		ENDIF ()
	ENDIF ()
ENDIF ()
IF ( ${DSEED_USE_TIFF} )
	FIND_PACKAGE ( liblzma )
	IF ( NOT DEFINED ZLIB_FOUND )
		FIND_PACKAGE ( zlib COMPONENTS zlibstatic )
	ENDIF ()
	FIND_PACKAGE ( tiff )
	IF ( ( NOT DEFINED LZMA_FOUND ) OR ( NOT DEFINED ZLIB_FOUND ) OR ( NOT DEFINED TIFF_FOUND ) )
		FIND_PATH ( LZMA_INCLUDE_DIRS lzma/lzma12.h )
		FIND_LIBRARY ( LZMA_LIBRARY NAMES liblzma )
		
		FIND_PATH ( ZLIB_INCLUDE_DIRS zlib.h )
		FIND_LIBRARY ( ZLIB_LIBRARY NAMES zlibstatic )

		FIND_PATH ( TIFF_INCLUDE_DIRS tiff.h tiffio.h )
		FIND_LIBRARY ( TIFF_LIBRARY NAMES tiff )

		IF ( ( NOT DEFINED ZLIB_INCLUDE_DIRS ) AND ( NOT DEFINED ZLIB_LIBRARY )
			AND ( NOT DEFINED LZMA_INCLUDE_DIRS ) AND ( NOT DEFINED LZMA_LIBRARY )
			AND ( NOT DEFINED TIFF_INCLUDE_DIRS ) AND ( NOT DEFINED TIFF_LIBRARY ) )
			MESSAGE ( "-- Not found xz or zlib or libtiff" )
			SET ( DSEED_USE_TIFF OFF )
		ENDIF ()
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
	FIND_LIBRARY ( SQUISH_LIBRARY NAMES squish squishd )
	IF ( ( NOT DEFINED SQUISH_INCLUDE_DIRS ) AND ( NOT DEFINED SQUISH_LIBRARY ) )
		MESSAGE ( "-- Not found squish" )
		SET ( DSEED_USE_SQUISH OFF )
	ENDIF ()
ENDIF ()
IF ( ${DSEED_USE_ZOPFLI} )
	FIND_PACKAGE ( Zopfli )
	IF ( NOT DEFINED ZOPFLI_FOUND )
	
		FIND_PATH ( ZOPFLI_INCLUDE_DIRS zopfli.h zopflipng.h )
		FIND_LIBRARY ( ZOPFLI_LIBRARY NAMES zopfli zopflipng )
		
		IF ( ( NOT DEFINED ZOPFLI_INCLUDE_DIRS ) AND ( NOT DEFINED ZOPFLI_LIBRARY ) )
			MESSAGE ( "-- Not found zopfli" )
			SET ( DSEED_USE_ZOPFLI OFF )
		ENDIF ()
	ENDIF ()
ENDIF ()

IF ( ${DSEED_USE_VORBIS} )
	FIND_PACKAGE ( ogg )
	IF ( NOT DEFINED OGG_FOUND )
		SET ( DSEED_USE_VORBIS OFF )
	ELSE ()
		FIND_PATH ( VORBIS_INCLUDE_DIRS vorbis/vorbisfile.h )
		FIND_LIBRARY ( VORBIS_LIBRARY NAMES vorbis vorbisenc vorbisfile )
		IF ( ( NOT DEFINED VORBIS_INCLUDE_DIRS ) AND ( NOT DEFINED VORBIS_LIBRARY ) )
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
		SET ( DSEED_USE_FLACK OFF )
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
	# Library Core
    src/dseed.cpp
	src/common.cpp

	# Reference Counting Object
	src/object.cpp

	# Scalar objects
	src/math/math.cpp
	src/math/math.simd.cpp
	src/geometry.cpp
	src/color.cpp
	src/time.cpp
	src/string.cpp

	# I/O Objects
	src/io.cpp
	src/hid.cpp

	# Debugging Objects
	src/diagnostics.cpp
	
	# Concurrency
	src/parallel.cpp

	# Extra
	src/extra.cpp

	# Bitmap
	src/bitmap/bitmap.cpp
	src/bitmap/bitmap.reformat.cpp
	src/bitmap/bitmap.resize.cpp
	src/bitmap/bitmap.filter.cpp
	src/bitmap/bitmap.flip.cpp
	src/bitmap/bitmap.operation.cpp

	# Bitmap Decoders with Non-external Library
	src/bitmap/decoders/bitmap.decoder.wic.cpp
	src/bitmap/decoders/bitmap.decoder.dib.cpp
	src/bitmap/decoders/bitmap.decoder.tga.cpp
	src/bitmap/decoders/bitmap.decoder.dds.cpp
	src/bitmap/decoders/bitmap.decoder.pkm.cpp
	src/bitmap/decoders/bitmap.decoder.ktx.cpp
	src/bitmap/decoders/bitmap.decoder.ico.cur.cpp
	src/bitmap/decoders/bitmap.decoder.astc.cpp

	# Bitmap Decoders with External Libraries
	src/bitmap/decoders/bitmap.decoder.png.cpp
	src/bitmap/decoders/bitmap.decoder.jpeg.cpp
	src/bitmap/decoders/bitmap.decoder.jp2.cpp
	src/bitmap/decoders/bitmap.decoder.webp.cpp
	src/bitmap/decoders/bitmap.decoder.tiff.cpp
	src/bitmap/decoders/bitmap.decoder.gif.cpp

	# Bitmap Encoders with Non-external Library
	src/bitmap/encoders/bitmap.encoder.dib.cpp
	src/bitmap/encoders/bitmap.encoder.dds.cpp
	src/bitmap/encoders/bitmap.encoder.ico.cur.cpp

	# Bitmap Encoders with External Libraries
	src/bitmap/encoders/bitmap.encoder.png.cpp
	src/bitmap/encoders/bitmap.encoder.jpeg.cpp
	src/bitmap/encoders/bitmap.encoder.webp.cpp
	src/bitmap/encoders/bitmap.encoder.gif.cpp

	# Video and Audio
	src/media/media.cpp
	src/media/audio.cpp
	src/media/audio.filter.cpp
	src/media/audio.spectrum.cpp
	src/media/audio.reformat.cpp
	src/media/audio.resample.cpp
	src/media/audio.rechannel.cpp

	# Media Decoders with Non-external Library
	src/media/decoders/media.decoder.mf.cpp
	src/media/decoders/media.decoder.wav.cpp
	src/media/decoders/media.decoder.mp3.cpp

	# Media Decoders with External Libraries
	src/media/decoders/media.decoder.vorbis.cpp
	src/media/decoders/media.decoder.opus.cpp
	src/media/decoders/media.decoder.flac.cpp

	# Media Encoders with Non-external Library
	src/media/encoders/media.encoder.wav.cpp

	# Media Encoders with External Libraries

	# Audio Streams
	src/media/streams/media.audiostream.basic.cpp
	src/media/streams/media.audiostream.filter.cpp

	# Platform Dependencies
	src/platform/platform.cpp
	src/platform/platform.win32.cpp
	src/platform/platform.uwp.cpp
	src/platform/platform.macos.mm
	src/platform/platform.ios.mm
	src/platform/platform.android.cpp
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
IF ( ${CMAKE_SYSTEM_NAME} MATCHES "$Windows^" OR ${CMAKE_SYSTEM_NAME} MATCHES "MSYS" )
	LIST ( APPEND DSEED_DEFINITIONS -DWINDOWS )
	LIST ( APPEND DSEED_DEFINITIONS -DUNICODE )
	SET ( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO" )
	SET ( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO" )
	SET ( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO" )
	LIST ( APPEND DSEED_LINK_LIBS windowscodecs.lib mf.lib mfplat.lib mfuuid.lib mfreadwrite.lib XInput.lib PortableDeviceGUIDs.lib sensorsapi.lib )
ELSEIF ( ${CMAKE_SYSTEM_NAME} MATCHES "WindowsStore" )
	SET ( PLATFORM STORE )
	LIST ( APPEND DSEED_DEFINITIONS -DUNICODE )
	SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /ZW" )
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
	LIST ( APPEND DSEED_PRIVATE_INCS ${PNG_INCLUDE_DIRS} )
	LIST ( APPEND DSEED_LINK_LIBS zlibstatic libpng16_static )
ENDIF ()
IF ( ${DSEED_USE_JPEG} )
	MESSAGE ( "-- Use libjpeg-turbo or mozjpeg" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_JPEG )
	LIST ( APPEND DSEED_PRIVATE_INCS ${JPEG_INCLUDE_DIRS} )
	LIST ( APPEND DSEED_LINK_LIBS jpeg-static )
ENDIF ()
IF ( ${DSEED_USE_JPEG2000} )
	MESSAGE ( "-- Use openjpeg" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_JPEG2000 )
	LIST ( APPEND DSEED_PRIVATE_INCS ${OPENJPEG_INCLUDE_DIRS} )
	LIST ( APPEND DSEED_LINK_LIBS openjp2 )
ENDIF ()
IF ( ${DSEED_USE_WEBP} )
	MESSAGE ( "-- Use libwebp" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_WEBP )
	LIST ( APPEND DSEED_PRIVATE_INCS ${WEBP_INCLUDE_DIRS} )
	LIST ( APPEND DSEED_LINK_LIBS webp webpmux webpdemux webpdecoder )
ENDIF ()
IF ( ${DSEED_USE_TIFF} )
	MESSAGE ( "-- Use libtiff" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_TIFF )
	LIST ( APPEND DSEED_PRIVATE_INCS ${TIFF_INCLUDE_DIRS} )
	LIST ( APPEND DSEED_LINK_LIBS liblzma zlibstatic tiff )
ENDIF ()
IF ( ${DSEED_USE_GIF} )
	MESSAGE ( "-- Use giflib" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_GIF )
	LIST ( APPEND DSEED_PRIVATE_INCS ${GIF_INCLUDE_DIRS} )
	LIST ( APPEND DSEED_LINK_LIBS giflib )
ENDIF ()
IF ( ${DSEED_USE_SQUISH} )
	MESSAGE ( "-- Use Squish" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_SQUISH )
	LIST ( APPEND DSEED_PRIVATE_INCS ${SQUISH_INCLUDE_DIRS} )
	IF ( ( ${CMAKE_BUILD_TYPE} MATCHES "DEBUG" ) OR ( ${CMAKE_BUILD_TYPE} MATCHES "Debug" ) )
		LIST ( APPEND DSEED_LINK_LIBS squishd )
	ELSE ()
		LIST ( APPEND DSEED_LINK_LIBS squish )
	ENDIF ()
ENDIF ()
IF ( ${DSEED_USE_ZOPFLI} )
	MESSAGE ( "-- Use zopfli" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_ZOPFLI )
	LIST ( APPEND DSEED_PRIVATE_INCS ${ZOPFLI_INCLUDE_DIRS} )
	IF ( ( ${CMAKE_BUILD_TYPE} MATCHES "DEBUG" ) OR ( ${CMAKE_BUILD_TYPE} MATCHES "Debug" ) )
		LIST ( APPEND DSEED_LINK_LIBS zopflid zopflipngd )
	ELSE ()
		LIST ( APPEND DSEED_LINK_LIBS zopfli zopflipng )
	ENDIF ()
ENDIF ()

IF ( ${DSEED_USE_MEDIA_ENCODERS} )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_MEDIA_ENCODERS )
ENDIF ()
IF ( ${DSEED_USE_VORBIS} )
	MESSAGE ( "-- Use vorbis" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_VORBIS )
	LIST ( APPEND DSEED_PRIVATE_INCS ${VORBIS_INCLUDE_DIRS} )
	LIST ( APPEND DSEED_LINK_LIBS ogg vorbis vorbisenc vorbisfile )
ENDIF ()
IF ( ${DSEED_USE_OPUS} )
	MESSAGE ( "-- Use opus" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_OPUS )
	LIST ( APPEND DSEED_PRIVATE_INCS ${OPUS_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src/libs/opusfile )
	LIST ( APPEND DSEED_LINK_LIBS ogg opus )
ENDIF ()
IF ( ${DSEED_USE_FLAC} )
	MESSAGE ( "-- Use flac" )
	LIST ( APPEND DSEED_DEFINITIONS -DUSE_FLAC )
	LIST ( APPEND DSEED_PRIVATE_INCS ${FLAC_INCLUDE_DIRS} )
	LIST ( APPEND DSEED_LINK_LIBS ogg FLAC )
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

IF ( ${CMAKE_SYSTEM_NAME} MATCHES "WindowsStore" )
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
	ADD_EXECUTABLE ( dseed_test_bitmap app/test_bitmap/test.cpp )
	TARGET_INCLUDE_DIRECTORIES ( dseed_test_bitmap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/ )
	TARGET_LINK_LIBRARIES ( dseed_test_bitmap dseed )
	
	ADD_EXECUTABLE ( dseed_test_audio app/test_audio/test.cpp )
	TARGET_INCLUDE_DIRECTORIES ( dseed_test_audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/ )
	TARGET_LINK_LIBRARIES ( dseed_test_audio dseed )
	
	ADD_EXECUTABLE ( dseed_test_simd app/test_simd/test.cpp )
	TARGET_INCLUDE_DIRECTORIES ( dseed_test_simd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/ )
	TARGET_LINK_LIBRARIES ( dseed_test_simd dseed )
	
	IF ( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" OR ${CMAKE_SYSTEM_NAME} MATCHES "MSYS" )
		SET ( TARGET_TO  "WIN32" )
	ENDIF ()
	
	SET ( TEST_APP_NAME dseed_test_app )
	SET ( TEST_APP_PACKAGE_GUID "6514377e-dfd4-4cdb-80df-4e0366346eff" )
	ADD_EXECUTABLE ( ${TEST_APP_NAME} ${TARGET_TO} app/test_app/test.cpp )
	TARGET_INCLUDE_DIRECTORIES ( ${TEST_APP_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/ )
	TARGET_LINK_LIBRARIES ( ${TEST_APP_NAME} dseed )
	IF ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
		MESSAGE ( "Enable add manifest to test_app." )
		#ADD_CUSTOM_COMMAND (
		#	TARGET dseed_test_app
		#	POST_BUILD
		#	COMMAND "mt.exe" -nologo -manifest \"${CMAKE_CURRENT_SOURCE_DIR}/app/test_app/app.manifest\" -outputresource:\"$<TARGET_FILE:${TEST_APP_NAME}>\"\;\#1
		#	COMMENT "Adding Win32 manifest resource..." 
		#)
	ENDIF ()

	IF ( ${CMAKE_SYSTEM_NAME} MATCHES "WindowsStore" )
		SET_PROPERTY ( TARGET ${TEST_APP_NAME} PROPERTY VS_WINRT_COMPONENT TRUE )
		CONFIGURE_FILE (
			${CMAKE_CURRENT_SOURCE_DIR}/app/test_app/app.appxmanifest.in
			${CMAKE_CURRENT_BINARY_DIR}/Package.appxmanifest
			@ONLY
		)
		SET_PROPERTY ( SOURCE ${CMAKE_CURRENT_BINARY_DIR}/Package.appxmanifest PROPERTY VS_DEPLOYMENT_CONTENT 1 )
	ENDIF ()
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
	IF ( NOT ${DSEED_SKIP_BUILD_APP} )
		INSTALL (
			TARGETS dseed_test_bitmap dseed_test_audio
			RUNTIME DESTINATION ${INSTALL_BIN_DIR}
		)
	ENDIF ()
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