# set(CMAKE_CXX_COMPILER "/usr/bin/g++-7")
cmake_minimum_required(VERSION 3.7.2 )
project(yacynth CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -O3 -mfpmath=sse -msse2 -msse3 -msse4.2 -ffast-math -funsafe-math-optimizations \
 -mfma -mfma4 -mavx -funsafe-loop-optimizations -funswitch-loops \
 -fvariable-expansion-in-unroller -Wall" )
	
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS}" )

file( GLOB_RECURSE project_HDRS ${PROJECT_SOURCE_DIR} *.hpp *.h )
file( GLOB_RECURSE project_SRCS ${PROJECT_SOURCE_DIR} *.cpp *.c )

include_directories( include src src/include )
set(CMAKE_BUILD_TYPE RELEASE CACHE STRING "" FORCE)
add_executable( yacynth ${project_SRCS} ${project_HDRS} )
target_link_libraries( yacynth
	jack
	pthread
)






