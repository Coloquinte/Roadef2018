
#include "problem.hpp"
#include "io_problem.hpp"

using namespace std;

Problem::Problem(Params params, std::vector<Item> items, std::vector<Defect> defects)
: items_(items)
, defects_(defects)
, params_(params)
{
  // Build the sequences

  // Build the plates
}

Problem Problem::read(std::string prefix) {
  IOProblem io(prefix);
  return io.read();
}

void Problem::write(std::string prefix) const {
  IOProblem io(prefix);
  io.write(*this);
}

