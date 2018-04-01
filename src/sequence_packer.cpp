
#include "sequence_packer.hpp"

using namespace std;

Solution SequencePacker::run(const Problem &problem, const std::vector<Item> &sequence) {
  SequencePacker packer(problem, sequence);
  packer.run();
  return packer.solution_;
}

SequencePacker::SequencePacker(const Problem &problem, const std::vector<Item> &sequence)
: problem_(problem)
, sequence_(sequence) {
  packedItems_ = 0;
}

void SequencePacker::run() {
  Rectangle plateRect = Rectangle::FromCoordinates(0, 0, problem_.params().widthPlates, problem_.params().heightPlates);

  for (int i = 0; i < problem_.params().nPlates; ++i) {
    PlateSolution plate = packPlate(packedItems_, plateRect);
    packedItems_ += plate.nItems();
    solution_.plates.push_back(plate);
  }
}

PlateSolution SequencePacker::packPlate(int fromItem, Rectangle plate) {
  // Dynamic programming on the first-level cuts

}

CutSolution SequencePacker::packCut(int fromItem, Rectangle cut) {
  // Dynamic programming on the rows i.e. second-level cuts
  
}

RowSolution SequencePacker::packRow(int fromItem, Rectangle row) {
  // Optimal placement at fixed ordering
  int currentX = row.minX();
  RowSolution solution(row);
  for (int i = fromItem; i < nItems(); ++i) {
    // Attempt to place the item with the best possible size
    Item item = sequence_[i];
    int height = max(item.width, item.height);
    int width = min(item.width, item.height);
    // Doesn't fit vertically; try rotating
    if (!fitsMinWaste(height, row.height()))
      swap(width, height);
    // Still doesn't fit
    if (!fitsMinWaste(height, row.height()))
      break;
    
    // Not actually 100% correct due to the minWaste parameter
    // The optimal solution involves the orientation of all items
    // And we'd need dynamic programming or brute-force for that
    int newX = currentX + width;
    if (!fitsMinWaste(newX, row.maxX()))
      break;

    ItemSolution sol(currentX, row.minY(), width, height);
    sol.itemId = item.id;

    solution.items.push_back(sol);
    currentX = newX;
  }

  return solution;
}

bool SequencePacker::fitsMinWaste(int a, int b) const {
  return a == b
  || a <= b - problem_.params().minWaste;
}
