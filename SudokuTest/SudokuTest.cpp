#include "gtest/gtest.h"
#include "SudokuLib/SudokuGrid.h"
#include <fstream>
using std::make_pair;
using std::ifstream;

TEST(SudokuGridTest, LoadGrid) {
  SudokuGrid grid;
  ASSERT_NO_THROW({
    ifstream gridFile("grids/easy");
    grid = SudokuGrid(gridFile);
  });
  EXPECT_EQ(grid.at(0, 0), 5);
  EXPECT_EQ(grid.at(8, 0), 3);
  EXPECT_EQ(grid.at(1, 1), 0);
  EXPECT_EQ(grid.at(4, 7), 5);
}

TEST(SudokuGridTest, VerifyGrid) {
  SudokuGrid grid;
  ASSERT_NO_THROW({
    ifstream gridFile("grids/easy_a"); // solved grid
    grid = SudokuGrid(gridFile);
  });
  EXPECT_TRUE(grid.verify());
  EXPECT_TRUE(grid.isComplete());
  ASSERT_NO_THROW({
    ifstream gridFile("grids/easy"); // unsolved but still valid grid
    grid = SudokuGrid(gridFile);
  });
  EXPECT_TRUE(grid.verify());
  EXPECT_FALSE(grid.isComplete());
  grid.at(0, 0) = 8; // duplicate a value...
  EXPECT_FALSE(grid.verify());
}

TEST(SudokuGridTest, CompareGrid) {
  SudokuGrid first, second;
  first.at(2, 4) = 5;
  second.at(2, 4) = 5;
  EXPECT_EQ(first, second);
  first.at(1, 6) = 1;
  EXPECT_NE(first, second);
  second.at(1, 6) = 7;
  EXPECT_NE(first, second);
  first.at(1, 6) = 7;
  EXPECT_EQ(first, second);
}

TEST(SudokuGridTest, CreateGrid) {
  // do this a few times to make sure
  for (int i = 0; i < 15; i++) {
    SudokuGrid grid(SudokuGrid::generateRandomSolution());
    EXPECT_TRUE(grid.isComplete());
    EXPECT_TRUE(grid.verify());
  }
}

class SudokuPuzzleTest : public testing::TestWithParam<int> {};

INSTANTIATE_TEST_CASE_P(CreatePuzzle, SudokuPuzzleTest, testing::Range(14, 21));

TEST_P(SudokuPuzzleTest, CreatePuzzle) {
  SudokuGrid grid(SudokuGrid::generateRandomPuzzle(GetParam()));
  EXPECT_FALSE(grid.isComplete());
  EXPECT_TRUE(grid.verify());
  EXPECT_TRUE(grid.solve());
}

typedef std::pair<const char *, const char *> GridFilePair;

class SudokuSolveTest : public testing::TestWithParam<GridFilePair> {};

INSTANTIATE_TEST_CASE_P(
    SolvedGrids, SudokuSolveTest,
    testing::Values(make_pair("grids/easy", "grids/easy_a"),
                    make_pair("grids/medium", "grids/medium_a"),
                    make_pair("grids/hard", "grids/hard_a"),
                    make_pair("grids/samurai", "grids/samurai_a")));

TEST_P(SudokuSolveTest, SolvedGrids) {
  SudokuGrid puzzle, solved;
  GridFilePair grids(GetParam());
  ASSERT_NO_THROW({
    ifstream puzzleFile(grids.first);
    puzzle = SudokuGrid(puzzleFile); // unsolved but still valid grid
    ifstream solvedFile(grids.second);
    solved = SudokuGrid(solvedFile); // solved grid
  });
  EXPECT_TRUE(puzzle.verify());
  EXPECT_FALSE(puzzle.isComplete());
  EXPECT_TRUE(solved.verify());
  EXPECT_TRUE(solved.isComplete());
  EXPECT_TRUE(puzzle.solve());
  EXPECT_EQ(puzzle, solved);
  EXPECT_TRUE(puzzle.isComplete());
}

class SudokuDifficultyTest
    : public testing::TestWithParam<std::pair<const char *, int> > {};

INSTANTIATE_TEST_CASE_P(GridDifficulty, SudokuDifficultyTest,
                        testing::Values(make_pair("grids/easy", 14),
                                        make_pair("grids/medium", 15),
                                        make_pair("grids/hard", 16),
                                        make_pair("grids/samurai", 19)));

TEST_P(SudokuDifficultyTest, GridDifficulty) {
  SudokuGrid puzzle;
  ASSERT_NO_THROW({
    ifstream gridFile(GetParam().first); // unsolved but still valid grid
    puzzle = SudokuGrid(gridFile);
  });
  EXPECT_EQ(puzzle.approximateDifficulty(), GetParam().second);
}
