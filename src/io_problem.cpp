
#include "io_problem.hpp"

#include <fstream>
#include <sstream>
#include <cmath>

using namespace std;

IOProblem::IOProblem(std::string namePrefix)
: namePrefix_(namePrefix) {
}

Problem IOProblem::read() {
  Params params = readParams();
  vector<Item> items = readItems();
  vector<Defect> defects = readDefects();
  return Problem(params, items, defects);
}

void IOProblem::write(const Problem &pb) {
  writeParams(pb.params());
  writeItems(pb.items());
  writeDefects(pb.defects());
}

Params IOProblem::readParams() {
  // TODO
  return Params();
}

std::string IOProblem::nameItems() const {
  return namePrefix_ + "_batch.csv";
}

std::string IOProblem::nameDefects() const {
  return namePrefix_ + "_defects.csv";
}

std::string IOProblem::nameParams() const {
  return namePrefix_ + "_params.csv";
}

std::vector<Item> IOProblem::readItems() {
  ifstream f(nameItems().c_str());
  string line;
  getline(f, line);

  std::vector<Item> ret;
  while (getline(f, line)) {
    readItem(line, ret);
  }

  return ret;
}

std::vector<Defect> IOProblem::readDefects() {
  std::ifstream f(nameDefects().c_str());
  string line;
  getline(f, line);

  std::vector<Defect> ret;
  while (getline(f, line)) {
    readDefect(line, ret);
  }
  return ret;
}

void IOProblem::readItem(const std::string &s, std::vector<Item> &items) {
  auto csv_fields = readCSVLine(s);
  if (csv_fields.empty()) return;
  if (csv_fields.size() != 5) throw runtime_error("Un item a 5 paramètres");

  std::vector<int> item_fields;
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
  items.push_back(item);
}

void IOProblem::readDefect(const std::string &s, std::vector<Defect> &defects) {
  auto csv_fields = readCSVLine(s);
  if (csv_fields.empty()) return;
  if (csv_fields.size() != 6) throw runtime_error("Un defect a 6 paramètres");

  std::vector<double> defect_fields;
  for (const string &s : csv_fields)
    defect_fields.push_back(stod(s));

  Defect defect (
    floor(defect_fields[2])
  , floor(defect_fields[3])
  , ceil(defect_fields[4])
  , ceil(defect_fields[5])
  );
  defect.id = defect_fields[0];
  defect.plate_id = defect_fields[1];
  defects.push_back(defect);
}

std::vector<std::string> IOProblem::readCSVLine(const string &s) {
  stringstream ss(s);
  string token;
  vector<string> ret;
  while (getline(ss, token, ';')) {
    ret.push_back(token);
  }
  return ret;
}

void IOProblem::writeParams(const Params &params) {
  ofstream f(nameParams().c_str());
  f << "NAME;VALUE" << endl;
  f << "nPlates;" << params.nPlates << endl;
  f << "widthPlates;" << params.widthPlates << endl;
  f << "heightPlates;" << params.heightPlates << endl;
  f << "minXX;" << params.minXX<< endl;
  f << "maxXX;" << params.maxXX<< endl;
  f << "minYY;" << params.minYY<< endl;
  f << "minWaste;" << params.minWaste<< endl;
}

void IOProblem::writeItems(const std::vector<Item> &items) {
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

void IOProblem::writeDefects(const std::vector<Defect> &defects) {
  ofstream f(nameDefects().c_str());
  f << "DEFECT_ID;PLATE_ID;X;Y;WIDTH;HEIGHT" << endl;
  for (Defect defect : defects) {
    f << defect.id << ";";
    f << defect.plate_id << ";";
    f << defect.minX() << ";";
    f << defect.minY() << ";";
    f << defect.width() << ";";
    f << defect.height() << endl;
  }
}


