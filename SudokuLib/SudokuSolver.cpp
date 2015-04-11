#include "SudokuSolver.h"
#include <functional>
#include <limits>
#include <set>
#include <vector>

namespace DLXPrivate {

void freeColumn(DLXColumn *col) {
  DLXItem *iter = col->nextRow;
  while (iter != col) {
    DLXItem *next = iter->nextRow;
    delete iter;
    iter = next;
  }
  delete col;
}

static const int constraintGroupSize = 9 * 9;

void mapColumnIndexes(int x, int y, int value, int cols[4]) {
  cols[0] = x + (y * 9);
  cols[1] = constraintGroupSize + value + (y * 9);
  cols[2] = (constraintGroupSize * 2) + value + (x * 9);
  cols[3] = (constraintGroupSize * 3) + value + ((x / 3) + (y / 3) * 3) * 9;
}

void coverColumn(DLXColumn *col) {
  col->removeCol();
  // Remove each row from *other* columns (keep them in this column so we can
  // iterate/restore them)
  for (DLXItem *itemY = col->nextRow; itemY != col; itemY = itemY->nextRow) {
    for (DLXItem *itemX = itemY->nextCol; itemX != itemY;
         itemX = itemX->nextCol) {
      itemX->removeRow();
    }
  }
}

void uncoverColumn(DLXColumn *col) {
  for (DLXItem *itemY = col->prevRow; itemY != col; itemY = itemY->prevRow) {
    for (DLXItem *itemX = itemY->prevCol; itemX != itemY;
         itemX = itemX->prevCol) {
      itemX->replaceRow();
    }
  }
  col->replaceCol();
}

// Iterate through the column headers accessible from this item to find
// appropriate position/value data
void getSudokuData(DLXItem *item, int &x, int &y, int &value) {
  bool posFound = false;
  bool valueFound = false;
  DLXItem *iter = item;
  do {
    switch (iter->columnHeader->type()) {
      case DLXColumn::Position: {
        DLXColumnPos *col = (DLXColumnPos *)iter->columnHeader;
        x = col->x;
        y = col->y;
        posFound = true;
      } break;
      case DLXColumn::Value: {
        value = ((DLXColumnValue *)iter->columnHeader)->value;
        valueFound = true;
      } break;
    }
    iter = iter->nextCol;
  } while (iter != item);
  if (!(posFound && valueFound))
    throw SudokuSolverException("invariant violated");
}
}
using namespace DLXPrivate;

void DLXItem::insertColumn(DLXItem *before) {
  nextCol = before;
  prevCol = before->prevCol;
  replaceCol();
}

void DLXItem::insertRow(DLXItem *before) {
  nextRow = before;
  prevRow = before->prevRow;
  columnHeader = before->columnHeader;
  replaceRow();
}

void DLXItem::removeCol() {
  prevCol->nextCol = nextCol;
  nextCol->prevCol = prevCol;
}

void DLXItem::replaceCol() {
  prevCol->nextCol = this;
  nextCol->prevCol = this;
}

void DLXItem::removeRow() {
  prevRow->nextRow = nextRow;
  nextRow->prevRow = prevRow;
  columnHeader->columnSum--;
}

void DLXItem::replaceRow() {
  prevRow->nextRow = this;
  nextRow->prevRow = this;
  columnHeader->columnSum++;
}

DLX::DLX(const SudokuGrid &grid) : m_root(new DLXColumnValue(0)) {
  m_root->nextCol = m_root->prevCol = m_root;
  int cols[4];
  // Mark constraints already given in the solution
  std::set<int> ignoreCols;
  for (int y = 0; y < 9; y++) {
    for (int x = 0; x < 9; x++) {
      if (grid.at(x, y) != 0) {
        currentSolution.at(x, y) = grid.at(x, y);
        mapColumnIndexes(x, y, grid.at(x, y) - 1, cols);
        for (int constraintType = 0; constraintType < 4; constraintType++)
          ignoreCols.insert(cols[constraintType]);
      }
    }
  }
  // During the construction phase, we store each constraint in a vector for
  // constant-time access
  std::vector<DLXColumn *> constraints;
  constraints.reserve(constraintGroupSize * 4);
  auto makePositionColumn =
      [](int x, int y) -> DLXColumn *{ return new DLXColumnPos(x, y); };
  auto makeValueColumn =
      [](int x, int y) -> DLXColumn *{ return new DLXColumnValue(x + 1); };
  std::function<DLXColumn *(int, int)> constraintFactories[4] = {
    makePositionColumn, makeValueColumn, makeValueColumn, makeValueColumn
  };
  for (int constraintType = 0; constraintType < 4; constraintType++) {
    for (int y = 0; y < 9; y++) {
      for (int x = 0; x < 9; x++) {
        if (ignoreCols.find(constraintType * 9 * 9 + y * 9 + x) ==
            ignoreCols.end()) {
          DLXColumn *c = constraintFactories[constraintType](x, y);
          c->insertColumn(m_root);
          constraints.push_back(c);
        } else {
          // skip column
          constraints.push_back(NULL);
        }
      }
    }
  }
  // Add each value of each cell
  for (int y = 0; y < 9; y++) {
    for (int x = 0; x < 9; x++) {
      for (int value = 0; value < 9; value++) {
        mapColumnIndexes(x, y, value, cols);
        // If this row includes a column with an already-satisfied constraint,
        // we skip it
        bool skip = false;
        for (int constraintType = 0; constraintType < 4; constraintType++) {
          if (!constraints[cols[constraintType]]) {
            skip = true;
            break;
          }
        }
        if (skip)
          continue;
        DLXItem *cellItem = new DLXItem;
        cellItem->insertRow(constraints[cols[0]]);
        for (int constraintType = 1; constraintType < 4; constraintType++) {
          DLXItem *rowItem = new DLXItem;
          rowItem->insertRow(constraints[cols[constraintType]]);
          rowItem->insertColumn(cellItem);
        }
      }
    }
  }
}

DLX::~DLX() {
  DLXColumn *iter = m_root->nextCol->columnHeader;
  while (iter != m_root) {
    DLXColumn *next = iter->nextCol->columnHeader;
    freeColumn(iter);
    iter = next;
  }
  delete m_root;
}

bool DLX::solve() {
  solutionFound = false;
  stepsTaken = 0;
  solveAll();
  return (solutionFound && uniqueSolution);
}

// Returns true if we should stop finding solutions.
bool DLX::solveAll() {
  if (m_root->nextCol == m_root) {
    // all constraints satisfied
    if (!solutionFound) {
      solution = currentSolution;
      solutionFound = true;
      uniqueSolution = true;
      // logicalSteps = cLogicalSteps;
      return false; // Keep finding solutions in case it's not unique
    } else {
      uniqueSolution = false;
      return true; // No need to continue; sudoku is invalid
    }
  }
  // Find smallest column
  DLXColumn *selectedColumn = NULL;
  int minSum = std::numeric_limits<int>::max();
  for (DLXColumn *col = m_root->nextCol->columnHeader; col != m_root;
       col = col->nextCol->columnHeader) {
    if (col->columnSum < minSum) {
      minSum = col->columnSum;
      selectedColumn = col;
    }
  }
  coverColumn(selectedColumn);
  bool stopFindingSolutions = false;
  // Try each row in the column
  for (DLXItem *itemY = selectedColumn->nextRow;
       itemY != selectedColumn && !stopFindingSolutions;
       itemY = itemY->nextRow) {
    // Cover columns of this row
    for (DLXItem *itemX = itemY->nextCol; itemX != itemY;
         itemX = itemX->nextCol) {
      coverColumn(itemX->columnHeader);
    }
    // fill in grid cell
    int x, y, value;
    getSudokuData(itemY, x, y, value);
    currentSolution.at(x, y) = value;
    // solve from this new state
    stepsTaken++;
    stopFindingSolutions = solveAll();
    // Undo our changes
    currentSolution.at(x, y) = 0;
    for (DLXItem *itemX = itemY->prevCol; itemX != itemY;
         itemX = itemX->prevCol) {
      uncoverColumn(itemX->columnHeader);
    }
  }
  uncoverColumn(selectedColumn);
  return stopFindingSolutions;
}
