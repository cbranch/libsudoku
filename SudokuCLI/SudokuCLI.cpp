#include <iostream>
#include <functional>
#include <string>
#include <vector>
#include "SudokuLib/SudokuGrid.h"
using std::endl;

void print_usage(char* programName, std::ostream& os) {
  os << "CLI for libSudoku 1.0 by <chris@chrisbranch.co.uk>" << endl;
  os << endl;
  os << "Usage:" << endl;
  os << programName << " [OPTIONS]" << endl;
  os << endl;
  os << "Options:" << endl;
  os << " -g=DIFFICULTY  Generate a sudoku with a difficulty from 1-99" << endl;
  os << " -s             Solve a sudoku passed into standard input" << endl;
  os << " -c             Check if a sudoku passed into stdin is solvable"
     << endl;
  os << " -e             Estimate difficulty of the sudoku passed into stdin"
     << endl;
}

void outputSudoku(std::ostream& os, const SudokuGrid& sudoku) {
  for (int y = 0; y < SudokuGrid::GRIDSIZE; ++y) {
    for (int x = 0; x < SudokuGrid::GRIDSIZE; ++x) {
      int value = sudoku.at(x, y);
      if (value == 0) {
        std::cout << ".";
      } else {
        std::cout << std::to_string(value);
      }
    }
    std::cout << endl;
  }
}

int generate(std::string difficultyString) {
  int difficulty = std::stoi(difficultyString);
  if (difficulty < 0) {
    std::cerr << "difficulty must be a positive integer" << endl;
    return 1;
  }
  outputSudoku(std::cout, SudokuGrid::generateRandomPuzzle(difficulty));
  return 0;
}

int solve() {
  SudokuGrid sudoku(std::cin);
  if (sudoku.solve()) {
    outputSudoku(std::cout, sudoku);
    return 0;
  } else {
    std::cout << "Unsolvable" << endl;
    return 1;
  }
}

int check() {
  SudokuGrid sudoku(std::cin);
  if (sudoku.verify()) {
    std::cout << (sudoku.isComplete() ? "Solved" : "Valid") << endl;
    return 0;
  } else {
    std::cout << "Unsolvable" << endl;
    return 1;
  }
}

int estimate_difficulty() {
  SudokuGrid sudoku(std::cin);
  std::cout << std::to_string(sudoku.approximateDifficulty()) << endl;
  return 0;
}

int main(int argc, char** argv) {
  if (argc == 1) {
    print_usage(argv[0], std::cout);
    return 0;
  }
  if (argc > 2) {
    std::cerr << "too many arguments" << endl << endl;
    print_usage(argv[0], std::cerr);
    return 1;
  }
  std::string arg(argv[1]);
  if (arg.compare(0, 3, "-g=") == 0) {
    return generate(arg.substr(3));
  }
  if (arg.compare("-s") == 0) {
    return solve();
  }
  if (arg.compare("-c") == 0) {
    return check();
  }
  if (arg.compare("-e") == 0) {
    return estimate_difficulty();
  }
  std::cerr << "unknown argument \"" << arg << "\"" << endl << endl;
  print_usage(argv[0], std::cerr);
  return 1;
}
