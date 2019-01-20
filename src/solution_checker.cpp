// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#include "solution_checker.hpp"
#include "problem.hpp"
#include "solution.hpp"
#include "utils.hpp"

#include <unordered_set>
#include <unordered_map>
#include <sstream>
#include <iostream>

using namespace std;

template<typename ... Args>
void SolutionChecker::error(const string& type, const string& format, Args ... args ) {
    stringstream header;
    if (plateId_ >= 0) {
      header << "Plate #" << plateId_;
    }
    if (cutId_ >= 0) {
      if (plateId_ >= 0) {
        header << ", cut #" << cutId_;
      }
      else {
        header << "Cut #" << cutId_;
      }
    }
    if (rowId_ >= 0) {
      if (cutId_ >= 0) {
        header << ", row #" << rowId_;
      }
      else {
        header << "Row #" << rowId_;
      }
    }
    if (plateId_ >= 0 || cutId_ >= 0 || rowId_ >= 0) {
      header << ": ";
    }

    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1;
    unique_ptr<char[]> buf( new char[ size ] );
    snprintf( buf.get(), size, format.c_str(), args ... );
    string msg(buf.get(), buf.get() + size - 1);
    errors_.at(type).push_back(header.str() + msg);
}

void SolutionChecker::report(const Problem &problem) {
  SolutionChecker checker(problem);
  checker.reportProblem();
}

void SolutionChecker::report(const Problem &problem, const Solution &solution) {
  SolutionChecker checker(problem);
  checker.checkSolution(solution);
  checker.reportErrors();
  checker.reportProblem();
  checker.reportQuality(solution);
}

int SolutionChecker::nViolations(const Problem &problem, const Solution &solution) {
  SolutionChecker checker(problem);
  checker.checkSolution(solution);
  return checker.nViolations();
}

double SolutionChecker::evalPercentMapped(const Problem &problem, const Solution &solution) {
  SolutionChecker checker(problem);
  return 100.0 * checker.evalAreaMapped(solution) / checker.evalTotalArea();
}

double SolutionChecker::evalPercentDensity(const Problem &problem, const Solution &solution) {
  SolutionChecker checker(problem);
  return 100.0 * checker.evalAreaMapped(solution) / checker.evalAreaUsage(solution);
}

SolutionChecker::SolutionChecker(const Problem &problem)
: problem_(problem)
, plateId_(-1)
, cutId_(-1)
, rowId_(-1) {
  errors_["Critical"].clear();
  errors_["Ordering"].clear();
  errors_["MinWaste"].clear();
  errors_["Defect"].clear();
}

void SolutionChecker::checkSolution(const Solution &solution) {
  checkItemUnicity(solution);
  checkSequences(solution);

  if (Params::nPlates < (int) solution.plates.size())
    error("Critical", "Too many plates in the solution");

  for (int i = 0; i < (int) solution.plates.size(); ++i) {
    plateId_ = i;
    bool lastPlate = i + 1 == (int) solution.plates.size();
    const PlateSolution &plate = solution.plates[i];
    checkPlate(plate, lastPlate);
  }
  plateId_ = -1;
}

int SolutionChecker::nViolations() {
  int violations = 0;
  for (const auto& err : errors_) {
    violations += err.second.size();
  }
  return violations;
}

long long SolutionChecker::evalAreaUsage(const Solution &solution) {
  long long heightPlates = Params::heightPlates;
  long long widthPlates = Params::widthPlates;
  long long areaPlates = widthPlates * heightPlates;

  int nbFull = solution.plates.size() - 1;
  for (; nbFull >= 0; --nbFull) {
    const PlateSolution &plate = solution.plates[nbFull];
    if (plate.cuts.empty()) continue;

    int usedOnPlate = heightPlates * plate.cuts.back().maxX();
    return areaPlates * nbFull + usedOnPlate;
  }
  return 0;
}

long long SolutionChecker::evalAreaMapped(const Solution &solution) {
  long long areaMapped = 0;
  for (const PlateSolution &plate : solution.plates) {
    for (const CutSolution &cut: plate.cuts) {
      for (const RowSolution &row: cut.rows) {
        for (const ItemSolution &sol: row.items) {
          Item item = problem_.items()[sol.itemId];
          areaMapped += item.area();
        }
      }
    }
  }
  return areaMapped;
}

long long SolutionChecker::evalTotalArea() {
  long long areaTotal = 0;
  for (Item item : problem_.items()) {
    areaTotal += item.area();
  }
  return areaTotal;
}

long long SolutionChecker::evalPlateArea() {
  return Params::widthPlates * Params::heightPlates;
}

int SolutionChecker::nItems() {
  return problem_.items().size();
}

int SolutionChecker::nMappedItems(const Solution &solution) {
  unordered_set<int> visitedItems;
  for (const PlateSolution &plate : solution.plates) {
    for (const CutSolution &cut: plate.cuts) {
      for (const RowSolution &row: cut.rows) {
        for (const ItemSolution &item: row.items) {
          visitedItems.insert(item.itemId);
        }
      }
    }
  }
  return visitedItems.size();
}

void SolutionChecker::checkPlate(const PlateSolution &plate, bool lastPlate) {
  checkPlateDivision(plate, lastPlate);
  for (int i = 0; i < (int) plate.cuts.size(); ++i) {
    cutId_ = i;
    const CutSolution &cut = plate.cuts[i];
    checkCut(cut);
  }
  cutId_ = -1;
}

void SolutionChecker::checkPlateDivision(const PlateSolution &plate, bool lastPlate) {
  if (plate.minX() != 0 || plate.minY() != 0)
    error("Critical", "Lower left is not at (0,0)");
  if (plate.maxX() != Params::widthPlates || plate.maxY() != Params::heightPlates)
    error("Critical", "Upper right is not at (%d,%d)", Params::widthPlates, Params::heightPlates);

  if (plate.cuts.empty()) return;

  if (plate.cuts.front().minX() != plate.minX())
    error("Critical", "First cut doesn't start at %d", plate.minX());
  if (!lastPlate) {
    if(plate.cuts.back().maxX() != plate.maxX())
      error("Critical", "Last cut doesn't end at %d", plate.maxX());
  }
  else {
    if (plate.cuts.back().maxX() != plate.maxX()
     && plate.cuts.back().maxX() > plate.maxX() - Params::minWaste)
      error("Critical", "Last cut on the last plate is at %d, which is not valid", plate.cuts.back().maxX());
  }

  for (const CutSolution &cut : plate.cuts) {
    if (cut.minY() != plate.minY() || cut.maxY() != plate.maxY())
      error("Critical", "Cut height from %d to %d doesn't match the plate", cut.minY(), cut.maxY());
  }

  for (int i = 0; i+1 < (int) plate.cuts.size(); ++i) {
    if (plate.cuts[i].maxX() != plate.cuts[i+1].minX())
      error("Critical", "Cut %d ends at %d but cut %d starts at %d",
          i, plate.cuts[i].maxX(),
          i+1, plate.cuts[i+1].minX());

    for (const Defect &defect : problem_.plateDefects()[plateId_]) {
      if (vCutIntersects(plate.cuts[i].maxX(), defect, plate.minY(), plate.maxY()))
        error("Defect", "Cut at %d intersects a defect (%d,%d)x(%d,%d)", plate.cuts[i].maxX(),
          defect.minX(), defect.maxX(), defect.minY(), defect.maxY());
    }
  }
}

void SolutionChecker::checkCut(const CutSolution &cut) {
  checkCutSize(cut);
  checkCutDivision(cut);
  for (int i = 0; i < (int) cut.rows.size(); ++i) {
    rowId_ = i;
    const RowSolution &row = cut.rows[i];
    checkRow(row);
  }
  rowId_ = -1;
}

void SolutionChecker::checkCutSize(const CutSolution &cut) {
  if (cut.width() > Params::maxXX)
      error("Critical", "Cut is %d-wide, which is larger than the maximum allowed value %d", cut.width(), Params::maxXX);
  if (cut.width() < Params::minXX)
      error("Critical", "Cut is %d-wide, which is smaller than the minimum allowed value %d", cut.width(), Params::minXX);
}

void SolutionChecker::checkCutDivision(const CutSolution &cut) {
  if (cut.rows.empty()) return;

  for (const RowSolution &row: cut.rows) {
    if (!cut.contains(row))
      error("Critical", "Row #%d not contained in cut", rowId_);
    if (row.minX() != cut.minX())
      error("Critical", "Row #%d starts at %d instead of %d", rowId_, row.minX(), cut.minX());
    if (row.maxX() != cut.maxX())
      error("Critical", "Row #%d ends at %d instead of %d", rowId_, row.maxX(), cut.maxX());
  }

  if (cut.rows.front().minY() != cut.minY())
    error("Critical", "First row doesn't start at %d", cut.minY());
  if (cut.rows.back().maxY() != cut.maxY())
    error("Critical", "Last row doesn't end at %d", cut.maxY());

  for (int i = 0; i+1 < (int) cut.rows.size(); ++i) {
    if (cut.rows[i].maxY() != cut.rows[i+1].minY())
      error("Critical", "Row #%d ends at %d but row #%d starts at %d",
          i, cut.rows[i].maxY(),
          i+1, cut.rows[i+1].minY());

    for (const Defect &defect : problem_.plateDefects()[plateId_]) {
      if (hCutIntersects(cut.rows[i].maxY(), defect, cut.minX(), cut.maxX()))
        error("Defect", "Cut at %d intersects a defect (%d,%d)x(%d,%d)", cut.rows[i].maxY(),
          defect.minX(), defect.maxX(), defect.minY(), defect.maxY());
    }
  }
}

void SolutionChecker::checkRow(const RowSolution &row) {
  checkRowSize(row);
  checkRowDivision(row);
  for (const ItemSolution &item : row.items)
    checkItem(item);
}

void SolutionChecker::checkRowSize(const RowSolution &row) {
  if (row.height() < Params::minYY)
      error("Critical", "Row is %d-high, which is smaller than the minimum allowed value %d", row.height(), Params::minYY);
}

void SolutionChecker::checkRowDivision(const RowSolution &row) {
  if (row.items.empty()) return;

  for (const ItemSolution &item : row.items) {
    if (!row.contains(item))
      error("Critical", "Item #%d not contained in the row", item.itemId);
    if (!utils::fitsMinWaste(row.minY(), item.minY()))
      error("MinWaste", "Below item #%d", item.itemId);
    if (!utils::fitsMinWaste(item.maxY(), row.maxY()))
      error("MinWaste", "Above item #%d", item.itemId);
    if (item.maxY() != row.maxY() && item.minY() != row.minY())
      error("Critical", "Cutting item #%d would require two 4-cuts", item.itemId);
  }

  if (!utils::fitsMinWaste(row.minX(), row.items.front().minX()))
    error("MinWaste", "Before item #%d", row.items.front().itemId);
  if (!utils::fitsMinWaste(row.items.back().maxX(), row.maxX()))
    error("MinWaste", "After item #%d", row.items.back().itemId);

  for (int i = 0; i+1 < (int) row.items.size(); ++i) {
    ItemSolution item1 = row.items[i];
    ItemSolution item2 = row.items[i+1];
    if (item1.maxX() > item2.minX())
      error("Critical", "Items #%d and #%d overlap",
          item1.itemId, item2.itemId);
    else if (!utils::fitsMinWaste(item1.maxX(), item2.minX()))
      error("MinWaste", "Between items #%d and #%d",
          item1.itemId, item2.itemId);

    for (const Defect &defect : problem_.plateDefects()[plateId_]) {
      if (vCutIntersects(item1.maxX(), defect, row.minY(), row.maxY()))
        error("Defect", "Cut at %d intersects a defect (%d,%d)x(%d,%d)", item1.maxX(),
          defect.minX(), defect.maxX(), defect.minY(), defect.maxY());
      if (item1.maxX() != item2.minX() && vCutIntersects(item2.minX(), defect, row.minY(), row.maxY()))
        error("Defect", "Cut at %d intersects a defect (%d,%d)x(%d,%d)", item2.minX(),
          defect.minX(), defect.maxX(), defect.minY(), defect.maxY());
    }
  }
}

void SolutionChecker::checkItem(const ItemSolution &sol) {
  if (sol.itemId < 0 || sol.itemId >= (int) problem_.items().size())
    error("Critical", "Item ID %d is not valid", sol.itemId);

  Item item = problem_.items()[sol.itemId];
  bool ok = (sol.width() == item.width && sol.height() == item.height)
         || (sol.width() == item.height && sol.height() == item.width);
  if (!ok)
    error("Critical", "Expected a size of %dx%d for item #%d, got %dx%d",
        item.width, item.height,
        sol.itemId, sol.width(), sol.height());

  for (const Defect &defect : problem_.plateDefects()[plateId_]) {
    if (sol.intersects(defect))
      error("Defect", "Item #%d intersects a defect (%d,%d)x(%d,%d)", sol.itemId,
          defect.minX(), defect.maxX(), defect.minY(), defect.maxY());
  }
}

void SolutionChecker::checkItemUnicity(const Solution &solution) {
  unordered_set<int> visitedItems;
  for (const PlateSolution &plate : solution.plates) {
    for (const CutSolution &cut: plate.cuts) {
      for (const RowSolution &row: cut.rows) {
        for (const ItemSolution &item: row.items) {
          if (visitedItems.count(item.itemId) != 0)
            error("Critical", "Item #%d is mapped multiple times", item.itemId);
          visitedItems.insert(item.itemId);
        }
      }
    }
  }
}

void SolutionChecker::checkSequences(const Solution &solution) {
  unordered_map<int, int> itemPositions;
  int pos = 0;
  for (const PlateSolution &plate : solution.plates) {
    for (const CutSolution &cut: plate.cuts) {
      for (const RowSolution &row: cut.rows) {
        for (const ItemSolution &item: row.items) {
          itemPositions[item.itemId] = pos++;
        }
      }
    }
  }

  for (const vector<Item> &sequence : problem_.stackItems()) {
    for (unsigned i = 0; i + 1 < sequence.size(); ++i) {
      int ida = sequence[i].id;
      int idb = sequence[i+1].id;
      if (itemPositions.count(idb) == 0)
        continue;
      if (itemPositions.count(ida) == 0)
        error("Ordering", "Item #%d is cut but item #%d is not", idb, ida);
      if (itemPositions[ida] > itemPositions[idb])
        error("Ordering", "Item #%d is cut before item #%d", idb, ida);
    }
  }
}

void SolutionChecker::reportErrors() {
  bool error = false;
  if (!errors_.at("Critical").empty()) {
    cout << endl << "Critical violations:" << endl;
    error = true;
  }
  for (auto m : errors_.at("Critical")) {
    cout << "\t" << m << endl;
  }

  if (!errors_.at("Ordering").empty()) {
    cout << endl << "Ordering violations:" << endl;
    error = true;
  }
  for (auto m : errors_.at("Ordering")) {
    cout << "\t" << m << endl;
  }

  if (!errors_.at("MinWaste").empty()) {
    cout << endl << "Minimum waste violations:" << endl;
    error = true;
  }
  for (auto m : errors_.at("MinWaste")) {
    cout << "\t" << m << endl;
  }

  if (!errors_.at("Defect").empty()) {
    cout << endl << "Defect violations:" << endl;
    error = true;
  }
  for (auto m : errors_.at("Defect")) {
    cout << "\t" << m << endl;
  }

  if (!error)
    cout << "No violation detected" << endl;
  else
    cout << endl;
}

void SolutionChecker::reportProblem() {
  cout << nItems() << " items to cut" << endl;
  cout << problem_.stackItems().size() << " stacks" << endl;
  int maxDim = 0;
  for (Item item : problem_.items()) maxDim = max(maxDim, item.height);
  cout << maxDim << " maximum size" << endl;
  int minDim = maxDim;
  for (Item item : problem_.items()) minDim = min(minDim, item.width);
  cout << minDim << " minimum size" << endl;
}

void SolutionChecker::reportQuality(const Solution &solution) {
  long long used = evalAreaUsage(solution);
  long long mapped = evalAreaMapped(solution);
  long long total = evalTotalArea();
  long long plate = evalPlateArea();
  long long wasted = used - mapped;
  if (nMappedItems(solution) != nItems()) {
    cout << "Only " << nMappedItems(solution) << " out of " << nItems() << " items are cut" << endl;
    cout << "That is " << 100.0 * mapped / total << "% of the area" << endl;
  }
  cout << 100.0 * mapped / used << "% density" << endl;
  cout << 100.0 * wasted / used << "% wasted" << endl;
  cout << 1.0 * used / plate << " plates used" << endl;
  cout << 1.0 * wasted / plate << " plates wasted" << endl;
  cout << total << " item area" << endl;
  cout << wasted << " objective value" << endl;
  cout << endl;
}

bool SolutionChecker::vCutIntersects(int x, const Defect &defect, int minY, int maxY) const {
  Rectangle cut = Rectangle::FromCoordinates(x, minY, x, maxY);
  return defect.intersects(cut);
}

bool SolutionChecker::hCutIntersects(int y, const Defect &defect, int minX, int maxX) const {
  Rectangle cut = Rectangle::FromCoordinates(minX, y, maxX, y);
  return defect.intersects(cut);
}

