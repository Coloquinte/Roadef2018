#!/bin/bash
OPTIONS="-g -std=c++14 -I ../include/ -lboost_program_options -Wl,--whole-archive -lpthread -Wl,--no-whole-archive"
mkdir -p release
cd release
g++ ../src/*.cpp -Og -o challengeSG_dbg $OPTIONS
g++ ../src/*.cpp -O2 -o challengeSG_pg -fprofile-generate -DNDEBUG $OPTIONS
./challengeSG_pg -p ../dataset/A/A14 -t 60 -v 0 -j 1
g++ ../src/*.cpp -O3 -flto -o challengeSG_opt -fprofile-use -static -DNDEBUG $OPTIONS
cd ..
cp release/challengeSG_opt challengeSG
