SET(SDL2_SEARCH_PATHS
    /usr/local/
    /usr/
)

FIND_PATH(SDL2_INCLUDE_DIR SDL2/SDL.h
    HINTS
    PATH_SUFFIXES include/SDL2 include
    PATHS ${SDL2_SEARCH_PATHS}
)

FIND_LIBRARY(SDL2_LIBRARY
    NAMES SDL2
    HINTS
    PATH_SUFFIXES lib64 lib
    PATHS ${SDL2_SEARCH_PATHS}
)

IF(MINGW)
    FIND_LIBRARY(SDL2MAIN_LIBRARY
        NAMES SDL2main
        HINTS
        PATH_SUFFIXES lib64 lib
        PATHS ${SDL2_SEARCH_PATHS}
    )
ELSE()
    SET(SDL2MAIN_LIBRARY "")
ENDIF()

IF(NOT APPLE)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    FIND_PACKAGE(Threads)
ENDIF(NOT APPLE)

IF(SDL2_INCLUDE_DIR AND SDL2_LIBRARY)
   SET(SDL2_FOUND TRUE)
ENDIF (SDL2_INCLUDE_DIR AND SDL2_LIBRARY)

IF(SDL2_FOUND)
    SET(SDL2_LIBRARY ${SDL2MAIN_LIBRARY} ${SDL2_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
    MESSAGE(STATUS "Found SDL2: ${SDL2_LIBRARY}")
ELSE(SDL2_FOUND)
    MESSAGE(WARNING "Could not find SDL2")
ENDIF(SDL2_FOUND)
