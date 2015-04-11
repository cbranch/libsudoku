#pragma once

#include "SudokuGrid.h"
#include <string>

struct DLXColumn;

struct DLXItem {
  DLXItem *prevCol;
  DLXItem *nextCol;
  DLXItem *prevRow;
  DLXItem *nextRow;
  // Refers to the single column header object in this column
  DLXColumn *columnHeader;

  DLXItem() : columnHeader(NULL) {
    prevCol = nextCol = prevRow = nextRow = this;
  }
  void insertColumn(DLXItem *before);
  void insertRow(DLXItem *before);
  void removeCol();
  void replaceCol();
  void removeRow();
  void replaceRow();
};

struct DLXColumn : public DLXItem {
  int columnSum;

  DLXColumn() : columnSum(0) { columnHeader = this; }
  virtual ~DLXColumn() {}

  enum ColumnType { Position, Value };
  virtual ColumnType type() = 0;
};

struct DLXColumnPos : public DLXColumn {
  int x;
  int y;

  DLXColumnPos(int x, int y) : x(x), y(y) {}
  virtual ~DLXColumnPos() {}
  virtual ColumnType type() { return Position; }
};

struct DLXColumnValue : public DLXColumn {
  int value;

  explicit DLXColumnValue(int v) : value(v) {}
  virtual ~DLXColumnValue() {}
  virtual ColumnType type() { return Value; }
};

class SudokuSolverException : public SudokuGridException {
public:
  SudokuSolverException(const SudokuSolverException &other)
      : SudokuGridException(other) {}
  SudokuSolverException(const char *what) : SudokuGridException(what) {}
};

class DLX {
public:
  DLX(const SudokuGrid &src);
  ~DLX();

  bool solve();

  int stepsTaken;
  bool solutionFound;
  bool uniqueSolution;
  SudokuGrid solution;

protected:
  bool solveAll();
  SudokuGrid currentSolution;
  DLXColumn *m_root;
};
