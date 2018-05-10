
#include "problem.hpp"
#include "solver.hpp"
#include "solution_checker.hpp"
#include "utils.hpp"

#include <iostream>
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

  desc.add_options()("moves", po::value<size_t>()->default_value(10000),
                     "Number of moves to perform");

  desc.add_options()("seed", po::value<size_t>()->default_value(0),
                     "Random seed");

  desc.add_options()("verbosity,v", po::value<int>()->default_value(1),
                     "Output verbosity");

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
  po::variables_map vm = parseArguments(argc, argv);

  string batchFile = vm["batch"].as<string>();
  string defectFile = vm.count("defects") ? vm["defects"].as<string>() : string();
  Problem pb = Problem::read(batchFile, defectFile);

  Solution solution = Solver::run(pb, vm["seed"].as<size_t>(), vm["moves"].as<size_t>());
  if (vm["verbosity"].as<int>() >= 2)
    solution.report();
  if (vm["verbosity"].as<int>() >= 1)
    SolutionChecker::report(pb, solution);
  if (vm.count("solution"))
    solution.write(vm["solution"].as<string>());

  return 0;
}


