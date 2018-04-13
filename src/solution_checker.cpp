
#include "solution_checker.hpp"
#include "problem.hpp"
#include "solution.hpp"

#include <unordered_set>
#include <unordered_map>
#include <iostream>

using namespace std;

void SolutionChecker::check(const Problem &problem, const Solution &solution) {
  SolutionChecker checker(problem, solution);
  checker.check();
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
}

void SolutionChecker::check() {
  checkItemUnicity();
  checkSequences();

  if (problem_.params().nPlates < (int) solution_.plates.size())
    topError("Too many plates in the solution");

  for (int i = 0; i < (int) solution_.plates.size(); ++i) {
    plateId_ = i;
    const PlateSolution &plate = solution_.plates[i];
    checkPlate(plate);
  }
  plateId_ = -1;

  report();
}

long long SolutionChecker::evalAreaViolation() {
  long long areaMapped = 0;
  long long areaTotal = 0;
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

  for (Item item : problem_.items()) {
    areaTotal += item.area();
  }

  return areaTotal - areaMapped;
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
    topError("Plate #%d: lower left is not at (0,0)", plateId_);
  if (plate.maxX() != problem_.params().widthPlates
      || plate.maxY() != problem_.params().heightPlates)
    topError("Plate #%d: upper right is not at (%d,%d)", plateId_, problem_.params().widthPlates, problem_.params().heightPlates);

  if (plate.cuts.empty()) return;

  if (plate.cuts.front().minX() != plate.minX())
    topError("Plate #%d: first cut doesn't start at %d", plateId_, plate.minX());
  if (plate.cuts.back().maxX() > plate.maxX())
    topError("Plate #%d: last cut doesn't end at %d", plateId_, plate.maxX());

  for (const CutSolution &cut : plate.cuts) {
    if (cut.minY() != plate.minY() || cut.maxY() != plate.maxY())
      topError("Plate #%d: cut height from %d to %d doesn't match the plate", plateId_, cut.minY(), cut.maxY());
  }

  for (int i = 0; i+1 < (int) plate.cuts.size(); ++i) {
    if (plate.cuts[i].maxX() != plate.cuts[i+1].minX())
      topError("Plate #%d: cut %d ends at %d but cut %d starts at %d", plateId_, i, i+1,
         plate.cuts[i].maxX(), plate.cuts[i+1].minX());
  }
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
      topError("Plate #%d: row #%d not contained in cut #%d", plateId_, rowId_, cutId_);
    if (row.minX() != cut.minX())
      topError("Plate #%d: row #%d in cut #%d starts at %d instead of %d", plateId_, rowId_, cutId_, row.minX(), cut.minX());
    if (row.maxX() != cut.maxX())
      topError("Plate #%d: row #%d in cut #%d ends at %d instead of %d", plateId_, rowId_, cutId_, row.maxX(), cut.maxX());
  }

  for (int i = 0; i+1 < (int) cut.rows.size(); ++i) {
    if (cut.rows[i].maxY() != cut.rows[i+1].minY())
      topError("Plate #%d: row #%d ends at %d but row #%d starts at %d", plateId_, i, i+1,
         cut.rows[i].maxY(), cut.rows[i+1].minY());
  }
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
      topError("Item #%d not contained in the row", item.itemId);
    if (!fitsMinWaste(row.minY(), item.minY()))
      wasteError("Below item #%d", item.itemId);
    if (!fitsMinWaste(item.maxY(), row.maxY()))
      wasteError("Above item #%d", item.itemId);
  }

  if (!fitsMinWaste(row.minX(), row.items.front().minX()))
    wasteError("Before item #%d", row.items.front().itemId);
  if (!fitsMinWaste(row.items.back().maxX(), row.maxX()))
    wasteError("After item #%d", row.items.back().itemId);

  for (int i = 0; i+1 < (int) row.items.size(); ++i) {
    ItemSolution item1 = row.items[i];
    ItemSolution item2 = row.items[i+1];
    if (item1.maxX() > item2.minX())
      topError("Items #%d and #%d overlap",
          item1.itemId, item2.itemId);
    else if (!fitsMinWaste(item1.maxX(), item2.minX()))
      wasteError("Between items #%d and #%d",
          item1.itemId, item2.itemId);
  }
}

void SolutionChecker::checkItem(const ItemSolution &sol) {
  if (sol.itemId < 0 || sol.itemId >= (int) problem_.items().size())
    topError("On plate #%d, cut #%d, row #%d, the item ID %d is not valid",
       plateId_, cutId_, rowId_, sol.itemId);

  Item item = problem_.items()[sol.itemId];
  bool ok = (sol.width() == item.width && sol.height() == item.height)
         || (sol.width() == item.height && sol.height() == item.width);
  if (!ok)
    topError("On plate #%d, expected a size of %dx%d for item #%d, got %dx%d",
        plateId_, item.width, item.height,
        sol.itemId, sol.width(), sol.height());

  for (Defect defect : problem_.plateDefects()[plateId_]) {
    if (sol.intersects(defect))
      defectError("On plate #%d, item #%d intersects a defect (%d,%d)x(%d,%d)",
          plateId_, sol.itemId,
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
            topError("Item #%d is mapped multiple times", item.itemId);
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
        orderingError("Item #%d is cut but item #%d is not", idb, ida);
      if (itemPositions[ida] > itemPositions[idb])
        orderingError("Item #%d is been cut before item #%d", idb, ida);
    }
  }
}

void SolutionChecker::report() {
  bool error =
       !topError.empty()
    || !orderingError.empty()
    || !wasteError.empty()
    || !defectError.empty();

  if (!error)
    return;

  cout << "The solution contains violations" << endl;

  for (auto m : topError.messages()) {
    cout << m << endl;
  }
  for (auto m : orderingError.messages()) {
    cout << "Ordering violation: " << m << endl;
  }
  for (auto m : wasteError.messages()) {
    cout << "Minimum waste violation: " << m << endl;
  }
  for (auto m : defectError.messages()) {
    cout << "Defects: " << m << endl;
  }
}

bool SolutionChecker::fitsMinWaste(int a, int b) const {
  return a == b
    || a <= b - problem_.params().minWaste;
}

