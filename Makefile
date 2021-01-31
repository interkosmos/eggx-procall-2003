.POSIX:
.SUFFIXES:

# Parameters:
#
#       FC          -       Fortran compiler.
#       AR          -       Archiver.
#       PREFIX      -       Change to `/usr` on Linux.
#
# Libraries:
#
#       EGGX_PATH   -       Path to the EGGX/ProCALL source code.
#       EGGX_LIB    -       EGGX/ProCALL static library.
#       TARGET      -       Output library name.
#
# Flags:
#
#       FFLAGS      -       Fortran compiler flags.
#       LDFLAGS     -       Linker flags.
#       LDLIBS      -       Linker libraries.
#       ARFLAGS     -       Archiver flags.
#
FC         = gfortran
AR         = ar
PREFIX     = /usr/local

EGGX_PATH  = vendor/eggx-0.94
EGGX_LIB   = $(EGGX_PATH)/libeggx.a
TARGET     = libeggx2003.a

FFLAGS     = -Wall -fmax-errors=1 -fcheck=all
LDFLAGS    = -I$(PREFIX)/include -L$(PREFIX)/lib
LDLIBS     = $(EGGX_LIB) $(TARGET) -lX11
ARFLAGS    = rcs

# Examples
JULIA      = examples/julia/julia
MANDELBROT = examples/mandelbrot/mandelbrot
PEANO      = examples/peano/peano
SNAKE      = examples/snake/snake
STARFIELD  = examples/starfield/starfield
TILES      = examples/tiles/tiles
WIREFRAME  = examples/wireframe/wireframe

.PHONY: all clean julia mandelbrot peano snake starfield tiles wireframe

all: $(TARGET)

$(EGGX_LIB):
	cd $(EGGX_PATH) && make

$(TARGET): $(EGGX_LIB)
	$(FC) $(FFLAGS) -c src/procall.f90
	$(FC) $(FFLAGS) -c src/eggx.f90
	$(AR) $(ARFLAGS) $(TARGET) procall.o eggx.o

# Examples
julia: $(EGGX_LIB) $(TARGET)
	$(FC) $(FFLAGS) $(LDFLAGS) -o $(JULIA) $(JULIA).f90 $(LDLIBS)

mandelbrot: $(EGGX_LIB) $(TARGET)
	$(FC) $(FFLAGS) $(LDFLAGS) -o $(MANDELBROT) $(MANDELBROT).f90 $(LDLIBS)

peano: $(EGGX_LIB) $(TARGET)
	$(FC) $(FFLAGS) $(LDFLAGS) -o $(PEANO) $(PEANO).f90 $(LDLIBS)

snake: $(EGGX_LIB) $(TARGET)
	$(FC) $(FFLAGS) $(LDFLAGS) -o $(SNAKE) $(SNAKE).f90 $(LDLIBS)

starfield: $(EGGX_LIB) $(TARGET)
	$(FC) $(FFLAGS) $(LDFLAGS) -o $(STARFIELD) $(STARFIELD).f90 $(LDLIBS)

tiles: $(EGGX_LIB) $(TARGET)
	$(FC) $(FFLAGS) $(LDFLAGS) -o $(TILES) $(TILES).f90 $(LDLIBS)

wireframe: $(EGGX_LIB) $(TARGET)
	$(FC) $(FFLAGS) $(LDFLAGS) -o $(WIREFRAME) $(WIREFRAME).f90 $(LDLIBS)

examples: julia mandelbrot peano snake starfield tiles wireframe

clean:
	cd $(EGGX_PATH) && make clean
	if [ `ls -1 *.mod 2>/dev/null | wc -l` -gt 0 ]; then rm *.mod; fi
	if [ `ls -1 *.o 2>/dev/null | wc -l` -gt 0 ]; then rm *.o; fi
	if [ -e $(TARGET) ]; then rm $(TARGET); fi
	if [ -e $(JULIA) ]; then rm $(JULIA); fi
	if [ -e $(MANDELBROT) ]; then rm $(MANDELBROT); fi
	if [ -e $(PEANO) ]; then rm $(PEANO); fi
	if [ -e $(SNAKE) ]; then rm $(SNAKE); fi
	if [ -e $(STARFIELD) ]; then rm $(STARFIELD); fi
	if [ -e $(TILES) ]; then rm $(TILES); fi
	if [ -e $(WIREFRAME) ]; then rm $(WIREFRAME); fi
