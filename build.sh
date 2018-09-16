#!/bin/bash
g++ src/* -Og -o gcut_dbg -g -I include/ -lboost_program_options
g++ src/* -O2 -o gcut_pg -fprofile-generate -g -DNDEBUG -I include/ -lboost_program_options
#g++ src/* -O2 -o gcut_prof -pg -g -DNDEBUG -I include/ -lboost_program_options
./gcut_pg --batch dataset/A14_batch.csv --defects dataset/A14_defects.csv -t 60 -v 0
g++ src/* -O3 -o gcut -fprofile-use -g -static -DNDEBUG -I include/ -lboost_program_options
