# bas2uef

Converts BBC BASIC 2 source code into UEF files.

Specifically: tokenises BBC BASIC 2 source code, packages it according to the standard cassette filing system and stores the result into a UEF file so that it can be `LOAD`ed or `CHAIN`ed on real hardware.

## Usage

`bas2uef [-i input file] [-o output file]`

`-i` supplies an input file. If none is supplied then input will be taken from stdin.

`-o` supplies an output file. If none is supplied then a default of `out.uef` is assumed.

## How to Build

If you have make installed, run `make`.

Otherwise compile and link together the two .cpp files.