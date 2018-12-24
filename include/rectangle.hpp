
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
  int area() const { return width() * height(); }


  Rectangle() {
    minX_ = 0;
    minY_ = 0;
    maxX_ = 0;
    maxY_ = 0;
  }

  static Rectangle FromDimensions(int x, int y, int w, int h) {
    return Rectangle(
      x,
      y,
      x + w,
      y + h
    );
  }

  static Rectangle FromCoordinates(int minX, int minY, int maxX, int maxY) {
    return Rectangle(
      minX,
      minY,
      maxX,
      maxY
    );
  }

  bool contains(const Rectangle &o) const {
    return minX_ <= o.minX_
        && maxX_ >= o.maxX_
        && minY_ <= o.minY_
        && maxY_ >= o.maxY_;
  }

  bool containsStrictly(const Rectangle &o) const {
    return minX_ < o.minX_
        && maxX_ > o.maxX_
        && minY_ < o.minY_
        && maxY_ > o.maxY_;
  }

  bool intersects(const Rectangle &o) const {
    return minX_ <= o.maxX_
        && maxX_ >= o.minX_
        && minY_ <= o.maxY_
        && maxY_ >= o.minY_;
  }

  bool intersectsVerticalLine(int x) const {
    return minX_ <= x
        && maxX_ >= x;
  }

  bool intersectsHorizontalLine(int y) const {
    return minY_ <= y
        && maxY_ >= y;
  }

  bool operator==(const Rectangle &o) const {
    return minX_ == o.minX_
        && maxX_ == o.maxX_
        && minY_ == o.minY_
        && maxY_ == o.maxY_;
  }

  bool operator!=(const Rectangle &o) const {
    return !operator==(o);
  }

 protected:
  Rectangle(int minX, int minY, int maxX, int maxY) {
    minX_ = minX;
    minY_ = minY;
    maxX_ = maxX;
    maxY_ = maxY;
  }

 public:
  int minX_;
  int maxX_;
  int minY_;
  int maxY_;
};

#endif

