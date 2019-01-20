// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#ifndef PACKER_FRONT_HPP
#define PACKER_FRONT_HPP

#include <vector>
#include <cassert>

class PackerFront {
 public:
  struct Element {
    int begin;
    int end;
    int value;
    int previous;

    Element (int begin, int end, int value, int previous)
    : begin(begin)
    , end(end)
    , value(value)
    , previous(previous) {
    }

    bool dominates(const Element &o) const {
      return end <= o.end && value >= o.value;
    }
  };

 public:
  PackerFront() {
    front_.reserve(16);
    helper_.reserve(16);
  }

  int size() const {
    return front_.size();
  }

  void clear() {
    front_.clear();
  }

  Element operator[](int i) const {
    return front_[i];
  }

  const std::vector<Element>& elements() {
    return front_;
  }

  bool useful(Element elt) {
    for (Element o : front_) {
      if (o.dominates(elt)) return false;
    }
    return true;
  }

  void init(int coord, int value) {
    assert (front_.empty());
    insert(coord-1 /* dummy */, coord, value, -1 /* dummy */);
  }

  void insert(int begin, int end, int value, int previous) {
    insert(Element(begin, end, value, previous));
  }

  void insert(Element elt) {
    if (!useful(elt)) return;

    helper_.clear();
    for (Element o : front_) {
      if (o.end < elt.end)
        helper_.push_back(o);
    }
    helper_.push_back(elt);
    for (Element o : front_) {
      if (o.end > elt.end && o.value > elt.value)
        helper_.push_back(o);
    }

    std::swap(helper_, front_);
  }

  void checkConsistency() const;
  void report() const;

 private:
  std::vector<Element> front_;
  std::vector<Element> helper_;
};

#endif


