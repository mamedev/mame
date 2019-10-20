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

#include "src/decoder/astc_file.h"

#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>

namespace astc_codec {

namespace {
static constexpr size_t kASTCHeaderSize = 16;

// Reads a value of size T from the buffer at the current offset, then
// increments the offset.
template<typename T>
inline T ReadVal(const char* file_data, size_t& offset) {
  T x;
  memcpy(&x, &file_data[offset], sizeof(T));
  offset += sizeof(T);
  return x;
}
}  // namespace

ASTCFile::ASTCFile(Header&& header, std::string&& blocks)
    : header_(std::move(header)), blocks_(std::move(blocks)) {}

std::unique_ptr<ASTCFile> ASTCFile::LoadFromMemory(const char* data,
                                                   size_t length,
                                                   std::string* error) {
  if (length < kASTCHeaderSize) {
    *error = "Incomplete header.";
    return nullptr;
  }

  base::Optional<Header> header_opt = ParseHeader(data);
  if (!header_opt) {
    *error = "Invalid ASTC header.";
    return nullptr;
  }

  Header header = header_opt.value();

  if (header.block_width_ == 0 || header.block_height_ == 0) {
    *error = "Invalid block size.";
    return nullptr;
  }

  std::string blocks(data + kASTCHeaderSize, data + length);

  // Check that this file has the expected number of blocks.
  const size_t expected_block_count =
      ((header.width_ + header.block_width_ - 1) / header.block_width_) *
      ((header.height_ + header.block_height_ - 1) / header.block_height_);

  if (blocks.size() % PhysicalASTCBlock::kSizeInBytes != 0 ||
      blocks.size() / PhysicalASTCBlock::kSizeInBytes != expected_block_count) {
    std::stringstream ss;
    ss << "Unexpected file length " << blocks.size() << " expected "
       << kASTCHeaderSize +
              expected_block_count * PhysicalASTCBlock::kSizeInBytes
       << " bytes.";
    *error = ss.str();
    return nullptr;
  }

  return std::unique_ptr<ASTCFile>(
      new ASTCFile(std::move(header), std::move(blocks)));
}

std::unique_ptr<ASTCFile> ASTCFile::LoadFile(const std::string& path,
                                             std::string* error) {
  std::ifstream is(path, std::ios::binary);
  if (!is) {
    *error = "File not found: " + path;
    return nullptr;
  }

  char header_data[kASTCHeaderSize] = {};
  if (!is.read(header_data, kASTCHeaderSize)) {
    *error = "Failed to load ASTC header.";
    return nullptr;
  }

  base::Optional<Header> header_opt = ParseHeader(header_data);
  if (!header_opt) {
    *error = "Invalid ASTC header.";
    return nullptr;
  }

  Header header = header_opt.value();

  std::string blocks;
  {
    std::ostringstream ss;
    ss << is.rdbuf();
    blocks = ss.str();
  }

  // Check that this file has the expected number of blocks.
  const size_t expected_block_count =
      ((header.width_ + header.block_width_ - 1) / header.block_width_) *
      ((header.height_ + header.block_height_ - 1) / header.block_height_);

  if (blocks.size() % PhysicalASTCBlock::kSizeInBytes != 0 ||
      blocks.size() / PhysicalASTCBlock::kSizeInBytes != expected_block_count) {
    std::stringstream ss;
    ss << "Unexpected file length " << blocks.size() << " expected "
       << kASTCHeaderSize +
              expected_block_count * PhysicalASTCBlock::kSizeInBytes
       << " bytes.";
    *error = ss.str();
    return nullptr;
  }

  return std::unique_ptr<ASTCFile>(
      new ASTCFile(std::move(header), std::move(blocks)));
}

base::Optional<Footprint> ASTCFile::GetFootprint() const {
  return Footprint::FromDimensions(int(header_.block_width_), int(header_.block_height_));
}

std::string ASTCFile::GetFootprintString() const {
  std::stringstream footprint;
  footprint << header_.block_width_ << "x" << header_.block_height_;
  return footprint.str();
}

const std::string& ASTCFile::GetRawBlockData() const {
  return blocks_;
}

PhysicalASTCBlock ASTCFile::GetBlock(size_t block_idx) const {
  const size_t sz = PhysicalASTCBlock::kSizeInBytes;
  const size_t offset = PhysicalASTCBlock::kSizeInBytes * block_idx;
  assert(offset <= blocks_.size() - sz);
  return PhysicalASTCBlock(blocks_.substr(offset, sz));
}

base::Optional<ASTCFile::Header> ASTCFile::ParseHeader(const char* header) {
  size_t offset = 0;
  // TODO(google): Handle endianness.
  const uint32_t magic = ReadVal<uint32_t>(header, offset);
  if (magic != 0x5CA1AB13) {
    return {};
  }

  const uint32_t block_width = ReadVal<uint8_t>(header, offset);
  const uint32_t block_height = ReadVal<uint8_t>(header, offset);
  const uint32_t block_depth = ReadVal<uint8_t>(header, offset);

  uint32_t width = 0;
  width |= ReadVal<uint8_t>(header, offset);
  width |= ReadVal<uint8_t>(header, offset) << 8;
  width |= ReadVal<uint8_t>(header, offset) << 16;

  uint32_t height = 0;
  height |= ReadVal<uint8_t>(header, offset);
  height |= ReadVal<uint8_t>(header, offset) << 8;
  height |= ReadVal<uint8_t>(header, offset) << 16;

  uint32_t depth = 0;
  depth |= ReadVal<uint8_t>(header, offset);
  depth |= ReadVal<uint8_t>(header, offset) << 8;
  depth |= ReadVal<uint8_t>(header, offset) << 16;
  assert(offset == kASTCHeaderSize);

  return Header(width, height, depth, block_width, block_height, block_depth);
}

}  // namespace astc_codec
