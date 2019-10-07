# astc-codec

astc-codec is a software ASTC decoder implementation, which supports the ASTC
LDR profile.

Example usage:

```
#include <astc-codec/astc-codec.h>

// ...

std::vector<uint8_t> astc = LoadMyASTCData();
const size_t width = 640;
const size_t height = 480;

std::vector<uint8_t> result;
result.resize(width * height * 4);

bool success = astc_codec::ASTCDecompressToRGBA(
    astc.data(), astc.size(), width, height, astc_codec::FootprintType::k4x4,
    result.data(), result.size(), /* stride */ width * 4);
```

## Building

### With bazel

Install [Bazel](https://bazel.build/), and then run:

```
bazel build :astc_codec -c opt
```

astc-codec has been tested on Mac and Linux.

### Run Tests

```
bazel test //...
```

### With CMake

Install [CMake](https://cmake.org/), and the run:

```
mkdir build && cd build && cmake .. && make
```

Or open the project in your favorite IDE and import CMakeLists.txt.

### Run Tests

In the build directory, execute:

```
ctest
```


## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for important contributing requirements.

## License

astc-codec project is licensed under the Apache License Version 2.0. You can
find a copy of it in [LICENSE](LICENSE).

This is not an officially supported Google product.
