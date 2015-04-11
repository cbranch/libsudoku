#include "SudokuGrid.h"
#include "SudokuSolver.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <random>
#include <set>
#include <string>

using std::vector;
using std::set;

namespace SudokuGridPrivate {

bool hasDuplicates(std::vector<int> values) {
  std::sort(values.begin(), values.end());
  return std::unique(values.begin(), values.end()) != values.end();
}

vector<int> getBlock(const vector<int> &grid, int posX, int posY) {
  posX -= posX % 3;
  posY -= posY % 3;
  vector<int> block;
  for (int y = 0; y < 3; y++) {
    size_t offset = (posY + y) * 9 + posX;
    size_t offsetEnd = offset + 3;
    if (offset >= grid.size())
      break;
    offsetEnd = std::min(offsetEnd, grid.size());
    std::copy(grid.begin() + offset, grid.begin() + offsetEnd,
              std::back_inserter(block));
  }
  return block;
}
template <class RandomNumberGenerator>
vector<int> getPossibilities(const vector<int> &grid,
                             RandomNumberGenerator &rng) {
  set<int> taken; // values disallowed
  int x = grid.size() % 9;
  int y = grid.size() / 9;
  vector<int> block(getBlock(grid, x, y));
  std::copy(block.begin(), block.end(), std::inserter(taken, taken.begin()));
  std::copy(grid.begin() + y * 9, grid.begin() + y * 9 + x,
            std::inserter(taken, taken.begin())); // row
  for (int i = 0; i < y; i++) {
    taken.insert(grid[i * 9 + x]); // col
  }
  vector<int> possibilities;
  possibilities.reserve(9 - taken.size());
  for (int i = 1; i <= 9; i++) {
    if (taken.find(i) == taken.end())
      possibilities.push_back(i);
  }
  std::shuffle(possibilities.begin(), possibilities.end(), rng);
  return possibilities;
}
template <class RandomNumberGenerator>
bool generateGrid(vector<int> &grid, RandomNumberGenerator &rng) {
  if (grid.size() == 9 * 9)
    return true;
  vector<int> possibilities(getPossibilities(grid, rng));
  for (vector<int>::const_iterator iter = possibilities.begin();
       iter != possibilities.end(); iter++) {
    grid.push_back(*iter);
    if (generateGrid(grid, rng)) {
      return true;
    } else {
      grid.pop_back();
    }
  }
  return false;
}

struct Coords {
  int x;
  int y;

  Coords() : x(0), y(0) {}
  Coords(int x, int y) : x(x), y(y) {}
};

template <class RandomNumberGenerator>
bool findPuzzle(const SudokuGrid &solution, SudokuGrid &puzzle, int difficulty,
                const vector<Coords> &revealed, vector<size_t> choices,
                RandomNumberGenerator &rng) {
  if (choices.empty())
    return true; // sanity check (should be impossible...)
  std::shuffle(choices.begin(), choices.end(), rng);
  for (vector<size_t>::iterator iter = choices.begin(); iter != choices.end();
       iter++) {
    // clear square
    Coords coords[2];
    coords[0] = revealed[*iter];
    coords[1] = Coords(SudokuGrid::GRIDSIZE - coords[0].x - 1,
                       SudokuGrid::GRIDSIZE - coords[0].y - 1);
    for (int i = 0; i < sizeof(coords) / sizeof(coords[0]); i++) {
      puzzle.at(coords[i].x, coords[i].y) = 0;
    }
    // try difficulty
    int d = puzzle.approximateDifficulty();
    if (d == difficulty) {
      // we're done!
      return true;
    } else if (d > difficulty || d == -1) {
      // too hard, or multiple solutions
    } else {
      // so far so good; try removing a further square
      vector<size_t> newChoices(choices.size() - 1);
      std::copy(iter + 1, choices.end(), newChoices.begin());
      bool success =
          findPuzzle(solution, puzzle, difficulty, revealed, newChoices, rng);
      if (success) {
        return true;
      }
    }
    // Undo modification
    for (int i = 0; i < sizeof(coords) / sizeof(coords[0]); i++) {
      puzzle.at(coords[i].x, coords[i].y) =
          solution.at(coords[i].x, coords[i].y);
    }
  }
  return false;
}
}
using namespace SudokuGridPrivate;

SudokuGridException::SudokuGridException(const SudokuGridException &other)
    : m_what(other.m_what) {}

SudokuGridException::SudokuGridException(const char *what) : m_what(what) {}

SudokuGrid::SudokuGrid() { m_grid.fill(0); }

SudokuGrid::SudokuGrid(const SudokuGrid &other) { m_grid = other.m_grid; }

SudokuGrid::SudokuGrid(std::vector<int> flatGrid) {
  std::copy(
      flatGrid.begin(),
      flatGrid.begin() +
          std::min(flatGrid.size(), static_cast<size_t>(GRIDSIZE * GRIDSIZE)),
      m_grid.begin());
  if (flatGrid.size() < GRIDSIZE * GRIDSIZE)
    std::fill(m_grid.begin() + flatGrid.size(), m_grid.end(), 0);
}

SudokuGrid::SudokuGrid(std::istream &input) {
  m_grid.fill(0); // save us needing to init cells with no set values
  for (int row = 0; row < GRIDSIZE; row++) {
    std::string line;
    std::getline(input, line);
    for (int column = 0;
         column < GRIDSIZE && column < static_cast<int>(line.length());
         column++) {
      char c = line.at(column);
      int &dest = at(column, row);
      if (c >= '0' && c <= '9') {
        dest = c - '0';
      }
    }
    if (input.eof())
      break;
  }
}

SudokuGrid::~SudokuGrid() {}

SudokuGrid SudokuGrid::generateRandomSolution() {
  vector<int> grid;
  grid.reserve(9 * 9);
  std::mt19937 rng(std::chrono::system_clock::now().time_since_epoch().count());
  generateGrid(grid, rng);
  return SudokuGrid(grid);
}

SudokuGrid SudokuGrid::generateRandomPuzzle(int difficulty) {
  // Can we guarantee this solution can be modified to fit the difficulty?
  // Maybe refresh it once every 100 attempts... or figure out a better
  // algorithm!
  SudokuGrid solution(SudokuGrid::generateRandomSolution());
  std::mt19937 rng(std::chrono::system_clock::now().time_since_epoch().count());
  SudokuGrid puzzle(solution);
  // Set up the coordinates we can remove
  vector<Coords> revealed;
  for (int y = 0; y < GRIDSIZE / 2; y++) {
    for (int x = 0; x < GRIDSIZE; x++) {
      revealed.push_back(Coords(x, y));
    }
  }
  for (int x = 0; x <= GRIDSIZE / 2; x++) {
    revealed.push_back(Coords(x, GRIDSIZE / 2));
  }
  // The valid choices we can use
  vector<size_t> choices;
  for (size_t i = 0; i < revealed.size(); i++) {
    choices.push_back(i);
  }
  findPuzzle(solution, puzzle, difficulty, revealed, choices, rng);
  return puzzle;
}

bool SudokuGrid::operator==(const SudokuGrid &rhs) const {
  return std::equal(m_grid.begin(), m_grid.end(), rhs.m_grid.begin());
}

bool SudokuGrid::operator!=(const SudokuGrid &rhs) const {
  return !std::equal(m_grid.begin(), m_grid.end(), rhs.m_grid.begin());
}

int &SudokuGrid::at(int x, int y) { return m_grid.at(x + (y * 9)); }

int SudokuGrid::at(int x, int y) const { return m_grid.at(x + (y * 9)); }

std::vector<int> SudokuGrid::column(int x) const {
  std::vector<int> entries;
  for (int y = 0; y < GRIDSIZE; y++) {
    if (at(x, y) != 0) {
      entries.push_back(at(x, y));
    }
  }
  return entries;
}

std::vector<int> SudokuGrid::row(int y) const {
  std::vector<int> entries;
  for (int x = 0; x < GRIDSIZE; x++) {
    if (at(x, y) != 0) {
      entries.push_back(at(x, y));
    }
  }
  return entries;
}

std::vector<int> SudokuGrid::square(int squareX, int squareY) const {
  std::vector<int> entries;
  for (int y = squareY * 3; y < (squareY + 1) * 3; y++) {
    for (int x = squareX * 3; x < (squareX + 1) * 3; x++) {
      if (at(x, y) != 0) {
        entries.push_back(at(x, y));
      }
    }
  }
  return entries;
}

bool SudokuGrid::verify() const {
  for (int i = 0; i < GRIDSIZE; i++) {
    if (hasDuplicates(row(i)) || hasDuplicates(column(i)) ||
        hasDuplicates(square(i % 3, i / 3)))
      return false;
  }
  return true;
}

bool SudokuGrid::isComplete() const {
  for (int i = 0; i < GRIDSIZE * GRIDSIZE; i++) {
    if (m_grid.at(i) == 0)
      return false;
  }
  return true;
}

bool SudokuGrid::solve() {
  DLX dlx(*this);
  if (dlx.solve()) {
    *this = dlx.solution;
    return true;
  } else {
    return false;
  }
}

int SudokuGrid::approximateDifficulty() const {
  DLX dlx(*this);
  if (dlx.solve()) {
    return static_cast<int>(
        floor(log(static_cast<double>(dlx.stepsTaken)) / log(1.3)));
  } else {
    return -1;
  }
}
