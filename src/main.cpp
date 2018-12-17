
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

  desc.add_options()("prefix,p", po::value<string>(),
                     "Instance name; will load <arg>_batch.csv and <arg>_defects.csv");

  desc.add_options()("solution,o", po::value<string>(),
                     "Solution file (.csv)");

  desc.add_options()("time,t", po::value<double>()->default_value(3.0),
                     "Time limit (seconds)");

  desc.add_options()("seed,s", po::value<size_t>()->default_value(0),
                     "Random seed");

  desc.add_options()("help,h", "Print this help");

  return desc;
}

po::options_description getAdvancedOptions() {
  po::options_description desc("GCUT advanced options");

  desc.add_options()("threads,j", po::value<size_t>()->default_value(4),
                     "Number of threads");

  desc.add_options()("batch", po::value<string>(),
                     "Batch file (.csv)");

  desc.add_options()("defects", po::value<string>(),
                     "Defects file (.csv)");

  desc.add_options()("initial", po::value<string>(),
                     "Initial solution file (.csv)");

  desc.add_options()("stats", "Simply report statistics");

  return desc;
}

po::options_description getHiddenOptions() {
  po::options_description desc;

  po::options_description dev("GCUT developer options");
  dev.add_options()("help-all", "Print the help for all command line arguments");
  dev.add_options()("verbosity,v", po::value<int>()->default_value(1),
                    "Output verbosity");
  dev.add_options()("check", "Fail and report on violation");
  dev.add_options()("moves", po::value<size_t>()->default_value(1000000000llu),
                    "Move limit");
  dev.add_options()("init-runs", po::value<size_t>()->default_value(5000llu),
                    "Initialization runs");
  dev.add_options()("permissive", "Tolerate infeasible problems");

  po::options_description pack("GCUT packing options");
  pack.add_options()("exact-row-packings", "Solve 3-cuts packings exactly");
  pack.add_options()("exact-cut-packings", "Solve 2-cuts packings exactly");
  pack.add_options()("exact-plate-packings", "Solve 1-cuts packings exactly");

  pack.add_options()("diagnose-row-packings", "Diagnose 3-cut packings suboptimalities");
  pack.add_options()("diagnose-cut-packings", "Diagnose 2-cut packings suboptimalities");
  pack.add_options()("diagnose-plate-packings", "Diagnose 1-cut packings suboptimalities");

  pack.add_options()("trace-packing-fronts", "Trace Pareto fronts in exact packing algorithms");

  po::options_description merge("GCUT merging options");
  merge.add_options()("exact-row-mergings", "Solve 3-cuts mergings exactly");
  merge.add_options()("exact-cut-mergings", "Solve 2-cuts mergings exactly");
  merge.add_options()("exact-plate-mergings", "Solve 1-cuts mergings exactly");

  merge.add_options()("diagnose-row-mergings", "Diagnose 3-cut mergings suboptimalities");
  merge.add_options()("diagnose-cut-mergings", "Diagnose 2-cut mergings suboptimalities");
  merge.add_options()("diagnose-plate-mergings", "Diagnose 1-cut mergings suboptimalities");

  merge.add_options()("trace-merging-fronts", "Trace Pareto fronts in exact merging algorithms");

  desc.add(dev).add(pack).add(merge);

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

SolverParams buildParams(const po::variables_map &vm) {
  SolverParams params;
  params.verbosity = vm["verbosity"].as<int>();
  params.seed = vm["seed"].as<size_t>();
  params.nbThreads = vm["threads"].as<size_t>();
  params.timeLimit = vm["time"].as<double>();
  params.failOnViolation = vm.count("check");
  params.initializationRuns = vm["init-runs"].as<size_t>();
  params.moveLimit = vm["moves"].as<size_t>();

  if (vm.count("exact-row-packings")) params.rowPacking = PackingOption::Exact;
  if (vm.count("diagnose-row-packings")) params.rowPacking = PackingOption::Diagnose;
  if (vm.count("exact-cut-packings")) params.cutPacking = PackingOption::Exact;
  if (vm.count("diagnose-cut-packings")) params.cutPacking = PackingOption::Diagnose;
  if (vm.count("exact-plate-packings")) params.platePacking = PackingOption::Exact;
  if (vm.count("diagnose-plate-packings")) params.platePacking = PackingOption::Diagnose;
  if (vm.count("trace-packing-fronts")) params.tracePackingFronts = true;

  if (vm.count("exact-row-mergings")) params.rowMerging = PackingOption::Exact;
  if (vm.count("diagnose-row-mergings")) params.rowMerging = PackingOption::Diagnose;
  if (vm.count("exact-cut-mergings")) params.cutMerging = PackingOption::Exact;
  if (vm.count("diagnose-cut-mergings")) params.cutMerging = PackingOption::Diagnose;
  if (vm.count("exact-plate-mergings")) params.plateMerging = PackingOption::Exact;
  if (vm.count("diagnose-plate-mergings")) params.plateMerging = PackingOption::Diagnose;
  if (vm.count("trace-merging-fronts")) params.traceMergingFronts = true;

  return params;
}

int main(int argc, char** argv) {
  cout << fixed << setprecision(2);
  cerr << fixed << setprecision(2);
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

  Problem pb = Problem::read(batchFile, defectFile, vm.count("permissive"));
  if (vm.count("stats")) {
    SolutionChecker::report(pb);
    return 0;
  }

  SolverParams params = buildParams(vm);

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


