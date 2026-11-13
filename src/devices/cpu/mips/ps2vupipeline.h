// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_CPU_MIPS_PS2VUPIPELINE_H
#define MAME_CPU_MIPS_PS2VUPIPELINE_H

#pragma once

#include "ps2vufloat.h"

#include <algorithm>
#include <cassert>

namespace ps2vu
{

struct fmac_op
{
	enum class operation { none, add, sub, mul, madd, msub };
	operation arithmetic = operation::none;
	unsigned fs = 0, ft = 0, fd = 0, mask = 0;
	unsigned fs_mask = 0, ft_mask = 0;
	bool acc = false, cross = false, broadcast = false, q = false;
	unsigned bc = 0;

	explicit operator bool() const { return arithmetic != operation::none; }
};

// Decode only the arithmetic instructions implemented by the two interpreters.
inline fmac_op decode_fmac(uint32_t op, bool macro)
{
	using operation = fmac_op::operation;
	fmac_op d;
	d.fs = (op >> 11) & 31;
	d.ft = (op >> 16) & 31;
	d.fd = (op >> 6) & 31;
	d.mask = (op >> 21) & 15;
	d.bc = op & 3;
	const unsigned function = op & 63;
	const unsigned ext = ((op >> 4) & (macro ? 0x7c : 0x3c)) | (op & 3);
	if (function >= 0x3c)
	{
		d.acc = true;
		if (ext >= 8 && ext <= 11)
		{
			d.arithmetic = operation::madd;
			d.broadcast = true;
		}
		else if (ext >= 0x18 && ext <= 0x1b)
		{
			d.arithmetic = operation::mul;
			d.broadcast = true;
		}
		else if (ext == 0x2e)
		{
			d.arithmetic = operation::mul;
			d.cross = true;
		}
	}
	else if (function >= 8 && function <= 11)
	{
		d.arithmetic = operation::madd;
		d.broadcast = true;
	}
	else
	{
		if (function <= 7 || (function >= 0x18 && function <= 0x1b))
		{
			d.arithmetic = function <= 3 ? operation::add : function <= 7 ? operation::sub : operation::mul;
			d.broadcast = true;
		}
		else if (function == 0x1c || function == 0x20)
		{
			d.arithmetic = function == 0x1c ? operation::mul : operation::add;
			d.q = true;
		}
		else if (function == 0x28 || function == 0x2a || function == 0x2c)
			d.arithmetic = function == 0x28 ? operation::add : function == 0x2a ? operation::mul : operation::sub;
		else if (function == 0x2e)
		{
			d.arithmetic = operation::msub;
			d.cross = true;
		}
	}
	if (d.cross)
		d.mask = d.fs_mask = d.ft_mask = 14;
	else
	{
		d.fs_mask = d.mask;
		d.ft_mask = d.q ? 0 : d.broadcast ? (d.mask ? 8 >> d.bc : 0) : d.mask;
	}
	return d;
}

// two vector writes per cycle, retiring four cycles after issue; ACC is forwarded
struct pipeline
{
	uint64_t cycle = 0;
	uint64_t due[8] = {};
	uint32_t value[8][4] = {};
	uint16_t mac[8] = {}, status[8] = {};
	uint8_t reg[8] = {}, mask[8] = {}, flags[8] = {};
	uint8_t head = 0, tail = 0, count = 0;
	bool acc_overflow[4] = {};
	uint64_t q_due = 0;
	uint32_t q_value = 0, q_flags = 0;
	uint64_t vi_due[32] = {};
	uint16_t vi_value[32] = {};

	bool integers_pending() const
	{
		return std::any_of(std::begin(vi_due), std::end(vi_due), [](uint64_t due) { return due != 0; });
	}

	unsigned enqueue(unsigned destination, unsigned fields)
	{
		assert(count < 8);
		const unsigned slot = tail;
		tail = (tail + 1) & 7;
		++count;
		due[slot] = cycle + 4;
		reg[slot] = destination;
		mask[slot] = destination ? fields : 0;
		mac[slot] = status[slot] = flags[slot] = 0;
		return slot;
	}

	void retire(float (&vf)[32][4], uint32_t &sf, uint32_t &mf, float &q, uint32_t *vi = nullptr)
	{
		while (count && due[head] <= cycle)
		{
			for (unsigned field = 0; field < 4; ++field)
			{
				if (mask[head] & (8 >> field))
					vf[reg[head]][field] = std::bit_cast<float>(value[head][field]);
			}
			if (flags[head])
			{
				mf = mac[head];
				sf = (sf & ~15U) | status[head];
			}
			head = (head + 1) & 7;
			--count;
		}
		if (vi)
		{
			for (unsigned r = 1; r < 32; ++r)
			{
				if (vi_due[r] && vi_due[r] <= cycle)
				{
					vi[r] = vi_value[r];
					vi_due[r] = 0;
				}
			}
		}
		if (q_due && q_due <= cycle)
		{
			q = std::bit_cast<float>(q_value);
			div_flags(sf, q_flags);
			q_due = 0;
		}
	}

	uint64_t ready(unsigned source, unsigned fields) const
	{
		uint64_t when = cycle;
		if (source)
		{
			for (unsigned n = 0; n < count; ++n)
			{
				const unsigned slot = (head + n) & 7;
				if (reg[slot] == source && (mask[slot] & fields))
					when = std::max(when, due[slot]);
			}
		}
		return when;
	}

	bool hazard(const fmac_op &d) const
	{
		return d && (ready(d.fs, d.fs_mask) > cycle || ready(d.ft, d.ft_mask) > cycle);
	}

	uint64_t control_ready(unsigned destination) const
	{
		uint64_t when = cycle;
		if (destination == 16 || destination == 17)
		{
			for (unsigned n = 0; n < count; ++n)
			{
				const unsigned slot = (head + n) & 7;
				if (flags[slot])
					when = std::max(when, due[slot]);
			}
		}
		if (destination == 16 || destination == 22)
			when = std::max(when, q_due);
		return when;
	}

	void fmac(const fmac_op &d, const float (&vf)[32][4], float (&acc)[4], float q)
	{
		using operation = fmac_op::operation;
		const unsigned slot = enqueue(d.acc ? 0 : d.fd, d.mask);
		flags[slot] = 1;
		for (unsigned field = 0; field < 4; ++field)
		{
			if (!(d.mask & (8 >> field)))
				continue;
			const uint32_t a = std::bit_cast<uint32_t>(vf[d.fs][d.cross ? (field + 1) % 3 : field]);
			const uint32_t b = std::bit_cast<uint32_t>(d.q ? q : vf[d.ft][d.cross ? (field + 2) % 3 : d.broadcast ? d.bc : field]);
			fmac_result r;
			switch (d.arithmetic)
			{
				case operation::add:  r = add(a, b); break;
				case operation::sub:  r = add(a, b ^ 0x80000000U); break;
				case operation::mul:  r = multiply(a, b); break;
				case operation::madd:
				case operation::msub: r = madd(std::bit_cast<uint32_t>(acc[field]), acc_overflow[field], a, b, d.arithmetic == operation::msub); break;
				default: assert(false); return;
			}
			value[slot][field] = r.value;
			if (d.acc)
			{
				acc[field] = std::bit_cast<float>(r.value);
				acc_overflow[field] = (r.flags & FLAG_O) != 0;
			}
			for (unsigned flag = 0; flag < 4; ++flag)
			{
				if (r.flags & (1 << flag))
					mac[slot] |= 1 << (flag * 4 + 3 - field);
			}
			status[slot] |= r.flags | (r.sticky << 6);
		}
	}
};

} // namespace ps2vu

#endif // MAME_CPU_MIPS_PS2VUPIPELINE_H
