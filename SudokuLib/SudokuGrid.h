#pragma once
#include <istream>
#include <array>
#include <vector>
#include <exception>

class SudokuGridException : public std::exception {
public:
  SudokuGridException(const SudokuGridException &);
  SudokuGridException(const char *what);
  virtual const char *what() const noexcept { return m_what; }

protected:
  const char *m_what;
};

class SudokuGrid {
public:
  SudokuGrid();
  SudokuGrid(const SudokuGrid &other);
  explicit SudokuGrid(std::vector<int> flatGrid);
  explicit SudokuGrid(std::istream &input);
  ~SudokuGrid();

  static SudokuGrid generateRandomSolution();
  static SudokuGrid generateRandomPuzzle(int difficulty);

  bool operator==(const SudokuGrid &rhs) const;
  bool operator!=(const SudokuGrid &rhs) const;

  static const int GRIDSIZE = 9;

  int &at(int x, int y);
  int at(int x, int y) const;

  /// Returns the filled-in values at the given column
  std::vector<int> column(int x) const;
  /// Returns the filled-in values at the given row
  std::vector<int> row(int y) const;
  /// Returns the filled-in values at the xth across, yth down square.
  std::vector<int> square(int squareX, int squareY) const;

  bool verify() const;
  bool isComplete() const;
  bool solve();
  int approximateDifficulty() const;

protected:
  std::array<int, GRIDSIZE * GRIDSIZE> m_grid;
};
