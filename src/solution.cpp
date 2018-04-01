
#include "solution.hpp"

ItemSolution::ItemSolution(int x, int y, int w, int h) {
  *(Rectangle*) this = Rectangle::FromDimensions(x, y, w, h);
}

RowSolution::RowSolution(int x, int y, int w, int h) {
  *(Rectangle*) this = Rectangle::FromDimensions(x, y, w, h);
}

CutSolution::CutSolution(int x, int y, int w, int h) {
  *(Rectangle*) this = Rectangle::FromDimensions(x, y, w, h);
}

PlateSolution::PlateSolution(int x, int y, int w, int h) {
  *(Rectangle*) this = Rectangle::FromDimensions(x, y, w, h);
}

int RowSolution::nItems() const {
  return items.size();
}

int CutSolution::nItems() const {
  int cnt = 0;
  for (const RowSolution &row : rows)
    cnt += row.nItems();
  return cnt;
}

int PlateSolution::nItems() const {
  int cnt = 0;
  for (const CutSolution &cut : cuts)
    cnt += cut.nItems();
  return cnt;
}

int Solution::nItems() const {
  int cnt = 0;
  for (const PlateSolution &plate : plates)
    cnt += plate.nItems();
  return cnt;
}


