Introduction
============

A Qt4-based manga reader that supports multiple monitors. Now, the dream of reading manga on two portrait monitors at once can be realized!

Requirements
===========

* Qt 4 headers (`apt-get install libqt4-dev`)
* Cmake
* Poppler Qt4 bindings (`apt-get install libpoppler-qt4-dev`)

Qt Creator is also recommended as a nice IDE.

Build Instructions
==================

Provided is a Makefile that simply invokes cmake. This means you can just:

    $ cd MultiscreenManga; make;

This is equivalent to the long form series of commands:

    $ cd MultiscreenManga; mkdir build; cd build; cmake ..; make;
