// license:BSD-3-Clause
// copyright-holders:blahm1d
// MK4 v3.0 experimental calibration, ported from the 2026-10-09 v17 RTL.
// These asset/signature filters and fitted coefficients are NOT decoded Zeus
// microcode or documented hardware flags. See docs/mk4-v17-research.md.
#ifndef MAME_WILLIAMS_MIDZEUS_MK4_V17_H
#define MAME_WILLIAMS_MIDZEUS_MK4_V17_H

#pragma once
#include <algorithm>
#include <cstdint>

namespace mk4_v17 {

struct material { uint32_t pointer, signature; int gain, bias; };
inline constexpr material materials[] = {
 {0x0020c0,0x85817c77,518,-128}, {0x050147,0x827d7975,347,-26},
 {0x0191b9,0x7e78736c,355,-16}, {0x01a050,0xa59f9890,199,-4},
 {0x00b090,0x706b655f,336,-29}, {0x00c1a5,0x9e978f87,201,-4},
 {0x01a1ae,0xc6c4c2c0,558,-128}, {0x01b1b9,0xc2c0bebd,363,2},
 {0x00e020,0xb0aca9a5,302,-1}, {0x004044,0xa6a39f9b,333,0},
 {0x00e101,0x9c989490,354,2}, {0x0501d7,0xa7a39e9a,237,-30},
 {0x0061c0,0x7b767069,454,-96}, {0x006000,0x88837e78,301,-2},
 {0x0060a0,0x8c87837f,342,-4}, {0x0051bc,0x938f8a84,367,-18},
 {0x0051cc,0x726d6862,442,-86}, {0x0100cf,0x726d6862,256,-3},
 {0x0150c2,0x9f99928c,330,-115}, {0x015090,0x7f7c7a77,406,-89},
 {0x015080,0xa09d9a97,409,-72}, {0x015112,0x96928f8c,331,-16},
 {0x0190f7,0x7e7b7875,461,-121}
};

inline int32_t sign10(uint32_t n) { return int32_t(n & 1023) - ((n & 512) ? 1024 : 0); }
inline int64_t floor_shift(int64_t n, unsigned bits)
{
 return n >= 0 ? n >> bits : -((-n + (int64_t(1) << bits) - 1) >> bits);
}
inline bool selected(uint32_t ptr, uint32_t tex)
{
 ptr &= 0xffffff;
 uint32_t const bank = ptr >> 12;
 if (tex == 0x009d138a || tex == 0x00a9d650 || tex == 0x00a9d970 || tex == 0x0d214eb2) return false;
 return (bank >= 0x001 && bank < 0x04c) || bank == 0x04f || bank == 0x050 ||
  bank == 0x185 || bank == 0x186 || bank == 0x189 || bank == 0x18a || bank == 0x18e ||
  (ptr == 0x04d12c && (tex == 0x00a92b00 || tex == 0x00a92b40 || tex == 0x00e55700)) ||
  (ptr == 0x04d0b0 && (tex == 0x00e552b4 || tex == 0x01219f80 || tex == 0x00a92a40)) ||
  (ptr == 0x04d144 && tex == 0x0115b770);
}
inline bool shaft(uint32_t ptr, uint32_t tex)
{
 return ((ptr & 0xffffff) == 0x058080 && tex == 0x006d7800) ||
        ((ptr & 0xffffff) == 0x02f170 && tex == 0x058252c0);
}
inline uint32_t depth_intensity(int64_t rounded_z)
{
 uint32_t const bits = uint32_t(floor_shift(rounded_z,15)) & 0xfffff;
 int32_t const z = int32_t(bits) - ((bits & 0x80000) ? 0x100000 : 0);
 int const fade = 328 - int(floor_shift(int64_t(z)*3,8));
 return fade >= 255 ? 65535 : uint32_t(std::max(fade,48)) << 8;
}

template <typename Reader>
uint32_t intensity(uint32_t packed, const int16_t matrix[3][3], const int16_t light[3],
 bool light_valid, uint32_t ptr, uint32_t tex, uint32_t blend, uint8_t alpha,
 bool long_format, int64_t rounded_z, Reader read)
{
 if (!long_format) return 65535;
 if (shaft(ptr,tex)) return depth_intensity(rounded_z);
 if (!selected(ptr,tex) || !light_valid) return 65535;
 uint32_t const bank = (ptr >> 12) & 0xfff;
 bool const environment = bank >= 1 && bank < 0x1d;
 bool const usable = (packed & 0x3fffffff) && (ptr >> 24) >= 15 &&
  ((ptr % 512) + ((ptr >> 12) % 2048)*512 <= 0xffff0);
 uint32_t value = 65535;
 bool calibrated = false;
 if (usable)
 {
  int32_t const normal[3] = {sign10(packed),sign10(packed >> 10),sign10(packed >> 20)};
  int64_t dot = 0, view = 0;
  for (unsigned col=0;col<3;col++)
  {
   int64_t model_light=0;
   for (unsigned row=0;row<3;row++) model_light += int64_t(matrix[row][col])*light[row];
   dot += model_light*normal[col];
   view += int64_t(matrix[2][col])*normal[col];
  }
  uint32_t const index = uint32_t(std::clamp<int64_t>(floor_shift(dot,26),-64,63)) & 127;
  value = uint32_t(read(index)) << 8;
  uint32_t const signature = uint32_t(read(0)) | (uint32_t(read(1))<<8) | (uint32_t(read(2))<<16) | (uint32_t(read(3))<<24);
  if (environment || bank == 0x050)
   for (auto const &m : materials)
    if (m.pointer == (ptr & 0xffffff) && m.signature == signature)
    {
     value = uint32_t(std::clamp<int64_t>(int64_t(value >> 2)*m.gain/64 + int64_t(m.bias)*256,0,131071));
     calibrated = true;
     break;
    }
  if (!calibrated)
  {
   if (environment) value = std::min(value + (value >> 2),65535U);
   else if ((ptr & 0xffffff) == 0x0230f4 && (blend == 0x4b23cb00 || blend == 0x4b23dd00))
   {
    int64_t projected = floor_shift(view,14);
    uint32_t gap = uint32_t(256 - std::min<int64_t>(projected < 0 ? -projected : projected,256));
    uint32_t square = (gap*gap) >> 8;
    uint32_t fourth = (square*square) >> 8;
    value = std::min(value + ((uint32_t(read(63))*fourth) >> 1),65535U);
   }
  }
 }
 if (!environment && bank != 0x04d && (blend == 0x4b23cb00 || blend == 0x4b23dd00))
 {
  if (usable && !(calibrated && bank == 0x050))
  {
   if (bank == 0x04f || bank == 0x050 || bank == 0x18e) value = std::min(value+(value >> 3),65535U);
   else value = uint32_t(std::clamp<int64_t>(int64_t(value)*2-32768,0,131071));
  }
  if (alpha != 255) value = uint32_t(4096 + floor_shift((int64_t(value)-4096)*alpha,8)) & 0x1ffff;
 }
 return value;
}

inline uint16_t rectangle_color(uint16_t color, uint32_t rgb)
{
 uint16_t result = color & 0x8000;
 for (unsigned c=0;c<3;c++)
 {
  uint32_t v=(color>>(5*c))&31;
  result |= (((v*8+v/4)*((rgb>>(8*c))&255))>>11) << (5*c);
 }
 return result;
}

// One channel after texture filtering, through blending and RGB555 packing.
inline uint32_t channel(uint32_t src, uint32_t dst, uint32_t i8, uint32_t alpha, uint32_t blend, bool depth)
{
 uint32_t sa=alpha&255, da=(alpha>>8)&255;
 bool const lit=blend==0x4b23cb00 || blend==0x4b23dd00;
 bool const fog=blend==0xdd23dd00;
 bool const add1=blend==0x40b68800, add2=blend==0xc9b78800;
 bool const mul=blend==0x4093c800;
 if ((add1 || add2) && i8<255) sa=(sa*i8)>>8;
 bool const plain=!lit && !fog && !add1 && !add2 && !mul;
 bool const shade=depth && ((lit && sa!=255) || ((fog || plain) && i8<255));
 uint32_t const factor=(lit || (plain && depth)) ? i8 : mul ? sa*2 : sa;
 uint32_t const a=(src*factor)>>8;
 uint32_t const b=((mul || shade ? a : dst)*(mul ? dst : shade ? (lit ? sa : (i8&255)) : da*2))>>8;
 uint32_t out=mul ? b : add2 ? a+b : add1 ? a+dst : (lit || fog) ? (shade ? b : a) : shade ? a : src;
 if ((lit && i8!=255) || (fog && depth && i8<255)) out+=4;
 return std::min(out,255U)>>3;
}
} // namespace mk4_v17
#endif
