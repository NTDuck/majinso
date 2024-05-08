# compiler & flags
cxx = g++
cxxflags = -Wall -I"include/sdl2" -I"include/headers"

# libs
libpath = -L"lib"
libs = -l"mingw32" -l"SDL2main" -l"SDL2" -l"SDL2_image"

# src & output
src = $(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
output = build/majinso.exe

all: $(output)

$(output): $(src)
	$(cxx) $(cxxflags) $(libpath) -o $@ $^ $(libs)

run:
	$(output)

clean:
	del $(output)

# https://stackoverflow.com/questions/64396979/how-do-i-use-sdl2-in-my-programs-correctly
# use i686-w64-mingw32 for 32-bit compilers, not x86_64-w64-mingw32