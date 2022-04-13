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

#include "src/decoder/endpoint_codec.h"
#include "src/decoder/quantization.h"

#include <algorithm>
#include <array>
#include <numeric>
#include <utility>

namespace astc_codec {

namespace {

template<typename T>
T Clamp(T value, T min, T max) {
  return value < min ? min : (value > max ? max : value);
}

// This is the 'blue_contract' function defined in Section C.2.14 of the ASTC
// specification.
template<typename ArrayType>
void BlueContract(ArrayType* const cptr) {
  ArrayType& c = *cptr;
  c[0] = (c[0] + c[2]) >> 1;
  c[1] = (c[1] + c[2]) >> 1;
}

// Returns the inverse of values in BlueContract, subjected to the constraint
// that the new values are stored in the range [0, 255].
template<typename ArrayType>
ArrayType InvertBlueContract(const ArrayType& c) {
  ArrayType result = c;
  result[0] = Clamp(2 * c[0] - c[2], 0, 255);
  result[1] = Clamp(2 * c[1] - c[2], 0, 255);
  return result;
}

// This is the 'bit_transfer_signed' function defined in Section C.2.14 of the
// ASTC specification.
void BitTransferSigned(int* const a, int* const b) {
  *b >>= 1;
  *b |= *a & 0x80;
  *a >>= 1;
  *a &= 0x3F;
  if ((*a & 0x20) != 0) {
    *a -= 0x40;
  }
}

// Takes two values, |a| in the range [-32, 31], and |b| in the range [0, 255],
// and returns the two values in [0, 255] that will reconstruct |a| and |b| when
// passed to the BitTransferSigned function.
void InvertBitTransferSigned(int* const a, int* const b) {
  assert(*a >= -32); assert(*a < 32);
  assert(*b >= 0);   assert(*b < 256);

  if (*a < 0) {
    *a += 0x40;
  }
  *a <<= 1;
  *a |= (*b & 0x80);
  *b <<= 1;
  *b &= 0xff;
}

template<typename ContainerType>
void Quantize(ContainerType* const c, size_t max_value) {
  for (auto& x : *c) {
    x = QuantizeCEValueToRange(x, int(max_value));
  }
}

template<typename ArrayType>
ArrayType QuantizeColor(const ArrayType& c, size_t max_value) {
  ArrayType result = c;
  Quantize(&result, max_value);
  return result;
}

template<typename ContainerType>
void Unquantize(ContainerType* const c, size_t max_value) {
  for (auto& x : *c) {
    x = UnquantizeCEValueFromRange(x, int(max_value));
  }
}

template<typename ArrayType>
ArrayType UnquantizeColor(const ArrayType& c, size_t max_value) {
  ArrayType result = c;
  Unquantize(&result, max_value);
  return result;
}

// Returns the average of the three RGB channels.
template<typename ContainerType>
int AverageRGB(const ContainerType& c) {
  // Each channel can be in the range [0, 255], and we need to divide by three.
  // However, we want to round the error properly. Both (x + 1) / 3 and
  // (x + 2) / 3 are relatively imprecise when it comes to rounding, so instead
  // we increase the precision by multiplying our numerator by some arbitrary
  // number. Here, we choose 256 to get 8 additional bits and maintain
  // performance since it turns into a shift rather than a multiply. Our
  // denominator then becomes  3 * 256 = 768.
  return (std::accumulate(c.begin(), c.begin() + 3, 0) * 256 + 384) / 768;
}

// Returns the sum of squared differences between each element of |a| and |b|,
// which are assumed to contain the same number of elements.
template<typename ContainerType>
const typename ContainerType::value_type SquaredError(
    const ContainerType& a, const ContainerType& b,
    size_t num_channels = std::tuple_size<ContainerType>::value) {
  using ValueTy = typename ContainerType::value_type;
  static_assert(std::is_signed<ValueTy>::value,
                "Value type assumed to be signed to avoid branch below.");
  ValueTy result = ValueTy(0);
  for (size_t i = 0; i < num_channels; ++i) {
    ValueTy error = a[i] - b[i];
    result += error * error;
  }
  return result;
}

constexpr int MaxValuesForModes(ColorEndpointMode mode_a,
                                ColorEndpointMode mode_b) {
  return (NumColorValuesForEndpointMode(mode_a) >
          NumColorValuesForEndpointMode(mode_b))
      ? NumColorValuesForEndpointMode(mode_a)
      : NumColorValuesForEndpointMode(mode_b);
}

// This function takes the two colors in |endpoint_low| and |endpoint_high| and
// encodes them into |vals| according to the ASTC spec in section C.2.14. It
// assumes that the two colors are close enough to grayscale that the encoding
// should use the ColorEndpointMode kLDRLumaBaseOffset or kLDRLumaDirect. Which
// one is chosen depends on which produces smaller error for the given
// quantization value stored in |max_value|
bool EncodeColorsLuma(const RgbaColor& endpoint_low,
                      const RgbaColor& endpoint_high,
                      int max_value, ColorEndpointMode* const astc_mode,
                      std::vector<int>* const vals) {
  assert(vals->size() ==
         size_t(NumValuesForEncodingMode(EndpointEncodingMode::kDirectLuma)));
  int avg1 = AverageRGB(endpoint_low);
  int avg2 = AverageRGB(endpoint_high);

  // For the offset mode, L1 is strictly greater than L2, so if we are using
  // it to encode the color values, we need to swap the weights and
  // endpoints so that the larger of the two is the second endpoint.
  bool needs_weight_swap = false;
  if (avg1 > avg2) {
    needs_weight_swap = true;
    std::swap(avg1, avg2);
  }
  assert(avg1 <= avg2);

  // Now, the first endpoint is based on the low-order six bits of the first
  // value, and the high order two bits of the second value. The low order
  // six bits of the second value are used as the (strictly positive) offset
  // from the first value.
  const int offset = std::min(avg2 - avg1, 0x3F);
  const int quant_off_low =
      QuantizeCEValueToRange((avg1 & 0x3F) << 2, max_value);
  const int quant_off_high =
      QuantizeCEValueToRange((avg1 & 0xC0) | offset, max_value);

  const int quant_low = QuantizeCEValueToRange(avg1, max_value);
  const int quant_high = QuantizeCEValueToRange(avg2, max_value);

  RgbaColor unquant_off_low, unquant_off_high;
  RgbaColor unquant_low, unquant_high;

  (*vals)[0] = quant_off_low;
  (*vals)[1] = quant_off_high;
  DecodeColorsForMode(
      *vals, max_value, ColorEndpointMode::kLDRLumaBaseOffset,
      &unquant_off_low, &unquant_off_high);

  (*vals)[0] = quant_low;
  (*vals)[1] = quant_high;
  DecodeColorsForMode(*vals, max_value, ColorEndpointMode::kLDRLumaDirect,
                      &unquant_low, &unquant_high);

  const auto calculate_error =
      [needs_weight_swap, &endpoint_low, &endpoint_high]
      (const RgbaColor& low, const RgbaColor& high) {
    int error = 0;
    if (needs_weight_swap) {
      error += SquaredError(low, endpoint_high);
      error += SquaredError(high, endpoint_low);
    } else {
      error += SquaredError(low, endpoint_low);
      error += SquaredError(high, endpoint_high);
    }
    return error;
  };

  const int direct_error = calculate_error(unquant_low, unquant_high);
  const int off_error = calculate_error(unquant_off_low, unquant_off_high);

  if (direct_error <= off_error) {
    (*vals)[0] = quant_low;
    (*vals)[1] = quant_high;
    *astc_mode = ColorEndpointMode::kLDRLumaDirect;
  } else {
    (*vals)[0] = quant_off_low;
    (*vals)[1] = quant_off_high;
    *astc_mode = ColorEndpointMode::kLDRLumaBaseOffset;
  }

  return needs_weight_swap;
}

class QuantizedEndpointPair {
 public:
  QuantizedEndpointPair(const RgbaColor& c_low, const RgbaColor& c_high,
                        int max_value)
      : orig_low_(c_low),
        orig_high_(c_high),
        quant_low_(QuantizeColor(c_low, max_value)),
        quant_high_(QuantizeColor(c_high, max_value)),
        unquant_low_(UnquantizeColor(quant_low_, max_value)),
        unquant_high_(UnquantizeColor(quant_high_, max_value)) { }

  const RgbaColor& QuantizedLow() const { return quant_low_; }
  const RgbaColor& QuantizedHigh() const { return quant_high_; }

  const RgbaColor& UnquantizedLow() const { return unquant_low_; }
  const RgbaColor& UnquantizedHigh() const { return unquant_high_; }

  const RgbaColor& OriginalLow() const { return orig_low_; }
  const RgbaColor& OriginalHigh() const { return orig_high_; }

 private:
  RgbaColor orig_low_;
  RgbaColor orig_high_;

  RgbaColor quant_low_;
  RgbaColor quant_high_;

  RgbaColor unquant_low_;
  RgbaColor unquant_high_;
};

class CEEncodingOption {
 public:
  CEEncodingOption() { }
  CEEncodingOption(
      int squared_error, const QuantizedEndpointPair* quantized_endpoints,
      bool swap_endpoints, bool blue_contract, bool use_offset_mode)
      : squared_error_(squared_error),
        quantized_endpoints_(quantized_endpoints),
        swap_endpoints_(swap_endpoints),
        blue_contract_(blue_contract),
        use_offset_mode_(use_offset_mode) { }

  // Returns true if able to generate valid |astc_mode| and |vals|. In some
  // instances, such as if the endpoints reprsent a base/offset pair, we may not
  // be able to guarantee blue-contract encoding due to how the base/offset pair
  // are represented and the specifics of the decoding procedure. Similarly,
  // some direct RGBA encodings also may not be able to emit blue-contract modes
  // due to an unlucky combination of channels. In these instances, this
  // function will return false, and all pointers will remain unmodified.
  bool Pack(bool with_alpha, ColorEndpointMode* const astc_mode,
            std::vector<int>* const vals, bool* const needs_weight_swap) const {
    auto unquantized_low = quantized_endpoints_->UnquantizedLow();
    auto unquantized_high = quantized_endpoints_->UnquantizedHigh();

    // In offset mode, we do BitTransferSigned before analyzing the values
    // of the endpoints in order to determine whether or not we're going to
    // be using blue-contract mode.
    if (use_offset_mode_) {
      for (size_t i = 0; i < std::tuple_size<RgbaColor>::value; ++i) {
        BitTransferSigned(&unquantized_high[i], &unquantized_low[i]);
      }
    }

    // Define variables as outlined in the ASTC spec C.2.14 for the RGB[A]
    // direct and base-offset modes
    int s0 = 0, s1 = 0;
    for (int i = 0; i < 3; ++i) {
      s0 += unquantized_low[i];
      s1 += unquantized_high[i];
    }

    // Can we guarantee a blue-contract mode if we want it? In other words,
    // if we swap which endpoint is high and which endpoint is low, can we
    // guarantee that we will hit the corresponding decode path?
    bool swap_vals = false;
    if (use_offset_mode_) {
      if (blue_contract_) {
        swap_vals = s1 >= 0;
      } else {
        swap_vals = s1 < 0;
      }

      // In offset mode, we have two different measurements that swap the
      // endpoints prior to encoding, so we don't need to swap them here.
      // If we need to swap them to guarantee a blue-contract mode, then
      // abort and wait until we get the other error measurement.
      if (swap_vals) {
        return false;
      }
    } else {
      if (blue_contract_) {
        // If we want a blue_contract path, but s1 == s0, then swapping the
        // values will have no effect.
        if (s1 == s0) {
          return false;
        }

        swap_vals = s1 > s0;
        // If we're encoding blue contract mode directly, then we implicitly
        // swap the endpoints during decode, meaning that we need to take
        // note of that here.
        *needs_weight_swap = !(*needs_weight_swap);
      } else {
        swap_vals = s1 < s0;
      }
    }

    const auto* quantized_low = &(quantized_endpoints_->QuantizedLow());
    const auto* quantized_high = &(quantized_endpoints_->QuantizedHigh());

    if (swap_vals) {
      assert(!use_offset_mode_);
      std::swap(quantized_low, quantized_high);
      *needs_weight_swap = !(*needs_weight_swap);
    }

    (*vals)[0] = quantized_low->at(0);
    (*vals)[1] = quantized_high->at(0);
    (*vals)[2] = quantized_low->at(1);
    (*vals)[3] = quantized_high->at(1);
    (*vals)[4] = quantized_low->at(2);
    (*vals)[5] = quantized_high->at(2);

    if (use_offset_mode_) {
      *astc_mode = ColorEndpointMode::kLDRRGBBaseOffset;
    } else {
      *astc_mode = ColorEndpointMode::kLDRRGBDirect;
    }

    if (with_alpha) {
      (*vals)[6] = quantized_low->at(3);
      (*vals)[7] = quantized_high->at(3);

      if (use_offset_mode_) {
        *astc_mode = ColorEndpointMode::kLDRRGBABaseOffset;
      } else {
        *astc_mode = ColorEndpointMode::kLDRRGBADirect;
      }
    }

    // If we swapped them to measure, then they need to be swapped after
    // decoding
    if (swap_endpoints_) {
      *needs_weight_swap = !(*needs_weight_swap);
    }

    return true;
  }

  bool BlueContract() const { return blue_contract_; }
  int Error() const { return squared_error_; }

 private:
  int squared_error_;
  const QuantizedEndpointPair* quantized_endpoints_;
  bool swap_endpoints_;
  bool blue_contract_;
  bool use_offset_mode_;
};

bool EncodeColorsRGBA(const RgbaColor& endpoint_low_rgba,
                      const RgbaColor& endpoint_high_rgba,
                      int max_value, bool with_alpha,
                      ColorEndpointMode* const astc_mode,
                      std::vector<int>* const vals) {
  const size_t num_channels = with_alpha ? std::tuple_size<RgbaColor>::value : 3;
  // The difficulty of encoding into this mode is determining whether or
  // not we'd like to use the 'blue contract' function to reconstruct
  // the endpoints and whether or not we'll be more accurate by using the
  // base/offset color modes instead of quantizing the color channels
  // directly. With that in mind, we:
  // 1. Generate the inverted values for blue-contract and offset modes.
  // 2. Quantize all of the different endpoints.
  // 3. Unquantize each sets and decide which one gives least error
  // 4. Encode the values correspondingly.

  // 1. Generate the inverted values for blue-contract and offset modes.
  const auto inv_bc_low = InvertBlueContract(endpoint_low_rgba);
  const auto inv_bc_high = InvertBlueContract(endpoint_high_rgba);

  RgbaColor direct_base, direct_offset;
  for (size_t i = 0; i < std::tuple_size<RgbaColor>::value; ++i) {
    direct_base[i] = endpoint_low_rgba[i];
    direct_offset[i] =
        Clamp(endpoint_high_rgba[i] - endpoint_low_rgba[i], -32, 31);
    InvertBitTransferSigned(&direct_offset[i], &direct_base[i]);
  }

  RgbaColor inv_bc_base, inv_bc_offset;
  for (size_t i = 0; i < std::tuple_size<RgbaColor>::value; ++i) {
    // Remember, for blue-contract'd offset modes, the base is compared
    // against the second endpoint and not the first.
    inv_bc_base[i] = inv_bc_high[i];
    inv_bc_offset[i] = Clamp(inv_bc_low[i] - inv_bc_high[i], -32, 31);
    InvertBitTransferSigned(&inv_bc_offset[i], &inv_bc_base[i]);
  }

  // The order of the endpoints for offset modes may determine how well they
  // approximate the given endpoints. It may be that the quantization value
  // produces more accurate values for the base than the offset or
  // vice/versa. For this reason, we need to generate quantized versions of
  // the endpoints as if they were swapped to see if we get better error
  // out of it.

  RgbaColor direct_base_swapped, direct_offset_swapped;
  for (size_t i = 0; i < std::tuple_size<RgbaColor>::value; ++i) {
    direct_base_swapped[i] = endpoint_high_rgba[i];
    direct_offset_swapped[i] =
        Clamp(endpoint_low_rgba[i] - endpoint_high_rgba[i], -32, 31);
    InvertBitTransferSigned(&direct_offset_swapped[i], &direct_base_swapped[i]);
  }

  RgbaColor inv_bc_base_swapped, inv_bc_offset_swapped;
  for (size_t i = 0; i < std::tuple_size<RgbaColor>::value; ++i) {
    // Remember, for blue-contract'd offset modes, the base is compared
    // against the second endpoint and not the first. Hence, the swapped
    // version will compare the base against the first endpoint.
    inv_bc_base_swapped[i] = inv_bc_low[i];
    inv_bc_offset_swapped[i] = Clamp(inv_bc_high[i] - inv_bc_low[i], -32, 31);
    InvertBitTransferSigned(&inv_bc_offset_swapped[i], &inv_bc_base_swapped[i]);
  }

  // 2. Quantize the endpoints directly.
  const QuantizedEndpointPair direct_quantized(
      endpoint_low_rgba, endpoint_high_rgba, max_value);
  const QuantizedEndpointPair bc_quantized(
      inv_bc_low, inv_bc_high, max_value);

  const QuantizedEndpointPair offset_quantized(
      direct_base, direct_offset, max_value);
  const QuantizedEndpointPair bc_offset_quantized(
      inv_bc_base, inv_bc_offset, max_value);

  const QuantizedEndpointPair offset_swapped_quantized(
      direct_base_swapped, direct_offset_swapped, max_value);
  const QuantizedEndpointPair bc_offset_swapped_quantized(
      inv_bc_base_swapped, inv_bc_offset_swapped, max_value);

  // 3. Unquantize each set and decide which one gives least error.
  std::array<CEEncodingOption, 6> errors;
  auto errors_itr = errors.begin();

  // 3.1 regular unquantized error
  {
    const auto rgba_low = direct_quantized.UnquantizedLow();
    const auto rgba_high = direct_quantized.UnquantizedHigh();

    const int sq_rgb_error =
        SquaredError(rgba_low, endpoint_low_rgba, num_channels) +
        SquaredError(rgba_high, endpoint_high_rgba, num_channels);

    const bool swap_endpoints = false;
    const bool blue_contract = false;
    const bool offset_mode = false;
    *(errors_itr++) = CEEncodingOption(
        sq_rgb_error, &direct_quantized,
        swap_endpoints, blue_contract, offset_mode);
  }

  // 3.2 Compute blue-contract'd error.
  {
    auto bc_low = bc_quantized.UnquantizedLow();
    auto bc_high = bc_quantized.UnquantizedHigh();
    BlueContract(&bc_low);
    BlueContract(&bc_high);

    const int sq_bc_error =
        SquaredError(bc_low, endpoint_low_rgba, num_channels) +
        SquaredError(bc_high, endpoint_high_rgba, num_channels);

    const bool swap_endpoints = false;
    const bool blue_contract = true;
    const bool offset_mode = false;
    *(errors_itr++) = CEEncodingOption(
        sq_bc_error, &bc_quantized,
        swap_endpoints, blue_contract, offset_mode);
  }

  // 3.3 Compute base/offset unquantized error.
  const auto compute_base_offset_error =
      [num_channels, &errors_itr, &endpoint_low_rgba, &endpoint_high_rgba]
      (const QuantizedEndpointPair& pair, bool swapped) {
    auto base = pair.UnquantizedLow();
    auto offset = pair.UnquantizedHigh();

    for (size_t i = 0; i < num_channels; ++i) {
      BitTransferSigned(&offset[i], &base[i]);
      offset[i] = Clamp(base[i] + offset[i], 0, 255);
    }

    int base_offset_error = 0;
    // If we swapped the endpoints going in, then without blue contract
    // we should be comparing the base against the high endpoint.
    if (swapped) {
      base_offset_error =
          SquaredError(base, endpoint_high_rgba, num_channels) +
          SquaredError(offset, endpoint_low_rgba, num_channels);
    } else {
      base_offset_error =
          SquaredError(base, endpoint_low_rgba, num_channels) +
          SquaredError(offset, endpoint_high_rgba, num_channels);
    }

    const bool blue_contract = false;
    const bool offset_mode = true;
    *(errors_itr++) = CEEncodingOption(
        base_offset_error, &pair, swapped, blue_contract, offset_mode);
  };

  compute_base_offset_error(offset_quantized, false);

  // 3.4 Compute base/offset blue-contract error.
  const auto compute_base_offset_blue_contract_error =
      [num_channels, &errors_itr, &endpoint_low_rgba, &endpoint_high_rgba]
      (const QuantizedEndpointPair& pair, bool swapped) {
    auto base = pair.UnquantizedLow();
    auto offset = pair.UnquantizedHigh();

    for (size_t i = 0; i < num_channels; ++i) {
      BitTransferSigned(&offset[i], &base[i]);
      offset[i] = Clamp(base[i] + offset[i], 0, 255);
    }

    BlueContract(&base);
    BlueContract(&offset);

    int sq_bc_error = 0;
    // Remember, for blue-contract'd offset modes, the base is compared
    // against the second endpoint and not the first. So, we compare
    // against the first if we swapped the endpoints going in.
    if (swapped) {
      sq_bc_error =
          SquaredError(base, endpoint_low_rgba, num_channels) +
          SquaredError(offset, endpoint_high_rgba, num_channels);
    } else {
      sq_bc_error =
          SquaredError(base, endpoint_high_rgba, num_channels) +
          SquaredError(offset, endpoint_low_rgba, num_channels);
    }

    const bool blue_contract = true;
    const bool offset_mode = true;
    *(errors_itr++) = CEEncodingOption(sq_bc_error, &pair,
                                       swapped, blue_contract, offset_mode);
  };

  compute_base_offset_blue_contract_error(bc_offset_quantized, false);

  // 3.5 Compute swapped base/offset error.
  compute_base_offset_error(offset_swapped_quantized, true);

  // 3.6 Compute swapped base/offset blue-contract error.
  compute_base_offset_blue_contract_error(
      bc_offset_swapped_quantized, true);

  std::sort(errors.begin(), errors.end(),
            [](const CEEncodingOption& a, const CEEncodingOption& b) {
              return a.Error() < b.Error();
            });

  // 4. Encode the values correspondingly.
  // For this part, we go through each measurement in order of increasing
  // error. Based on the properties of each measurement, we decide how to
  // best encode the quantized endpoints that produced that error value. If
  // for some reason we cannot encode that metric, then we skip it and move
  // to the next one.
  for (const auto& measurement : errors) {
    bool needs_weight_swap = false;
    if (measurement.Pack(with_alpha, astc_mode, vals, &needs_weight_swap)) {
      // Make sure that if we ask for a blue-contract mode that we get it *and*
      // if we don't ask for it then we don't get it.
      assert(!(measurement.BlueContract() ^
               UsesBlueContract(max_value, *astc_mode, *vals)));

      // We encoded what we got.
      return needs_weight_swap;
    }
  }

  assert(false && "Shouldn't have reached this point -- some combination of "
                  "endpoints should be possible to encode!");
  return false;
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////

bool UsesBlueContract(int max_value, ColorEndpointMode mode,
                      const std::vector<int>& vals) {
  assert(vals.size() >= size_t(NumColorValuesForEndpointMode(mode)));

  switch (mode) {
    case ColorEndpointMode::kLDRRGBDirect:
    case ColorEndpointMode::kLDRRGBADirect: {
      constexpr int kNumVals = MaxValuesForModes(
          ColorEndpointMode::kLDRRGBDirect, ColorEndpointMode::kLDRRGBADirect);
      std::array<int, kNumVals> v {};
      std::copy(vals.begin(), vals.end(), v.begin());
      Unquantize(&v, max_value);

      const int s0 = v[0] + v[2] + v[4];
      const int s1 = v[1] + v[3] + v[5];

      return s0 > s1;
    }

    case ColorEndpointMode::kLDRRGBBaseOffset:
    case ColorEndpointMode::kLDRRGBABaseOffset: {
      constexpr int kNumVals = MaxValuesForModes(
          ColorEndpointMode::kLDRRGBBaseOffset,
          ColorEndpointMode::kLDRRGBABaseOffset);
      std::array<int, kNumVals> v {};
      std::copy(vals.begin(), vals.end(), v.begin());
      Unquantize(&v, max_value);

      BitTransferSigned(&v[1], &v[0]);
      BitTransferSigned(&v[3], &v[2]);
      BitTransferSigned(&v[5], &v[4]);

      return v[1] + v[3] + v[5] < 0;
    }

    default:
      return false;
  }
}

bool EncodeColorsForMode(
    const RgbaColor& endpoint_low_rgba, const RgbaColor& endpoint_high_rgba,
    int max_value, EndpointEncodingMode encoding_mode,
    ColorEndpointMode* const astc_mode, std::vector<int>* const vals) {
  bool needs_weight_swap = false;
  vals->resize(NumValuesForEncodingMode(encoding_mode));

  switch (encoding_mode) {
    case EndpointEncodingMode::kDirectLuma:
      return EncodeColorsLuma(
          endpoint_low_rgba, endpoint_high_rgba, max_value, astc_mode, vals);

    case EndpointEncodingMode::kDirectLumaAlpha: {
      // TODO(google): See if luma-alpha base-offset is better
      const int avg1 = AverageRGB(endpoint_low_rgba);
      const int avg2 = AverageRGB(endpoint_high_rgba);

      (*vals)[0] = QuantizeCEValueToRange(avg1, max_value);
      (*vals)[1] = QuantizeCEValueToRange(avg2, max_value);
      (*vals)[2] = QuantizeCEValueToRange(endpoint_low_rgba[3], max_value);
      (*vals)[3] = QuantizeCEValueToRange(endpoint_high_rgba[3], max_value);
      *astc_mode = ColorEndpointMode::kLDRLumaAlphaDirect;
    }
    break;

    case EndpointEncodingMode::kBaseScaleRGB:
    case EndpointEncodingMode::kBaseScaleRGBA: {
      RgbaColor base = endpoint_high_rgba;
      RgbaColor scaled = endpoint_low_rgba;

      // Similar to luma base-offset, the scaled value is strictly less than
      // the base value here according to the decode procedure. In this case,
      // if the base is larger than the scale then we need to swap.
      int num_channels_ge = 0;
      for (int i = 0; i < 3; ++i) {
        num_channels_ge +=
            static_cast<int>(endpoint_high_rgba[i] >= endpoint_low_rgba[i]);
      }

      if (num_channels_ge < 2) {
        needs_weight_swap = true;
        std::swap(base, scaled);
      }

      // Since the second endpoint is just a direct copy of the RGB values, we
      // can start by quantizing them.
      const auto q_base = QuantizeColor(base, max_value);
      const auto uq_base = UnquantizeColor(q_base, max_value);

      // The first endpoint (scaled) is defined by piecewise multiplying the
      // second endpoint (base) by the scale factor and then dividing by 256.
      // This means that the inverse operation is to first piecewise multiply
      // the first endpoint by 256 and then divide by the unquantized second
      // endpoint. We take the average of each of each of these scale values as
      // our final scale value.
      // TODO(google): Is this the best way to determine the scale factor?
      int num_samples = 0;
      int scale_sum = 0;
      for (int i = 0; i < 3; ++i) {
        int x = uq_base[i];
        if (x != 0) {
          ++num_samples;
          scale_sum += (scaled[i] * 256) / x;
        }
      }

      (*vals)[0] = q_base[0];
      (*vals)[1] = q_base[1];
      (*vals)[2] = q_base[2];
      if (num_samples > 0) {
        const int avg_scale = Clamp(scale_sum / num_samples, 0, 255);
        (*vals)[3] = QuantizeCEValueToRange(avg_scale, max_value);
      } else {
        // In this case, all of the base values are zero, so we can use whatever
        // we want as the scale -- it won't affect the outcome.
        (*vals)[3] = max_value;
      }
      *astc_mode = ColorEndpointMode::kLDRRGBBaseScale;

      if (encoding_mode == EndpointEncodingMode::kBaseScaleRGBA) {
        (*vals)[4] = QuantizeCEValueToRange(scaled[3], max_value);
        (*vals)[5] = QuantizeCEValueToRange(base[3], max_value);
        *astc_mode = ColorEndpointMode::kLDRRGBBaseScaleTwoA;
      }
    }
    break;

    case EndpointEncodingMode::kDirectRGB:
    case EndpointEncodingMode::kDirectRGBA:
      return EncodeColorsRGBA(
          endpoint_low_rgba, endpoint_high_rgba, max_value,
          encoding_mode == EndpointEncodingMode::kDirectRGBA, astc_mode, vals);

    default:
      assert(false && "Unimplemented color encoding.");
  }

  return needs_weight_swap;
}

// These decoding procedures follow the code outlined in Section C.2.14 of
// the ASTC specification.
void DecodeColorsForMode(const std::vector<int>& vals,
                         int max_value, ColorEndpointMode mode,
                         RgbaColor* const endpoint_low_rgba,
                         RgbaColor* const endpoint_high_rgba) {
  assert(vals.size() >= size_t(NumColorValuesForEndpointMode(mode)));
  switch (mode) {
    case ColorEndpointMode::kLDRLumaDirect: {
      const int l0 = UnquantizeCEValueFromRange(vals[0], max_value);
      const int l1 = UnquantizeCEValueFromRange(vals[1], max_value);

      *endpoint_low_rgba = {{ l0, l0, l0, 255 }};
      *endpoint_high_rgba = {{ l1, l1, l1, 255 }};
    }
    break;

    case ColorEndpointMode::kLDRLumaBaseOffset: {
      const int v0 = UnquantizeCEValueFromRange(vals[0], max_value);
      const int v1 = UnquantizeCEValueFromRange(vals[1], max_value);

      const int l0 = (v0 >> 2) | (v1 & 0xC0);
      const int l1 = std::min(l0 + (v1 & 0x3F), 0xFF);

      *endpoint_low_rgba = {{ l0, l0, l0, 255 }};
      *endpoint_high_rgba = {{ l1, l1, l1, 255 }};
    }
    break;

    case ColorEndpointMode::kLDRLumaAlphaDirect: {
      constexpr int kNumVals =
          NumColorValuesForEndpointMode(ColorEndpointMode::kLDRLumaAlphaDirect);

      std::array<int, kNumVals> v;
      std::copy(vals.begin(), vals.end(), v.begin());
      Unquantize(&v, max_value);

      *endpoint_low_rgba = {{ v[0], v[0], v[0], v[2] }};
      *endpoint_high_rgba = {{ v[1], v[1], v[1], v[3] }};
    }
    break;

    case ColorEndpointMode::kLDRLumaAlphaBaseOffset: {
      constexpr int kNumVals = NumColorValuesForEndpointMode(
          ColorEndpointMode::kLDRLumaAlphaBaseOffset);

      std::array<int, kNumVals> v;
      std::copy(vals.begin(), vals.end(), v.begin());
      Unquantize(&v, max_value);

      BitTransferSigned(&v[1], &v[0]);
      BitTransferSigned(&v[3], &v[2]);

      *endpoint_low_rgba = {{ v[0], v[0], v[0], v[2] }};
      const int high_luma = v[0] + v[1];
      *endpoint_high_rgba = {{ high_luma, high_luma, high_luma, v[2] + v[3] }};

      for (auto& c : *endpoint_low_rgba) { c = Clamp(c, 0, 255); }
      for (auto& c : *endpoint_high_rgba) { c = Clamp(c, 0, 255); }
    }
    break;

    case ColorEndpointMode::kLDRRGBBaseScale: {
      constexpr int kNumVals =
          NumColorValuesForEndpointMode(ColorEndpointMode::kLDRRGBBaseScale);

      std::array<int, kNumVals> v;
      std::copy(vals.begin(), vals.end(), v.begin());
      Unquantize(&v, max_value);

      *endpoint_high_rgba = {{ v[0], v[1], v[2], 255 }};
      for (int i = 0; i < 3; ++i) {
        const int x = endpoint_high_rgba->at(i);
        endpoint_low_rgba->at(i) = (x * v[3]) >> 8;
      }
      endpoint_low_rgba->at(3) = 255;
    }
    break;

    case ColorEndpointMode::kLDRRGBDirect: {
      constexpr int kNumVals =
          NumColorValuesForEndpointMode(ColorEndpointMode::kLDRRGBDirect);

      std::array<int, kNumVals> v;
      std::copy(vals.begin(), vals.end(), v.begin());
      Unquantize(&v, max_value);

      const int s0 = v[0] + v[2] + v[4];
      const int s1 = v[1] + v[3] + v[5];

      *endpoint_low_rgba = {{ v[0], v[2], v[4], 255 }};
      *endpoint_high_rgba = {{ v[1], v[3], v[5], 255 }};

      if (s1 < s0) {
        std::swap(*endpoint_low_rgba, *endpoint_high_rgba);
        BlueContract(endpoint_low_rgba);
        BlueContract(endpoint_high_rgba);
      }
    }
    break;

    case ColorEndpointMode::kLDRRGBBaseOffset: {
      constexpr int kNumVals =
          NumColorValuesForEndpointMode(ColorEndpointMode::kLDRRGBBaseOffset);

      std::array<int, kNumVals> v;
      std::copy(vals.begin(), vals.end(), v.begin());
      Unquantize(&v, max_value);

      BitTransferSigned(&v[1], &v[0]);
      BitTransferSigned(&v[3], &v[2]);
      BitTransferSigned(&v[5], &v[4]);

      *endpoint_low_rgba = {{ v[0], v[2], v[4], 255 }};
      *endpoint_high_rgba = {{ v[0] + v[1], v[2] + v[3], v[4] + v[5], 255 }};

      if (v[1] + v[3] + v[5] < 0) {
        std::swap(*endpoint_low_rgba, *endpoint_high_rgba);
        BlueContract(endpoint_low_rgba);
        BlueContract(endpoint_high_rgba);
      }

      for (auto& c : *endpoint_low_rgba) { c = Clamp(c, 0, 255); }
      for (auto& c : *endpoint_high_rgba) { c = Clamp(c, 0, 255); }
    }
    break;

    case ColorEndpointMode::kLDRRGBBaseScaleTwoA: {
      constexpr int kNumVals = NumColorValuesForEndpointMode(
          ColorEndpointMode::kLDRRGBBaseScaleTwoA);

      std::array<int, kNumVals> v;
      std::copy(vals.begin(), vals.end(), v.begin());
      Unquantize(&v, max_value);

      // Base
      *endpoint_low_rgba = *endpoint_high_rgba = {{ v[0], v[1], v[2], 255 }};

      // Scale
      for (int i = 0; i < 3; ++i) {
        auto& x = endpoint_low_rgba->at(i);
        x = (x * v[3]) >> 8;
      }

      // Two A
      endpoint_low_rgba->at(3) = v[4];
      endpoint_high_rgba->at(3) = v[5];
    }
    break;

    case ColorEndpointMode::kLDRRGBADirect: {
      constexpr int kNumVals =
          NumColorValuesForEndpointMode(ColorEndpointMode::kLDRRGBADirect);

      std::array<int, kNumVals> v;
      std::copy(vals.begin(), vals.end(), v.begin());
      Unquantize(&v, max_value);

      const int s0 = v[0] + v[2] + v[4];
      const int s1 = v[1] + v[3] + v[5];

      *endpoint_low_rgba = {{ v[0], v[2], v[4], v[6] }};
      *endpoint_high_rgba = {{ v[1], v[3], v[5], v[7] }};

      if (s1 < s0) {
        std::swap(*endpoint_low_rgba, *endpoint_high_rgba);
        BlueContract(endpoint_low_rgba);
        BlueContract(endpoint_high_rgba);
      }
    }
    break;

    case ColorEndpointMode::kLDRRGBABaseOffset: {
      constexpr int kNumVals =
          NumColorValuesForEndpointMode(ColorEndpointMode::kLDRRGBABaseOffset);

      std::array<int, kNumVals> v;
      std::copy(vals.begin(), vals.end(), v.begin());
      Unquantize(&v, max_value);

      BitTransferSigned(&v[1], &v[0]);
      BitTransferSigned(&v[3], &v[2]);
      BitTransferSigned(&v[5], &v[4]);
      BitTransferSigned(&v[7], &v[6]);

      *endpoint_low_rgba = {{ v[0], v[2], v[4], v[6] }};
      *endpoint_high_rgba = {{
          v[0] + v[1], v[2] + v[3], v[4] + v[5], v[6] + v[7] }};

      if (v[1] + v[3] + v[5] < 0) {
        std::swap(*endpoint_low_rgba, *endpoint_high_rgba);
        BlueContract(endpoint_low_rgba);
        BlueContract(endpoint_high_rgba);
      }

      for (auto& c : *endpoint_low_rgba) { c = Clamp(c, 0, 255); }
      for (auto& c : *endpoint_high_rgba) { c = Clamp(c, 0, 255); }
    }
    break;

    default:
      // Unimplemented color encoding.
      // TODO(google): Is this the correct error handling?
      *endpoint_high_rgba = *endpoint_low_rgba = {{ 0, 0, 0, 0 }};
  }
}

}  // namespace astc_codec
