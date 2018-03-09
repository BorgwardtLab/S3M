# S3M &mdash; Statistically Significant Shapelet Mining

This is the implementation of the method described in the ISMB 2018 paper "Statistically Significant Shapelet Mining in Biomedical Time Series".

# Building

The tool is written in C++ and requires the following dependencies:

* A C++ compiler with support for C++11; please refer to [this table](https://en.cppreference.com/w/cpp/compiler_support)
  in order to find a suitable compiler
* The [`CMake`](https://cmake.org) build system for building the tool
* The [`Boost`](http://www.boost.org) libraries&nbsp;(technically, only a subset
  of them) for statistical calculations

How to install these dependencies depends on your flavour of operating system.
For most Linux distributions, a package manager is available, so that variants
of the following commands should be sufficient:

    $ apt-get install cmake libboost-all-dev build-essential # For Ubuntu
    $ pacman -S boost cmake gcc                              # For Arch Linux

For Mac OS X, we recommend installing the [`Homebrew` package manager](https://brew.sh). Afterwards,
the required dependencies can be installed via:

    $ brew install boost cmake llvm

After cloning the repository and navigating to the repository folder in
a terminal, please follow these steps to build the software:

    # This assumes that you have cloned the repository to some folder and
    # navigated to the folder via your terminal application.
    $ mkdir build
    $ cd build
    $ cmake ../
    -- The CXX compiler identification is AppleClang 9.0.0.9000039
    -- Check for working CXX compiler: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++
    -- Check for working CXX compiler: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ -- works
    -- Detecting CXX compiler ABI info
    -- Detecting CXX compiler ABI info - done
    -- Detecting CXX compile features
    -- Detecting CXX compile features - done
    -- CMAKE_ROOT:           /usr/local/Cellar/cmake/3.10.2/share/cmake
    -- CMAKE_SYSTEM_VERSION: 17.4.0
    [...]
    $ make

The `build` directory should now contain the `s3m` executable.

# Contributors

S3M is developed and maintained by members of the [Machine Learning and
Computational Biology Lab](https://www.bsse.ethz.ch/mlcb) of [Prof. Dr.
Karsten Borgwardt](https://www.bsse.ethz.ch/mlcb/karsten.html):

- Christian Bock ([GitHub](https://github.com/chrisby))
- Thomas Gumbsch ([GitHub](https://github.com/tgumbsch))
- Bastian Rieck ([GitHub](https://github.com/Submanifold))
