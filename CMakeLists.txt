cmake_minimum_required ( VERSION 3.10 )

INCLUDE ( GenerateExportHeader )
INCLUDE ( CMakePackageConfigHelpers )

SET ( DSEED_VERSION_MAJOR 1 )
SET ( DSEED_VERSION_MINOR 0 )
SET ( DSEED_VERSION_REVISION 0 )
SET ( DSEED_VERSION ${DSEED_VERSION_MAJOR}.${DSEED_VERSION_MINOR}.${DSEED_VERSION_REVISION} )

PROJECT ( dseed
	VERSION ${DSEED_VERSION}
	LANGUAGES C CXX
	DESCRIPTION "This is Core library for personal."
)

SET ( INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for executables." )
SET ( INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation directory for libraries." )
SET ( INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers." )
SET ( INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" CACHE PATH "Installation directory for pkgconfig (.pc) files" )

OPTION ( DSEED_SKIP_INSTALL "libdseed do not install." OFF )

SET ( DSEED_BUILD_FILES 
    src/dseed.cpp

	src/object.cpp

	src/math/math.cpp
	src/geometry.cpp
	src/color.cpp
	src/time.cpp
	src/string.cpp

	src/io.cpp
	src/hid.cpp

	src/diagnostics.cpp
	
	src/parallel.cpp

	src/bitmap/bitmap.cpp
	src/bitmap/bitmap.decoder.wic.cpp
	src/bitmap/bitmap.decoder.dib.cpp
	src/bitmap/bitmap.decoder.tga.cpp
	src/bitmap/bitmap.decoder.dds.cpp
	src/bitmap/bitmap.decoder.pkm.cpp
	src/bitmap/bitmap.decoder.ktx.cpp

	src/media/media.cpp
	src/media/audio.cpp
	src/media/media.decoder.mf.cpp
)

link_directories ( ${INSTALL_LIB_DIR} )
SET ( DSEED_LINK_LIBS  )

IF ( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" OR ${CMAKE_SYSTEM_NAME} MATCHES "MSYS" )
	LIST ( APPEND DSEED_LINK_LIBS windowscodecs.lib mf.lib mfplat.lib mfuuid.lib mfreadwrite.lib )
ENDIF ()

SET ( DSEED_BUILD_TYPE SHARED )
IF ( MSVC )
	SET ( DSEED_DEFINITIONS -DDSEED_WINDOWS_DLL_EXPORT )
ENDIF ()

ADD_LIBRARY ( dseed ${DSEED_BUILD_TYPE} ${DSEED_BUILD_FILES} )
TARGET_COMPILE_DEFINITIONS ( dseed PRIVATE ${DSEED_DEFINITIONS} )
TARGET_INCLUDE_DIRECTORIES (
    dseed
    PUBLIC

    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
	$<INSTALL_INTERFACE:${INSTALL_INC_DIR}>
)
TARGET_LINK_LIBRARIES ( dseed ${DSEED_LINK_LIBS} )

SET_TARGET_PROPERTIES ( dseed PROPERTIES CXX_STANDARD 17 )

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