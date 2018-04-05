
#include "utils.hpp"

#include <cassert>
#include <cmath>

using namespace std;

class DownScaler {
 public:
  DownScaler(const Problem &problem, int pitch)
  : problem_(problem)
  , pitch_(pitch) {
  }

  Problem run() {
    downscaleParams();
    downscaleItems();
    downscaleDefects();
    return Problem(params_, items_, defects_);
  }

 private:
  void downscaleParams() {
    params_ = problem_.params();

    divideUp(params_.minXX);
    divideUp(params_.minYY);
    divideUp(params_.minWaste);

    divideDown(params_.widthPlates);
    divideDown(params_.heightPlates);
    divideDown(params_.maxXX);
  }

  void downscaleItems() {
    items_ = problem_.items();
    for (Item &item : items_) {
      downscale(item);
    }
  }

  void downscaleDefects() {
    defects_ = problem_.defects();
    for (Defect &defect : defects_) {
      downscale(defect);
    }
  }

  void downscale(Rectangle &r) {
    divideDown(r.minX_);
    divideDown(r.minY_);
    divideUp(r.maxX_);
    divideUp(r.maxY_);
  }

  void downscale(Item &item) {
    divideUp(item.width);
    divideUp(item.height);

    // Fix the case where we can't fit the item anymore
    int w = params_.widthPlates;
    int h = params_.heightPlates;
    if ( (item.width > w || item.height > h)
      && (item.width > h || item.height > w)) {
      item.width = w;
      item.height = h;
    }
  }

  void divideUp(int &l) {
    l = (l + pitch_ - 1) / pitch_;
  }

  void divideDown(int &l) {
    l = l / pitch_;
  }

 private:
  const Problem &problem_;
  Params params_;
  std::vector<Item> items_;
  std::vector<Defect> defects_;
  int pitch_;
};

Problem downscale(const Problem &problem, int pitch) {
  DownScaler downscaler(problem, pitch);
  return downscaler.run();
}

class Upscaler {
 public:
  Upscaler(const Problem &problem, const Solution &solution, int pitch)
  : problem_(problem)
  , solution_(solution)
  , pitch_(pitch) {
  }

  Solution run() {
    // TODO: handle limits
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

  void upscale(PlateSolution &plate) {
    plate.maxX_ = problem_.params().widthPlates;
    plate.maxY_ = problem_.params().heightPlates;
  }

  void upscale(CutSolution &cut) {
    cut.minX_ *= pitch_;
    cut.maxX_ *= pitch_;
    cut.maxY_ = problem_.params().heightPlates;
  }

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

