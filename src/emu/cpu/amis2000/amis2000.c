// license:BSD-3-Clause
// copyright-holders:hap
/*

  American Microsystems, Inc.(AMI) S2000-family 4-bit MCU cores, introduced late 1970s
  
  TODO:
  - x
  - add S2200/S2400

*/

#include "amis2000.h"
#include "debugger.h"


// S2000 is the most basic one, 64 nibbles internal RAM and 1KB internal ROM
// S2150 increased RAM to 80 nibbles and ROM to 1.5KB
// high-voltage output versions of these chips (S2000C and S2150C) are identical overall
const device_type AMI_S2000 = &device_creator<amis2000_device>;
const device_type AMI_S2150 = &device_creator<amis2150_device>;


// internal memory maps
static ADDRESS_MAP_START(program_1k, AS_PROGRAM, 8, amis2000_device)
	AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(program_1_5k, AS_PROGRAM, 8, amis2000_device)
	AM_RANGE(0x000, 0x3ff) AM_ROM
	AM_RANGE(0x400, 0x5ff) AM_ROM AM_MIRROR(0x200)
ADDRESS_MAP_END


static ADDRESS_MAP_START(data_64x4, AS_DATA, 8, amis2000_device)
	AM_RANGE(0x00, 0x3f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_80x4, AS_DATA, 8, amis2000_device)
	AM_RANGE(0x00, 0x3f) AM_RAM
	AM_RANGE(0x40, 0x4f) AM_RAM
ADDRESS_MAP_END


// device definitions
amis2000_device::amis2000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, AMI_S2000, "AMI S2000", tag, owner, clock, "amis2000", __FILE__),
	m_program_config("program", ENDIANNESS_BIG, 8, 13, 0, ADDRESS_MAP_NAME(program_1k)),
	m_data_config("data", ENDIANNESS_BIG, 8, 6, 0, ADDRESS_MAP_NAME(data_64x4)),
	m_bu_bits(2),
	m_stack_bits(10),
	m_read_k(*this),
	m_read_i(*this),
	m_read_d(*this),
	m_write_d(*this),
	m_write_a(*this)
{
}

amis2000_device::amis2000_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 bu_bits, UINT8 stack_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source),
	m_program_config("program", ENDIANNESS_BIG, 8, prgwidth, 0, program),
	m_data_config("data", ENDIANNESS_BIG, 8, datawidth, 0, data),
	m_bu_bits(bu_bits),
	m_stack_bits(stack_bits),
	m_read_k(*this),
	m_read_i(*this),
	m_read_d(*this),
	m_write_d(*this),
	m_write_a(*this)
{
}

amis2150_device::amis2150_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: amis2000_device(mconfig, AMI_S2150, "AMI S2150", tag, owner, clock, 3, 11, 13, ADDRESS_MAP_NAME(program_1_5k), 7, ADDRESS_MAP_NAME(data_80x4), "amis2150", __FILE__)
{
}


// disasm
void amis2000_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%c%c%c%c%c",
				m_f & 0x20 ? '6':'.',
				m_f & 0x10 ? '5':'.',
				m_f & 0x08 ? '4':'.',
				m_f & 0x04 ? '3':'.',
				m_f & 0x02 ? '2':'.',
				m_f & 0x01 ? '1':'.'
			);
			break;

		default: break;
	}
}

offs_t amis2000_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(amis2000);
	return CPU_DISASSEMBLE_NAME(amis2000)(this, buffer, pc, oprom, opram, options);
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

enum
{
	S2000_PC=1, S2000_BL, S2000_BU,
	S2000_A, S2000_E
};

void amis2000_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	m_read_k.resolve_safe(0);
	m_read_i.resolve_safe(0);
	m_read_d.resolve_safe(0);
	m_write_d.resolve_safe();
	m_write_a.resolve_safe();

	m_bu_mask = (1 << m_bu_bits) - 1;
	m_stack_mask = (1 << m_stack_bits) - 1;

	// zerofill
	m_stack[0] = m_stack[1] = m_stack[2] = 0;
	m_pc = 0;
	m_op = 0;
	m_f = 0;
	m_bl = 0;
	m_bu = 0;
	m_a = 0;
	m_e = 0;

	// register for savestates
	save_item(NAME(m_stack));
	save_item(NAME(m_pc));
	save_item(NAME(m_op));
	save_item(NAME(m_f));
	save_item(NAME(m_bl));
	save_item(NAME(m_bu));
	save_item(NAME(m_a));
	save_item(NAME(m_e));

	// register state for debugger
	state_add(S2000_PC,     "PC",     m_pc    ).formatstr("%04X");
	state_add(S2000_BL,     "BL",     m_bl    ).formatstr("%01X");
	state_add(S2000_BU,     "BU",     m_bu    ).formatstr("%01X");
	state_add(S2000_A,      "A",      m_a     ).formatstr("%01X");
	state_add(S2000_E,      "E",      m_a     ).formatstr("%01X");

	state_add(STATE_GENPC, "curpc", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_f).formatstr("%6s").noshow();

	m_icountptr = &m_icount;
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void amis2000_device::device_reset()
{
	m_pc = 0;
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

#include "amis2000op.inc"

void amis2000_device::execute_run()
{
	do
	{
		m_icount--;

		debugger_instruction_hook(this, m_pc);
		m_op = m_program->read_byte(m_pc);
		m_pc = (m_pc + 1) & 0x1fff;

		switch (m_op)
		{
			default: break;
		}

	} while (m_icount > 0);
}
