// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
#ifndef MAME_VIDEO_HANTRO_G1_H264_H
#define MAME_VIDEO_HANTRO_G1_H264_H
#pragma once

#include <cstdint>
#include <vector>

namespace hantro_g1
{
// The G1 receives parsed SPS/PPS fields in registers, not necessarily their
// original NAL units. Reconstitute the supported progressive 8-bit 4:2:0
// parameter sets for the software entropy/reconstruction backend.
class rbsp_writer
{
public:
	void bits(std::uint32_t value, unsigned count)
	{
		while (count--)
		{
			if (!(m_bits & 7))
				m_data.push_back(0);
			m_data.back() |= ((value >> count) & 1) << (7 - (m_bits++ & 7));
		}
	}

	void ue(unsigned value)
	{
		unsigned count = 0;
		for (unsigned n = value + 1; n >>= 1;)
			++count;
		bits(0, count);
		bits(value + 1, count + 1);
	}

	void se(int value) { ue(value <= 0 ? -2 * value : 2 * value - 1); }

	void nal(std::vector<std::uint8_t> &out, std::uint8_t header)
	{
		bits(1, 1);
		while (m_bits & 7)
			bits(0, 1);
		out.insert(out.end(), { 0, 0, 0, 1, header });
		unsigned zeros = 0;
		for (auto b : m_data)
		{
			if (zeros == 2 && b <= 3)
			{
				out.push_back(3);
				zeros = 0;
			}
			out.push_back(b);
			zeros = b ? 0 : zeros + 1;
		}
	}

private:
	std::vector<std::uint8_t> m_data;
	unsigned m_bits = 0;
};

inline bool parameter_sets(std::uint32_t const *r, std::vector<std::uint8_t> &out)
{
	unsigned const width = (r[4] >> 23) & 511, height = (r[4] >> 11) & 255;
	unsigned const frame_bits = (r[7] >> 16) & 31, poc_bits = r[9] & 255;
	unsigned const refs = r[4] & 31, l0 = (r[9] >> 14) & 31, l1 = (r[9] >> 19) & 31;
	if (!width || !height || width > 120 || height > 68 || refs > 16 || frame_bits < 4 || frame_bits > 16 || poc_bits < 4 ||
			poc_bits > 16 || !l0 || !l1 || (r[3] & 0x08802400) || (r[5] & 0x01000001) || (r[7] & 0x40000000))
		return false; // RLC, interlace, MBAFF, MVC, custom matrices or monochrome.
	unsigned const qp = (r[6] >> 25) & 63;
	if (qp > 51)
		return false;
	auto signed5 = [](unsigned v)
	{
		return int(v & 31) - int(v & 16) * 2;
	};
	rbsp_writer s;
	s.bits(100, 8);
	s.bits(0, 8);
	s.bits(42, 8); // High profile, level 4.2
	s.ue(0);
	s.ue(1);
	s.ue(0);
	s.ue(0);
	s.bits(0, 1);
	s.bits(0, 1);
	s.ue(frame_bits - 4);
	s.ue(0);
	s.ue(poc_bits - 4);
	s.ue(refs);
	s.bits(0, 1);
	s.ue(width - 1);
	s.ue(height - 1);
	s.bits(1, 1);
	s.bits((r[7] >> 29) & 1, 1);
	s.bits(0, 1);
	s.bits(0, 1);
	s.nal(out, 0x67);
	rbsp_writer p;
	p.ue(r[9] >> 24);
	p.ue(0);
	p.bits(r[7] >> 31, 1);
	p.bits(0, 1);
	p.ue(0);
	p.ue(l0 - 1);
	p.ue(l1 - 1);
	p.bits((r[7] >> 28) & 1, 1);
	p.bits((r[7] >> 26) & 3, 2);
	p.se(int(qp) - 26);
	p.se(0);
	p.se(signed5(r[5] >> 19));
	p.bits((r[8] >> 30) & 1, 1);
	p.bits(r[8] >> 31, 1);
	p.bits((r[8] >> 29) & 1, 1);
	p.bits((r[8] >> 28) & 1, 1);
	p.bits(0, 1);
	p.se(signed5(r[5] >> 14));
	p.nal(out, 0x68);
	return true;
}
}
#endif
