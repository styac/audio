cmake_minimum_required(VERSION 3.10)

project(velvet_noise CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
 -O3 -mfpmath=sse -msse2 -msse3 -msse4.2 -ffast-math -funsafe-math-optimizations \
 -mfma -mfma4 -mavx -funsafe-loop-optimizations -funswitch-loops \
 -fvariable-expansion-in-unroller -Wall"
)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS}" )

add_definitions(-DFRAME_SIZE_EXP=10 )

file( GLOB HEADERS ${PROJECT_SOURCE_DIR}
    src/io/*.h
    src/dsp/*.h
)

# no effect
source_group("HEADERS" FILES ${HEADERS})

file( GLOB SOURCES ${PROJECT_SOURCE_DIR}
    src/cli/*.cpp
    src/io/*.cpp
    src/dsp/*.cpp
)

# no effect
source_group("SOURCES" FILES ${SOURCES})

include_directories(
    src
)

set(CMAKE_BUILD_TYPE RELEASE CACHE STRING "" FORCE)

add_executable( ${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries( ${PROJECT_NAME}
    sndfile
)

