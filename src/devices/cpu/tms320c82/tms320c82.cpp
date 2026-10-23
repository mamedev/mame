// license:BSD-3-Clause
// copyright-holders:Ville Linde, Eduardo Cruz
/*
    Texas Instruments TMS320C82 DSP Emulator

    Written by Ville Linde

*/

#include "emu.h"
#include "tms320c82.h"

#include "mp_dasm.h"
#include "pp_dasm.h"

#include <algorithm>
#include <bit>
#include <iterator>


DEFINE_DEVICE_TYPE(TMS320C82_MP, tms320c82_mp_device, "tms320c82_mp", "Texas Instruments TMS320C82 MP")
DEFINE_DEVICE_TYPE(TMS320C82_PP, tms320c82_pp_device, "tms320c82_pp", "Texas Instruments TMS320C82 PP")




// Master Processor

// internal memory map
void tms320c82_mp_device::mp_internal_map(address_map &map)
{
	map(0x00000000, 0x00000fff).ram().share("pp0_data0");
	map(0x00001000, 0x00001fff).ram().share("pp1_data0");
	map(0x00008000, 0x00008fff).ram().share("pp0_data1");
	map(0x00009000, 0x00009fff).ram().share("pp1_data1");
	map(0x01000000, 0x01000fff).ram().share("pp0_param");
	map(0x01001000, 0x01001fff).ram().share("pp1_param");
	map(0x01010000, 0x01010fff).rw(FUNC(tms320c82_mp_device::mp_param_r), FUNC(tms320c82_mp_device::mp_param_w));
	map(0x01820000, 0x0182000f).rw(FUNC(tms320c82_mp_device::tc_reg_r), FUNC(tms320c82_mp_device::tc_reg_w));
}

const uint32_t tms320c82_mp_device::SHIFT_MASK[] =
{
	0x00000000, 0x00000001, 0x00000003, 0x00000007, 0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
	0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
	0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
	0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
	0xffffffff
};



tms320c82_mp_device::tms320c82_mp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, TMS320C82_MP, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(tms320c82_mp_device::mp_internal_map), this))
	, m_cmd_callback(*this)
	, m_pp(*this, "pp%u", 0U)
	, m_peripheral_read_cb(*this)
	, m_peripheral_write_cb(*this, 0xffffffff)
	, m_vram_read_cb(*this)
	, m_tc_timer(nullptr)
	, m_tc_request_timer{}
{
}

void tms320c82_mp_device::device_add_mconfig(machine_config &config)
{
	for (unsigned i = 0; i < m_pp.size(); i++)
		TMS320C82_PP(config, m_pp[i], DERIVED_CLOCK(1, 1), i);
}

device_memory_interface::space_config_vector tms320c82_mp_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

device_memory_interface::space_config_vector tms320c82_pp_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}


std::unique_ptr<util::disasm_interface> tms320c82_mp_device::create_disassembler()
{
	return std::make_unique<tms320c82_mp_disassembler>();
}

namespace
{
// C82 data and parameter RAM (SPRU261 4.2). Packet descriptors must be on chip.
bool tc_ram(uint32_t address)
{
	return address < 0x2000 || (address >= 0x8000 && address < 0xa000)
			|| (address >= 0x01000000 && address < 0x01002000)
			|| (address >= 0x01010000 && address < 0x01011000);
}
uint32_t tc_pointer(unsigned channel)
{
	return channel < 16 ? 0x010100c0 + channel * 4 : 0x010000fc + ((channel - 16) << 12);
}
uint32_t tc_suspend_base(unsigned channel)
{
	return channel == 15 ? 0x01010000 : 0x01000000 + ((channel - 16) << 12);
}
} // namespace

void tms320c82_mp_device::tc_start()
{
	m_tc_timer = timer_alloc(FUNC(tms320c82_mp_device::tc_tick), this);
	for (unsigned i = 0; i < 3; i++)
		m_tc_request_timer[i] = timer_alloc(FUNC(tms320c82_mp_device::tc_request_tick), this);
	save_item(NAME(m_tc_regs));
	save_item(NAME(m_tc_active));
	save_item(NAME(m_tc_round_robin));
	for (unsigned i = 0; i < 18; i++)
	{
		save_item(NAME(m_tc[i].control), i);
		save_item(NAME(m_tc[i].descriptor), i);
		save_item(NAME(m_tc[i].words), i);
		save_item(NAME(m_tc[i].progress), i);
		save_item(NAME(m_tc[i].total), i);
		save_item(NAME(m_tc[i].elapsed), i);
		save_item(NAME(m_tc[i].cursor), i);
		save_item(NAME(m_tc[i].cursor_wrap), i);
		save_item(NAME(m_tc[i].loaded), i);
		save_item(NAME(m_tc[i].transparent_data), i);
		save_item(NAME(m_tc[i].transparent_mask), i);
	}
}

void tms320c82_mp_device::tc_reset()
{
	for (auto &t : m_tc)
		t = tc_channel{};
	m_tc_regs[0] = m_tc_regs[3] = 0;
	m_tc_regs[1] = m_tc_regs[2] = 0x10000;
	m_tc_active = -1;
	std::fill(std::begin(m_tc_round_robin), std::end(m_tc_round_robin), 0);
	m_tc_timer->adjust(attotime::never);
	for (auto *timer : m_tc_request_timer)
		timer->adjust(attotime::never);
}

uint32_t tms320c82_mp_device::tc_reg_r(offs_t offset)
{
	return m_tc_regs[offset];
}

void tms320c82_mp_device::tc_reg_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == 3)
	{
		m_tc_regs[offset] &= ~(data & mem_mask); // FLTSTS: write one to clear
		tc_kick();
	}
	else
	{
		COMBINE_DATA(&m_tc_regs[offset]);
		if (offset == 1 || offset == 2)
			m_tc_regs[offset] &= 0x00ffffff;
	}
}

uint32_t tms320c82_mp_device::packet_status(unsigned processor) const
{
	return m_tc[processor == 2 ? 15 : 16 + processor].control;
}

void tms320c82_mp_device::packet_reset(unsigned processor)
{
	// A PP reset abandons its requests without a completion/error interrupt
	// (SPRU110 9.6.1). The other processors' transfers remain queued.
	unsigned const channel = 16 + processor;
	if (m_tc[channel].control & 2)
		m_tc_round_robin[tc_priority(channel) - 1] = channel - 15;
	m_tc_request_timer[processor]->adjust(attotime::never);
	m_tc[channel] = tc_channel{};
	if (m_tc_active == int(channel))
		m_tc_active = -1;
	tc_kick();
}

void tms320c82_mp_device::packet_control(unsigned processor, uint32_t data)
{
	unsigned const channel = processor == 2 ? 15 : 16 + processor;
	auto &t = m_tc[channel];
	uint32_t const old = t.control;
	// Q is read only. P can be written only while S is clear or being cleared.
	t.control = (old & 2) | (data & (processor == 2 ? 0x1c : 0x0c)) | (BIT(data, 2) ? old & 1 : data & 1);
	if (!BIT(data, 2) && BIT(data, 0))
		m_tc_request_timer[processor]->adjust(clocks_to_attotime(processor == 2 ? 1 : 2), channel);
	if (BIT(data, 2))
		m_tc_request_timer[processor]->adjust(attotime::never);
	tc_kick();
}

TIMER_CALLBACK_MEMBER(tms320c82_mp_device::tc_request_tick)
{
	auto &t = m_tc[param];
	if (BIT(t.control, 2) || !BIT(t.control, 0))
		return;
	if (BIT(t.control, 1))
	{
		tc_signal(param, 0); // request while busy
	}
	else
	{
		t.control |= 2;
		t.loaded = false;
	}
	t.control &= ~1U;
	tc_kick();
}

bool tms320c82_mp_device::external_transfer(int channel)
{
	if (channel < 1 || channel > 15 || !BIT(m_config, 27) || (m_tc_regs[3] & 0x1e0))
		return false;
	auto &t = m_tc[15 - channel];
	if (!t.control && m_program.read_dword(tc_pointer(15 - channel)))
	{
		t.control = 2;
		t.loaded = false;
		tc_kick();
		return true;
	}
	return false;
}

void tms320c82_mp_device::tc_kick()
{
	if (!m_tc_timer->enabled() || m_tc_timer->expire().is_never())
		m_tc_timer->adjust(clocks_to_attotime(1));
}

unsigned tms320c82_mp_device::tc_priority(unsigned channel) const
{
	if (channel < 15)
		return 4;
	if (channel == 15 && BIT(m_tc[channel].control, 4))
		return 3;
	return BIT(m_tc[channel].control, 3) ? 2 : 1;
}

int tms320c82_mp_device::tc_select() const
{
	// External requests are above processor requests.
	if (!(m_tc_regs[3] & 0x1e0))
	{
		for (unsigned i = 0; i < 15; i++)
		{
			if (m_tc[i].control & 2)
				return i;
		}
	}
	static constexpr unsigned ORDER[3] = {16, 17, 15};
	for (unsigned priority = 3; priority; priority--)
	{
		for (unsigned n = 0; n < 3; n++)
		{
			unsigned const token = BIT(m_config, 29) ? (n + 2) % 3 : (n + m_tc_round_robin[priority - 1]) % 3;
			unsigned const channel = ORDER[token];
			if ((m_tc[channel].control & 6) == 2 && tc_priority(channel) == priority)
				return channel;
		}
	}
	return -1;
}

void tms320c82_mp_device::tc_signal(unsigned channel, unsigned kind)
{
	// kind: 0 = request while busy, 1 = error, 2 = complete.
	// SPRU109 INTPEN PB/BP/PC and SPRU110 intflg PTQ/PTERR/PTEND.
	static constexpr uint8_t MP_BITS[3] = { 27, 28, 26 };
	if (channel == 15)
		m_intpen |= 1U << MP_BITS[kind];
	else if (channel >= 16)
		m_pp[channel - 16]->signal_interrupt(1U << (17 + kind));
}

bool tms320c82_mp_device::tc_guided(tc_channel const &t)
{
	return !BIT(t.words[1], 20) && (((t.words[1] >> 12) & 7) >= 3 || ((t.words[1] >> 4) & 7) >= 4);
}

// Guide entries are loaded one at a time, when the TC reaches them.
// Returns -1 on error, 0 at the end, 1 for data, 2 after loading an entry (one beat).
int tms320c82_mp_device::tc_prepare(unsigned channel, unsigned side)
{
	auto &t = m_tc[channel];
	auto &c = t.cursor[side];
	unsigned const mode = (t.words[1] >> (side ? 4 : 12)) & 7;
	uint64_t const size = uint64_t(c[1] & 0xffff) * ((c[1] >> 16) + 1);
	if ((c[3] || t.cursor_wrap[side]) && c[2] < size)
		return 1;
	uint64_t const patches = uint64_t(t.words[6 + side]) + (mode == 0);
	if (t.cursor_wrap[side] || c[3] >= patches)
		return 0;
	bool const reverse = BIT(t.words[1], side ? 25 : 22);
	if (mode >= 3)
	{
		unsigned const stride = (mode == 4 || mode == 5) ? 8 : 4;
		uint32_t const pointer = t.words[10 + side] + (reverse ? -c[3] * stride : c[3] * stride);
		if ((pointer & (stride - 1)) || !tc_ram(pointer) || !tc_ram(pointer + stride - 1))
		{
			tc_error(channel, true);
			return -1;
		}
		uint32_t offset = m_program.read_dword(pointer + stride - 4);
		if (mode == 3)
		{
			// LUT scale is the leftmost set bit in A count's low nibble.
			unsigned const low = t.words[4 + side] & 15;
			offset <<= (low & 8) ? 3 : (low & 4) ? 2 : (low & 2) ? 1 : 0;
		}
		c[0] = ((mode == 4 || mode == 6) ? c[0] : t.words[2 + side]) + offset;
		c[1] = stride == 8 ? m_program.read_dword(pointer) : t.words[4 + side];
	}
	else
	{
		c[0] += reverse ? -t.words[10 + side] : t.words[10 + side];
		c[1] = t.words[4 + side];
	}
	c[2] = 0;
	if (!++c[3])
		t.cursor_wrap[side] = true;
	return 2;
}

uint64_t tms320c82_mp_device::tc_length(tc_channel const &t, unsigned side)
{
	if (BIT(t.words[1], 20))
		return t.words[1] & 0xffff;
	unsigned const mode = (t.words[1] >> (side ? 4 : 12)) & 7;
	if (mode == 4 || mode == 5)
		return ~uint64_t(0); // counts are fetched from each guide entry
	return uint64_t(t.words[4 + side] & 0xffff) * ((t.words[4 + side] >> 16) + 1) * (uint64_t(t.words[6 + side]) + (mode < 3));
}

uint32_t tms320c82_mp_device::tc_address(tc_channel const &t, unsigned side, uint64_t n)
{
	uint32_t const options = t.words[1];
	bool const guided = tc_guided(t);
	uint32_t const start = guided ? t.cursor[side][0] : t.words[2 + side];
	if (guided)
		n = t.cursor[side][2];
	bool const reverse = BIT(options, 23);
	if (BIT(options, 20))
	{
		// VRAM read requests repeat a serial-register load at the same tap.
		// Count is a repetition count (or a post-update), not a byte copy.
		unsigned const pam = (options >> 16) & 7;
		if (pam == 2 || pam == 3)
			return start;
		return reverse ? start - uint32_t(n) : start + uint32_t(n);
	}
	uint32_t const counts = guided ? t.cursor[side][1] : t.words[4 + side];
	uint32_t const a = counts & 0xffff, b = (counts >> 16) + 1;
	if (!a)
		return start;
	uint32_t const row = ((n / a) % b) * t.words[8 + side];
	uint32_t const patch = guided ? 0 : (n / (uint64_t(a) * b)) * t.words[10 + side];
	// Both A counts count mask bytes for block-write (one byte selects eight
	// destination bytes). B/C pitches remain byte addresses, not scaled counts.
	unsigned const expand = ((options >> 16) & 7) == 2 && side == (BIT(options, 19) ^ 1) ? 8 : 1;
	uint32_t const column = uint32_t(n % a) * expand;
	return start + (reverse ? -column : column)
			+ (BIT(options, side ? 24 : 21) ? -row : row)
			+ (BIT(options, side ? 25 : 22) ? -patch : patch);
}

uint64_t tms320c82_mp_device::tc_completed_patches(tc_channel const &t, unsigned side)
{
	auto const &cursor = t.cursor[side];
	uint64_t const loaded = uint64_t(cursor[3]) + (uint64_t(t.cursor_wrap[side]) << 32);
	uint64_t const size = uint64_t(cursor[1] & 0xffff) * ((cursor[1] >> 16) + 1);
	return loaded - ((loaded && cursor[2] < size) ? 1 : 0);
}

void tms320c82_mp_device::tc_suspend(unsigned channel, bool software, unsigned fault)
{
	auto &t = m_tc[channel];
	if (t.loaded)
	{
		uint32_t const base = tc_suspend_base(channel);
		for (unsigned i = 0; i < 16; i++)
			m_program.write_dword(base + i * 4, t.words[i]);
		m_program.write_dword(base, t.descriptor);
		m_program.write_dword(base + 4, (t.words[1] & ~0x60000000U) | ((fault ? fault : 1) << 29));
		for (unsigned side = 0; side < 2; side++)
		{
			m_program.write_dword(base + 8 + side * 4, tc_address(t, side, t.progress));
			if (tc_guided(t))
			{
				uint64_t const completed = tc_completed_patches(t, side);
				unsigned const mode = (t.words[1] >> (side ? 4 : 12)) & 7;
				uint32_t const remaining = t.words[6 + side] - std::min<uint64_t>(completed, t.words[6 + side]);
				m_program.write_dword(base + 24 + side * 4, remaining);
				if (mode >= 3)
				{
					// Guide pointers advance after each completed patch (SPRU261 9.5.1).
					unsigned const stride = (mode == 4 || mode == 5) ? 8 : 4;
					uint32_t const advance = uint32_t(completed * stride);
					uint32_t const pointer = t.words[10 + side] + (BIT(t.words[1], side ? 25 : 22) ? -advance : advance);
					m_program.write_dword(base + 40 + side * 4, pointer);
				}
			}
			else if (!BIT(t.words[1], 20) && (t.words[4 + side] & 0xffff))
			{
				uint64_t const patch_size = uint64_t(t.words[4 + side] & 0xffff) * ((t.words[4 + side] >> 16) + 1);
				m_program.write_dword(base + 24 + side * 4, t.words[6 + side] - uint32_t(t.progress / patch_size));
			}
		}
		// SPRU261 9.5: the upper 64 bytes hold opaque TC state, enough to resume
		// the transfer even from a copy of the table.
		if (tc_guided(t))
		{
			for (unsigned side = 0; side < 2; side++)
			{
				m_program.write_dword(base + 0x40 + side * 4, t.words[2 + side]);
				m_program.write_dword(base + 0x48 + side * 4, t.words[6 + side]);
				for (unsigned i = 0; i < 4; i++)
					m_program.write_dword(base + 0x54 + side * 16 + i * 4, t.cursor[side][i]);
			}
			m_program.write_dword(base + 0x50, t.words[0]);
			m_program.write_dword(base + 0x74, t.progress >> 32);
			m_program.write_dword(base + 0x78, t.progress);
			m_program.write_dword(base + 0x7c, unsigned(t.cursor_wrap[0]) | (unsigned(t.cursor_wrap[1]) << 1));
		}
		else
		{
			m_program.write_dword(base + 0x40, t.words[2]);
			m_program.write_dword(base + 0x44, t.words[3]);
			m_program.write_dword(base + 0x48, t.progress >> 32);
			m_program.write_dword(base + 0x4c, t.progress);
			m_program.write_dword(base + 0x50, t.total >> 32);
			m_program.write_dword(base + 0x54, t.total);
			m_program.write_dword(base + 0x58, t.words[6]);
			m_program.write_dword(base + 0x5c, t.words[7]);
			m_program.write_dword(base + 0x60, t.words[0]);
		}
		// An incomplete transparency group goes in the last parameter
		// doubleword, which software doesn't use (SPRU261 6.5.15).
		if (((t.words[1] >> 16) & 7) >= 4)
		{
			m_program.write_dword(base + 0x38, t.transparent_data >> 32);
			m_program.write_dword(base + 0x3c, t.transparent_data);
			m_program.write_dword(base + 0x7c, unsigned(t.cursor_wrap[0]) | (unsigned(t.cursor_wrap[1]) << 1) | (unsigned(t.transparent_mask) << 8));
		}
		m_program.write_dword(tc_pointer(channel), base);
		t.loaded = false;
	}
	if (software)
	{
		m_tc_round_robin[tc_priority(channel) - 1] = channel == 15 ? 0 : channel - 15;
		t.control = (t.control & ~3U) | ((t.control >> 1) & 1);
	}
	if (m_tc_active == int(channel))
		m_tc_active = -1;
}

void tms320c82_mp_device::tc_error(unsigned channel, bool save)
{
	auto &t = m_tc[channel];
	if (channel < 15)
	{
		m_tc_regs[3] = (m_tc_regs[3] & ~0x1e0U) | ((15 - channel) << 5);
		m_intpen |= 1U << 14;
	}
	else
	{
		if (save)
			tc_suspend(channel, false);
		tc_signal(channel, 1);
		m_tc_round_robin[tc_priority(channel) - 1] = channel == 15 ? 0 : channel - 15;
	}
	t.control &= ~3U;
	t.loaded = false;
	m_tc_active = -1;
}

bool tms320c82_mp_device::tc_load(unsigned channel)
{
	auto &t = m_tc[channel];
	uint32_t const address = m_program.read_dword(tc_pointer(channel));
	if ((address & 15) || !tc_ram(address) || !tc_ram(address + 15))
	{
		tc_error(channel, false);
		return false;
	}
	t.descriptor = address;
	t.words[0] = m_program.read_dword(address);
	t.words[1] = m_program.read_dword(address + 4);
	unsigned const count = BIT(t.words[1], 20) ? 4 : 16;
	if ((count == 16 && (address & 63)) || !tc_ram(address + count * 4 - 1))
	{
		tc_error(channel, false);
		return false;
	}
	for (unsigned i = 2; i < count; i++)
		t.words[i] = m_program.read_dword(address + i * 4);
	for (unsigned i = count; i < 16; i++)
		t.words[i] = 0;
	t.progress = t.total = t.elapsed = 0;
	t.transparent_data = t.transparent_mask = 0;
	for (auto &cursor : t.cursor)
		std::fill(std::begin(cursor), std::end(cursor), 0);
	std::fill(std::begin(t.cursor_wrap), std::end(t.cursor_wrap), false);
	t.loaded = true;
	if (t.words[1] & 0x60000000)
	{
		if ((address & 127) || !tc_ram(address + 127))
		{
			tc_error(channel, false);
			return false;
		}
		if (((t.words[1] >> 16) & 7) >= 4)
		{
			t.transparent_data = (uint64_t(t.words[14]) << 32) | t.words[15];
			t.transparent_mask = m_program.read_dword(address + 0x7c) >> 8;
		}
		t.descriptor = t.words[0];
		if (tc_guided(t))
		{
			t.words[0] = m_program.read_dword(address + 0x50);
			t.words[1] &= ~0x60000000U;
			for (unsigned side = 0; side < 2; side++)
			{
				t.words[2 + side] = m_program.read_dword(address + 0x40 + side * 4);
				t.words[6 + side] = m_program.read_dword(address + 0x48 + side * 4);
				for (unsigned i = 0; i < 4; i++)
					t.cursor[side][i] = m_program.read_dword(address + 0x54 + side * 16 + i * 4);
			}
			t.progress = (uint64_t(m_program.read_dword(address + 0x74)) << 32) | m_program.read_dword(address + 0x78);
			t.total = ~uint64_t(0);
			t.cursor_wrap[0] = BIT(m_program.read_dword(address + 0x7c), 0);
			t.cursor_wrap[1] = BIT(m_program.read_dword(address + 0x7c), 1);
			for (unsigned side = 0; side < 2; side++)
			{
				unsigned const mode = (t.words[1] >> (side ? 4 : 12)) & 7;
				if (mode >= 3)
				{
					// Recover the guide origin from its exported current pointer.
					unsigned const stride = (mode == 4 || mode == 5) ? 8 : 4;
					uint32_t const advance = uint32_t(tc_completed_patches(t, side) * stride);
					t.words[10 + side] += BIT(t.words[1], side ? 25 : 22) ? advance : -advance;
				}
			}
			return true;
		}
		t.words[0] = m_program.read_dword(address + 0x60);
		t.words[1] &= ~0x60000000U;
		t.words[2] = m_program.read_dword(address + 0x40);
		t.words[3] = m_program.read_dword(address + 0x44);
		t.progress = (uint64_t(m_program.read_dword(address + 0x48)) << 32) | m_program.read_dword(address + 0x4c);
		t.total = (uint64_t(m_program.read_dword(address + 0x50)) << 32) | m_program.read_dword(address + 0x54);
		t.words[6] = m_program.read_dword(address + 0x58);
		t.words[7] = m_program.read_dword(address + 0x5c);
		return true;
	}
	unsigned const source = BIT(t.words[1], 19), destination = source ^ 1;
	unsigned const pam = (t.words[1] >> 16) & 7;
	bool const short_form = BIT(t.words[1], 20);
	unsigned const stm = short_form ? 0 : (t.words[1] >> 12) & 7, dtm = short_form ? 0 : (t.words[1] >> 4) & 7;
	if ((short_form && pam >= 4) || (!short_form && pam == 3))
	{
		tc_error(channel, true);
		return false;
	}
	if (short_form && (pam == 2 || pam == 3))
	{
		// SPRU261 5.4.1/5.4.4: SU requests one read then updates by Count;
		// otherwise Count reads repeat at the unchanged source address.
		if (tc_ram(t.words[2 + source]))
		{
			tc_error(channel, true);
			return false;
		}
		t.total = BIT(t.words[1], source ? 25 : 22) ? 1 : (t.words[1] & 0xffff);
		return true;
	}
	if (pam == 2 && (channel < 15 || tc_ram(t.words[2 + destination]) || (t.words[2 + destination] & 63)))
	{
		// SPRU261 8.1: block-write needs an off-chip, 64-byte-aligned initial
		// destination (later rows may start within a block); long-form XPT
		// block-write is illegal (9.4).
		tc_error(channel, true);
		return false;
	}
	unsigned const src_mode = source ? dtm : stm, dst_mode = source ? stm : dtm;
	if (src_mode == 2 || (dst_mode && dst_mode < 4) || (source && (src_mode == 1 || src_mode == 3)))
	{
		tc_error(channel, true);
		return false;
	}
	if (tc_guided(t))
	{
		for (unsigned side = 0; side < 2; side++)
		{
			unsigned const mode = side ? dtm : stm;
			if (mode >= 3)
			{
				unsigned const stride = (mode == 4 || mode == 5) ? 8 : 4;
				uint32_t const pointer = t.words[10 + side];
				if ((pointer & (stride - 1)) || !tc_ram(pointer) || !tc_ram(pointer + stride - 1))
				{
					// Initial guide-address errors precede transfer execution (SPRU261 9.4).
					tc_error(channel, false);
					return false;
				}
				if (!t.words[6 + side])
				{
					tc_error(channel, true);
					return false;
				}
			}
			t.cursor_wrap[side] = false;
			t.cursor[side][0] = t.words[2 + side];
			t.cursor[side][1] = t.words[4 + side];
			t.cursor[side][2] = 0;
			t.cursor[side][3] = mode >= 3 ? 0 : 1;
		}
	}
	bool const fill = !short_form && stm == 1;
	// SPRU261 6.6.6: exchanging a fill or unequal-length normal transfer
	// is a packet error, even if the exchanged destination would be shorter.
	if (fill && source)
	{
		tc_error(channel, true);
		return false;
	}
	if (pam == 1)
	{
		bool const read = t.words[2 + source] && !t.words[2 + destination];
		bool const write = !t.words[2 + source] && t.words[2 + destination];
		if ((!read && !write) || fill || tc_ram(t.words[2 + (read ? source : destination)]))
		{
			tc_error(channel, false);
			return false;
		}
		t.total = tc_length(t, read ? source : destination);
	}
	else
	{
		t.total = tc_length(t, destination);
		if (!fill && t.total != ~uint64_t(0) && tc_length(t, source) != ~uint64_t(0) && (t.total > tc_length(t, source) || (source && t.total != tc_length(t, source))))
		{
			tc_error(channel, true);
			return false;
		}
	}
	if (pam >= 4 && !tc_guided(t) && tc_ram(t.words[2 + destination]))
	{
		tc_error(channel, true);
		return false;
	}
	return true;
}

// SPRU261 6.5.13 and 8.2: compare after aligning source bytes to the
// destination bus lanes. A matching group inhibits writes, not source reads.
void tms320c82_mp_device::tc_transparency_flush(tc_channel &t)
{
	if (!t.transparent_mask)
		return;
	unsigned const destination = BIT(t.words[1], 19) ^ 1;
	auto previous = t;
	if (tc_guided(t))
		--previous.cursor[destination][2];
	uint32_t const base = tc_address(previous, destination, t.progress - 1) & ~7U;
	uint64_t const transparent = (uint64_t(t.words[12]) << 32) | t.words[13];
	unsigned const size = 1U << (((t.words[1] >> 16) & 7) - 4);
	uint64_t strobes = 0;
	for (unsigned lane = 0; lane < 8; lane++)
	{
		if (BIT(t.transparent_mask, lane))
			strobes |= uint64_t(0xff) << (lane * 8);
	}
	for (unsigned lane = 0; lane < 8; lane += size)
	{
		uint64_t const group = (uint64_t(~0ULL) >> (64 - size * 8)) << (lane * 8);
		// Inactive byte lanes cannot cause writes, or affect another pixel.
		if (!((t.transparent_data ^ transparent) & group & strobes))
			strobes &= ~group;
	}
	if (strobes >> 32)
		m_program.write_dword(base, t.transparent_data >> 32, strobes >> 32);
	if (uint32_t(strobes))
		m_program.write_dword(base + 4, t.transparent_data, strobes);
	t.transparent_data = t.transparent_mask = 0;
}

unsigned tms320c82_mp_device::tc_step(unsigned channel)
{
	auto &t = m_tc[channel];
	bool const guided = tc_guided(t);
	if (!guided && t.progress == t.total)
	{
		tc_finish(channel);
		return 1;
	}
	uint32_t const options = t.words[1];
	unsigned const source = BIT(options, 19), destination = source ^ 1;
	unsigned const pam = (options >> 16) & 7;
	if (BIT(options, 20) && (pam == 2 || pam == 3))
	{
		m_vram_read_cb(t.words[2 + source], pam == 2);
		++t.progress;
		++t.elapsed;
		if (t.progress == t.total)
			tc_finish(channel);
		return 1; // TODO: VRAM cycle timing
	}
	bool const peripheral = ((options >> 16) & 7) == 1;
	bool const peripheral_read = peripheral && !t.words[2 + destination];
	bool const peripheral_write = peripheral && !t.words[2 + source];
	bool const fill = !BIT(options, 20) && ((options >> 12) & 7) == 1;
	if (guided)
	{
		unsigned const terminating = peripheral_read ? source : destination;
		int const dst_ready = tc_prepare(channel, terminating);
		if (dst_ready < 0)
			return 1;
		if (dst_ready == 2)
		{
			++t.elapsed;
			return 1;
		}
		if (!dst_ready)
		{
			// Exchange requires equal streams, including variable-size patches.
			if (source && !peripheral)
			{
				int const remaining = tc_prepare(channel, source);
				if (remaining < 0)
					return 1;
				if (remaining == 2)
				{
					++t.elapsed;
					return 1;
				}
				if (remaining)
				{
					tc_error(channel, true);
					return 1;
				}
			}
			tc_finish(channel);
			return 1;
		}
		if (!fill && !peripheral)
		{
			int const src_ready = tc_prepare(channel, source);
			if (src_ready < 0)
				return 1;
			if (src_ready == 2)
			{
				++t.elapsed;
				return 1;
			}
			if (!src_ready)
			{
				tc_error(channel, true);
				return 1;
			}
		}
	}
	uint32_t const sa = tc_address(t, source, t.progress), da = tc_address(t, destination, t.progress);
	if ((peripheral_read && tc_ram(sa)) || (peripheral_write && tc_ram(da)))
	{
		// A later guide or address update can leave the external memory bus.
		tc_error(channel, true);
		return 1;
	}
	if (pam == 2)
	{
		if (tc_ram(da) || tc_ram(da + 7))
		{
			tc_error(channel, true);
			return 1;
		}
		// Simulated block-write, also used for 4x/8x VRAM block cycles (SPRU261
		// 8.1.5): each mask bit, MSB first, enables the write strobe of one
		// destination byte. Color bytes stay aligned to the 64-bit bus.
		unsigned const lane = (da >> 3) & 7;
		uint8_t mask;
		if (fill)
			mask = t.words[(lane & 4) ? 8 : 10] >> (8 * (3 - (lane & 3)));
		else
			mask = m_program.read_dword(sa & ~3U, 0xff000000U >> (8 * (sa & 3))) >> (8 * (3 - (sa & 3)));
		for (unsigned word = 0; word < 3; word++)
		{
			uint32_t const address = (da & ~3U) + word * 4;
			uint32_t strobes = 0;
			for (unsigned bit = 0; bit < 8; bit++)
			{
				if (BIT(mask, 7 - bit) && ((da + bit) & ~3U) == address)
					strobes |= 0xff000000U >> (8 * ((da + bit) & 3));
			}
			if (strobes)
				m_program.write_dword(address, t.words[12 + ((address >> 2) & 1)], strobes);
		}
		++t.progress;
		if (guided)
		{
			if (!fill)
				++t.cursor[source][2];
			++t.cursor[destination][2];
		}
		unsigned const cycles = !fill && !tc_ram(sa) ? 2 : 1;
		t.elapsed += cycles;
		if (!guided && t.progress == t.total)
			tc_finish(channel);
		return cycles;
	}
	bool const transparency = pam >= 4;
	if (transparency && tc_ram(da))
	{
		tc_error(channel, true);
		return 1;
	}
	bool const reverse = BIT(options, 23);
	unsigned count = guided ? 4 : unsigned(std::min<uint64_t>(4, t.total - t.progress));
	auto limit = [&count, &t, reverse, options, guided] (unsigned side, uint32_t address)
	{
		count = std::min(count, reverse ? (address & 3) + 1 : 4 - (address & 3));
		if (!BIT(options, 20))
		{
			uint32_t const a = (guided ? t.cursor[side][1] : t.words[4 + side]) & 0xffff;
			if (a)
				count = std::min(count, a - uint32_t((guided ? t.cursor[side][2] : t.progress) % a));
		}
	};
	if (!peripheral_write && !fill)
		limit(source, sa);
	if (!peripheral_read)
		limit(destination, da);
	uint32_t const low_src = reverse ? sa - count + 1 : sa, low_dst = reverse ? da - count + 1 : da;
	unsigned const src_shift = 8 * (4 - (low_src & 3) - count), dst_shift = 8 * (4 - (low_dst & 3) - count);
	uint32_t const bytes = 0xffffffffU >> (32 - count * 8);
	uint32_t data;
	if (peripheral_write)
	{
		data = m_peripheral_write_cb(low_dst & ~3U, bytes << dst_shift);
		m_program.write_dword(low_dst & ~3U, data, bytes << dst_shift);
	}
	else
	{
		if (fill)
			data = (t.words[(low_dst & 4) ? 8 : 10] >> dst_shift) & bytes;
		else
			data = m_program.read_dword(low_src & ~3U, bytes << src_shift);
		if (peripheral_read)
		{
			m_peripheral_read_cb(low_src & ~3U, data, bytes << src_shift);
		}
		else
		{
			if (!fill)
				data = (data >> src_shift) & bytes;
			if (transparency)
			{
				unsigned const shift = 8 * (8 - (low_dst & 7) - count);
				t.transparent_data |= uint64_t(data) << shift;
				t.transparent_mask |= ((1U << count) - 1) << (shift / 8);
			}
			else
			{
				m_program.write_dword(low_dst & ~3U, data << dst_shift, bytes << dst_shift);
			}
		}
	}
	t.progress += count;
	if (guided)
	{
		if (!fill && !peripheral_write)
			t.cursor[source][2] += count;
		if (!peripheral_read)
			t.cursor[destination][2] += count;
	}
	if (transparency)
	{
		uint32_t const a = (guided ? t.cursor[destination][1] : t.words[4 + destination]) & 0xffff;
		uint64_t const position = guided ? t.cursor[destination][2] : t.progress;
		bool const row_end = a && !(position % a);
		bool const bus_end = reverse ? !(low_dst & 7) : !((da + count) & 7);
		if (row_end || bus_end || (!guided && t.progress == t.total))
			tc_transparency_flush(t);
	}
	// Two external accesses share the data bus and two on-chip accesses share
	// the TC crossbar port (SPRU261 9.1.1).
	// TODO: external wait states, refresh and page timing
	unsigned const cycles = !peripheral && !fill && tc_ram(sa) == tc_ram(da) ? 2 : 1;
	t.elapsed += cycles;
	if (!guided && t.progress == t.total)
		tc_finish(channel);
	return cycles;
}

void tms320c82_mp_device::tc_finish(unsigned channel)
{
	auto &t = m_tc[channel];
	uint32_t options = t.words[1];
	for (unsigned side = 0; side < 2; side++)
	{
		if (BIT(options, 20))
		{
			if (BIT(options, side ? 25 : 22))
				m_program.write_dword(t.descriptor + 8 + side * 4, t.words[2 + side] + (BIT(options, 23) ? -(options & 0xffff) : options & 0xffff));
		}
		else
		{
			unsigned const mode = (options >> (side ? 0 : 8)) & 3;
			if (!mode)
				continue;
			if (tc_guided(t))
			{
				auto const &c = t.cursor[side];
				uint32_t const a = c[1] & 0xffff;
				uint32_t const row = a && c[2] ? (c[2] - 1) / a : 0;
				uint32_t const pitch = mode == 1 ? t.words[8 + side] * (row + 1) : t.words[10 + side];
				bool const reverse = BIT(options, mode == 1 ? (side ? 24 : 21) : (side ? 25 : 22));
				m_program.write_dword(t.descriptor + 8 + side * 4, c[0] + (reverse ? -pitch : pitch));
				if (mode == 3)
					options ^= 1U << (side ? 25 : 22);
				continue;
			}
			uint64_t const a = t.words[4 + side] & 0xffff, b = (t.words[4 + side] >> 16) + 1;
			uint64_t const last = t.total ? t.total - 1 : 0;
			uint32_t const row = a ? (last / a) % b : 0, patch = a ? last / (a * b) : 0;
			uint32_t update = t.words[2 + side];
			uint32_t const bp = t.words[8 + side] * (row + 1), cp = t.words[10 + side] * (patch + (mode != 1));
			update += BIT(options, side ? 25 : 22) ? -cp : cp;
			if (mode == 1)
				update += BIT(options, side ? 24 : 21) ? -bp : bp;
			m_program.write_dword(t.descriptor + 8 + side * 4, update);
			if (mode == 3)
				options ^= 1U << (side ? 25 : 22);
		}
	}
	if (options != t.words[1])
		m_program.write_dword(t.descriptor + 4, options);
	m_program.write_dword(tc_pointer(channel), t.words[0]);
	t.loaded = false;
	if (channel >= 15 && (options & 0x90000000))
		tc_signal(channel, 2);
	if (BIT(options, 31))
	{
		if (channel >= 15)
			m_tc_round_robin[tc_priority(channel) - 1] = (channel == 15 ? 0 : channel - 15);
		t.control &= ~2U;
		m_tc_active = -1;
	}
}

TIMER_CALLBACK_MEMBER(tms320c82_mp_device::tc_tick)
{
	for (unsigned i = 15; i < 18; i++)
	{
		if ((m_tc[i].control & 6) == 6)
			tc_suspend(i, true);
	}
	int selected = tc_select();
	if (m_tc_active >= 0)
	{
		unsigned const active = m_tc_active;
		auto &t = m_tc[active];
		bool const higher = selected >= 0 && tc_priority(selected) > tc_priority(active);
		bool const timeout = t.elapsed >= uint64_t(m_tc_regs[1]) + m_tc_regs[2] && t.elapsed;
		// At a linked-list boundary the previous packet has already completed.
		// A higher-priority request wins before the next descriptor is loaded;
		// no suspension block is needed (SPRU261 9.3).
		if (active >= 15 && higher && !t.loaded)
		{
			m_tc_active = -1;
		}
		else if (active >= 15 && t.loaded && t.elapsed >= m_tc_regs[1] && (higher || timeout))
		{
			// Short-form XPTs interrupt long-form transfers without suspending them.
			bool const short_xpt = higher && selected < 15 && BIT(m_program.read_dword(m_program.read_dword(tc_pointer(selected)) + 4), 20);
			if (!short_xpt)
				tc_suspend(active, false);
			else
				m_tc_active = -1;
			if (timeout && !higher)
			{
				m_tc_round_robin[tc_priority(active) - 1] = active == 15 ? 0 : active - 15;
				selected = tc_select();
			}
		}
		else
		{
			selected = active;
		}
	}
	if (selected < 0)
		return;
	m_tc_active = selected;
	if (selected >= 15)
		m_tc_round_robin[tc_priority(selected) - 1] = selected == 15 ? 2 : selected - 16;
	auto &t = m_tc[selected];
	unsigned cycles = 1;
	if (!t.loaded)
	{
		if (tc_load(selected))
			cycles = BIT(t.words[1], 20) ? 3 : 9; // pointer plus 64-bit descriptor words
	}
	else
	{
		cycles = tc_step(selected);
	}
	if (tc_select() >= 0 || m_tc_active >= 0)
		m_tc_timer->adjust(clocks_to_attotime(cycles));
}

uint32_t tms320c82_mp_device::mp_param_r(offs_t offset, uint32_t mem_mask)
{
	//printf("mp_param_w: %08X, %08X\n", offset, mem_mask);
	return m_param_ram[offset];
}

void tms320c82_mp_device::mp_param_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	//printf("mp_param_w: %08X, %08X, %08X\n", offset, data, mem_mask);

	COMBINE_DATA(&m_param_ram[offset]);
}



void tms320c82_mp_device::device_start()
{
	m_cmd_callback.resolve();
	tc_start();

	save_item(NAME(m_pc));
	save_item(NAME(m_fetchpc));
	save_item(NAME(m_reg));
	save_item(NAME(m_acc));

	save_item(NAME(m_in0p));
	save_item(NAME(m_in1p));
	save_item(NAME(m_outp));
	save_item(NAME(m_ie));
	save_item(NAME(m_fpst));
	save_item(NAME(m_config));
	save_item(NAME(m_param_ram));
	save_item(NAME(m_intpen));
	save_item(NAME(m_tcount));
	save_item(NAME(m_tscale));
	save_item(NAME(m_epc));
	save_item(NAME(m_eip));
	save_item(NAME(m_pp_status));

	// Register state for debugger
	state_add(MP_PC, "pc", m_pc).formatstr("%08X");

	state_add(MP_R0, "r0", m_reg[0]).formatstr("%08X");
	state_add(MP_R1, "r1", m_reg[1]).formatstr("%08X");
	state_add(MP_R2, "r2", m_reg[2]).formatstr("%08X");
	state_add(MP_R3, "r3", m_reg[3]).formatstr("%08X");
	state_add(MP_R4, "r4", m_reg[4]).formatstr("%08X");
	state_add(MP_R5, "r5", m_reg[5]).formatstr("%08X");
	state_add(MP_R6, "r6", m_reg[6]).formatstr("%08X");
	state_add(MP_R7, "r7", m_reg[7]).formatstr("%08X");
	state_add(MP_R8, "r8", m_reg[8]).formatstr("%08X");
	state_add(MP_R9, "r9", m_reg[9]).formatstr("%08X");
	state_add(MP_R10, "r10", m_reg[10]).formatstr("%08X");
	state_add(MP_R11, "r11", m_reg[11]).formatstr("%08X");
	state_add(MP_R12, "r12", m_reg[12]).formatstr("%08X");
	state_add(MP_R13, "r13", m_reg[13]).formatstr("%08X");
	state_add(MP_R14, "r14", m_reg[14]).formatstr("%08X");
	state_add(MP_R15, "r15", m_reg[15]).formatstr("%08X");
	state_add(MP_R16, "r16", m_reg[16]).formatstr("%08X");
	state_add(MP_R17, "r17", m_reg[17]).formatstr("%08X");
	state_add(MP_R18, "r18", m_reg[18]).formatstr("%08X");
	state_add(MP_R19, "r19", m_reg[19]).formatstr("%08X");
	state_add(MP_R20, "r20", m_reg[20]).formatstr("%08X");
	state_add(MP_R21, "r21", m_reg[21]).formatstr("%08X");
	state_add(MP_R22, "r22", m_reg[22]).formatstr("%08X");
	state_add(MP_R23, "r23", m_reg[23]).formatstr("%08X");
	state_add(MP_R24, "r24", m_reg[24]).formatstr("%08X");
	state_add(MP_R25, "r25", m_reg[25]).formatstr("%08X");
	state_add(MP_R26, "r26", m_reg[26]).formatstr("%08X");
	state_add(MP_R27, "r27", m_reg[27]).formatstr("%08X");
	state_add(MP_R28, "r28", m_reg[28]).formatstr("%08X");
	state_add(MP_R29, "r29", m_reg[29]).formatstr("%08X");
	state_add(MP_R30, "r30", m_reg[30]).formatstr("%08X");
	state_add(MP_R31, "r31", m_reg[31]).formatstr("%08X");

	state_add(MP_ACC0, "acc0", m_acc[0]).formatstr("%016X");
	state_add(MP_ACC1, "acc1", m_acc[1]).formatstr("%016X");
	state_add(MP_ACC2, "acc2", m_acc[2]).formatstr("%016X");
	state_add(MP_ACC3, "acc3", m_acc[3]).formatstr("%016X");

	state_add(MP_IN0P, "in0p", m_in0p).formatstr("%08X");
	state_add(MP_IN1P, "in1p", m_in1p).formatstr("%08X");
	state_add(MP_OUTP, "outp", m_outp).formatstr("%08X");
	state_add(MP_IE, "ie", m_ie).formatstr("%08X");
	state_add(MP_INTPEN, "intpen", m_intpen).formatstr("%08X");

	state_add(MP_TCOUNT, "tcount", m_tcount).formatstr("%08X");
	state_add(MP_TSCALE, "tscale", m_tscale).formatstr("%08X");

	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();

	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);

	set_icountptr(m_icount);
}

void tms320c82_mp_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = "?";
			break;
	}
}

void tms320c82_mp_device::device_reset()
{
	m_pc = 0;
	m_fetchpc = 0xfffffff8;

	for (int i=0; i < 32; i++)
	{
		m_reg[i] = 0;
	}

	m_acc[0] = 0;
	m_acc[1] = 0;
	m_acc[2] = 0;
	m_acc[3] = 0;

	m_in0p = 0;
	m_in1p = 0;
	m_outp = 0;

	m_intpen = 0;
	m_ie = 0;
	m_fpst = 1U << 19; // IEEE mode, round to nearest (SPRU109 3.3.3)
	// Type 0010: two PPs; Deriv 0010: TMS320C82 (SPRS048 figure 10);
	// Release 0011: production silicon
	m_config = 0x2230;
	// Timer contents are undefined at power-on.
	m_tcount = m_tscale = 0;
	m_epc = m_eip = 0;

	tc_reset();
	m_pp_status = 0x30000;      // C82 has two PPs, both halted at reset
}

void tms320c82_mp_device::processor_command(uint32_t command)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(tms320c82_mp_device::processor_command_sync), this), command);
}

TIMER_CALLBACK_MEMBER(tms320c82_mp_device::processor_command_sync)
{
	uint32_t const command = param;
	// Reset dominates unhalt; otherwise unhalt dominates halt (SPRU110 3.2).
	if (BIT(command, 31))
		m_pp_status |= (command & 3) << 16;
	else if (BIT(command, 29))
		m_pp_status &= ~((command & 3) << 16);
	else if (BIT(command, 30))
		m_pp_status |= (command & 3) << 16;
	if (BIT(command, 13) && BIT(command, 8))
		m_intpen |= 1U << 25;
	for (auto &pp : m_pp)
		pp->command(command);
	if (!m_cmd_callback.isnull())
		m_cmd_callback(space(AS_PROGRAM), command);
}

void tms320c82_mp_device::pp_command(unsigned processor, uint32_t command)
{
	// Commands to the sending PP itself take effect at once; the rest are
	// delivered at the send time.
	if (BIT(command, processor) && BIT(command, 30))
		m_pp[processor]->command(0x40000000U | (1U << processor));
	if (BIT(command, 13) && BIT(command, processor))
		m_pp[processor]->signal_interrupt(1U << (24 + processor));

	// Reset, unhalt, task and TC fields of PP commands are ignored. Bit 16 of
	// the payload holds the sender.
	uint32_t const payload = (command & 0x40002103U) | (processor << 16);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(tms320c82_mp_device::pp_command_sync), this), payload);
}

TIMER_CALLBACK_MEMBER(tms320c82_mp_device::pp_command_sync)
{
	unsigned const processor = BIT(param, 16);
	if (BIT(param, processor) && BIT(param, 30))
		m_pp_status |= 1U << (16 + processor);
	if (BIT(param, 13))
	{
		if (BIT(param, 8))
			m_intpen |= 1U << (16 + processor);
		unsigned const recipient = processor ^ 1;
		if (BIT(param, recipient))
			m_pp[recipient]->signal_interrupt(1U << (24 + processor));
	}
}

uint32_t tms320c82_mp_device::read_creg(int reg)
{
	switch (reg)
	{
		case 0x0:           // EPC
			return m_epc;

		case 0x1:           // EIP
			return m_eip;

		case 0x2:           // CONFIG
			return m_config;

		case 0x4:           // INTPEN
			return m_intpen;

		case 0x6:           // IE
			return m_ie;

		case 0x8:           // FPST
			return m_fpst;

		case 0xa:           // PPERROR
			return m_pp_status;

		case 0xd:           // PKTREQ
			return packet_status(2);

		case 0xe:           // TCOUNT
			return m_tcount;

		case 0xf:           // TSCALE
			return m_tscale;

		case 0x4000:        // IN0P
			return m_in0p;

		case 0x4001:        // IN1P
			return m_in1p;

		case 0x4002:        // OUTP
			return m_outp;

		default:
			//printf("read_creg(): %08X\n", reg);
			break;
	}
	return 0;
}

void tms320c82_mp_device::write_creg(int reg, uint32_t data)
{
	switch (reg)
	{
		case 0x0:           // EPC
			m_epc = data;
			break;

		case 0x1:           // EIP
			m_eip = data;
			break;

		case 0x2:           // CONFIG: only R, T, H and X are writable (SPRU109 3.6)
			m_config = (m_config & ~0x78000000U) | (data & 0x78000000U);
			break;

		case 0x4:           // INTPEN
		{
			for (int i=0; i < 32; i++)
			{
				if (data & (1 << i))
					m_intpen &= ~(1 << i);
			}
			break;
		}

		case 0x6:           // IE
			m_ie = data;
			break;

		case 0x8:           // FPST
			// SPRU109 3.3.3: only bits 17..26 are writable.
			// TODO: exception reporting, fast mode and stalls
			m_fpst = (m_fpst & ~0x07fe0000U) | (data & 0x07fe0000U);
			break;

		case 0xd:           // PKTREQ
			packet_control(2, data);
			break;

		case 0xe:           // TCOUNT
			m_tcount = data;
			break;

		case 0xf:           // TSCALE
			m_tscale = data;
			break;

		case 0x4000:        // IN0P
			m_in0p = data;
			break;

		case 0x4001:        // IN1P
			m_in1p = data;
			break;

		case 0x4002:        // OUTP
			m_outp = data;
			break;

		default:
			//printf("write_creg(): %08X, %08X\n", reg, data);
			break;
	}
}

void tms320c82_mp_device::check_interrupts()
{
	uint32_t const pending = m_ie & m_intpen & ~1U;
	if ((m_ie & 1) && pending)
	{
		// SPRU109: lower numbered enabled interrupts have higher priority.
		unsigned const interrupt = std::countr_zero(pending);
		m_epc = (m_fetchpc & ~3);
		m_epc |= (m_ie & 1); // save global interrupt mask
		// TODO: user mode bit to EPC

		m_eip = m_pc;
		m_ie &= ~1U; // clear global interrupt mask
		m_fetchpc = m_pc = m_program.read_dword(0x01010180 + (interrupt * 4));
	}
}

void tms320c82_mp_device::execute_set_input(int inputnum, int state)
{
	if (state == ASSERT_LINE)
	{
		switch (inputnum)
		{
			case INPUT_X1:
				m_intpen |= (1 << 11);
				break;
			case INPUT_X2:
				m_intpen |= (1 << 12);
				break;
			case INPUT_X3:
				m_intpen |= (1 << 29);
				break;
			case INPUT_X4:
				m_intpen |= (1 << 30);
				break;
			default:
				break;
		}
	}
}

uint32_t tms320c82_mp_device::fetch()
{
	uint32_t w = m_cache.read_dword(m_fetchpc);
	m_fetchpc += 4;
	return w;
}

void tms320c82_mp_device::clock_timer()
{
	// SPRU109 3.5: zero lasts one clock before reload and IRQ10. A reload
	// value of N therefore gives a period of N+1 processor clocks.
	if (m_tcount)
	{
		--m_tcount;
	}
	else
	{
		m_tcount = m_tscale;
		m_intpen |= 1U << 10;
	}
}

void tms320c82_mp_device::delay_slot()
{
	debugger_instruction_hook(m_pc);
	m_ir = fetch();
	clock_timer();
	execute();

	m_icount--;
}

void tms320c82_mp_device::execute_run()
{
	// Idle loops made only of aligned dword loads from RAM/ROM and annulled
	// branches repeat exactly while the register file is unchanged, so whole
	// periods can be skipped up to the end of the timeslice.
	uint32_t poll_regs[32], poll_pc = 0;
	int poll_count = 0;
	bool candidate = false, previous_safe = false;
	while (m_icount > 0)
	{
		m_pc = m_fetchpc;
		if ((m_ie & 1) && (m_ie & m_intpen & ~1U))
			candidate = false;
		check_interrupts();
		if (!(machine().debug_flags & DEBUG_FLAG_ENABLED))
		{
			if ((!candidate || poll_count - m_icount > 16) && previous_safe)
			{
				std::copy(std::begin(m_reg), std::end(m_reg), poll_regs);
				poll_pc = m_fetchpc;
				poll_count = m_icount;
				candidate = true;
			}
			else if (candidate && m_fetchpc == poll_pc && std::equal(std::begin(m_reg), std::end(m_reg), poll_regs))
			{
				unsigned const period = poll_count - m_icount;
				uint64_t budget = m_icount;
				// Stop before an enabled timer interrupt can become pending.
				if ((m_ie & 0x401) == 0x401)
					budget = std::min<uint64_t>(budget, m_tcount);
				unsigned const skipped = (budget / period) * period;
				if (skipped <= m_tcount)
				{
					m_tcount -= skipped;
				}
				else
				{
					uint64_t const after_reload = uint64_t(skipped) - m_tcount - 1;
					m_tcount = m_tscale - (after_reload % (uint64_t(m_tscale) + 1));
					m_intpen |= 1U << 10;
				}
				m_icount -= skipped;
				if (!m_icount)
					return;
				candidate = false;
			}
		}
		debugger_instruction_hook(m_pc);
		m_ir = fetch();
		previous_safe = true;
		unsigned const op = (m_ir >> 15) & 0x7f;
		if (((m_ir >> 20) & 3) == 3)
		{
			previous_safe = false;
		}
		else if (op == 0x22 || op == 0x26)
		{
			uint32_t const address = m_reg[(m_ir >> 22) & 31] + util::sext(m_ir, 15);
			if ((address & 3) || !space(AS_PROGRAM).get_read_ptr(address))
				previous_safe = false;
		}
		else if (op != 0x49 && op != 0x4b && op != 0x4d)
		{
			previous_safe = false;
		}
		if (previous_safe && !space(AS_PROGRAM).get_read_ptr(m_pc))
			previous_safe = false;
		candidate &= previous_safe;
		clock_timer();
		execute();
		m_icount--;
	}
}

// Parallel Processor

// internal memory map
void tms320c82_pp_device::pp_internal_map(address_map &map)
{
	// All processors see the same physical RAM and peripheral bus.
	map(0, 0xffffffff).rw(FUNC(tms320c82_pp_device::bus_r), FUNC(tms320c82_pp_device::bus_w));
}

uint32_t tms320c82_pp_device::bus_r(offs_t offset, uint32_t mem_mask)
{
	return m_program.read_dword(offset * 4, mem_mask);
}

void tms320c82_pp_device::bus_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_program.write_dword(offset * 4, data, mem_mask);
}

tms320c82_pp_device::tms320c82_pp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, unsigned number)
	: cpu_device(mconfig, TMS320C82_PP, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(tms320c82_pp_device::pp_internal_map), this))
	, m_master(*this, DEVICE_SELF_OWNER)
	, m_pp_number(number)
{
}

tms320c82_pp_device::tms320c82_pp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms320c82_pp_device(mconfig, tag, owner, clock, 0)
{
}

std::unique_ptr<util::disasm_interface> tms320c82_pp_device::create_disassembler()
{
	return std::make_unique<tms320c82_pp_disassembler>();
}

void tms320c82_pp_device::device_start()
{
	m_master->space(AS_PROGRAM).specific(m_program);
	m_master->space(AS_PROGRAM).cache(m_cache);
	save_item(NAME(m_pc));
	save_item(NAME(m_fetchpc));
	save_item(NAME(m_regs));
	save_item(NAME(m_pipe_pc));
	save_item(NAME(m_pipe_op));
	save_item(NAME(m_ea));
	save_item(NAME(m_transfer_pending));
	save_item(NAME(m_halt_transfer_ready));
	save_item(NAME(m_transfer_value));
	save_item(NAME(m_halted));
	save_item(NAME(m_reset_pending));
	save_item(NAME(m_executed));
	save_item(NAME(m_protect));
	save_item(NAME(m_pipe_irq));
	save_item(NAME(m_irq_next));
	save_item(NAME(m_irq_vector));
	save_item(NAME(m_irq_pc));
	save_item(NAME(m_irq_sr));
	state_add(PP_PC, "pc", m_pc).formatstr("%08X");
	state_add(PP_CYCLES, "cycles", m_executed).formatstr("%016X");
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	// Registers held in m_regs, named as in SPRS048 table 18
	static constexpr struct { uint8_t code; char const *name; } REGISTERS[] =
	{
		{ 0x00, "a0" }, { 0x01, "a1" }, { 0x02, "a2" }, { 0x03, "a3" }, { 0x04, "a4" }, { 0x06, "sp" },
		{ 0x08, "a8" }, { 0x09, "a9" }, { 0x0a, "a10" }, { 0x0b, "a11" }, { 0x0c, "a12" },
		{ 0x10, "x0" }, { 0x11, "x1" }, { 0x12, "x2" }, { 0x18, "x8" }, { 0x19, "x9" }, { 0x1a, "x10" },
		{ 0x20, "d0" }, { 0x21, "d1" }, { 0x22, "d2" }, { 0x23, "d3" }, { 0x24, "d4" }, { 0x25, "d5" }, { 0x26, "d6" }, { 0x27, "d7" },
		{ 0x29, "sr" }, { 0x2a, "mf" }, { 0x3b, "iprs" }, { 0x3c, "inten" }, { 0x3d, "intflg" }, { 0x3f, "lctl" },
		{ 0x60, "lc0" }, { 0x61, "lc1" }, { 0x62, "lc2" }, { 0x64, "lr0" }, { 0x65, "lr1" }, { 0x66, "lr2" },
		{ 0x70, "ls0" }, { 0x71, "ls1" }, { 0x72, "ls2" }, { 0x74, "le0" }, { 0x75, "le1" }, { 0x76, "le2" }
	};
	for (auto const &reg : REGISTERS)
		state_add(PP_REGS + reg.code, reg.name, m_regs[reg.code]).formatstr("%08X");
	set_icountptr(m_icount);
}

void tms320c82_pp_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	if (entry.index() == STATE_GENFLAGS)
		str = string_format("%c%c%c%c", BIT(m_regs[0x29], 31) ? 'N' : '-', BIT(m_regs[0x29], 30) ? 'C' : '-', BIT(m_regs[0x29], 29) ? 'V' : '-', BIT(m_regs[0x29], 28) ? 'Z' : '-');
}

void tms320c82_pp_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	std::fill(std::begin(m_pipe_pc), std::end(m_pipe_pc), 0);
	std::fill(std::begin(m_pipe_op), std::end(m_pipe_op), 0x8800000000104100ULL);
	std::fill(std::begin(m_ea), std::end(m_ea), 0);
	m_regs[0x3e] = m_pp_number;
	m_transfer_pending = false;
	m_halt_transfer_ready = false;
	m_transfer_value[0] = m_transfer_value[1] = 0;
	m_pc = m_fetchpc = 0;
	m_halted = m_reset_pending = true;
	m_executed = 0;
	m_protect = m_irq_next = m_irq_vector = m_irq_sr = 0;
	std::fill(std::begin(m_pipe_irq), std::end(m_pipe_irq), 0);
	std::fill(std::begin(m_irq_pc), std::end(m_irq_pc), 0);
}
