
#ifndef SOLUTION_CHECKER_HPP
#define SOLUTION_CHECKER_HPP

#include "solution.hpp"
#include "problem.hpp"

#include <string>
#include <memory>

class ErrorLogger {
 public:
  template<typename ... Args>
  void operator()( const std::string& format, Args ... args ) {
    std::size_t size = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1;
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    messages_.emplace_back( buf.get(), buf.get() + size - 1 );
  }

  bool empty() const {
    return messages_.empty();
  }

  const std::vector<std::string> &messages() const {
    return messages_;
  }

 private:
  std::vector<std::string> messages_;
};

class SolutionChecker {
 public:
  static void check(const Problem &problem, const Solution &solution);
  static long long evalAreaViolation(const Problem &problem, const Solution &solution);
  static long long evalAreaUsage(const Problem &problem, const Solution &solution);

 private:
  SolutionChecker(const Problem &problem, const Solution &solution);
  void check();
  long long evalAreaViolation();
  long long evalAreaUsage();

  void checkPlate(const PlateSolution &plate);
  void checkCut(const CutSolution &cut);
  void checkRow(const RowSolution &row);
  void checkItem(const ItemSolution &row);

  void checkPlateDivision(const PlateSolution &plate);
  void checkCutDivision(const CutSolution &cut);
  void checkRowDivision(const RowSolution &row);

  void checkItemUnicity();
  void checkSequences();

  bool fitsMinWaste(int a, int b) const;

  void report();

 private:
  const Problem &problem_;
  const Solution &solution_;

  ErrorLogger topError;
  ErrorLogger orderingError;
  ErrorLogger wasteError;
  ErrorLogger defectError;

  int plateId_;
  int cutId_;
  int rowId_;
};

#endif

