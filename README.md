as64 is yet another 6502 cross assembler. I decided to archive the source
code of some very old (mid-late 80's) projects of mine before the original
magnetic media gave up the ghost. These projects were written in assembly
language targetting the Commodore 64. At the time, I was using Chris Miller's
"Buddy Assembler" on a Commodore 128 to build the software. I thought it
would be handy to be able to build this old software on modern hardware,
but I wasn't able to find an existing 6502 cross assembler that supported
the original assembler's syntax for temporary labels (e.g. '+' and '-').

Staring down tens of thousands of lines of vintage source code that I did
not wish to manually edit to uniquely name every branch target, and having
some free time, I decided to write an assembler that supports at least the
subset of Buddy Assembler features that I was using circa 1987-89.

as64 is written in C++14 and has no dependencies other than the C and C++
standard libraries. I've tested it on Ubuntu 18.04 LTS using gcc and on
the Mac using clang. It should also work on Windows, although it has not
been tested there.
