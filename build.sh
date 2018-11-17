#!/bin/bash
OPTIONS="-g -std=c++14 -I include/ -lboost_program_options -Wl,--whole-archive -lpthread -Wl,--no-whole-archive"
g++ src/*.cpp -Og -o gcut_dbg $OPTIONS
g++ src/*.cpp -O2 -o gcut_pg -fprofile-generate -DNDEBUG $OPTIONS
./gcut_pg --batch dataset/A14_batch.csv --defects dataset/A14_defects.csv -t 60 -v 0 -j 1
g++ src/*.cpp -O3 -o gcut -fprofile-use -static -DNDEBUG $OPTIONS
