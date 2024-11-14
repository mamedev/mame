// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    SGS-Thomson ST905x series

    Currently this device is just a stub with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "st905x.h"
#include "st9dasm.h"

// device type definitions
DEFINE_DEVICE_TYPE(ST90R50, st90r50_device, "st90r50", "SGS-Thomson ST90R50")

st9_device::st9_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor regmap)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 8, 16, 0)
	, m_data_config("data", ENDIANNESS_BIG, 8, 16, 0)
	, m_register_config("register", ENDIANNESS_BIG, 8, 8, 0, regmap)
	, m_pc(0)
	, m_sspr(0)
	, m_uspr(0)
	, m_cicr(0)
	, m_flagr(0)
	, m_rpr{0, 0}
	, m_ppr(0)
	, m_moder(0)
	, m_icount(0)
{
}

st90r50_device::st90r50_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: st9_device(mconfig, ST90R50, tag, owner, clock, address_map_constructor(FUNC(st90r50_device::register_map), this))
{
}

void st90r50_device::register_map(address_map &map)
{
	map(0x00, 0xdf).ram();
	map(0xe6, 0xe6).rw(FUNC(st90r50_device::cicr_r), FUNC(st90r50_device::cicr_w));
	map(0xe7, 0xe7).rw(FUNC(st90r50_device::flagr_r), FUNC(st90r50_device::flagr_w));
	map(0xe8, 0xe9).rw(FUNC(st90r50_device::rpr_r), FUNC(st90r50_device::rpr_w));
	map(0xea, 0xea).rw(FUNC(st90r50_device::ppr_r), FUNC(st90r50_device::ppr_w));
	map(0xeb, 0xeb).rw(FUNC(st90r50_device::moder_r), FUNC(st90r50_device::moder_w));
	map(0xec, 0xef).rw(FUNC(st90r50_device::spr_r), FUNC(st90r50_device::spr_w));
}

std::unique_ptr<util::disasm_interface> st9_device::create_disassembler()
{
	return std::make_unique<st9_disassembler>();
}

device_memory_interface::space_config_vector st9_device::memory_space_config() const
{
	if (has_configured_map(AS_DATA))
	{
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_DATA,    &m_data_config),
			std::make_pair(AS_IO,      &m_register_config)
		};
	}
	else
	{
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_IO,      &m_register_config)
		};
	}
}

u8 st9_device::cicr_r()
{
	return m_cicr;
}

void st9_device::cicr_w(u8 data)
{
	m_cicr = data;
}

u8 st9_device::flagr_r()
{
	return m_flagr;
}

void st9_device::flagr_w(u8 data)
{
	m_flagr = data;
}

u8 st9_device::rpr_r(offs_t offset)
{
	return m_rpr[offset];
}

void st9_device::rpr_w(offs_t offset, u8 data)
{
	m_rpr[offset] = data & 0xfc;
	if (BIT(data, 2))
		m_rpr[offset ^ 1] |= 0x04;
	else
		m_rpr[offset ^ 1] &= 0xf8;
}

u8 st9_device::ppr_r()
{
	return m_ppr;
}

void st9_device::ppr_w(u8 data)
{
	m_ppr = data & 0xfc;
}

u8 st9_device::moder_r()
{
	return m_moder;
}

void st9_device::moder_w(u8 data)
{
	m_moder = data;
}

u8 st9_device::spr_r(offs_t offset)
{
	return BIT(BIT(offset, 1) ? m_sspr : m_uspr, BIT(offset, 0) ? 0 : 8, 8);
}

void st9_device::spr_w(offs_t offset, u8 data)
{
	u16 &spr = BIT(offset, 1) ? m_sspr : m_uspr;
	if (BIT(offset, 0))
		spr = (spr & 0xff00) | data;
	else
		spr = u16(data) << 8 | (spr & 0x00ff);
}

u8 st9_device::debug_register_r(int r)
{
	auto dis = machine().disable_side_effects();
	return m_register.read_byte(BIT(m_rpr[0], 2) ? (m_rpr[BIT(r, 3)] & 0xf8) | (r & 7) : (m_rpr[0] & 0xf0) | r);
}

void st9_device::debug_register_w(int r, u8 data)
{
	auto dis = machine().disable_side_effects();
	m_register.write_byte(BIT(m_rpr[0], 2) ? (m_rpr[BIT(r, 3)] & 0xf8) | (r & 7) : (m_rpr[0] & 0xf0) | r, data);
}

u16 st9_device::debug_rpair_r(int rr)
{
	auto dis = machine().disable_side_effects();
	return m_register.read_word(BIT(m_rpr[0], 2) ? (m_rpr[BIT(rr, 3)] & 0xf8) | (rr & 6) : (m_rpr[0] & 0xf0) | rr);
}

void st9_device::debug_rpair_w(int rr, u16 data)
{
	auto dis = machine().disable_side_effects();
	m_register.write_word(BIT(m_rpr[0], 2) ? (m_rpr[BIT(rr, 3)] & 0xf8) | (rr & 6) : (m_rpr[0] & 0xf0) | rr, data);
}

void st9_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	space(has_space(AS_DATA) ? AS_DATA : AS_PROGRAM).specific(m_data);
	space(AS_IO).specific(m_register);

	set_icountptr(m_icount);

	using namespace std::placeholders;
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(STATE_GENFLAGS, "CURFLAGS", m_flagr).noshow().formatstr("%10s");
	state_add(ST9_PC, "PC", m_pc);
	state_add(ST9_SSPR, "SSPR", m_sspr);
	state_add(ST9_USPR, "USPR", m_uspr);
	state_add(ST9_CICR, "CICR", m_cicr);
	state_add(ST9_FLAGR, "FLAGR", m_flagr);
	state_add(ST9_RP0R, "RP0R", m_rpr[0], std::bind(&st9_device::rpr_w, this, 0, _1)).mask(0xfc);
	state_add(ST9_RP1R, "RP1R", m_rpr[1], std::bind(&st9_device::rpr_w, this, 1, _1)).mask(0xfc);
	state_add(ST9_PPR, "PPR", m_ppr).mask(0xfc);
	state_add(ST9_MODER, "MODER", m_moder);
	for (int i = 0; i < 16; i++)
		state_add<u8>(ST9_R0 + i, util::string_format("r%d", i).c_str(),
						std::bind(&st9_device::debug_register_r, this, i),
						std::bind(&st9_device::debug_register_w, this, i, _1));
	for (int i = 0; i < 16; i += 2)
		state_add<u16>(ST9_RR0 + i / 2, util::string_format("rr%d", i).c_str(),
						std::bind(&st9_device::debug_rpair_r, this, i),
						std::bind(&st9_device::debug_rpair_w, this, i, _1)).noshow();

	save_item(NAME(m_pc));
	save_item(NAME(m_sspr));
	save_item(NAME(m_uspr));
	save_item(NAME(m_cicr));
	save_item(NAME(m_flagr));
	save_item(NAME(m_rpr));
	save_item(NAME(m_ppr));
	save_item(NAME(m_moder));
}

void st9_device::device_reset()
{
	m_cicr = 0x87;
	m_moder = 0xe0;
}

void st9_device::execute_run()
{
	m_pc = m_cache.read_word(0x0000);
	debugger_instruction_hook(m_pc);

	m_icount = 0;
}

void st9_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = util::string_format("%c%c%c%c%c%c%c %cM",
				BIT(m_flagr, 7) ? 'C' : '.',
				BIT(m_flagr, 6) ? 'Z' : '.',
				BIT(m_flagr, 5) ? 'S' : '.',
				BIT(m_flagr, 4) ? 'V' : '.',
				BIT(m_flagr, 3) ? 'D' : '.',
				BIT(m_flagr, 2) ? 'H' : '.',
				BIT(m_flagr, 1) ? 'U' : '.',
				BIT(m_flagr, 0) ? 'D' : 'P');
		break;
	}
}
