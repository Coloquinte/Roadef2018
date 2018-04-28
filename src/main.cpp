
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
  //double sumMapped = 0.0;
  //double sumDensity = 0.0;
  //double bestDensity = 0.0;
  //for (int i = 0; i < nRuns; ++i) {
  //  Solution solution = Solver::run(pb, i);
  //  double mapped = SolutionChecker::evalPercentMapped(pb, solution);
  //  double density = SolutionChecker::evalPercentDensity(pb, solution);
  //  sumMapped += mapped;
  //  sumDensity += density;
  //  if (mapped > 99 && density > bestDensity)
  //    bestDensity = density;
  //}
  //cout << "Average " << sumMapped / nRuns << "% mapped" << endl;
  //cout << "Average " << sumDensity / nRuns << "% density" << endl;
  //cout << "Best density is " << bestDensity << "%" << endl;
  //cout << endl;

  Solution solution = Solver::run(pb);
  solution.report(cout);
  SolutionChecker::report(pb, solution);
  double mapped = SolutionChecker::evalPercentMapped(pb, solution);
  double density = SolutionChecker::evalPercentDensity(pb, solution);
  cout << "Mapped " << mapped << "%, " << density << "% density" << endl;
  solution.write(argv[1]);

  return 0;
}


