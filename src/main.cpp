
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

  desc.add_options()("prefix,p", po::value<string>(),
                     "Instance name - loads <NAME>_batch.csv and <NAME>_defects.csv; writes <NAME>_solution.csv");

  desc.add_options()("solution,o", po::value<string>(),
                     "Solution file (.csv)");

  desc.add_options()("time,t", po::value<double>()->default_value(3.0),
                     "Time limit (seconds)");

  desc.add_options()("seed,s", po::value<size_t>()->default_value(0),
                     "Random seed");

  return desc;
}

po::options_description getAdvancedOptions() {
  po::options_description desc("GCUT advanced options");

  desc.add_options()("threads,j", po::value<size_t>()->default_value(8),
                     "Number of threads");

  desc.add_options()("batch", po::value<string>(),
                     "Batch file (.csv)");

  desc.add_options()("defects", po::value<string>(),
                     "Defects file (.csv)");

  desc.add_options()("initial", po::value<string>(),
                     "Initial solution file (.csv)");

  return desc;
}

po::options_description getHiddenOptions() {
  po::options_description desc("GCUT hidden options");

  desc.add_options()("help-all", "Print the help for all command line arguments");

  desc.add_options()("verbosity,v", po::value<int>()->default_value(1),
                     "Output verbosity");

  desc.add_options()("check", "Fail and report on violation");

  desc.add_options()("moves", po::value<size_t>()->default_value(1000000000llu),
                     "Move limit");

  desc.add_options()("init-runs", po::value<size_t>()->default_value(1000llu),
                     "Initialization runs");

  return desc;
}

bool fileOptionPresent(const po::variables_map &vm, const string &option) {
  return vm.count(option) && !vm[option].as<string>().empty();
}

po::variables_map parseArguments(int argc, char **argv) {
  po::options_description desc = getOptions();
  po::options_description adv = getAdvancedOptions();
  po::options_description hid = getHiddenOptions();

  po::options_description visibleOptions;
  visibleOptions.add(desc).add(adv);

  po::options_description allOptions;
  allOptions.add(desc).add(adv).add(hid);

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, allOptions), vm);
    po::notify(vm);
  } catch (po::error &e) {
    cerr << "Error parsing command line arguments: ";
    cerr << e.what() << endl << endl;
    cout << desc << endl;
    exit(1);
  }

  if (vm.count("help")) {
    cout << visibleOptions << endl;
    exit(0);
  }
  if (vm.count("help-all")) {
    cout << allOptions << endl;
    exit(0);
  }

  bool prefixPresent = fileOptionPresent(vm, "prefix");
  bool batchPresent  = fileOptionPresent(vm, "batch");
  bool defectPresent = fileOptionPresent(vm, "defects");

  if (!prefixPresent && !batchPresent) {
    cout << "Missing input file" << endl << endl;
    cout << desc << endl;
    exit(1);
  }
  if (prefixPresent && (batchPresent || defectPresent)) {
    cout << "-prefix option cannot be used with --batch or --defects" << endl << endl;
    cout << visibleOptions << endl;
    exit(1);
  }

  return vm;
}

int main(int argc, char** argv) {
  cout << fixed << setw(4) << setprecision(4);
  cerr << fixed << setw(4) << setprecision(4);
  po::variables_map vm = parseArguments(argc, argv);

  string batchFile;
  string defectFile;
  if (fileOptionPresent(vm, "prefix")) {
    batchFile = vm["prefix"].as<string>() + "_batch.csv";
    defectFile = vm["prefix"].as<string>() + "_defects.csv";
  }
  else {
    batchFile = vm["batch"].as<string>();
    defectFile = vm.count("defects") ? vm["defects"].as<string>() : string();
  }

  Problem pb = Problem::read(batchFile, defectFile);

  SolverParams params;
  params.verbosity = vm["verbosity"].as<int>();
  params.seed = vm["seed"].as<size_t>();
  params.nbThreads = vm["threads"].as<size_t>();
  params.timeLimit = vm["time"].as<double>();
  params.failOnViolation = vm.count("check");
  params.initializationRuns = vm["init-runs"].as<size_t>();
  params.moveLimit = vm["moves"].as<size_t>();

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


