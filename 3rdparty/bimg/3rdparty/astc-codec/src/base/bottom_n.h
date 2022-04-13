// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ASTC_CODEC_BASE_BOTTOM_N_H_
#define ASTC_CODEC_BASE_BOTTOM_N_H_

#include <algorithm>
#include <functional>
#include <vector>

namespace astc_codec {
namespace base {

// Used to aggregate the lowest N values of data supplied.
template<typename T, typename CompareFn = std::less<T>>
class BottomN {
 public:
  typedef std::vector<T> ContainerType;

  // Creates an empty BottomN with limit |max_size|.
  BottomN(size_t max_size) : max_size_(max_size) { }

  bool Empty() const { return data_.empty(); }
  size_t Size() const { return data_.size(); }

  const T& Top() const { return data_.front(); }

  void Push(const T& value) {
    if (data_.size() < max_size_ || compare_(value, Top())) {
      data_.push_back(value);
      std::push_heap(data_.begin(), data_.end(), compare_);

      if (Size() > max_size_) {
        PopTop();
      }
    }
  }

  std::vector<T> Pop() {
    const size_t len = Size();
    std::vector<T> result(len);

    for (size_t i = 0; i < len; ++i) {
      result[len - i - 1] = PopTop();
    }

    return result;
  }

 private:
  T PopTop() {
    std::pop_heap(data_.begin(), data_.end(), compare_);
    T result = data_.back();
    data_.pop_back();
    return result;
  }

  ContainerType data_;
  CompareFn compare_;

  const size_t max_size_;
};

}  // namespace base
}  // namespace astc_codec

#endif  // ASTC_CODEC_BASE_BOTTOM_N_H_
