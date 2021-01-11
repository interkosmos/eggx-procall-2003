# EGGX/ProCALL 2003
Modern Fortran modules that contain interfaces to the free X11 graphics
library [EGGX/ProCALL](https://www.ir.isas.jaxa.jp/~cyamauch/eggx_procall/).
The EGGX/ProCALL library provides fast and simple drawing routines on Linux,
Unix, and Cygwin, directly callable from C or FORTRAN 77.

EGGX/ProCALL 2003 is a wrapper around EGGX/ProCALL that includes:

* Fortran 90 interfaces to the FORTRAN 77 ProCALL subroutines (module `procall`).
* Selected Fortran 2003 ISO C binding interfaces to C functions in EGGX that do not have a FORTRAN counterpart (module `eggx`).
* Named parameters of key codes, colour palettes, and output formats.

## Build Instructions
Simply clone the GitHub repository and execute the Makefile:

```
$ git clone https://github.com/interkosmos/eggx-procall-2003
$ cd eggx-procall-2003/
$ make
```

The EGGX/ProCALL 0.94 source code is included in directory `vendor/eggx-0.94/`.

Link your Fortran application with `vendor/eggx-0.94/libeggx.a`,
`libeggx2003.a`, and `-lX11`, for instance:

```
$ gfortran -o demo demo.f90 libeggx.a libeggx2003.a -lX11
```

## Examples
See directory `examples/` for some demo applications:

* **julia** draws the Julia set.
* **mandelbrot** draws the Mandelbrot set.
* **peano** draws a [Peano curve](https://rosettacode.org/wiki/Peano_curve).
* **snake** is a simple implementation of the snake game.
* **starfield** lets you fly through a starfield.
* **tiles** draws text and a tileset to screen.

Compile all programs with:

```
$ make examples
```

Or, use the name of a particular example. Run each program from its respective
directory:

```
$ make <name>
$ cd examples/<name>/
$ ./<name>
```

## Documentation
For the official documentation, see the PDF file
[vendor/eggx-0.94/eggx_procall.pdf](vendor/eggx-0.94/eggx_procall.pdf).

Generate the source code documentation with
[FORD](https://github.com/cmacmackin/ford). Add FORD with `pip`, for example:

```
$ python3 -m venv virtual-environment/
$ source virtual-environment/bin/activate
$ python3 -m pip install ford
```

Or, instead, just install the package in your user directory:

```
$ python3 -m pip install --user ford
```

Then, run:

```
$ ford project.md -d ./src
```

Open `doc/index.html` in a web browser.

## Coverage
### ProCALL

| FORTRAN 77 routine | Fortran 2003 interface |
|--------------------|------------------------|
| arc                | arc                    |
| arohd              | arohd                  |
| circ1              | circ1                  |
| clsc               | clsc                   |
| clsx               | clsx                   |
| drawarc            | drawarc                |
| drawarrow          | drawarrow              |
| drawcirc           | drawcirc               |
| drawline           | drawline               |
| drawlines          | drawlines              |
| drawnum            | drawnum                |
| drawpoly           | drawpoly               |
| drawpts            | drawpts                |
| drawrect           | drawrect               |
| drawstr            | drawstr                |
| drawsym            | drawsym                |
| drawsyms           | drawsyms               |
| fillarc            | fillarc                |
| fillcirc           | fillcirc               |
| fillpoly           | fillpoly               |
| fillrect           | fillrect               |
| gclose             | gclose                 |
| gcloseall          | gcloseall              |
| gclr               | gclr                   |
| ggetch             | ggetch                 |
| ggetdisplayinfo    | ggetdisplayinfo        |
| ggetevent          | ggetevent              |
| ggetxpress         | ggetxpress             |
| gopen              | gopen                  |
| gsetbgcolor        | gsetbgcolor            |
| gsetnonblock       | gsetnonblock           |
| isnan              | isnan                  |
| layer              | layer                  |
| line               | line                   |
| lineto             | lineto                 |
| makecolor          | makecolor              |
| moveto             | moveto                 |
| msleep             | msleep                 |
| newcolor           | newcolor               |
| newcoordinate      | newcoordinate          |
| newfontset         | newfontset             |
| newhsvcolor        | newhsvcolor            |
| newlinestyle       | newlinestyle           |
| newpen             | newpen                 |
| newpencolor        | newpencolor            |
| newrgbcolor        | newrgbcolor            |
| newwindow          | newwindow              |
| number             | number                 |
| plot               | plot                   |
| plots              | plots                  |
| pset               | pset                   |
| putimg24           | putimg24               |
| rtoc               | rtoc                   |
| saveimg            | saveimg                |
| selwin             | selwin                 |
| setal              | setal                  |
| symbol             | symbol                 |
| tclr               | tclr                   |
| vport              | vport                  |
| window             | window                 |

### EGGX
| C function         | Fortran 2003 interface |
|--------------------|------------------------|
| gputimage          | eggx_gputimage         |

## Licence
GNU GPL (EGGX/ProCALL), ISC (EGXX/ProCALL 2003)
