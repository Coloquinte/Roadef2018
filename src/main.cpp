
#include "problem.hpp"
#include "solver.hpp"
#include "solution_checker.hpp"
#include "utils.hpp"

#include <iostream>
#include <iomanip>
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

po::options_description getOptions() {
  po::options_description desc("GCUT options");
  desc.add_options()("help,h", "Print this help");

  desc.add_options()("batch", po::value<string>(),
                     "Batch file (.csv)");

  desc.add_options()("defects", po::value<string>(),
                     "Defects file (.csv)");

  desc.add_options()("solution,o", po::value<string>(),
                     "Solution file (.csv)");

  desc.add_options()("moves", po::value<size_t>()->default_value(numeric_limits<size_t>::max()),
                     "Move limit");

  desc.add_options()("time,t", po::value<double>()->default_value(3.0),
                     "Time limit (seconds)");

  desc.add_options()("seed,s", po::value<size_t>()->default_value(0),
                     "Random seed");

  desc.add_options()("initial,i", po::value<string>(),
                     "Initial solution file (.csv)");

  desc.add_options()("verbosity,v", po::value<int>()->default_value(1),
                     "Output verbosity");

  desc.add_options()("check", "Fail and report on violation");

  return desc;
}

po::variables_map parseArguments(int argc, char **argv) {
  po::options_description desc = getOptions();

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  } catch (po::error &e) {
    cerr << "Error parsing command line arguments: ";
    cerr << e.what() << endl << endl;
    cout << desc << endl;
    exit(1);
  }

  if (vm.count("help")) {
    cout << desc << endl;
    exit(0);
  }
  if (!vm.count("batch") || vm["batch"].as<string>().empty()) {
    cout << "Missing input file" << endl;
    cout << desc << endl;
    exit(1);
  }

  return vm;
}


int main(int argc, char** argv) {
  cout << fixed << setw(4) << setprecision(4);
  cerr << fixed << setw(4) << setprecision(4);
  po::variables_map vm = parseArguments(argc, argv);

  string batchFile = vm["batch"].as<string>();
  string defectFile = vm.count("defects") ? vm["defects"].as<string>() : string();
  Problem pb = Problem::read(batchFile, defectFile);

  SolverParams params;
  params.verbosity = vm["verbosity"].as<int>();
  params.seed = vm["seed"].as<size_t>();
  params.moveLimit = vm["moves"].as<size_t>();
  params.timeLimit = vm["time"].as<double>();
  params.failOnViolation = vm.count("check");

  vector<int> initialOrder;
  if (vm.count("initial"))
    initialOrder = Solution::readOrdering(vm["initial"].as<string>());

  Solution solution = Solver::run(pb, params, initialOrder);
  if (params.verbosity >= 3)
    solution.report();
  if (params.verbosity >= 1)
    SolutionChecker::report(pb, solution);
  if (vm.count("solution"))
    solution.write(vm["solution"].as<string>());

  return 0;
}


