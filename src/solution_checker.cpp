
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
    checkDefects(plate, problem_.plateDefects()[i]);
  }
}

long long SolutionChecker::evalAreaViolation() {
  long long areaMapped = 0;
  long long areaTotal = 0;
  for (const PlateSolution &plate : solution_.plates) {
    for (const CutSolution &cut: plate.cuts) {
      for (const RowSolution &row: cut.rows) {
        for (const ItemSolution &sol: row.items) {
          areaMapped += sol.item.area();
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

    int usedOnPlate = heightPlates * plate.cuts.back().r.maxX();
    return areaPlates * nbFull + usedOnPlate;
  }
  return 0;
}

void SolutionChecker::checkPlate(const PlateSolution &plate) {
  if (plate.r.minX() != 0 || plate.r.minY() != 0)
      throw runtime_error("Plate coordinate mismatch");
  if (plate.r.maxX() != problem_.params().widthPlates
      || plate.r.maxY() != problem_.params().heightPlates)
      throw runtime_error("Plate coordinate mismatch");

  if (plate.cuts.empty()) return;

  if (plate.cuts.front().r.minX() != plate.r.minX())
      throw runtime_error("Cut coordinate mismatch");
  if (plate.cuts.back().r.maxX() > plate.r.maxX())
      throw runtime_error("Cut coordinate mismatch");

  for (int i = 0; i+1 < (int) plate.cuts.size(); ++i) {
    if (plate.cuts[i].r.maxX() != plate.cuts[i].r.minX())
      throw runtime_error("Cut coordinate mismatch");
  }

  for (const CutSolution &cut : plate.cuts) {
    if (cut.r.minY() != plate.r.minY())
      throw runtime_error("Cut coordinate mismatch");
    if (cut.r.maxY() != plate.r.maxY())
      throw runtime_error("Cut coordinate mismatch");
    checkCut(cut);
  }
}

void SolutionChecker::checkCut(const CutSolution &cut) {

}

void SolutionChecker::checkRow(const RowSolution &row) {
}

void SolutionChecker::checkItem(const ItemSolution &row) {
}

void SolutionChecker::checkDefects(const PlateSolution &plate, const std::vector<Defect> &defects) {
}

void SolutionChecker::checkItemUnicity() {
  unordered_set<int> visitedItems;
  for (const PlateSolution &plate : solution_.plates) {
    for (const CutSolution &cut: plate.cuts) {
      for (const RowSolution &row: cut.rows) {
        for (const ItemSolution &item: row.items) {
          if (visitedItems.count(item.item.id) != 0)
            throw runtime_error("Un item est présent plusieurs fois");
          visitedItems.insert(item.item.id);
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
          itemPositions[item.item.id] = pos++;
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

