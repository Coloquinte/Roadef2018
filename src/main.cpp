
#include "problem.hpp"
#include "solver.hpp"
#include "solution_checker.hpp"
#include "utils.hpp"

#include <iostream>

using namespace std;

int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "cut NAME" << std::endl;
    cout << "will run the optimization on NAME_batch.csv with defects NAME_defects.csv" << endl;
    exit(1);
  }

  Problem pb = Problem::read(argv[1]);

  if (argc >= 3) {
    pb.write(argv[2]);
  }

  //int nRuns = 100;
  //for (int i = 0; i < nRuns; ++i) {
  //  Solution solution = Solver::run(pb, i);
  //  double mapped = SolutionChecker::evalPercentMapped(pb, solution);
  //  double density = SolutionChecker::evalPercentDensity(pb, solution);
  //  cout << "Mapped " << mapped << "%, " << density << "% density" << endl;
  //}

  Solution solution = Solver::run(pb);
  solution.write(cout);
  SolutionChecker::report(pb, solution);
  double mapped = SolutionChecker::evalPercentMapped(pb, solution);
  double density = SolutionChecker::evalPercentDensity(pb, solution);
  cout << "Mapped " << mapped << "%, " << density << "% density" << endl;

  return 0;
}


