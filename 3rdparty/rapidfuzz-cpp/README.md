  <h1 align="center">
<img src="https://raw.githubusercontent.com/maxbachmann/rapidfuzz/master/docs/img/RapidFuzz.svg?sanitize=true" alt="RapidFuzz" width="400">
</h1>
<h4 align="center">Rapid fuzzy string matching in C++ using the Levenshtein Distance</h4>

<p align="center">
  <a href="https://github.com/maxbachmann/rapidfuzz/actions">
    <img src="https://github.com/maxbachmann/rapidfuzz/workflows/Build/badge.svg"
         alt="Continous Integration">
  </a>
  <a href="https://maxbachmann.github.io/rapidfuzz">
    <img src="https://img.shields.io/badge/-documentation-blue"
         alt="Documentation">
  </a>
  <a href="https://github.com/maxbachmann/rapidfuzz/blob/dev/LICENSE">
    <img src="https://img.shields.io/github/license/maxbachmann/rapidfuzz"
         alt="GitHub license">
  </a>
</p>

<p align="center">
  <a href="#description">Description</a> •
  <a href="#installation">Installation</a> •
  <a href="#usage">Usage</a> •
  <a href="#license">License</a>
</p>

---
## Description
RapidFuzz is a fast string matching library for Python and C++, which is using the string similarity calculations from [FuzzyWuzzy](https://github.com/seatgeek/fuzzywuzzy). However there are two aspects that set RapidFuzz apart from FuzzyWuzzy:
1) It is MIT licensed so it can be used whichever License you might want to choose for your project, while you're forced to adopt the GPL license when using FuzzyWuzzy
2) It is mostly written in C++ and on top of this comes with a lot of Algorithmic improvements to make string matching even faster, while still providing the same results. More details on these performance improvements in form of benchmarks can be found [here](https://github.com/maxbachmann/rapidfuzz/blob/master/Benchmarks.md)

The Library is splitted across multiple repositories for the different supported programming languages:
- The C++ version is versioned in this repository
- The Python version can be found at [maxbachmann/rapidfuzz](https://github.com/maxbachmann/rapidfuzz)


## CMake Integration

There are severals ways to integrate `rapidfuzz` in your CMake project.

### By Installing it
```bash
git clone https://github.com/maxbachmann/rapidfuzz-cpp.git rapidfuzz-cpp
cd rapidfuzz-cpp
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
cmake --build . --target install
```

Then in your CMakeLists.txt:
```cmake
find_package(rapidfuzz REQUIRED)
add_executable(foo main.cpp)
target_link_libraries(foo rapidfuzz::rapidfuzz)
```

### Add this repository as a submodule
```bash
git submodule add https://github.com/maxbachmann/rapidfuzz-cpp.git 3rdparty/RapidFuzz
```
Then you can either:

1. include it as a subdirectory
    ```cmake
    add_subdirectory(3rdparty/RapidFuzz)
    add_executable(foo main.cpp)
    target_link_libraries(foo rapidfuzz::rapidfuzz)
    ```
2. build it at configure time with `FetchContent`
    ```cmake
    FetchContent_Declare(
      rapidfuzz
      SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/RapidFuzz
      PREFIX ${CMAKE_CURRENT_BINARY_DIR}/rapidfuzz
      CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> "${CMAKE_OPT_ARGS}"
    )
    FetchContent_MakeAvailable(rapidfuzz)
    add_executable(foo main.cpp)
    target_link_libraries(foo PRIVATE rapidfuzz::rapidfuzz)
    ```
### Download it at configure time

If you don't want to add `rapidfuzz-cpp` as a submodule, you can also download it with `FetchContent`:
```cmake
FetchContent_Declare(rapidfuzz
  GIT_REPOSITORY https://github.com/maxbachmann/rapidfuzz-cpp.git
  GIT_TAG master)
FetchContent_MakeAvailable(rapidfuzz)
add_executable(foo main.cpp)
target_link_libraries(foo PRIVATE rapidfuzz::rapidfuzz)
```
It will be downloaded each time you run CMake in a blank folder.

## CMake option

There are CMake options available:

1. `BUILD_TESTS` : to build test (default OFF and requires [Catch2](https://github.com/catchorg/Catch2))

2. `BUILD_BENCHMARKS` : to build benchmarks (default OFF and requires [Google Benchmark](https://github.com/google/benchmark))

## Usage
```cpp
#include "rapidfuzz/fuzz.hpp"
#include "rapidfuzz/utils.hpp"
```

### Simple Ratio
```cpp
using rapidfuzz::fuzz::ratio;

// score is 96.55171966552734
double score = rapidfuzz::fuzz::ratio("this is a test", "this is a test!");
```

### Partial Ratio
```cpp
// score is 100
double score = rapidfuzz::fuzz::partial_ratio("this is a test", "this is a test!");
```

### Token Sort Ratio
```cpp
// score is 90.90908813476562
double score = rapidfuzz::fuzz::ratio("fuzzy wuzzy was a bear", "wuzzy fuzzy was a bear")

// score is 100
double score = rapidfuzz::fuzz::token_sort_ratio("fuzzy wuzzy was a bear", "wuzzy fuzzy was a bear")
```

### Token Set Ratio
```cpp
// score is 83.8709716796875
double score = rapidfuzz::fuzz::token_sort_ratio("fuzzy was a bear", "fuzzy fuzzy was a bear")

// score is 100
double score = rapidfuzz::fuzz::token_set_ratio("fuzzy was a bear", "fuzzy fuzzy was a bear")
```

### Process
In the Python implementation there is a module process, which is used to compare e.g. a string to a list of strings.
In Python this both saves the time to implement those features yourself and can be a lot more efficient than repeated type
conversions between Python and C++. Implementing a similar function in C++ using templates is not easily possible and probably slower than implementing them on your own. Thats why this section describes how users can implement those features with a couple lines of code using the C++ library.

### extract

The following function compares a query string to all strings in a list of choices. It returns all
elements with a similarity over score_cutoff. Generally make use of the cached implementations when comparing
a string to multiple strings.


```cpp
template <typename Sentence1,
          typename Iterable, typename Sentence2 = typename Iterable::value_type>
std::vector<std::pair<Sentence2, percent>>
extract(const Sentence1& query, const Iterable& choices, const percent score_cutoff = 0.0)
{
  std::vector<std::pair<Sentence2, percent>> results;

  auto scorer = rapidfuzz::fuzz::CachedRatio<Sentence1>(query);

  for (const auto& choice : choices) {
    double score = scorer.ratio(choice, score_cutoff);

    if (score >= score_cutoff) {
      results.emplace_back(choice, score);
    }
  }

  return results;
}
```

### extractOne

The following function compares a query string to all strings in a list of choices.

```cpp
template <typename Sentence1,
          typename Iterable, typename Sentence2 = typename Iterable::value_type>
std::optional<std::pair<Sentence2, percent>>
extractOne(const Sentence1& query, const Iterable& choices, const percent score_cutoff = 0.0)
{
  bool match_found = false;
  double best_score = score_cutoff;
  Sentence2 best_match;

  auto scorer = rapidfuzz::fuzz::CachedRatio<Sentence1>(query);

  for (const auto& choice : choices) {
    double score = scorer.ratio(choice, best_score);

    if (score >= best_score) {
      match_found = true;
      best_score = score;
      best_match = choice;
    }
  }

  if (!match_found) {
    return nullopt;
  }

  return std::make_pair(best_match, best_score);
}
```

### multithreading

It is very simple to use those scorers e.g. with open OpenMP to achieve better performance.

```cpp
template <typename Sentence1,
          typename Iterable, typename Sentence2 = typename Iterable::value_type>
std::vector<std::pair<Sentence2, percent>>
extract(const Sentence1& query, const Iterable& choices, const percent score_cutoff = 0.0)
{
  std::vector<std::pair<Sentence2, percent>> results(choices.size());

  auto scorer = rapidfuzz::fuzz::CachedRatio<Sentence1>(query);

  #pragma omp parallel for
  for (std::size_t i = 0; i < choices.size(); ++i) {
    double score = scorer.ratio(choices[i], score_cutoff);
    results[i] = std::make_pair(choices[i], score);
  }

  return results;
}
```

Note that the scorers do not take ownership of the data you pass them.
Internally they fully operate on string_views. This is especially important for the cached implementations,
since it might be tempting to pass the first string to the constructor of the cached scorer and delete it afterwards.


## License
RapidFuzz is licensed under the MIT license since I believe that everyone should be able to use it without being forced to adopt the GPL license. Thats why the library is based on an older version of fuzzywuzzy that was MIT licensed as well.
This old version of fuzzywuzzy can be found [here](https://github.com/seatgeek/fuzzywuzzy/tree/4bf28161f7005f3aa9d4d931455ac55126918df7).
