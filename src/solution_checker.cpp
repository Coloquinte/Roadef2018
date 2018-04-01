
#include "solution_checker.hpp"
#include "problem.hpp"
#include "solution.hpp"

#include <unordered_set>
#include <unordered_map>

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
, solution_(solution) {
}

void SolutionChecker::check() {
  checkItemUnicity();
  checkSequences();

  if (problem_.params().nPlates < (int) solution_.plates.size())
      throw runtime_error("Trop de plates sont présentes dans la solution");

  for (int i = 0; i < (int) solution_.plates.size(); ++i) {
    const PlateSolution &plate = solution_.plates[i];
    checkPlate(plate);
  }
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
  for (const CutSolution &cut : plate.cuts)
    checkCut(cut);
}

void SolutionChecker::checkPlateDivision(const PlateSolution &plate) {
  if (plate.minX() != 0 || plate.minY() != 0)
      throw runtime_error("Plate coordinate mismatch");
  if (plate.maxX() != problem_.params().widthPlates
      || plate.maxY() != problem_.params().heightPlates)
      throw runtime_error("Plate coordinate mismatch");

  if (plate.cuts.empty()) return;

  if (plate.cuts.front().minX() != plate.minX())
      throw runtime_error("Cut coordinate mismatch");
  if (plate.cuts.back().maxX() > plate.maxX())
      throw runtime_error("Cut coordinate mismatch");

  for (const CutSolution &cut : plate.cuts) {
    if (cut.minY() != plate.minY())
      throw runtime_error("Cut coordinate mismatch");
    if (cut.maxY() != plate.maxY())
      throw runtime_error("Cut coordinate mismatch");
  }

  for (int i = 0; i+1 < (int) plate.cuts.size(); ++i) {
    if (plate.cuts[i].maxX() != plate.cuts[i].minX())
      throw runtime_error("Cut coordinate mismatch");
  }
}

void SolutionChecker::checkCut(const CutSolution &cut) {
  checkCutDivision(cut);
  for (const RowSolution &row : cut.rows)
    checkRow(row);
}

void SolutionChecker::checkCutDivision(const CutSolution &cut) {
  if (cut.rows.empty()) return;

  for (const RowSolution &row: cut.rows) {
    if (!cut.contains(row))
      throw runtime_error("Row not contained in the cut");
    if (row.minX() != cut.minX())
      throw runtime_error("Row coordinate mismatch");
    if (row.maxX() != cut.maxX())
      throw runtime_error("Row coordinate mismatch");
  }

  for (int i = 0; i+1 < (int) cut.rows.size(); ++i) {
    if (cut.rows[i].maxY() != cut.rows[i].minY())
      throw runtime_error("Row coordinate mismatch");
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
      throw runtime_error("Item not contained in the row");
    if (!fitsMinWaste(row.minY(), item.minY()))
      throw runtime_error("Minimum waste size violation");
    if (!fitsMinWaste(item.maxY(), row.maxY()))
      throw runtime_error("Minimum waste size violation");
  }

  if (!fitsMinWaste(row.minX(), row.items.front().minX()))
    throw runtime_error("Minimum waste size violation");
  if (!fitsMinWaste(row.items.back().maxX(), row.maxX()))
    throw runtime_error("Minimum waste size violation");

  for (int i = 0; i+1 < (int) row.items.size(); ++i) {
    if (row.items[i].maxX() > row.items[i+1].minX())
      throw runtime_error("Items misordered or overlapped");
    if (!fitsMinWaste(row.items[i].maxX(), row.items[i+1].minX()))
      throw runtime_error("Minimum waste size violation");
  }
}

void SolutionChecker::checkItem(const ItemSolution &sol) {
  if (sol.itemId < 0 || sol.itemId >= (int) problem_.items().size())
      throw runtime_error("Invalid item ID");

  Item item = problem_.items()[sol.itemId];
  bool ok = (sol.width() == item.width && sol.height() == item.height)
         || (sol.width() == item.height && sol.height() == item.width);
  if (!ok)
      throw runtime_error("Item size mismatch");
}

void SolutionChecker::checkItemUnicity() {
  unordered_set<int> visitedItems;
  for (const PlateSolution &plate : solution_.plates) {
    for (const CutSolution &cut: plate.cuts) {
      for (const RowSolution &row: cut.rows) {
        for (const ItemSolution &item: row.items) {
          if (visitedItems.count(item.itemId) != 0)
            throw runtime_error("Un item est présent plusieurs fois");
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
      if (itemPositions[ida] > itemPositions[idb])
        throw runtime_error("Une séquence n'est pas dans l'ordre");
    }
  }
}

bool SolutionChecker::fitsMinWaste(int a, int b) const {
  return a == b
    || a <= b - problem_.params().minWaste;
}

