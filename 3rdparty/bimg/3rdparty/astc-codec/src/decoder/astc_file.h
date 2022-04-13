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

#ifndef ASTC_CODEC_DECODER_ASTC_FILE_H_
#define ASTC_CODEC_DECODER_ASTC_FILE_H_

#include "src/base/optional.h"
#include "src/decoder/footprint.h"
#include "src/decoder/physical_astc_block.h"

#include <memory>
#include <string>

namespace astc_codec {

// A thin wrapper around a .astc file on disk. This class simply reads the ASTC
// header, and stores the block data in memory.
class ASTCFile {
 private:
  struct Header {
    Header(size_t width, size_t height, size_t depth, size_t block_width,
           size_t block_height, size_t block_depth)
        : width_(width),
          height_(height),
          depth_(depth),
          block_width_(block_width),
          block_height_(block_height),
          block_depth_(block_depth) {}

    size_t width_;
    size_t height_;
    size_t depth_;

    size_t block_width_;
    size_t block_height_;
    size_t block_depth_;
  };

  ASTCFile(ASTCFile::Header&& header, std::string&& blocks);

 public:
  // Load an ASTC file from memory.
  // If loading failed, nullptr is returned and an error string is populated
  // in the error parameter.
  static std::unique_ptr<ASTCFile> LoadFromMemory(const char* data,
                                                  size_t length,
                                                  std::string* error);

  // Load an ASTC file from file.
  // If loading failed, nullptr is returned and an error string is populated
  // in the error parameter.
  static std::unique_ptr<ASTCFile> LoadFile(const std::string& path,
                                            std::string* error);

  // Returns the footprint for the file, if it is considered to be a valid
  // footprint.
  base::Optional<Footprint> GetFootprint() const;

  // Returns the string of the form "NxM" where N and M are the width and height
  // of the block footprint, respectively.
  std::string GetFootprintString() const;

  // Get the raw block data for the astc file.
  const std::string& GetRawBlockData() const;

  // Returns the physical block at the associated block index.
  PhysicalASTCBlock GetBlock(size_t block_idx) const;

  size_t GetWidth() const { return header_.width_; }
  size_t GetHeight() const { return header_.height_; }
  size_t GetDepth() const { return header_.depth_; }

  size_t NumBlocks() const {
    return blocks_.size() / PhysicalASTCBlock::kSizeInBytes;
  }

 private:
  static base::Optional<ASTCFile::Header> ParseHeader(const char* header);

  const Header header_;
  const std::string blocks_;
};

}  // namespace astc_codec

#endif  // ASTC_CODEC_DECODER_ASTC_FILE_H_
