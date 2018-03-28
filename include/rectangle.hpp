
#ifndef RECTANGLE_HPP
#define RECTANGLE_HPP

class Rectangle {
 public:
  int minX() const { return minX_; }
  int maxX() const { return maxX_; }
  int minY() const { return minY_; }
  int maxY() const { return maxY_; }

  int width() const { return maxX_ - minX_; }
  int height() const { return maxY_ - minY_; }

  Rectangle(int x, int y, int w, int h) {
    minX_ = x;
    minY_ = y;
    maxX_ = x + w;
    maxY_ = y + h;
  }

  bool contains(const Rectangle &o) const {
    return minX_ <= o.minX_
        && maxX_ <= o.maxX_
        && minY_ <= o.minY_
        && maxY_ <= o.maxY_;
  }


 private:
  int minX_;
  int maxX_;
  int minY_;
  int maxY_;
};

#endif

