
#ifndef PROBLEM_HPP
#define PROBLEM_HPP

#include "item.hpp"
#include "defect.hpp"
#include "params.hpp"

#include <vector>
#include <string>

class Solution;

class Problem {
 public:
  Problem(Params params, std::vector<Item> items, std::vector<Defect> defects);

  static Problem read(std::string prefix);
  void write(std::string prefix) const;

  const std::vector<Item>& items() const{ return items_; }
  const std::vector<std::vector<Item> >& sequenceItems() const { return sequenceItems_; }

  const std::vector<Defect>& defects() const { return defects_; }
  const std::vector<std::vector<Defect> >& plateDefects() const { return plateDefects_; }

  const Params params() const { return params_; }

 private:
  void buildSequences();
  void buildPlates();

 private:
  std::vector<Item> items_;
  std::vector<std::vector<Item> > sequenceItems_;

  std::vector<Defect> defects_;
  std::vector<std::vector<Defect> > plateDefects_;

  Params params_;
};

#endif

