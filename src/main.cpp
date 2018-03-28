
#include "problem.hpp"

#include <iostream>

using namespace std;

int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "cut NAME" << std::endl;
    cout << "will run the optimization on NAME_batch.csv with defects NAME_defects.csv" << endl;
    exit(1);
  }

  Problem pb = Problem::read(argv[1]);

  if (argc >= 3) {
    pb.write(argv[2]);
  }

  return 0;
}

