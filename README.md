# Roadef2018
Glass cutting algorithm for the [ROADEF 2018 challenge](http://www.roadef.org/challenge/2018/en/sujet.php)

## Build
Make sure you have a C++14 compiler, CMake and the Boost libraries installed. Build with

    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make

A less portable script (build.sh) makes use of PGO and LTO for better performance.
