
#include "solution.hpp"

#include <ostream>
#include <fstream>

using namespace std;

ItemSolution::ItemSolution(int x, int y, int w, int h) {
  *(Rectangle*) this = Rectangle::FromDimensions(x, y, w, h);
}

RowSolution::RowSolution(int x, int y, int w, int h) {
  *(Rectangle*) this = Rectangle::FromDimensions(x, y, w, h);
  items.reserve(8);
}

CutSolution::CutSolution(int x, int y, int w, int h) {
  *(Rectangle*) this = Rectangle::FromDimensions(x, y, w, h);
  rows.reserve(8);
}

PlateSolution::PlateSolution(int x, int y, int w, int h) {
  *(Rectangle*) this = Rectangle::FromDimensions(x, y, w, h);
  cuts.reserve(8);
}

int RowSolution::nItems() const {
  return items.size();
}

int RowSolution::maxUsedY() const {
  int maxUsed = minY();
  for (ItemSolution item : items) {
    maxUsed = max(item.maxY(), maxUsed);
  }
  return maxUsed;
}

int RowSolution::maxUsedX() const {
  if (items.empty())
    return minX();
  else
    return items.back().maxX();
}

int CutSolution::nItems() const {
  int cnt = 0;
  for (const RowSolution &row : rows)
    cnt += row.nItems();
  return cnt;
}

int CutSolution::maxUsedX() const {
  int maxUsed = minX();
  for (const RowSolution &row : rows) {
    maxUsed = max(row.maxUsedX(), maxUsed);
  }
  return maxUsed;
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

void Solution::report(ostream &s) const {
  int plateId = 0;
  for (const PlateSolution &plate : plates) {
    s << "Plate #" << plateId++ << endl;
    int cutId = 0;
    for (const CutSolution &cut : plate.cuts) {
      s << "\tCut #" << cutId++ << " from " << cut.minX() << " to " << cut.maxX() << "  (" << cut.width() << "x" << cut.height() << ")" << endl;
      int rowId = 0;
      for (const RowSolution &row : cut.rows) {
        s << "\t\tRow #" << rowId++ << " from " << row.minY() << " to " << row.maxY() << "  (" << row.width() << "x" << row.height() << ")" << endl;
        for (const ItemSolution &item: row.items) {
          s << "\t\t\tItem #" << item.itemId << " from " << item.minX() << " to " << item.maxX() << "  (" << item.width() << "x" << item.height() << ")" << endl;
          //s << "\t\t\tItem #" << item.itemId << " at (" << item.minX() << ", " << item.maxX() << ")x(" << item.minY() << ", " << item.maxY() << ")" << endl;
        }
      }
    }
  }
  s << endl;
}

class SolutionWriter {
 public:
  static void run(const Solution &solution, ostream &s);

 private:
  SolutionWriter(const Solution &solution, ostream &s);
  void run();

  void writeHeader();

  void writePlate(const PlateSolution &plate);
  void writeCut(const CutSolution &cut, int parent);
  void writeRow(const RowSolution &row, int parent);
  void writeItem(const ItemSolution &item, const RowSolution &row, int parent);

  void writeRowWaste(const RowSolution &row, int parent, int begin, int end);
  void writeResidual(const PlateSolution &plate, int parent);

  int writeRectangle(Rectangle rect, int type, int cutLevel, int parent=-1);

 private:
  const Solution &solution_;
  ostream &s_;
  int plateId_;
  int nodeId_;

  static const int WASTE = -1;
  static const int PATTERN = -2;
  static const int RESIDUAL = -3;
};

void SolutionWriter::run(const Solution &solution, ostream &s) {
  SolutionWriter writer(solution, s);
  writer.run();
}

SolutionWriter::SolutionWriter(const Solution &solution, ostream &s)
: solution_(solution)
, s_(s)
, plateId_(0)
, nodeId_(0) {
}

void SolutionWriter::run() {
  writeHeader();
  for (plateId_ = 0; plateId_ < solution_.nPlates(); ++plateId_) {
    writePlate(solution_.plates[plateId_]);
  }
}

void SolutionWriter::writeHeader() {
  s_ << "PLATE_ID;NODE_ID;X;Y;WIDTH;HEIGHT;TYPE;CUT;PARENT" << endl;
}

void SolutionWriter::writePlate(const PlateSolution &plate) {
  int type = plate.cuts.empty() ? WASTE : PATTERN;
  int id = writeRectangle(plate, type, 0);
  for (const CutSolution &cut : plate.cuts) {
    writeCut(cut, id);
  }
  writeResidual(plate, id);
}

void SolutionWriter::writeCut(const CutSolution &cut, int parent) {
  int type = cut.rows.empty() ? WASTE : PATTERN;
  int id = writeRectangle(cut, type, 1, parent);
  for (const RowSolution &row : cut.rows) {
    writeRow(row, id);
  }
}

void SolutionWriter::writeRow(const RowSolution &row, int parent) {
  int type = row.items.empty() ? WASTE : PATTERN;
  int id = writeRectangle(row, type, 2, parent);

  if (row.items.empty())
    return;

  writeRowWaste(row, id, row.minX(), row.items.front().minX());

  for (int i = 0; i < row.nItems(); ++i) {
    ItemSolution item = row.items[i];
    writeItem(item, row, id);

    if (i + 1 < row.nItems())
      writeRowWaste(row, id, item.maxX(), row.items[i+1].minX());
  }

  writeRowWaste(row, id, row.items.back().maxX(), row.maxX());
}

void SolutionWriter::writeItem(const ItemSolution &item, const RowSolution &row, int parent) {
  if (item.minY() != row.minY() || item.maxY() != row.maxY()) {
    Rectangle master = Rectangle::FromCoordinates(item.minX(), row.minY(), item.maxX(), row.maxY());
    int id = writeRectangle(master, PATTERN, 3, parent);
    if (item.minY() != row.minY()) {
      Rectangle waste = Rectangle::FromCoordinates(item.minX(), row.minY(), item.maxX(), item.minY());
      writeRectangle(waste, WASTE, 4, id);
    }
    writeRectangle(item, item.itemId, 4, id);
    if (item.maxY() != row.maxY()) {
      Rectangle waste = Rectangle::FromCoordinates(item.minX(), item.maxY(), item.maxX(), row.maxY());
      writeRectangle(waste, WASTE, 4, id);
    }
  }
  else {
    writeRectangle(item, item.itemId, 3, parent);
  }
}

void SolutionWriter::writeRowWaste(const RowSolution &row, int parent, int begin, int end) {
  if (begin == end)
    return;
  Rectangle waste = Rectangle::FromCoordinates(begin, row.minY(), end, row.maxY());
  writeRectangle(waste, WASTE, 3, parent);
}

void SolutionWriter::writeResidual(const PlateSolution &plate, int parent) {
  if (plateId_ != solution_.nPlates() - 1)
    return;
  if (plate.cuts.empty())
    return;

  CutSolution lastCut = plate.cuts.back();
  if (lastCut.maxX() == plate.maxX())
    return;

  Rectangle residual = Rectangle::FromCoordinates(lastCut.maxX(), plate.minY(), plate.maxX(), plate.maxY());
  writeRectangle(residual, RESIDUAL, 1, parent);
}

int SolutionWriter::writeRectangle(Rectangle r, int type, int cutLevel, int parent) {
  s_ << plateId_ << ";" << nodeId_ << ";";
  s_ << r.minX() << ";" << r.minY() << ";";
  s_ << r.width() << ";" << r.height() << ";";
  s_ << type << ";" << cutLevel << ";";
  if (parent >= 0)
    s_ << parent;
  s_ << endl;
  return nodeId_++;
}

void Solution::write(string prefix) const {
  ofstream solutionFile(prefix + "_solution.csv");
  SolutionWriter::run(*this, solutionFile);
}

