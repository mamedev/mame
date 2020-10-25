// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Panasonic MN1880 family

    Currently this device is just a stub with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "mn1880.h"
#include "mn1880d.h"

// device type definitions
DEFINE_DEVICE_TYPE(MN1880, mn1880_device, "mn1880", "Panasonic MN1880")

mn1880_device::mn1880_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, MN1880, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 8, 16, 0)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_ip(0)
	, m_fs(0)
	, m_xp(0)
	, m_yp(0)
	, m_sp(0)
	, m_lp(0)
	, m_icount(0)
{
}

std::unique_ptr<util::disasm_interface> mn1880_device::create_disassembler()
{
	return std::make_unique<mn1880_disassembler>();
}

device_memory_interface::space_config_vector mn1880_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

void mn1880_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_DATA).specific(m_data);
	set_icountptr(m_icount);

	state_add(MN1880_IP, "IP", m_ip);
	state_add(STATE_GENPC, "GENPC", m_ip).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_ip).noshow();
	state_add(MN1880_FS, "FS", m_fs);
	state_add(STATE_GENFLAGS, "FLAGS", m_fs).formatstr("%10s").noshow();
	state_add(MN1880_XP, "XP", m_xp);
	state_add(MN1880_YP, "YP", m_yp);
	state_add<u8>(MN1880_XPL, "XPl",
		[this] () { return m_xp & 0x00ff; },
		[this] (u8 data) { m_xp = (m_xp & 0xff00) | data; }
	).noshow();
	state_add<u8>(MN1880_XPH, "XPh",
		[this] () { return (m_xp & 0xff00) >> 8; },
		[this] (u8 data) { m_xp = (m_xp & 0x00ff) | u16(data) << 8; }
	).noshow();
	state_add<u8>(MN1880_YPL, "YPl",
		[this] () { return m_yp & 0x00ff; },
		[this] (u8 data) { m_yp = (m_yp & 0xff00) | data; }
	).noshow();
	state_add<u8>(MN1880_YPH, "YPh",
		[this] () { return (m_yp & 0xff00) >> 8; },
		[this] (u8 data) { m_yp = (m_yp & 0x00ff) | u16(data) << 8; }
	).noshow();
	state_add(MN1880_SP, "SP", m_sp);
	state_add(MN1880_LP, "LP", m_lp);

	save_item(NAME(m_ip));
	save_item(NAME(m_fs));
	save_item(NAME(m_xp));
	save_item(NAME(m_yp));
	save_item(NAME(m_sp));
	save_item(NAME(m_lp));
}

void mn1880_device::device_reset()
{
	m_fs &= 0xc0; // CF & ZF might or might not be cleared as well
	m_sp = 0x0100;
	// TBD: is LP also initialized at reset?
}

void mn1880_device::execute_run()
{
	m_ip = m_cache.read_word(0);

	debugger_instruction_hook(m_ip);

	m_icount = 0;
}

void mn1880_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c:RC=%-2d",
				BIT(m_fs, 7) ? 'C' : '.', // Carry flag
				BIT(m_fs, 6) ? 'Z' : '.', // Zero flag
				BIT(m_fs, 5) ? 'D' : '.', // Direct flag
				BIT(m_fs, 4) ? 'A' : '.', // Auto-repeat flag
				m_fs & 0x0f);
		break;
	}
}
