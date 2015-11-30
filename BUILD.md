In order to build this program, you'll need CUDA and Boost installed.

Currently there are only VS project files, but I'll add cmake scripts later.

# Windows
Launch Visual Studio and open "rbn.sln". Add boost to include dirs in project properties. If your VS-CUDA toolkit integration is set up correctly,  the code will build successfully.
(Branches other than master are not guaranteed to build though).

# Linux
mkdir build && cd build
cmake ..
make