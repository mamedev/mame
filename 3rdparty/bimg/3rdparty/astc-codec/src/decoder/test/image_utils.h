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

#include <gtest/gtest.h>

#include <fstream>
#include <vector>

static constexpr size_t kMaxVectorOutput = 128;

class ImageBuffer {
 public:
  static constexpr size_t Align = 4;

  void Allocate(size_t width, size_t height, size_t bytes_per_pixel) {
    width_ = width;
    height_ = height;
    bytes_per_pixel_ = bytes_per_pixel;
    stride_ = AlignBytes(width * bytes_per_pixel);
    data_.resize(stride_ * height);
  }

  uint8_t* operator()(size_t x, size_t y) {
    assert(x < width_ && y < height_);
    return &data_[y * Stride() + x * bytes_per_pixel_];
  }

  size_t Stride() const { return stride_; }
  size_t BytesPerPixel() const { return bytes_per_pixel_; }

  std::vector<uint8_t>& Data() { return data_; }
  const std::vector<uint8_t>& Data() const { return data_; }
  size_t DataSize() const { return data_.size(); }

 private:
  size_t AlignBytes(size_t bytes) const {
    return (bytes + (Align - 1)) / Align * Align;
  }

  size_t width_ = 0;
  size_t height_ = 0;
  size_t stride_ = 0;
  size_t bytes_per_pixel_ = 0;
  std::vector<uint8_t> data_;
};

namespace std {
static void PrintTo(const vector<uint8_t>& vec, ostream* os) {
  ios::fmtflags origFlags(os->flags());

  *os << '{';
  size_t count = 0;
  for (vector<uint8_t>::const_iterator it = vec.begin(); it != vec.end();
       ++it, ++count) {
    if (count > 0) {
      *os << ", ";
    }

    if (count == kMaxVectorOutput) {
      *os << "... ";
      break;
    }

    if ((count % 16) == 0) {
      *os << "\n";
    }

    if (*it == 0) {
      *os << "    ";
    } else {
      *os << "0x" << std::hex << std::uppercase << std::setw(2)
          << std::setfill('0') << int(*it) << std::dec;
    }
  }

  *os << '}';

  os->flags(origFlags);
}
}  // namespace std

static inline std::string LoadFile(const std::string& path) {
  std::ifstream is(path, std::ios::binary);
  EXPECT_TRUE(is) << "Failed to load file " << path;
  if (!is) {
    return "";
  }

  std::ostringstream ss;
  ss << is.rdbuf();
  return ss.str();
}

static inline std::string LoadASTCFile(const std::string& basename) {
  const std::string filename =
      std::string("src/decoder/testdata/") + basename + ".astc";

  std::string result = LoadFile(filename);
  // Don't parse the header here, we already know what kind of astc encoding it
  // is.
  if (result.size() < 16) {
    return "";
  } else {
    return result.substr(16);
  }
}

static inline void LoadGoldenBmp(const std::string& path, ImageBuffer* result) {
  constexpr size_t kBmpHeaderSize = 54;

  SCOPED_TRACE(testing::Message() << "LoadGoldenBmp " << path);

  const std::string data = LoadFile(path);
  ASSERT_FALSE(data.empty()) << "Failed to open golden image: " << path;

  ASSERT_GE(data.size(), kBmpHeaderSize);
  ASSERT_EQ('B', data[0]);
  ASSERT_EQ('M', data[1]);

  uint32_t dataPos = *reinterpret_cast<const uint32_t*>(&data[0x0A]);
  uint32_t imageSize = *reinterpret_cast<const uint32_t*>(&data[0x22]);
  const uint16_t bitsPerPixel = *reinterpret_cast<const uint16_t*>(&data[0x1C]);
  int width = *reinterpret_cast<const int*>(&data[0x12]);
  int height = *reinterpret_cast<const int*>(&data[0x16]);

  SCOPED_TRACE(testing::Message()
               << "dataPos=" << dataPos << ", imageSize=" << imageSize
               << ", bitsPerPixel=" << bitsPerPixel << ", width=" << width
               << ", height=" << height);

  if (height < 0) {
    height = -height;
  }

  if (imageSize == 0) {
    imageSize = width * height * 3;
  }

  if (dataPos < kBmpHeaderSize) {
    dataPos = kBmpHeaderSize;
  }

  ASSERT_TRUE(bitsPerPixel == 24 || bitsPerPixel == 32)
      << "BMP bits per pixel mismatch, expected 24 or 32";

  result->Allocate(width, height, bitsPerPixel == 24 ? 3 : 4);
  ASSERT_LE(imageSize, result->DataSize());

  std::vector<uint8_t>& resultData = result->Data();
  const size_t stride = result->Stride();

  // Copy the data row-by-row to make sure that stride is right.
  for (size_t row = 0; row < static_cast<size_t>(height); ++row) {
    memcpy(&resultData[row * stride], &data[dataPos + row * stride],
           width * bitsPerPixel / 8);
  }

  if (bitsPerPixel == 32) {
    // Swizzle the data from ABGR to ARGB.
    for (size_t row = 0; row < static_cast<size_t>(height); ++row) {
      uint8_t* rowData = resultData.data() + row * stride;

      for (size_t i = 3; i < stride; i += 4) {
        const uint8_t b = rowData[i - 3];
        rowData[i - 3] = rowData[i - 1];
        rowData[i - 1] = b;
      }
    }
  } else {
    // Swizzle the data from BGR to RGB.
    for (size_t row = 0; row < static_cast<size_t>(height); ++row) {
      uint8_t* rowData = resultData.data() + row * stride;

      for (size_t i = 2; i < stride; i += 3) {
        const uint8_t tmp = rowData[i - 2];
        rowData[i - 2] = rowData[i];
        rowData[i] = tmp;
      }
    }
  }
}

static inline void CompareSumOfSquaredDifferences(const ImageBuffer& golden,
                                                  const ImageBuffer& image,
                                                  double threshold) {
  ASSERT_EQ(golden.DataSize(), image.DataSize());
  ASSERT_EQ(golden.Stride(), image.Stride());
  ASSERT_EQ(golden.BytesPerPixel(), image.BytesPerPixel());

  const std::vector<uint8_t>& image_data = image.Data();
  const std::vector<uint8_t>& golden_data = golden.Data();

  double sum = 0.0;
  for (size_t i = 0; i < image_data.size(); ++i) {
    const double diff = static_cast<double>(image_data[i]) - golden_data[i];
    sum += diff * diff;
  }

  EXPECT_LE(sum, threshold * image_data.size())
      << "Per pixel " << (sum / image_data.size())
      << ", expected <= " << threshold;
  if (sum > threshold * image_data.size()) {
    // Fall back to comparison which will dump first chunk of vector.
    EXPECT_EQ(golden_data, image_data);
  }
}
