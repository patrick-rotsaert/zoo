# zoo
TO DO...

# Building

Requirements:
- C++20 compiler
- CMake >= 3.25 if using [CMakePresets.json](CMakePresets.json)
- CMake >= 3.19 otherwise

## Linux

Package names mentioned below are for Debian and derivatives. For other distributions, please consult the relevant package repository.

```shell
sudo apt install build-essential
```

### Without vcpkg

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

### With vcpkg

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
