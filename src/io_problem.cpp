
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
    throw runtime_error("Couldn't open file \"" + nameItems() + "\"");
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
    throw runtime_error("Couldn't open file \"" + nameDefects() + "\"");
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
  if (csv_fields.size() != 5) throw runtime_error("Un item a 5 paramètres");

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
  items.push_back(item);
}

void IOProblem::readDefect(const string &s, vector<Defect> &defects) {
  auto csv_fields = readCSVLine(s);
  if (csv_fields.empty()) return;
  if (csv_fields.size() != 6) throw runtime_error("Un defect a 6 paramètres");

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
  defect.plate_id = defect_fields[1];
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
    f << defect.plate_id << ";";
    f << defect.minX() << ";";
    f << defect.minY() << ";";
    f << defect.width() << ";";
    f << defect.height() << endl;
  }
}


