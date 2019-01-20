// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#include "problem.hpp"
#include "solver.hpp"
#include "solution_checker.hpp"
#include "sequence_packer.hpp"
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
  dev.add_options()("permissive", "Tolerate infeasible problems");

  po::options_description move("GCUT move options");
  move.add_options()("moves", po::value<size_t>()->default_value(1000000000llu),
                     "Move limit");
  move.add_options()("init-moves", po::value<size_t>()->default_value(5000llu),
                     "Initialization move limit");

  po::options_description pack("GCUT packing options");
  pack.add_options()("exact-row-packings", "Solve 3-cuts packings exactly");
  pack.add_options()("exact-cut-packings", "Solve 2-cuts packings exactly");
  pack.add_options()("exact-plate-packings", "Solve 1-cuts packings exactly");

  pack.add_options()("diagnose-row-packings", "Diagnose 3-cut packings suboptimalities");
  pack.add_options()("diagnose-cut-packings", "Diagnose 2-cut packings suboptimalities");
  pack.add_options()("diagnose-plate-packings", "Diagnose 1-cut packings suboptimalities");

  pack.add_options()("trace-packing-fronts", "Trace Pareto fronts in exact packing algorithms");

  po::options_description expe("GCUT experimental options");
  pack.add_options()("first-plate", po::value<int>(), "First plate to consider from the initial solution");
  pack.add_options()("last-plate" , po::value<int>(), "Last plate to consider from the initial solution");

  desc.add(dev).add(move).add(pack).add(expe);

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
  params.moveLimit = vm["moves"].as<size_t>();
  params.initializationRuns = vm["init-moves"].as<size_t>();

  if (vm.count("exact-row-packings")) params.rowPacking = PackingOption::Exact;
  if (vm.count("diagnose-row-packings")) params.rowPacking = PackingOption::Diagnose;
  if (vm.count("exact-cut-packings")) params.cutPacking = PackingOption::Exact;
  if (vm.count("diagnose-cut-packings")) params.cutPacking = PackingOption::Diagnose;
  if (vm.count("exact-plate-packings")) params.platePacking = PackingOption::Exact;
  if (vm.count("diagnose-plate-packings")) params.platePacking = PackingOption::Diagnose;
  if (vm.count("trace-packing-fronts")) params.tracePackingFronts = true;

  return params;
}

void makeInitial(Problem &pb, Solution &initial, const po::variables_map &vm, const SolverParams &params) {
  if (!vm.count("initial")) return;

  vector<int> initialOrder = Solution::readOrdering(vm["initial"].as<string>());
  vector<Item> initialSequence;
  for (int id : initialOrder) {
    initialSequence.push_back(pb.items()[id]);
  }

  initial = SequencePacker::run(pb, initialSequence, params);

  if (!vm.count("first-plate") && !vm.count("last-plate")) return;

  int firstPlate = vm.count("first-plate") ? min(vm["first-plate"].as<int>(), initial.nPlates()) : 0;
  int lastPlate = vm.count("last-plate") ? min(vm["last-plate"].as<int>() + 1, initial.nPlates()) : initial.nPlates();

  // Now reduce the problem size; keep only the items that are on those plates
  initial.plates.erase(initial.plates.begin() + lastPlate, initial.plates.end());
  initial.plates.erase(initial.plates.begin(), initial.plates.begin() + firstPlate);

  vector<Item> items;
  vector<Defect> defects;

  int itemId = 0;
  for (PlateSolution &plate : initial.plates) {
    for (CutSolution &cut: plate.cuts) {
      for (RowSolution &row: cut.rows) {
        for (ItemSolution &isol: row.items) {
          Item item = pb.items()[isol.itemId];
          item.id = itemId;
          isol.itemId = itemId;
          items.push_back(item);
          ++itemId;
        }
      }
    }
  }

  for (Defect defect : pb.defects()) {
    if (defect.plateId < firstPlate) continue;
    if (defect.plateId >= lastPlate) continue;
    defect.plateId -= firstPlate;
    defects.push_back(defect);
  }

  int stackId = 0;
  map<int, int> stackIdMapping;
  for (Item &item : items) {
    if (!stackIdMapping.count(item.stack))
      stackIdMapping.emplace(item.stack, stackId++);
    item.stack = stackIdMapping[item.stack];
  }

  pb = Problem(items, defects);
}

void run(int argc, char** argv) {
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
    return;
  }

  SolverParams params = buildParams(vm);

  Solution initial;
  makeInitial(pb, initial, vm, params);

  Solution solution = Solver::run(pb, params, initial);

  if (params.verbosity >= 3)
    solution.report();
  if (params.verbosity >= 1)
    SolutionChecker::report(pb, solution);

  if (vm.count("solution"))
    solution.write(vm["solution"].as<string>());
}

int main(int argc, char** argv) {
  try {
    run(argc, argv);
  } catch (const exception &e) {
    cerr << e.what() << endl;
    return 1;
  }
  return 0;
}


