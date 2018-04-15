
#include "solution_checker.hpp"
#include "problem.hpp"
#include "solution.hpp"

#include <unordered_set>
#include <unordered_map>
#include <sstream>
#include <iostream>

using namespace std;

template<typename ... Args>
void SolutionChecker::error(const std::string& type, const std::string& format, Args ... args ) {
    stringstream header;
    if (plateId_ >= 0)
      header << "Plate #" << plateId_;
    if (cutId_ >= 0)
      header << ", cut #" << cutId_;
    if (rowId_ >= 0)
      header << ", row #" << rowId_;
    if (plateId_ >= 0)
      header << ": ";

    std::size_t size = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1;
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    string msg(buf.get(), buf.get() + size - 1);
    errors_.at(type).push_back(header.str() + msg);
}

void SolutionChecker::report(const Problem &problem, const Solution &solution) {
  SolutionChecker checker(problem, solution);
  checker.check();
  checker.reportErrors();
  checker.reportQuality();
}

bool SolutionChecker::check(const Problem &problem, const Solution &solution) {
  SolutionChecker checker(problem, solution);
  return checker.check();
}

long long SolutionChecker::evalAreaViolation(const Problem &problem, const Solution &solution) {
  SolutionChecker checker(problem, solution);
  return checker.evalAreaViolation();
}

long long SolutionChecker::evalAreaUsage(const Problem &problem, const Solution &solution) {
  SolutionChecker checker(problem, solution);
  return checker.evalAreaUsage();
}

SolutionChecker::SolutionChecker(const Problem &problem, const Solution &solution)
: problem_(problem)
, solution_(solution)
, plateId_(-1)
, cutId_(-1)
, rowId_(-1) {
  errors_["Critical"].clear();
  errors_["Ordering"].clear();
  errors_["MinWaste"].clear();
  errors_["Defects"].clear();
}

bool SolutionChecker::check() {
  checkItemUnicity();
  checkSequences();

  if (problem_.params().nPlates < (int) solution_.plates.size())
    error("Critical", "Too many plates in the solution");

  for (int i = 0; i < (int) solution_.plates.size(); ++i) {
    plateId_ = i;
    const PlateSolution &plate = solution_.plates[i];
    checkPlate(plate);
  }
  plateId_ = -1;

  bool hasError = false;
  for (const auto& err : errors_) {
    if (!err.second.empty())
      hasError = true;
  }
  return hasError;
}

long long SolutionChecker::evalAreaViolation() {
  return evalTotalArea() - evalAreaMapped();
}

long long SolutionChecker::evalAreaUsage() {
  long long heightPlates = problem_.params().heightPlates;
  long long widthPlates = problem_.params().widthPlates;
  long long areaPlates = widthPlates * heightPlates;

  int nbFull = solution_.plates.size() - 1;
  for (; nbFull >= 0; --nbFull) {
    const PlateSolution &plate = solution_.plates[nbFull];
    if (plate.cuts.empty()) continue;

    int usedOnPlate = heightPlates * plate.cuts.back().maxX();
    return areaPlates * nbFull + usedOnPlate;
  }
  return 0;
}

long long SolutionChecker::evalAreaMapped() {
  long long areaMapped = 0;
  for (const PlateSolution &plate : solution_.plates) {
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

int SolutionChecker::nItems() {
  return problem_.items().size();
}

int SolutionChecker::nMappedItems() {
  unordered_set<int> visitedItems;
  for (const PlateSolution &plate : solution_.plates) {
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

void SolutionChecker::checkPlate(const PlateSolution &plate) {
  checkPlateDivision(plate);
  for (int i = 0; i < (int) plate.cuts.size(); ++i) {
    cutId_ = i;
    const CutSolution &cut = plate.cuts[i];
    checkCut(cut);
  }
  cutId_ = -1;
}

void SolutionChecker::checkPlateDivision(const PlateSolution &plate) {
  if (plate.minX() != 0 || plate.minY() != 0)
    error("Critical", "Lower left is not at (0,0)");
  if (plate.maxX() != problem_.params().widthPlates
      || plate.maxY() != problem_.params().heightPlates)
    error("Critical", "Upper right is not at (%d,%d)", problem_.params().widthPlates, problem_.params().heightPlates);

  if (plate.cuts.empty()) return;

  if (plate.cuts.front().minX() != plate.minX())
    error("Critical", "First cut doesn't start at %d", plate.minX());
  if (plate.cuts.back().maxX() > plate.maxX())
    error("Critical", "Last cut doesn't end at %d", plate.maxX());

  for (const CutSolution &cut : plate.cuts) {
    if (cut.minY() != plate.minY() || cut.maxY() != plate.maxY())
      error("Critical", "Cut height from %d to %d doesn't match the plate", cut.minY(), cut.maxY());
  }

  for (int i = 0; i+1 < (int) plate.cuts.size(); ++i) {
    if (plate.cuts[i].maxX() != plate.cuts[i+1].minX())
      error("Critical", "Cut %d ends at %d but cut %d starts at %d", i, i+1,
         plate.cuts[i].maxX(), plate.cuts[i+1].minX());
  }

  // TODO: check that no defect is cut
}

void SolutionChecker::checkCut(const CutSolution &cut) {
  checkCutDivision(cut);
  for (int i = 0; i < (int) cut.rows.size(); ++i) {
    rowId_ = i;
    const RowSolution &row = cut.rows[i];
    checkRow(row);
  }
  rowId_ = -1;
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

  for (int i = 0; i+1 < (int) cut.rows.size(); ++i) {
    if (cut.rows[i].maxY() != cut.rows[i+1].minY())
      error("Critical", "Row #%d ends at %d but row #%d starts at %d", i, i+1,
         cut.rows[i].maxY(), cut.rows[i+1].minY());
  }

  // TODO: check that no defect is cut
}

void SolutionChecker::checkRow(const RowSolution &row) {
  checkRowDivision(row);
  for (const ItemSolution &item : row.items)
    checkItem(item);
}

void SolutionChecker::checkRowDivision(const RowSolution &row) {
  if (row.items.empty()) return;

  for (const ItemSolution &item : row.items) {
    if (!row.contains(item))
      error("Critical", "Item #%d not contained in the row", item.itemId);
    if (!fitsMinWaste(row.minY(), item.minY()))
      error("MinWaste", "Below item #%d", item.itemId);
    if (!fitsMinWaste(item.maxY(), row.maxY()))
      error("MinWaste", "Above item #%d", item.itemId);
  }

  if (!fitsMinWaste(row.minX(), row.items.front().minX()))
    error("MinWaste", "Before item #%d", row.items.front().itemId);
  if (!fitsMinWaste(row.items.back().maxX(), row.maxX()))
    error("MinWaste", "After item #%d", row.items.back().itemId);

  for (int i = 0; i+1 < (int) row.items.size(); ++i) {
    ItemSolution item1 = row.items[i];
    ItemSolution item2 = row.items[i+1];
    if (item1.maxX() > item2.minX())
      error("Critical", "Items #%d and #%d overlap",
          item1.itemId, item2.itemId);
    else if (!fitsMinWaste(item1.maxX(), item2.minX()))
      error("MinWaste", "Between items #%d and #%d",
          item1.itemId, item2.itemId);
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

  for (Defect defect : problem_.plateDefects()[plateId_]) {
    if (sol.intersects(defect))
      error("Defects", "Item #%d intersects a defect (%d,%d)x(%d,%d)", sol.itemId,
          defect.minX(), defect.maxX(), defect.minY(), defect.maxY());
  }
}

void SolutionChecker::checkItemUnicity() {
  unordered_set<int> visitedItems;
  for (const PlateSolution &plate : solution_.plates) {
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

void SolutionChecker::checkSequences() {
  unordered_map<int, int> itemPositions;
  int pos = 0;
  for (const PlateSolution &plate : solution_.plates) {
    for (const CutSolution &cut: plate.cuts) {
      for (const RowSolution &row: cut.rows) {
        for (const ItemSolution &item: row.items) {
          itemPositions[item.itemId] = pos++;
        }
      }
    }
  }

  for (const std::vector<Item> &sequence : problem_.sequenceItems()) {
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
  if (!errors_.at("Critical").empty())
    cout << endl << "Critical violations:" << endl;
  for (auto m : errors_.at("Critical")) {
    cout << "\t" << m << endl;
  }

  if (!errors_.at("Ordering").empty())
    cout << endl << "Ordering violations:" << endl;
  for (auto m : errors_.at("Ordering")) {
    cout << "\t" << m << endl;
  }

  if (!errors_.at("MinWaste").empty())
    cout << endl << "Minimum waste violations:" << endl;
  for (auto m : errors_.at("MinWaste")) {
    cout << "\t" << m << endl;
  }

  if (!errors_.at("Defects").empty())
    cout << endl << "Defect violations:" << endl;
  for (auto m : errors_.at("Defects")) {
    cout << "\t" << m << endl;
  }
}

void SolutionChecker::reportQuality() {
  if (nMappedItems() != nItems()) {
    cout << "Only " << nMappedItems() << " out of " << nItems() << " are cut" << endl;
    cout << "That is " << 100.0 * evalAreaMapped() / evalTotalArea() << "% of the area" << endl;
  }
  else
    cout << "Every item has been cut" << endl;

  cout << "An area of " << evalAreaMapped() << " is mapped while using an area of " << evalAreaUsage() << endl;
  cout << "That is " << 100.0 * evalAreaMapped() / evalAreaUsage() << "% density" << endl;

}

bool SolutionChecker::fitsMinWaste(int a, int b) const {
  return a == b
    || a <= b - problem_.params().minWaste;
}

