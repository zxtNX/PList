@echo off
IF NOT EXIST build (mkdir build)
cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cd ..
pause