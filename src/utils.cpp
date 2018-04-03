
#include "utils.hpp"

#include <cassert>
#include <cmath>

using namespace std;

void divideUp(int pitch, int &l) {
  l = (l + pitch - 1) / pitch;
}

void divideDown(int pitch, int &l) {
  l = l / pitch;
}

Problem downscale(const Problem &problem, int pitch) {
  Params params = problem.params();
  divideUp(pitch, params.minXX);
  divideUp(pitch, params.minYY);
  divideUp(pitch, params.minWaste);

  divideDown(pitch, params.widthPlates);
  divideDown(pitch, params.heightPlates);
  divideDown(pitch, params.maxXX);

  std::vector<Item> items = problem.items();
  for (Item &item : items) {
    divideUp(pitch, item.width);
    divideUp(pitch, item.height);
  }

  std::vector<Defect> defects = problem.defects();
  for (Defect &defect : defects) {
    divideDown(pitch, defect.minX_);
    divideDown(pitch, defect.minY_);
    divideUp(pitch, defect.maxX_);
    divideUp(pitch, defect.maxY_);
  }

  return Problem(params, items, defects);
}

class Upscaler {
 public:
  Upscaler(const Problem &problem, const Solution &solution, int pitch)
  : problem_(problem)
  , solution_(solution)
  , pitch_(pitch) {
  }

  Solution run() {
    upscaled_ = solution_;
    for (PlateSolution &plate: upscaled_.plates) {
      upscale(plate);
      for (CutSolution &cut: plate.cuts) {
        upscale(cut);
        for (RowSolution &row: cut.rows) {
          upscale(row);
          for (ItemSolution &item: row.items) {
            upscale(item);
          }
        }
      }
    }

    return upscaled_;
  }

 private:

  void upscale(Rectangle &r) {
    // Upscale
    r.minX_ *= pitch_;
    r.minY_ *= pitch_;
    r.maxX_ *= pitch_;
    r.maxY_ *= pitch_;

    // Then round
    int maxX = problem_.params().widthPlates;
    int maxY = problem_.params().heightPlates;
    r.maxX_ = min(r.maxX_, maxX);
    r.maxY_ = min(r.maxY_, maxY);
  }

  void upscale(ItemSolution &item) {
    int w = item.width();
    int h = item.height();

    // Determine the orientation
    Item orig = problem_.items()[item.itemId];
    int upW, upH;
    if (orig.width <= w * pitch_ && orig.height <= h * pitch_) {
      upW = orig.width;
      upH = orig.height;
    }
    else {
      upW = orig.height;
      upH = orig.width;
    }

    item.minX_ *= pitch_;
    item.minY_ *= pitch_;
    item.maxX_ = item.minX_ + upW;
    item.maxY_ = item.minY_ + upH;
  }

 private:
  const Problem &problem_;
  const Solution &solution_;
  Solution upscaled_;
  int pitch_;
};

Solution upscale(const Problem &problem, const Solution &solution, int pitch) {
  Upscaler upscaler(problem, solution, pitch);
  return upscaler.run();
}

