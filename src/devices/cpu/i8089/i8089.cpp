// license:GPL-2.0+
// copyright-holders:Dirk Best,Carl
/***************************************************************************

    Intel 8089 I/O Processor

***************************************************************************/

#include "emu.h"
#include "i8089.h"
#include "i8089_dasm.h"
#include "i8089_channel.h"


//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define VERBOSE      0


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(I8089, i8089_device, "i8089", "Intel 8089 I/O Processor")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i8089_device - constructor
//-------------------------------------------------

i8089_device::i8089_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	cpu_device(mconfig, I8089, tag, owner, clock),
	m_ch1(*this, "1"),
	m_ch2(*this, "2"),
	m_write_sintr1(*this),
	m_write_sintr2(*this), m_data_width(0), m_mem(nullptr), m_io(nullptr),
	m_sysbus(0),
	m_scb(0),
	m_soc(0), m_initialized(false),
	m_master(false),
	m_current_tp(0),
	m_ca(0),
	m_sel(0), m_last_chan(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8089_device::device_start()
{
	// set our instruction counter
	set_icountptr(m_icount);

	// register debugger states
	state_add(SYSBUS, "SYSBUS", m_sysbus).mask(0x01).formatstr("%1s");
	state_add(SCB, "SCB", m_scb).mask(0xfffff);
	state_add(SOC, "SOC", m_soc).mask(0x03).formatstr("%2s");
	state_add_divider(DIVIDER1);
	state_add(CH1_GA, "CH1  GA", m_ch1->m_r[i8089_channel_device::GA].w).mask(0xfffff).formatstr("%8s");
	state_add(CH1_GB, "CH1  GB", m_ch1->m_r[i8089_channel_device::GB].w).mask(0xfffff).formatstr("%8s");
	state_add(CH1_GC, "CH1  GC", m_ch1->m_r[i8089_channel_device::GC].w).mask(0xfffff).formatstr("%8s");
	state_add(CH1_TP, "CH1  TP", m_ch1->m_r[i8089_channel_device::TP].w).mask(0xfffff).formatstr("%8s");
	state_add(CH1_BC, "CH1  BC", m_ch1->m_r[i8089_channel_device::BC].w).mask(0xffff);
	state_add(CH1_IX, "CH1  IX", m_ch1->m_r[i8089_channel_device::IX].w).mask(0xffff);
	state_add(CH1_CC, "CH1  CC", m_ch1->m_r[i8089_channel_device::CC].w).mask(0xffff);
	state_add(CH1_MC, "CH1  MC", m_ch1->m_r[i8089_channel_device::MC].w).mask(0xffff);
	state_add(CH1_CP, "CH1  CP", m_ch1->m_r[i8089_channel_device::CP].w).mask(0xfffff);
	state_add(CH1_PP, "CH1  PP", m_ch1->m_r[i8089_channel_device::PP].w).mask(0xfffff);
	state_add(CH1_PSW, "CH1 PSW", m_ch1->m_r[i8089_channel_device::PSW].w).callimport().callexport().formatstr("%12s");
	state_add_divider(DIVIDER2);
	state_add(CH2_GA, "CH2  GA", m_ch2->m_r[i8089_channel_device::GA].w).mask(0xfffff).formatstr("%8s");
	state_add(CH2_GB, "CH2  GB", m_ch2->m_r[i8089_channel_device::GB].w).mask(0xfffff).formatstr("%8s");
	state_add(CH2_GC, "CH2  GC", m_ch2->m_r[i8089_channel_device::GC].w).mask(0xfffff).formatstr("%8s");
	state_add(CH2_TP, "CH2  TP", m_ch2->m_r[i8089_channel_device::TP].w).mask(0xfffff).formatstr("%8s");
	state_add(CH2_BC, "CH2  BC", m_ch2->m_r[i8089_channel_device::BC].w).mask(0xffff);
	state_add(CH2_IX, "CH2  IX", m_ch2->m_r[i8089_channel_device::IX].w).mask(0xffff);
	state_add(CH2_CC, "CH2  CC", m_ch2->m_r[i8089_channel_device::CC].w).mask(0xffff);
	state_add(CH2_MC, "CH2  MC", m_ch2->m_r[i8089_channel_device::MC].w).mask(0xffff);
	state_add(CH2_CP, "CH2  CP", m_ch2->m_r[i8089_channel_device::CP].w).mask(0xfffff);
	state_add(CH2_PP, "CH2  PP", m_ch2->m_r[i8089_channel_device::PP].w).mask(0xfffff);
	state_add(CH2_PSW, "CH2 PSW", m_ch2->m_r[i8089_channel_device::PSW].w).callimport().callexport().formatstr("%12s");
	state_add(STATE_GENPC, "GENPC", m_current_tp).mask(0xfffff).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_current_tp).mask(0xfffff).noshow();

	// register for save states
	save_item(NAME(m_sysbus));
	save_item(NAME(m_scb));
	save_item(NAME(m_soc));
	save_item(NAME(m_master));
	save_item(NAME(m_ca));
	save_item(NAME(m_sel));
	save_item(NAME(m_last_chan));

	// assign memory spaces
	m_mem = &space(AS_PROGRAM);
	m_io = &space(AS_IO);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void i8089_device::device_config_complete()
{
	m_program_config = address_space_config("program", ENDIANNESS_LITTLE, m_data_width, 20);
	m_io_config = address_space_config("io", ENDIANNESS_LITTLE, m_data_width, 16);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8089_device::device_reset()
{
	m_initialized = false;
	m_last_chan = 0;
}

//-------------------------------------------------
//  memory_space_config - device-specific address
//  space configurations
//-------------------------------------------------

device_memory_interface::space_config_vector i8089_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}

//-------------------------------------------------
//  disassemble - disassembler
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> i8089_device::create_disassembler()
{
	return std::make_unique<i8089_disassembler>();
}

//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void i8089_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	const i8089_channel_device *ch = m_ch1;

	if (entry.index() >= CH2_GA && entry.index() <= CH2_PSW)
		ch = m_ch2;

	switch (entry.index())
	{
	case SYSBUS:
		str = string_format("%c", sysbus_width() ? 'W' : '.');
		break;
	case SOC:
		str = string_format("%c%c", remotebus_width() ? 'I' : '.', request_grant() ? 'R' : '.');
		break;
	case CH1_GA:
	case CH2_GA:
		str = string_format("%d %05X", ch->m_r[i8089_channel_device::GA].t & 1, ch->m_r[i8089_channel_device::GA].w);
		break;
	case CH1_GB:
	case CH2_GB:
		str = string_format("%d %05X", ch->m_r[i8089_channel_device::GB].t & 1, ch->m_r[i8089_channel_device::GB].w);
		break;
	case CH1_GC:
	case CH2_GC:
		str = string_format("%d %05X", ch->m_r[i8089_channel_device::GC].t & 1, ch->m_r[i8089_channel_device::GC].w);
		break;
	case CH1_TP:
	case CH2_TP:
		str = string_format("%d %05X", ch->m_r[i8089_channel_device::TP].t & 1, ch->m_r[i8089_channel_device::TP].w);
		break;
	case CH1_PSW:
	case CH2_PSW:
		str = string_format("%c%s%c%s%s%s%c%c",
			BIT(ch->m_r[i8089_channel_device::PSW].w, 7) ? 'P':'.',
			BIT(ch->m_r[i8089_channel_device::PSW].w, 6) ? "XF":"..",
			BIT(ch->m_r[i8089_channel_device::PSW].w, 5) ? 'B':'.',
			BIT(ch->m_r[i8089_channel_device::PSW].w, 4) ? "IS":"..",
			BIT(ch->m_r[i8089_channel_device::PSW].w, 3) ? "IC":"..",
			BIT(ch->m_r[i8089_channel_device::PSW].w, 2) ? "TB":"..",
			BIT(ch->m_r[i8089_channel_device::PSW].w, 1) ? 'S':'.',
			BIT(ch->m_r[i8089_channel_device::PSW].w, 0) ? 'D':'.');
		break;
	}
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void i8089_device::device_add_mconfig(machine_config &config)
{
	I8089_CHANNEL(config, m_ch1, 0).sintr().set(FUNC(i8089_device::ch1_sintr_w));
	I8089_CHANNEL(config, m_ch2, 0).sintr().set(FUNC(i8089_device::ch2_sintr_w));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

// the i8089 actually executes a program from internal rom here:
//
// MOVB SYSBUS from FFFF6
// LPD System Configuration Block from FFFF8
// MOVB SOC from (SCB)
// LPD Control Pointer (CP) from (SCB) + 2
// MOVBI "00" to CP + 1 (clear busy flag)

void i8089_device::initialize()
{
	assert(!m_initialized);

	// get system bus width
	m_sysbus = m_mem->read_byte(0xffff6);

	// get system configuration block address
	uint16_t scb_offset = read_word(0, 0xffff8);
	uint16_t scb_segment = read_word(0, 0xffffa);
	m_scb = ((scb_segment << 4) + scb_offset) & 0x0fffff;

	// get system operation command
	m_soc = read_byte(0, m_scb);
	m_master = !m_sel;

	// get control block address
	uint16_t cb_offset = read_word(0, m_scb + 2);
	uint16_t cb_segment = read_word(0, m_scb + 4);
	offs_t cb_address = ((cb_segment << 4) + cb_offset) & 0x0fffff;

	// initialize channels
	m_ch1->set_reg(i8089_channel_device::CP, cb_address);
	m_ch2->set_reg(i8089_channel_device::CP, cb_address + 8);

	// clear busy
	uint16_t ccw = read_word(0, cb_address);
	write_word(0, cb_address, ccw & 0x00ff);

	// done
	m_initialized = true;

	// output some debug info
	if (VERBOSE)
	{
		logerror("%s('%s'): ---- initializing ----\n", shortname(), basetag());
		logerror("%s('%s'): %s system bus\n", shortname(), basetag(), sysbus_width() ? "16-bit" : "8-bit");
		logerror("%s('%s'): system configuration block location: %06x\n", shortname(), basetag(), m_scb);
		logerror("%s('%s'): %s remote bus\n", shortname(), basetag(), remotebus_width() ? "16-bit" : "8-bit");
		logerror("%s('%s'): request/grant: %d\n", shortname(), basetag(), request_grant());
		logerror("%s('%s'): is %s\n", shortname(), basetag(), m_master ? "master" : "slave");
		logerror("%s('%s'): channel control block location: %06x\n", shortname(), basetag(), cb_address);
	}
}

uint8_t i8089_device::read_byte(bool space, offs_t address)
{
	return (space ? m_io : m_mem)->read_byte(address);
}

uint16_t i8089_device::read_word(bool space, offs_t address)
{
	uint16_t data;
	address_space *aspace = (space ? m_io : m_mem);

	if (sysbus_width() && WORD_ALIGNED(address))
	{
		data = aspace->read_word(address);
	}
	else
	{
		data  = aspace->read_byte(address);
		data |= aspace->read_byte(address + 1) << 8;
	}

	return data;
}

void i8089_device::write_byte(bool space, offs_t address, uint8_t data)
{
	(space ? m_io : m_mem)->write_byte(address, data);
}

void i8089_device::write_word(bool space, offs_t address, uint16_t data)
{
	address_space *aspace = (space ? m_io : m_mem);

	if (sysbus_width() && WORD_ALIGNED(address))
	{
		aspace->write_word(address, data);
	}
	else
	{
		aspace->write_byte(address, data & 0xff);
		aspace->write_byte(address + 1, (data >> 8) & 0xff);
	}
}

void i8089_device::execute_run()
{
	do
	{
		bool next_chan;

		if(m_ch1->chan_prio() < m_ch2->chan_prio())
			next_chan = 0;
		else if(m_ch1->chan_prio() > m_ch2->chan_prio())
			next_chan = 1;
		else if(m_ch1->priority() && !m_ch2->priority())
			next_chan = 0;
		else if(!m_ch1->priority() && m_ch2->priority())
			next_chan = 1;
		else
			next_chan = !m_last_chan;

		m_last_chan = next_chan;
		if(!next_chan)
			m_icount -= m_ch1->execute_run();
		else
			m_icount -= m_ch2->execute_run();
	}
	while (m_icount > 0);
}


//**************************************************************************
//  EXTERNAL INPUTS
//**************************************************************************

void i8089_device::ca_w(int state)
{
	if (VERBOSE)
		logerror("%s('%s'): ca_w: %u\n", shortname(), basetag(), state);

	if (m_ca == 1 && state == 0)
	{
		if (!m_initialized)
			initialize();
		else
		{
			if (m_sel == 0)
				m_ch1->ca();
			else
				m_ch2->ca();
		}
	}

	m_ca = state;
}

void i8089_device::drq1_w(int state) { m_ch1->drq_w(state); }
void i8089_device::drq2_w(int state) { m_ch2->drq_w(state); }
void i8089_device::ext1_w(int state) { m_ch1->ext_w(state); }
void i8089_device::ext2_w(int state) { m_ch2->ext_w(state); }
