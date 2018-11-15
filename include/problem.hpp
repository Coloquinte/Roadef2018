
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
  Problem(std::vector<Item> items, std::vector<Defect> defects);

  static Problem read(std::string nameItems, std::string nameDefects = std::string());
  void write(std::string nameItems, std::string nameDefects = std::string(), std::string nameParams = std::string()) const;

  const std::vector<Item>& items() const{ return items_; }
  const std::vector<std::vector<Item> >& stackItems() const { return stackItems_; }

  const std::vector<Defect>& defects() const { return defects_; }
  const std::vector<std::vector<Defect> >& plateDefects() const { return plateDefects_; }

  void checkConsistency() const;

 private:
  void buildSequences();
  void buildPlates();

 private:
  std::vector<Item> items_;
  std::vector<std::vector<Item> > stackItems_;

  std::vector<Defect> defects_;
  std::vector<std::vector<Defect> > plateDefects_;
};

#endif

