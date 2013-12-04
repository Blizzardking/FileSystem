FileSystem
==========

How to make
===========

Because Makefile included, so to compile project, just execute command

$ make

#Instructions about cpin cpout mkdir

external file do NOT exceed 20 * 2048 Bytes, because only direct block implemented

Notice that only cpin and cpout are permitted to access file recursively
absolute path is also a recursive path
ex: cpin externfile happy/nice/v6       is permitted
so is the cpout

But mkdir happy/nice/v6 is NOT permitted
if you want to create dir tree, do:
mkdir happy
cd happy
mkdir nice
mkdir good
..... 
