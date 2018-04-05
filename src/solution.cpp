
#include "solution.hpp"

#include <ostream>

using namespace std;

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

void Solution::write(ostream &s) const {
  int plateId = 0;
  for (const PlateSolution &plate : plates) {
    s << "Plate #" << plateId++ << endl;
    for (const CutSolution &cut : plate.cuts) {
      s << "\tCut from " << cut.minX() << " to " << cut.maxX() << endl;
      for (const RowSolution &row : cut.rows) {
        s << "\t\tRow from " << row.minY() << " to " << row.maxY() << endl;
        for (const ItemSolution &item: row.items) {
          s << "\t\t\tItem #" << item.itemId << " from " << item.minX() << " to " << item.maxX() << endl;
        }
      }
    }
  }
}

