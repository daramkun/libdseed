cmake_minimum_required ( VERSION 3.8 )

SET ( VERSION 1.0.0 )
PROJECT ( dseed
	VERSION ${VERSION}
	LANGUAGES CXX
	DESCRIPTION "This is Core library for personal."
)

SET ( INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for executables." )
SET ( INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation directory for libraries." )
SET ( INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers." )
SET ( INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_PREFIX}/share/pkgconfig" CACHE PATH "Installation directory for pkgconfig (.pc) files" )

OPTION ( DSEED_STATIC_BUILD "libdseed build to static library." OFF )
OPTION ( DSEED_CRT_LINK_STATIC "libdseed built with static CRT library." OFF )
OPTION ( DSEED_SKIP_INSTALL "libdseed do not install." OFF )

IF ( DSEED_CRT_LINK_STATIC AND MSVC )
	SET ( CompilerFlags
		  CMAKE_CXX_FLAGS
		  CMAKE_CXX_FLAGS_DEBUG
		  CMAKE_CXX_FLAGS_RELEASE
		  CMAKE_C_FLAGS
		  CMAKE_C_FLAGS_DEBUG
		  CMAKE_C_FLAGS_RELEASE
    )
	FOREACH ( CompilerFlag ${CompilerFlags} )
		STRING ( REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}" )
		STRING ( REPLACE "/MDd" "/MTd" ${CompilerFlag} "${${CompilerFlag}}" )
	ENDFOREACH()
ENDIF ()

SET ( DSEED_BUILD_FILES 
    src/dseed.cpp
	src/dseed.geometry.cpp
	src/dseed.time.cpp
	src/dseed.string.cpp
	src/dseed.stream.cpp
	src/dseed.math.cpp

	src/dmath.cpp
)

IF ( NOT DSEED_STATIC_BUILD )
	SET ( DSEED_BUILD_TYPE SHARED )
	IF ( MSVC )
		SET ( DSEED_DEFINITIONS -DDSEED_WINDOWS_DLL_EXPORT )
	ENDIF ()
ELSE ()
	SET ( DSEED_BUILD_TYPE STATIC )
	IF ( MSVC )
		SET ( DSEED_DEFINITIONS -DDSEED_WINDOWS_STATIC )
	ENDIF ()
ENDIF ()

ADD_LIBRARY ( dseed ${DSEED_BUILD_TYPE} ${DSEED_BUILD_FILES} )
target_compile_definitions ( dseed PRIVATE ${DSEED_DEFINITIONS} )

TARGET_INCLUDE_DIRECTORIES (
    dseed
    PUBLIC

    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

IF ( NOT ${DSEED_SKIP_INSTALL} )
	SET ( DSEED_PC ${CMAKE_CURRENT_BINARY_DIR}/dseed.pc )
	CONFIGURE_FILE ( ${CMAKE_CURRENT_SOURCE_DIR}/dseed.pc.in ${DSEED_PC} @ONLY )

	INSTALL ( TARGETS dseed
				RUNTIME DESTINATION ${INSTALL_BIN_DIR}
				LIBRARY DESTINATION ${INSTALL_LIB_DIR}
				ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
				FRAMEWORK DESTINATION ${INSTALL_LIB_DIR}
				PUBLIC_HEADER DESTINATION ${INSTALL_INC_DIR}
	)
	INSTALL ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${INSTALL_INC_DIR} )
	INSTALL ( FILES ${DSEED_PC} DESTINATION ${INSTALL_PKGCONFIG_DIR} )
ENDIF ()