## Dependencies

- CMake >= 3.16
- C++17 compiler
- OpenCV 4
- MAVSDK

### macOS

```bash
brew install opencv mavsdk
```

## Building depthai-core

First, build and install the DepthAI library:

```bash
cd thirdParty/depthai-core
git submodule update --init --recursive
cmake -S . -B build -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=../depthai-install
cmake --build build --parallel 4
cmake --build build --target install
cd ../..
```

## Compiling

```bash
./build.sh
```

### Build Options

| Option          | Description                  |
| --------------- | ---------------------------- |
| `--no-hardware` | Disable both OAK and Pixhawk |
| `--no-oak`      | Disable OAK camera only      |
| `--no-pixhawk`  | Disable Pixhawk only         |
| `--clean`       | Clean build directory first  |
