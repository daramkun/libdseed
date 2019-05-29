cmake_minimum_required ( VERSION 3.8 )

OPTION ( DSEED_STATIC_BUILD "libdseed build to static library." OFF )
OPTION ( DSEED_CRT_LINK_STATIC "libdseed built with static CRT library." OFF )
OPTION ( DSEED_SKIP_INSTALL "libdseed do not install." OFF )

IF ( NOT DSEED_STATIC_BUILD )
	IF ( MSVC )
		ADD_DEFINITIONS ( -DDSEED_WINDOWS_DLL_EXPORT )
	ENDIF ()
	SET ( DSEED_BUILD_TYPE SHARED )
ELSE ()
	IF ( MSVC )
		ADD_DEFINITIONS ( -DDSEED_WINDOWS_STATIC )
	ENDIF ()
	SET ( DSEED_BUILD_TYPE STATIC )
ENDIF ()

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

ADD_LIBRARY ( dseed ${DSEED_BUILD_TYPE}
    src/dseed.cpp
	src/dseed.geometry.cpp
	src/dseed.time.cpp
	src/dseed.string.cpp
	src/dseed.stream.cpp
	src/dseed.math.cpp

	src/dmath.cpp
)

TARGET_INCLUDE_DIRECTORIES (
    dseed
    PUBLIC

    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

IF ( NOT ${DSEED_SKIP_INSTALL} )
	INSTALL ( DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include )
	INSTALL ( TARGETS dseed
				RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
				LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
				ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	)
ENDIF ()