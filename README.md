# libsudoku
A Sudoku generating and solving library.

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
