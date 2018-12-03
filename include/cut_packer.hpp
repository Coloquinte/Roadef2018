
#ifndef CUT_PACKER_HPP
#define CUT_PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "packer.hpp"
#include "row_packer.hpp"
#include "packer_front.hpp"

class CutPacker : Packer {
 public:
  struct CutDescription {
    int nItems;
    int maxUsedX;
    bool tightX;

    CutDescription() {
      nItems = 0;
      maxUsedX = 0;
      tightX = true;
    }
  };

 public:
  CutPacker(const std::vector<Item> &sequence, SolverParams options);
  CutSolution run(Rectangle cut, int start, const std::vector<Defect> &defects);
  CutDescription count(Rectangle cut, int start, const std::vector<Defect> &defects);

 private:
  CutSolution runApproximate();
  CutSolution runExact();
  CutSolution runDiagnostic();
  CutDescription countApproximate();
  CutDescription countExact();
  CutDescription countDiagnostic();

  void setup(Rectangle cut, int start, const std::vector<Defect> &defects);
  void commonApproximate();
  void commonExact();
  void propagate(int previousFront, int beginCoord);
  void propagateBreakpoints(int after);

  CutSolution backtrack();
  CutDescription countBacktrack();

  RowPacker::RowDescription countRow(int start, int minY, int maxY);
  RowSolution packRow(int start, int minY, int maxY);

  bool isAdmissibleCutLine(int y) const;

 private:
  PackerFront front_;
  RowPacker rowPacker_;
  std::vector<int> slices_;
};

#endif

