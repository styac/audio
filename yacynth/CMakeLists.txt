cmake_minimum_required(VERSION 3.10 )

project(yacynth CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -O3 -mfpmath=sse -msse2 -msse3 -msse4.2 -ffast-math -funsafe-math-optimizations \
 -mfma -mfma4 -mavx -funsafe-loop-optimizations -funswitch-loops \
 -fvariable-expansion-in-unroller -Wall" )
	
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS}" )

file( GLOB project_HDRS 
    ${PROJECT_SOURCE_DIR}/src/*/*.h 
    ${PROJECT_SOURCE_DIR}/external/*.h 
    ${PROJECT_SOURCE_DIR}/external/*.hpp 
)

file( GLOB project_SRCS 
    ${PROJECT_SOURCE_DIR}/src/*/*.cpp
    ${PROJECT_SOURCE_DIR}/external/*.cpp
)

include_directories(
    ${PROJECT_SOURCE_DIR}/external 
    ${PROJECT_SOURCE_DIR}/include 
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include 
)

set( CMAKE_BUILD_TYPE RELEASE CACHE STRING "" FORCE)

add_executable( yacynth ${project_SRCS} ${project_HDRS} )

target_link_libraries( yacynth
	jack
	pthread
)






