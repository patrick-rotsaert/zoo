# ZOO - C++ libraries

This repository is a collection of pet C++ projects.

- [squid](zoo/squid): Database access library.
- [spider](zoo/spider): Web services toolkit.
- [fs](zoo/fs): File access library.
- [bitcask](zoo/bitcask): A well... bitcask implementation.

## Motivation

These projects started off in separate repositories but with some common code that needed deduplication.
Moving common code to submodules or fetch content did not work for me, so a mono-repo was the simplest solution.
It is however possible to use only parts of it.

## Building

Requirements:
- C++20 compiler
- CMake >= 3.25 if using [CMakePresets.json](CMakePresets.json)
- CMake >= 3.19 otherwise
- Ninja, only required for the dist scripts.

### Windows

Building on Windows has currently only been tested with:
- Visual Studio 2019
- Vcpkg

To build using vcpkg, set the CMake variable `CMAKE_TOOLCHAIN_FILE` to the path of `vcpkg/scripts/buildsystems/vcpkg.cmake`.

Configurations in [CMakePresets.json](CMakePresets.json) for windows do this automatically.

Make sure git submodules are pulled:
```bat
git submodule update --init --recursive
```

Bootstrap vcpkg, this needs to be done once before the first CMake configure:
```bat
.\vcpkg\bootstrap-vcpkg.bat
```

Finally, if using the CMake presets, e.g.
```bat
cmake --preset vc142-x64-static-debug
```

### Linux

Package names mentioned below are for Debian and derivatives. For other distributions, please consult the relevant package repository.

```shell
sudo apt install build-essential
```

#### Without vcpkg

When building against system installed libraries, at least the following packages need to be installed:
```shell
sudo apt install libfmt-dev libspdlog-dev
```

Also, the following Boost libraries >= 1.81 need to be installed:
```shell
sudo apt install \
  libboost-dev \
  libboost-date-time-dev \
  libboost-system-dev \
  libboost-filesystem-dev \
  libboost-thread-dev \
  libboost-url-dev \
  libboost-json-dev \
  libboost-regex-dev \
  libboost-serialization-dev \
  libboost-exception-dev
```

If the default Boost version on your system is less than 1.81, then you can pull them from a ppa, e.g.
```shell
sudo add-apt-repository ppa:mhier/libboost-latest
sudo apt update
# sudo apt install libboost1.83-all-dev
```

When `ZOO_WITH_FS=ON`
```shell
sudo apt install libssh-dev
```

When `ZOO_SQUID_WITH_POSTGRESQL=ON`
```shell
sudo apt install libpq-dev
```

When `ZOO_SQUID_WITH_MYSQL=ON`
```shell
sudo apt install libmysqlclient-dev
```

When `ZOO_SQUID_WITH_SQLITE3=ON`
```shell
sudo apt install libsqlite3-dev
```

When `ZOO_TEST=ON`, i.e. when tests are enabled
```shell
sudo apt install google-mock libgmock-dev googletest libgtest-dev
```

#### With vcpkg

To build using vcpkg, set the CMake variable `CMAKE_TOOLCHAIN_FILE` to the path of `vcpkg/scripts/buildsystems/vcpkg.cmake`.

Configurations in [CMakePresets.json](CMakePresets.json) with a name ending with "-vcpkg" do this automatically.

Make sure git submodules are pulled:
```shell
git submodule update --init --recursive
```

Bootstrap vcpkg, this needs to be done once before the first CMake configure:
```shell
./vcpkg/bootstrap-vcpkg.sh
```

When using the presets [CMakePresets.json](CMakePresets.json), Nina is used as the generator.
```shell
sudo apt install ninja-build
```

Finally, if using the CMake presets, e.g.
```shell
cmake --preset linux-x64-gcc-static-release-vcpkg
```

All dependencies will be downloaded and built by vcpkg.

## Integration

{TODO}: Describe
- Subdirectory
- Fetch content
- Installed libraries

## Distribution

Creating redistributables is done with CPack, with the following generators:
- TGZ
  - Generates a .tar.gz file.
  - Available on Unix-like platforms.
- DEB
  - Generates a .deb file (Debian package).
  - Available on Debian and derivatives.
  - It is possible to build this package incombination with vcpkg, but this is not recommended.
    The package will contain the shared libraries of the dependencies.
    If these installed in the default location `/usr/bin`, they may conflict with system installed versions of those libraries.
- ZIP
  - Generates a .zip file.
  - Available on Windows.
- NSIS
  - Generates a .exe file (installer).
  - Available on Windows.

Component-aware generators (DEB and NSIS) split the packages into two components:
1. Development
    - The static libraries.
    - The header files.
    - The CMake configuration files.
2. Runtime
    - The shared libraries.

### Windows

Run [.\dist_x86.cmd](dist_x86.cmd) (32-bit) or [.\dist_amd64.cmd](dist_amd64.cmd) (64-bit) to build and package all combinations of static/shared and release/debug libaries, the header files and CMake configuration files.

### Linux

```shell
sudo apt install ninja-build
```

Run [./dist.sh](dist.sh) to build and package all combinations of static/shared and release/debug libaries, the header files and CMake configuration files.

## License

These libraries are distributed under the terms of the [Boost Software License](http://www.boost.org/LICENSE_1_0.txt).

## TODO list

- [ ] Add rpm packaging
