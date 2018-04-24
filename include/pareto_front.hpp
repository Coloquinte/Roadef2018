
#ifndef PARETO_FRONT_HPP
#define PARETO_FRONT_HPP

#include <vector>
#include <cassert>

class ParetoFront {
 public:
  struct Element {
    int coord;
    int valeur;
    int previous;

    Element (int coord, int valeur, int previous)
    : coord(coord)
    , valeur(valeur)
    , previous(previous) {
    }

    bool dominates(const Element &o) const {
      return coord <= o.coord && valeur >= o.valeur;
    }
  };

 public:
  int size() const {
    return front_.size();
  }

  Element operator[](int i) {
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

  void insert(int coord, int valeur, int previous) {
    insert(Element(coord, valeur, previous));
  }

  void insert(Element elt) {
    if (!useful(elt)) return;

    helper_.clear();
    for (Element o : front_) {
      if (o.coord < elt.coord)
        helper_.push_back(o);
    }
    helper_.push_back(elt);
    for (Element o : front_) {
      if (o.coord > elt.coord && o.valeur > elt.valeur)
        helper_.push_back(o);
    }

    std::swap(helper_, front_);
  }

  void checkConsistency() const {
    for (int i = 0; i < size(); ++i) {
      assert (front_[i].previous < i);
    }
    for (int i = 0; i + 1 < size(); ++i) {
      assert (front_[i].coord < front_[i+1].coord);
      assert (front_[i].valeur < front_[i+1].valeur);
    }
  }

 private:
  std::vector<Element> front_;
  std::vector<Element> helper_;
};

#endif


