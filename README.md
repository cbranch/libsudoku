# libsudoku
A Sudoku generating and solving library.

## Synopsis
Generates, validates and solves 9x9 Sudoku puzzles. It uses the [Dancing Links](http://en.wikipedia.org/wiki/Dancing_Links) implementation of Knuth's Algorithm X, which is a fancy use of doubly-linked lists to efficiently add and remove elements while preserving their original order. Given the small set of possibilities for Sudoku it is overkill compared to the naive brute-force algorithm, but nonetheless an interesting and efficient approach for the generalized NP-complete problem.

## Building
The library itself (SudokuLib/*) should be portable, but the build system (Makefiles) assumes
you're on some kind of POSIX system. The tests, especially, are only set up to build on Linux & OS X.

The typical make targets work:
- all
- test
- clean

## Output
tests: sudoku_tests
CLI interface to library: sudoku
