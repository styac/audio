set(CMAKE_CXX_COMPILER "/usr/bin/g++-6")
cmake_minimum_required(VERSION 3.8.2 )
project(yacynth CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -O3 -mfpmath=sse -msse2 -msse3 -msse4.2 -ffast-math -funsafe-math-optimizations \
 -mfma -mfma4 -mavx -funsafe-loop-optimizations -funswitch-loops \
 -fvariable-expansion-in-unroller")
	
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS}" )

file(GLOB_RECURSE HDR_FILES  ${PROJECT_SOURCE_DIR} *.hpp *.h )

set( SRC_FILES
./src/control/Sysman.cpp
./src/control/Controllers.cpp
./src/control/Statistics.cpp
./src/control/SynthFrontend.cpp
./src/control/Setting.cpp
./src/control/main.cpp
./src/yaio/YaIoJackPort.cpp
./src/yaio/YaIoInQueue.cpp
./src/yaio/YaIo.cpp
./src/yaio/IOthread.cpp
./src/yaio/YaIoJack.cpp
./src/net/Server.cpp
./src/router/AbstractRouter.cpp
./src/router/SimpleMidiRouter.cpp
./src/oscillator/OscillatorOutput.cpp
./src/oscillator/BaseOscillator.cpp
./src/oscillator/OscillatorArray.cpp
./src/oscillator/ToneShaperMatrix.cpp
./src/oscillator/Tables.cpp
./src/oscillator/Oscillator.cpp
./src/message/Midi.cpp
./src/effects/FxOutNoise.cpp
./src/effects/FxEarlyReflection.cpp
./src/effects/FxFlanger.cpp
./src/effects/EmptyBuffer.cpp
./src/effects/FxOscillatorMixer.cpp
./src/effects/CombInterpolated.cpp
./src/effects/FxModulator.cpp
./src/effects/FxChorus.cpp
./src/effects/FxFilter.cpp
./src/effects/FxBase.cpp
./src/effects/FxOutOscillator.cpp
./src/effects/FxEcho.cpp
./src/effects/Filter.cpp
./src/effects/FxMixer.cpp
./src/effects/Comb.cpp
./src/effects/FxLateReverb.cpp
./src/utils/Fastexp.cpp
./src/utils/GaloisNoiser.cpp
./src/utils/Fastsincos.cpp
./src/utils/Limiters.cpp
./src/utils/FilterBase.cpp
./src/utils/Primes.cpp
./src/utils/BiquadMath.cpp
./src/utils/Barks.cpp
./external/sha3.cpp
)


include_directories( ./include ./src ./src/include )
set(CMAKE_BUILD_TYPE RELEASE CACHE STRING "" FORCE)
add_executable(yacynth ${SRC_FILES} ${HDR_FILES} )
target_link_libraries( yacynth
	jack
	pthread
)






