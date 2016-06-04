
g++-6 -S -O3 -mfpmath=sse -msse2 -msse3 -msse4.2 -ffast-math -funsafe-math-optimizations -mfma -mfma4 -mavx -funroll-loops -funsafe-loop-optimizations -funswitch-loops -fvariable-expansion-in-unroller -ftree-vectorizer-verbose=2   -c -O2 -Isrc/include -std=c++14 -MMD -MP -MF "build/Release/GNU-Linux/src/effects/Filter.o.d" -o build/Release/GNU-Linux/src/effects/Filter.s src/effects/Filter.cpp

