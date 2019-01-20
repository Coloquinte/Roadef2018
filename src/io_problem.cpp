// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#include "io_problem.hpp"

#include <fstream>
#include <sstream>
#include <cmath>

using namespace std;

IOProblem::IOProblem(string nameItems, string nameDefects, string nameParams)
: nameItems_(nameItems)
, nameDefects_(nameDefects)
, nameParams_(nameParams) {
}

Problem IOProblem::read() {
  vector<Item> items = readItems();
  vector<Defect> defects = readDefects();
  return Problem(items, defects);
}

void IOProblem::write(const Problem &pb) {
  writeParams();
  writeItems(pb.items());
  writeDefects(pb.defects());
}

void IOProblem::setPermissive(bool permissive) {
  permissive_ = permissive;
}

string IOProblem::nameItems() const {
  return nameItems_;
}

string IOProblem::nameDefects() const {
  return nameDefects_;
}

string IOProblem::nameParams() const {
  return nameParams_;
}

vector<Item> IOProblem::readItems() {
  ifstream f(nameItems().c_str());
  if (f.fail())
    throw runtime_error("Couldn't open file \"" + nameItems() + "\".");
  string line;
  getline(f, line);

  vector<Item> ret;
  while (getline(f, line)) {
    readItem(line, ret);
  }

  return ret;
}

vector<Defect> IOProblem::readDefects() {
  if (nameDefects().empty())
    return vector<Defect>();
  ifstream f(nameDefects().c_str());
  if (f.fail())
    throw runtime_error("Couldn't open file \"" + nameDefects() + "\".");
  string line;
  getline(f, line);

  vector<Defect> ret;
  while (getline(f, line)) {
    readDefect(line, ret);
  }
  return ret;
}

void IOProblem::readItem(const string &s, vector<Item> &items) {
  auto csv_fields = readCSVLine(s);
  if (csv_fields.empty()) return;
  if (csv_fields.size() != 5) throw runtime_error("An item must have 5 parameters but the following line was received: \"" + s + "\".");

  vector<int> item_fields;
  for (const string &s : csv_fields)
    item_fields.push_back(stoi(s));

  int width = item_fields[1];
  int height = item_fields[2];
  Item item;
  item.id = item_fields[0];
  item.width = min(width, height);
  item.height = max(width, height);
  item.stack = item_fields[3];
  item.sequence = item_fields[4];

  int minDim = min(item.width, item.height);
  int maxDim = max(item.width, item.height);
  int minMax = min(Params::maxXX, Params::heightPlates);
  int maxMax = max(Params::maxXX, Params::heightPlates);

  if (!permissive_) {
    if (minDim < Params::minWaste)
      throw runtime_error("One of the item's dimensions is smaller than the minimum allowed waste size. To find solutions without this item, use the --permissive option.");
    if (maxDim > maxMax)
      throw runtime_error("One of the item's dimensions is larger than the maximum cut size. To find solutions without this item, use the --permissive option.");
    if (minDim > minMax)
      throw runtime_error("Both of the item's dimensions are larger than both maximum cut size. To find solutions without this item, use the --permissive option.");
  }
  else {
    if (minDim < Params::minWaste)
      return;
    if (maxDim > maxMax)
      return;
    if (minDim > minMax)
      return;
  }

  items.push_back(item);
}

void IOProblem::readDefect(const string &s, vector<Defect> &defects) {
  auto csv_fields = readCSVLine(s);
  if (csv_fields.empty()) return;
  if (csv_fields.size() != 6) throw runtime_error("A defect must have 6 parameters but the following line was received: \"" + s + "\".");

  vector<double> defect_fields;
  for (const string &s : csv_fields)
    defect_fields.push_back(stod(s));

  Defect defect (
    floor(defect_fields[2])
  , floor(defect_fields[3])
  , ceil(defect_fields[4])
  , ceil(defect_fields[5])
  );
  defect.id = defect_fields[0];
  defect.plateId = defect_fields[1];
  defects.push_back(defect);
}

vector<string> IOProblem::readCSVLine(const string &s) {
  stringstream ss(s);
  string token;
  vector<string> ret;
  while (getline(ss, token, ';')) {
    ret.push_back(token);
  }
  return ret;
}

void IOProblem::writeParams() {
  ofstream f(nameParams().c_str());
  f << "NAME;VALUE" << endl;
  f << "nPlates;" << Params::nPlates << endl;
  f << "widthPlates;" << Params::widthPlates << endl;
  f << "heightPlates;" << Params::heightPlates << endl;
  f << "minXX;" << Params::minXX << endl;
  f << "maxXX;" << Params::maxXX << endl;
  f << "minYY;" << Params::minYY << endl;
  f << "minWaste;" << Params::minWaste << endl;
}

void IOProblem::writeItems(const vector<Item> &items) {
  ofstream f(nameItems().c_str());
  f << "ITEM_ID;WIDTH_ITEM;HEIGHT_ITEM;STACK;SEQUENCE" << endl;
  for (Item item : items) {
    f << item.id << ";";
    f << item.width<< ";";
    f << item.height << ";";
    f << item.stack << ";";
    f << item.sequence << endl;
  }
}

void IOProblem::writeDefects(const vector<Defect> &defects) {
  ofstream f(nameDefects().c_str());
  f << "DEFECT_ID;PLATE_ID;X;Y;WIDTH;HEIGHT" << endl;
  for (Defect defect : defects) {
    f << defect.id << ";";
    f << defect.plateId << ";";
    f << defect.minX() << ";";
    f << defect.minY() << ";";
    f << defect.width() << ";";
    f << defect.height() << endl;
  }
}


