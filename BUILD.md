In order to build this program, you'll need CUDA and Boost installed.

# Windows
Launch Visual Studio and open "rbn.sln". Add boost to include dirs in project properties. If your VS-CUDA toolkit integration is set up correctly,  the code will build successfully.
(Branches other than master are not guaranteed to build though).

# Linux
Build using CMake build system. CMake automatically finds CUDA and boost if they are installed. You should build the project in a custom directory as CMake produces many temporary files. I added "/build" directory to .gitignore, so you probably should use this name:

$ mkdir build && cd build

$ cmake ../rbn

$ make

If CMake doesn't find boost, try setting BOOST_ROOT environment variable.

That's it. If you have any questions or suggestions, please, feel free to contact me via email: kvkuvshinov@yandex.ru
